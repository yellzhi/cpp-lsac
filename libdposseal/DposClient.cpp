#include "DposClient.h"
#include "Dpos.h"
#include "DposHostCapability.h"
#include <boost/filesystem/path.hpp>
#include <libdevcore/Log.h>
#include <time.h>
using namespace std;
using namespace dev;
using namespace dev::eth;
using namespace p2p;
using namespace dev::bacd;
namespace fs = boost::filesystem;

DposClient& dev::bacd::asDposClient(Interface& _c)
{
    if (dynamic_cast<Dpos*>(_c.sealEngine()))
        return dynamic_cast<DposClient&>(_c);
    throw InvalidDposSealEngine();
}

DposClient* dev::bacd::asDposClient(Interface* _c)
{
    if (dynamic_cast<Dpos*>(_c->sealEngine()))
        return &dynamic_cast<DposClient&>(*_c);
    throw InvalidDposSealEngine();
}

dev::bacd::DposClient::DposClient(ChainParams const& _params, int _networkID, p2p::Host& _host,
    std::shared_ptr<GasPricer> _gpForAdoption, boost::filesystem::path const& _dbPath,
    boost::filesystem::path const& _snapshotPath, WithExisting _forceAction,
    TransactionQueue::Limits const& _l)
  : Client(_params, _networkID, _host, _gpForAdoption, _dbPath, _snapshotPath, _forceAction, _l)
{
    // will throw if we're not an dpos seal engine.
    asDposClient(*this);
    m_params = _params;
    init(_host, _networkID);
    LOG(m_logger)<< "init the dposClient ...";
}

dev::bacd::DposClient::~DposClient() 
{
    // to wake up the thread from Client::doWork()
    m_signalled.notify_all();
    terminate();
}

Dpos* dev::bacd::DposClient::dpos() const
{
    return dynamic_cast<Dpos*>(Client::sealEngine());
}

void dev::bacd::DposClient::startSealing()
{
    setName("DposClient");
    //TODO DposCLient func
    //LOG(m_logger) << "DposClient: into startSealing";
    Client::startSealing();
}

void dev::bacd::DposClient::doWork(bool _doWait)
{
    bool t = true;
    //compare_exchange_strong(T& expected, T val, ...)
    //比较本身值和expected, 如果相等者旧值=val, 如果不等 expected=旧值
    if(m_syncBlockQueue.compare_exchange_strong(t, false))
        syncBlockQueue();

    if(m_needStateReset)
    {
        resetState();
        m_needStateReset = false;
    }

    t = true;
    bool isSealed = false;
    DEV_READ_GUARDED(x_working)
        isSealed = m_working.isSealed();

    if(!isSealed && !isMajorSyncing() && !m_remoteWorking && m_syncTransactionQueue.compare_exchange_strong(t, false))
        syncTransactionQueue();

    tick();

    rejigSealing();

    callQueuedFunctions();

    DEV_READ_GUARDED(x_working)
        isSealed = m_working.isSealed();
    // If the block is sealed, we have to wait for it to tickle through the block queue
    // (which only signals as wanting to be synced if it is ready).
    if(!m_syncBlockQueue && !m_syncTransactionQueue && (_doWait || isSealed) && isWorking())
    {
        std::unique_lock<std::mutex> l(x_signalled);
        m_signalled.wait_for(l, chrono::milliseconds(dpos()->dposConfig().blockInterval));
    }
}

void dev::bacd::DposClient::rejigSealing()
{
    if(!m_wouldSeal)
        return;
    //打包出块验证 包括出块周期，出块时间，出块人验证
    uint64_t _time = utcTimeMilliSec();            //这里得到的是系统时间，考虑使用网络心跳同步时间最佳
    if(!isBlockSeal(_time))
    {
        return;
    }

    if((wouldSeal() || remoteActive()) && !isMajorSyncing())
    {
        if(sealEngine()->shouldSeal(this))
        {
            m_wouldButShouldnot = false;

            LOG(m_loggerDetail) << "Rejigging seal engine...";
            DEV_WRITE_GUARDED(x_working)
            {
                if(m_working.isSealed())
                {
                    LOG(m_logger) << "Tried to seal sealed block...";
                    return;
                }
                // TODO is that needed? we have "Generating seal on" below
                LOG(m_loggerDetail) << "Starting to seal block #" << m_working.info().number();
                //设置Block 出块开关 封装extraData数据 transation 数据
                m_working.commitToSeal(bc(), m_extraData);
                //验证交易
                dpos()->verifyTransationVote(m_working.info());
                //添加对应出块数量
                dpos()->varlitorAddBlockNum(author(), 1);
                //尝试进行下一轮 投票统计
                dpos()->tryElect(utcTimeMilliSec());
                //添加 dpos 数据
                BlockHeader _h;
				_h.setDposContext(dpos()->dposContext());
                m_working.setDposData(_h);
                LOG(m_loggerDetail) <<EthYellow "seal block add dposData is ok" << EthYellow;
            }
            DEV_READ_GUARDED(x_working)
            {
                DEV_WRITE_GUARDED(x_postSeal)
                    m_postSeal = m_working;
                m_sealingInfo = m_working.info();
            }
            //出块
            if(wouldSeal())
            {
                //调用父类接口 声明回调，提供证明后调用 保存在 m_onSealGenerated
                sealEngine()->onSealGenerated([=](bytes const& _header){
                    LOG(m_logger) << "Block sealed #" << BlockHeader(_header, HeaderData).number();
                    if(this->submitSealed(_header))
                    {
                        m_onBlockSealed(_header);
                        //处理dpos投票数据
                        LOG(m_logger) << "submitSealed deal vote ... start !";
                        BlockHeader _h_temp = BlockHeader(_header, HeaderData);
                        dpos()->printDposData(_h_temp.dposContext());
                        std::vector<bytes> const& _ts = m_working.getDposTransations();
						
                        for(auto val : _ts)
                            m_dpos_state->excuteTransation(val, _h_temp);
                    }
                    else
                        LOG(m_logger) << "Submitting block failed...";
                                              });
                ctrace << "Generating seal on " << m_sealingInfo.hash(WithoutSeal) << " #" << m_sealingInfo.number();
                sealEngine()->generateSeal(m_sealingInfo);
            }
        }
        else
            m_wouldButShouldnot = true;
    }
    if(!m_wouldSeal)
        sealEngine()->cancelGeneration();
}

void dev::bacd::DposClient::init(p2p::Host & _host, int _netWorkId)
{
    //关联 host 管理的CapabilityHostFace 接口
    cdebug << "capabilityHost :: DposHostCapability";
    auto ethCapability = make_shared<DposHostcapality>(_host.capabilityHost(),
                            _netWorkId,
                            [this](NodeID _nodeid, unsigned _id, RLP const& _r){
                                dpos()->onDposMsg(_nodeid, _id, _r);
                            },
                            [this](NodeID const& _nodeid, u256 const& _peerCapabilityVersion){
                                dpos()->requestStatus(_nodeid, _peerCapabilityVersion);
                            });
    _host.registerCapability(ethCapability);
    dpos()->initEnv(ethCapability);
    dpos()->initConfigAndGenesis(m_params);
    dpos()->setDposClient(this);
    dpos()->startGeneration();

    m_dpos_state = std::make_shared<DposVoteState>(dpos()->dposConfig());
}

bool dev::bacd::DposClient::isBlockSeal(uint64_t _now)
{
    //LOG(m_logger) << "start to check time and varlitor ...";
    //验证时间 考虑创世区块时间
    if(!dpos()->checkDeadline(_now))
        return false;

    //验证出块人，周期
    if(!dpos()->isBolckSeal(_now))
        return false;
    return true;
}