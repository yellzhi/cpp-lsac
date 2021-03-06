#pragma once

#include <memory>
#include <iosfwd>
#include <jsonrpccpp/server.h>
#include <jsonrpccpp/common/exception.h>
#include <libdevcore/Common.h>
#include "SessionManager.h"
#include "BrcFace.h"


namespace dev
{
class NetworkFace;
class KeyPair;
namespace brc
{
class AccountHolder;
struct TransactionSkeleton;
class Interface;
}

}

namespace dev
{

namespace rpc
{

// Should only be called within a catch block
std::string exceptionToErrorMessage();

/**
 * @brief JSON-RPC api implementation
 */
class Brc: public dev::rpc::BrcFace
{
public:
	Brc(brc::Interface& _brc, brc::AccountHolder& _brcAccounts);

	virtual RPCModules implementedModules() const override
	{
		return RPCModules{RPCModule{"brc", "1.0"}};
	}

	brc::AccountHolder const& brcAccounts() const { return m_brcAccounts; }

	virtual std::string brc_protocolVersion() override;
	virtual std::string brc_hashrate() override;
	virtual std::string brc_coinbase() override;
	virtual bool brc_mining() override;
	virtual std::string brc_gasPrice() override;
	virtual Json::Value brc_accounts() override;
	virtual std::string brc_blockNumber()override;
	virtual std::string brc_getBalance(std::string const& _address, std::string const& _blockNumber) override;
    virtual std::string brc_getBallot(std::string const& _address, std::string const& _blockNumber) override;
	virtual std::string brc_getStorageAt(std::string const& _address, std::string const& _position, std::string const& _blockNumber) override;
	virtual std::string brc_getStorageRoot(std::string const& _address, std::string const& _blockNumber) override;
	virtual std::string brc_getTransactionCount(std::string const& _address, std::string const& _blockNumber) override;
	virtual Json::Value brc_pendingTransactions() override;
	virtual Json::Value brc_getBlockTransactionCountByHash(std::string const& _blockHash) override;
	virtual Json::Value brc_getBlockTransactionCountByNumber(std::string const& _blockNumber) override;
	virtual Json::Value brc_getUncleCountByBlockHash(std::string const& _blockHash) override;
	virtual Json::Value brc_getUncleCountByBlockNumber(std::string const& _blockNumber) override;
	virtual std::string brc_getCode(std::string const& _address, std::string const& _blockNumber) override;
	virtual std::string brc_sendTransaction(Json::Value const& _json) override;
	virtual std::string brc_call(Json::Value const& _json, std::string const& _blockNumber) override;
	virtual std::string brc_estimateGas(Json::Value const& _json) override;
	virtual bool brc_flush() override;
	virtual Json::Value brc_getBlockByHash(std::string const& _blockHash, bool _includeTransactions) override;
	virtual Json::Value brc_getBlockByNumber(std::string const& _blockNumber, bool _includeTransactions) override;
	virtual Json::Value brc_getTransactionByHash(std::string const& _transactionHash) override;
	virtual Json::Value brc_getTransactionByBlockHashAndIndex(std::string const& _blockHash, std::string const& _transactionIndex) override;
	virtual Json::Value brc_getTransactionByBlockNumberAndIndex(std::string const& _blockNumber, std::string const& _transactionIndex) override;
	virtual Json::Value brc_getTransactionReceipt(std::string const& _transactionHash) override;
	virtual Json::Value brc_getUncleByBlockHashAndIndex(std::string const& _blockHash, std::string const& _uncleIndex) override;
	virtual Json::Value brc_getUncleByBlockNumberAndIndex(std::string const& _blockNumber, std::string const& _uncleIndex) override;
	virtual std::string brc_newFilter(Json::Value const& _json) override;
	virtual std::string brc_newFilterEx(Json::Value const& _json) override;
	virtual std::string brc_newBlockFilter() override;
	virtual std::string brc_newPendingTransactionFilter() override;
	virtual bool brc_uninstallFilter(std::string const& _filterId) override;
	virtual Json::Value brc_getFilterChanges(std::string const& _filterId) override;
	virtual Json::Value brc_getFilterChangesEx(std::string const& _filterId) override;
	virtual Json::Value brc_getFilterLogs(std::string const& _filterId) override;
	virtual Json::Value brc_getFilterLogsEx(std::string const& _filterId) override;
	virtual Json::Value brc_getLogs(Json::Value const& _json) override;
	virtual Json::Value brc_getLogsEx(Json::Value const& _json) override;
	virtual Json::Value brc_getWork() override;
	virtual bool brc_submitWork(std::string const& _nonce, std::string const&, std::string const& _mixHash) override;
	virtual bool brc_submitHashrate(std::string const& _hashes, std::string const& _id) override;
	virtual std::string brc_register(std::string const& _address) override;
	virtual bool brc_unregister(std::string const& _accountId) override;
	virtual Json::Value brc_fetchQueuedTransactions(std::string const& _accountId) override;
	virtual Json::Value brc_signTransaction(Json::Value const& _transaction) override;
	virtual Json::Value brc_inspectTransaction(std::string const& _rlp) override;
	virtual std::string brc_sendRawTransaction(std::string const& _rlp) override;
	virtual bool brc_notePassword(std::string const&) override { return false; }
	virtual Json::Value brc_syncing() override;
	virtual std::string brc_chainId() override;
	
	void setTransactionDefaults(brc::TransactionSkeleton& _t);
protected:

	brc::Interface* client() { return &m_brc; }
	
	brc::Interface& m_brc;
	brc::AccountHolder& m_brcAccounts;

};

}
} //namespace dev
