#include "JsonHelper.h"

#include <jsonrpccpp/common/exception.h>
#include <libbrccore/CommonJS.h>
#include <libbrccore/SealEngine.h>
#include <libbrcdchain/Client.h>
#include <libwebthree/WebThree.h>
using namespace std;
using namespace dev;
using namespace brc;

namespace dev
{
Json::Value toJson(unordered_map<u256, u256> const& _storage)
{
    Json::Value res(Json::objectValue);
    for (auto i : _storage)
        res[toJS(i.first)] = toJS(i.second);
    return res;
}

Json::Value toJson(map<h256, pair<u256, u256>> const& _storage)
{
    Json::Value res(Json::objectValue);
    for (auto i : _storage)
        res[toJS(u256(i.second.first))] = toJS(i.second.second);
    return res;
}

Json::Value toJson(Address const& _address)
{
    return toJS(_address);
}

// ////////////////////////////////////////////////////////////////////////////////
// p2p
// ////////////////////////////////////////////////////////////////////////////////
namespace p2p
{
Json::Value toJson(p2p::PeerSessionInfo const& _p)
{
    //@todo localAddress
    //@todo protocols
    Json::Value ret;
    ret["id"] = _p.id.hex();
    ret["name"] = _p.clientVersion;
    ret["network"]["remoteAddress"] = _p.host + ":" + toString(_p.port);
    ret["lastPing"] = (int)chrono::duration_cast<chrono::milliseconds>(_p.lastPing).count();
    for (auto const& i : _p.notes)
        ret["notes"][i.first] = i.second;
    for (auto const& i : _p.caps)
        ret["caps"].append(i.first + "/" + toString((unsigned)i.second));
    return ret;
}

}  // namespace p2p

// ////////////////////////////////////////////////////////////////////////////////
// brc
// ////////////////////////////////////////////////////////////////////////////////

namespace brc
{
Json::Value toJson(dev::brc::BlockHeader const& _bi, SealEngineFace* _sealer)
{
    Json::Value res;
    if (_bi)
    {
        DEV_IGNORE_EXCEPTIONS(res["hash"] = toJS(_bi.hash()));
        res["parentHash"] = toJS(_bi.parentHash());
        res["sha3Uncles"] = toJS(_bi.sha3Uncles());
        res["author"] = toJS(_bi.author());
        res["stateRoot"] = toJS(_bi.stateRoot());
        res["transactionsRoot"] = toJS(_bi.transactionsRoot());
        res["receiptsRoot"] = toJS(_bi.receiptsRoot());
        res["number"] = toJS(_bi.number());
        res["gasUsed"] = toJS(_bi.gasUsed());
        res["gasLimit"] = toJS(_bi.gasLimit());
        res["extraData"] = toJS(_bi.extraData());
        res["logsBloom"] = toJS(_bi.logBloom());
        res["timestamp"] = toJS(_bi.timestamp());

        res["sign"] = toJS((h520)_bi.sign_data());


        // TODO: remove once JSONRPC spec is updated to use "author" over "miner".
        res["miner"] = toJS(_bi.author());
        if (_sealer)
            for (auto const& i : _sealer->jsInfo(_bi))
                res[i.first] = i.second;
    }
    return res;
}

Json::Value toJson(
    dev::brc::Transaction const& _t, std::pair<h256, unsigned> _location, BlockNumber _blockNumber)
{
    Json::Value res;
    if (_t)
    {
        res["hash"] = toJS(_t.sha3());
        res["input"] = toJS(_t.data());
        res["to"] = _t.isCreation() ? Json::Value() : toJS(_t.receiveAddress());
        res["from"] = toJS(_t.safeSender());
        res["gas"] = toJS(_t.gas());
        res["gasPrice"] = toJS(_t.gasPrice());
        res["nonce"] = toJS(_t.nonce());
        res["value"] = toJS(_t.value());
        res["blockHash"] = toJS(_location.first);
        res["transactionIndex"] = toJS(_location.second);
        res["blockNumber"] = toJS(_blockNumber);
        res["v"] = toJS(_t.signature().v);
        res["r"] = toJS(_t.signature().r);
        res["s"] = toJS(_t.signature().s);
    }
    return res;
}

Json::Value toJson(dev::brc::BlockHeader const& _bi, BlockDetails const& _bd,
    UncleHashes const& _us, Transactions const& _ts, SealEngineFace* _face)
{
    Json::Value res = toJson(_bi, _face);
    if (_bi)
    {
        res["totalDifficulty"] = toJS(_bd.totalDifficulty);
        res["size"] = toJS(_bd.size);
        res["uncles"] = Json::Value(Json::arrayValue);
        for (h256 h : _us)
            res["uncles"].append(toJS(h));
        res["transactions"] = Json::Value(Json::arrayValue);
        for (unsigned i = 0; i < _ts.size(); i++)
            res["transactions"].append(
                toJson(_ts[i], std::make_pair(_bi.hash(), i), (BlockNumber)_bi.number()));
    }
    return res;
}

Json::Value toJson(dev::brc::BlockHeader const& _bi, BlockDetails const& _bd,
    UncleHashes const& _us, TransactionHashes const& _ts, SealEngineFace* _face)
{
    Json::Value res = toJson(_bi, _face);
    if (_bi)
    {
        res["totalDifficulty"] = toJS(_bd.totalDifficulty);
        res["size"] = toJS(_bd.size);
        res["uncles"] = Json::Value(Json::arrayValue);
        for (h256 h : _us)
            res["uncles"].append(toJS(h));
        res["transactions"] = Json::Value(Json::arrayValue);
        for (h256 const& t : _ts)
            res["transactions"].append(toJS(t));
    }
    return res;
}

Json::Value toJson(dev::brc::TransactionSkeleton const& _t)
{
    Json::Value res;
    res["to"] = _t.creation ? Json::Value() : toJS(_t.to);
    res["from"] = toJS(_t.from);
    res["gas"] = toJS(_t.gas);
    res["gasPrice"] = toJS(_t.gasPrice);
    res["value"] = toJS(_t.value);
    res["data"] = toJS(_t.data, 32);
    return res;
}

Json::Value toJson(dev::brc::TransactionReceipt const& _t)
{
    Json::Value res;
    if (_t.hasStatusCode())
        res["status"] = toString(_t.statusCode());
    else
        res["stateRoot"] = toJS(_t.stateRoot());
    res["gasUsed"] = toJS(_t.cumulativeGasUsed());
    res["bloom"] = toJS(_t.bloom());
    res["log"] = dev::toJson(_t.log());
    return res;
}

Json::Value toJson(dev::brc::LocalisedTransactionReceipt const& _t)
{
    Json::Value res;
    res["transactionHash"] = toJS(_t.hash());
    res["transactionIndex"] = _t.transactionIndex();
    res["blockHash"] = toJS(_t.blockHash());
    res["blockNumber"] = _t.blockNumber();
    res["from"] = toJS(_t.from());
    res["to"] = toJS(_t.to());
    res["cumulativeGasUsed"] = toJS(_t.cumulativeGasUsed());
    res["gasUsed"] = toJS(_t.gasUsed());
    res["contractAddress"] = toJS(_t.contractAddress());
    res["logs"] = dev::toJson(_t.localisedLogs());
    res["logsBloom"] = toJS(_t.bloom());
    if (_t.hasStatusCode())
        res["status"] = toString(_t.statusCode());
    else
        res["stateRoot"] = toJS(_t.stateRoot());
    return res;
}

Json::Value toJson(dev::brc::Transaction const& _t)
{
    Json::Value res;
    res["to"] = _t.isCreation() ? Json::Value() : toJS(_t.to());
    res["from"] = toJS(_t.from());
    res["gas"] = toJS(_t.gas());
    res["gasPrice"] = toJS(_t.gasPrice());
    res["value"] = toJS(_t.value());
    res["data"] = toJS(_t.data(), 32);
    res["nonce"] = toJS(_t.nonce());
    res["hash"] = toJS(_t.sha3(WithSignature));
    res["sighash"] = toJS(_t.sha3(WithoutSignature));
    res["r"] = toJS(_t.signature().r);
    res["s"] = toJS(_t.signature().s);
    res["v"] = toJS(_t.signature().v);
    return res;
}

Json::Value toJson(dev::brc::Transaction const& _t, bytes const& _rlp)
{
    Json::Value res;
    res["raw"] = toJS(_rlp);
    res["tx"] = toJson(_t);
    return res;
}

Json::Value toJson(dev::brc::LocalisedTransaction const& _t)
{
    Json::Value res;
    if (_t)
    {
        res["hash"] = toJS(_t.sha3());
        res["input"] = toJS(_t.data());
        res["to"] = _t.isCreation() ? Json::Value() : toJS(_t.receiveAddress());
        res["from"] = toJS(_t.safeSender());
        res["gas"] = toJS(_t.gas());
        res["gasPrice"] = toJS(_t.gasPrice());
        res["nonce"] = toJS(_t.nonce());
        res["value"] = toJS(_t.value());
        res["blockHash"] = toJS(_t.blockHash());
        res["transactionIndex"] = toJS(_t.transactionIndex());
        res["blockNumber"] = toJS(_t.blockNumber());
    }
    return res;
}

Json::Value toJson(dev::brc::LocalisedLogEntry const& _e)
{
    Json::Value res;

    if (_e.isSpecial)
        res = toJS(_e.special);
    else
    {
        res = toJson(static_cast<dev::brc::LogEntry const&>(_e));
        res["polarity"] = _e.polarity == BlockPolarity::Live ? true : false;
        if (_e.mined)
        {
            res["type"] = "mined";
            res["blockNumber"] = _e.blockNumber;
            res["blockHash"] = toJS(_e.blockHash);
            res["logIndex"] = _e.logIndex;
            res["transactionHash"] = toJS(_e.transactionHash);
            res["transactionIndex"] = _e.transactionIndex;
        }
        else
        {
            res["type"] = "pending";
            res["blockNumber"] = Json::Value(Json::nullValue);
            res["blockHash"] = Json::Value(Json::nullValue);
            res["logIndex"] = Json::Value(Json::nullValue);
            res["transactionHash"] = Json::Value(Json::nullValue);
            res["transactionIndex"] = Json::Value(Json::nullValue);
        }
    }
    return res;
}

Json::Value toJson(dev::brc::LogEntry const& _e)
{
    Json::Value res;
    res["data"] = toJS(_e.data);
    res["address"] = toJS(_e.address);
    res["topics"] = Json::Value(Json::arrayValue);
    for (auto const& t : _e.topics)
        res["topics"].append(toJS(t));
    return res;
}

Json::Value toJson(std::unordered_map<h256, dev::brc::LocalisedLogEntries> const& _entriesByBlock,
    vector<h256> const& _order)
{
    Json::Value res(Json::arrayValue);
    for (auto const& i : _order)
    {
        auto entries = _entriesByBlock.at(i);
        Json::Value currentBlock(Json::objectValue);
        LocalisedLogEntry entry = entries[0];
        if (entry.mined)
        {
            currentBlock["blockNumber"] = entry.blockNumber;
            currentBlock["blockHash"] = toJS(entry.blockHash);
            currentBlock["type"] = "mined";
        }
        else
            currentBlock["type"] = "pending";

        currentBlock["polarity"] = entry.polarity == BlockPolarity::Live ? true : false;
        currentBlock["logs"] = Json::Value(Json::arrayValue);

        for (LocalisedLogEntry const& e : entries)
        {
            Json::Value log(Json::objectValue);
            log["logIndex"] = e.logIndex;
            log["transactionIndex"] = e.transactionIndex;
            log["transactionHash"] = toJS(e.transactionHash);
            log["address"] = toJS(e.address);
            log["data"] = toJS(e.data);
            log["topics"] = Json::Value(Json::arrayValue);
            for (auto const& t : e.topics)
                log["topics"].append(toJS(t));

            currentBlock["logs"].append(log);
        }

        res.append(currentBlock);
    }

    return res;
}

Json::Value toJsonByBlock(LocalisedLogEntries const& _entries)
{
    vector<h256> order;
    unordered_map<h256, LocalisedLogEntries> entriesByBlock;

    for (dev::brc::LocalisedLogEntry const& e : _entries)
    {
        if (e.isSpecial)  // skip special log
            continue;

        if (entriesByBlock.count(e.blockHash) == 0)
        {
            entriesByBlock[e.blockHash] = LocalisedLogEntries();
            order.push_back(e.blockHash);
        }

        entriesByBlock[e.blockHash].push_back(e);
    }

    return toJson(entriesByBlock, order);
}

TransactionSkeleton toTransactionSkeleton(Json::Value const& _json)
{
    TransactionSkeleton ret;
    if (!_json.isObject() || _json.empty())
        return ret;
    try
    {
		if(!_json["from"].empty())
			ret.from = jsToAddress(_json["from"].asString());
		if(!_json["to"].empty() && _json["to"].asString() != "0x" && !_json["to"].asString().empty())
			ret.to = jsToAddress(_json["to"].asString());
		else
			ret.creation = true;

		if(!_json["value"].empty())
			ret.value = jsToU256(_json["value"].asString());

		if(!_json["gas"].empty())
			ret.gas = jsToU256(_json["gas"].asString());

		if(!_json["gasPrice"].empty())
			ret.gasPrice = jsToU256(_json["gasPrice"].asString());

		if(!_json["data"].empty())							// brcdChain.js has preconstructed the data array
		{
			if(_json["to"].asString() == "0x00000000000000000000000000000000766f7465")
			{
				std::cout << "1111111111111111111111111111111";
				int size = _json["data"].size();
				std::cout << "data size:" << size << std::endl;
				std::vector<bytes> _op_datas;
				for(int i = 0; i < size; i++)
				{
					Json::Value _itemData = _json["data"][i];
					std::cout << "1222222222222222222222222222" << std::endl;
					transationTool::op_type _type = (transationTool::op_type)_itemData["method_type"].asInt();
					Address _from = Address(_itemData["from"].asString());
					Address _to = Address(_itemData["to"].asString());
					std::cout << "3333333333333333333333333333333" << std::endl;
					switch(_type)
					{
					case transationTool::vote:
					{
						uint8_t _vote_type = (uint8_t)_itemData["type"].asInt();
						size_t _vote_num = (size_t)_itemData["tickets"].asInt();
						transationTool::vote_operation vote_op { _type, _from, _to, _vote_type, _vote_num };
						_op_datas.push_back(vote_op.serialize());
						std::cout << "444444444444444444444444444444444" << std::endl;
					}
					break;
					default:
					break;
					}
				}
				RLPStream _s;
				_s.appendVector<bytes>(_op_datas);
				ret.data = _s.out();
			}
			else
			{
				ret.data = jsToBytes(_json["data"].asString(), OnFailed::Throw);
			}
		}

		if(!_json["code"].empty())
			ret.data = jsToBytes(_json["code"].asString(), OnFailed::Throw);

		if(!_json["nonce"].empty())
			ret.nonce = jsToU256(_json["nonce"].asString());
    }
    catch (boost::exception const&)
    {
        throw jsonrpc::JsonRpcException("Invalid toTransactionSkeleton from json: ");
    }

    return ret;
}

dev::brc::LogFilter toLogFilter(Json::Value const& _json)
{
    dev::brc::LogFilter filter;
    if (!_json.isObject() || _json.empty())
        return filter;

    // check only !empty. it should throw exceptions if input params are incorrect
    if (!_json["fromBlock"].empty())
        filter.withEarliest(jsToFixed<32>(_json["fromBlock"].asString()));
    if (!_json["toBlock"].empty())
        filter.withLatest(jsToFixed<32>(_json["toBlock"].asString()));
    if (!_json["address"].empty())
    {
        if (_json["address"].isArray())
            for (auto i : _json["address"])
                filter.address(jsToAddress(i.asString()));
        else
            filter.address(jsToAddress(_json["address"].asString()));
    }
    if (!_json["topics"].empty())
        for (unsigned i = 0; i < _json["topics"].size(); i++)
        {
            if (_json["topics"][i].isArray())
            {
                for (auto t : _json["topics"][i])
                    if (!t.isNull())
                        filter.topic(i, jsToFixed<32>(t.asString()));
            }
            else if (!_json["topics"][i].isNull())  // if it is anything else then string, it should
                                                    // and will fail
                filter.topic(i, jsToFixed<32>(_json["topics"][i].asString()));
        }
    return filter;
}

// TODO: this should be removed once we decide to remove backward compatibility with old log filters
dev::brc::LogFilter toLogFilter(Json::Value const& _json,
    Interface const& _client)  // commented to avoid warning. Uncomment once in use @ PoC-7.
{
    dev::brc::LogFilter filter;
    if (!_json.isObject() || _json.empty())
        return filter;

    // check only !empty. it should throw exceptions if input params are incorrect
    if (!_json["fromBlock"].empty())
        filter.withEarliest(_client.hashFromNumber(jsToBlockNumber(_json["fromBlock"].asString())));
    if (!_json["toBlock"].empty())
        filter.withLatest(_client.hashFromNumber(jsToBlockNumber(_json["toBlock"].asString())));
    if (!_json["address"].empty())
    {
        if (_json["address"].isArray())
            for (auto i : _json["address"])
                filter.address(jsToAddress(i.asString()));
        else
            filter.address(jsToAddress(_json["address"].asString()));
    }
    if (!_json["topics"].empty())
        for (unsigned i = 0; i < _json["topics"].size(); i++)
        {
            if (_json["topics"][i].isArray())
            {
                for (auto t : _json["topics"][i])
                    if (!t.isNull())
                        filter.topic(i, jsToFixed<32>(t.asString()));
            }
            else if (!_json["topics"][i].isNull())  // if it is anything else then string, it should
                                                    // and will fail
                filter.topic(i, jsToFixed<32>(_json["topics"][i].asString()));
        }
    return filter;
}

}  // namespace brc

// ////////////////////////////////////////////////////////////////////////////////////
// rpc
// ////////////////////////////////////////////////////////////////////////////////////

namespace rpc
{
h256 h256fromHex(string const& _s)
{
    try
    {
        return h256(_s);
    }
    catch (boost::exception const&)
    {
        throw jsonrpc::JsonRpcException("Invalid hex-encoded string: " + _s);
    }
}
}  // namespace rpc

}  // namespace dev
