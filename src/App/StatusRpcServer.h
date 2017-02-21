#ifndef TOWN_CRIER_STATUSRPCSERVER_H
#define TOWN_CRIER_STATUSRPCSERVER_H

#include "abstractstatusserver.h"

#include <jsonrpccpp/server/connectors/httpserver.h>
#include <sgx_eid.h>

using namespace ::jsonrpc;

class StatusRpcServer : public AbstractStatusServer {
 private:
  sgx_enclave_id_t eid;
 public:
  StatusRpcServer(AbstractServerConnector &connector, sgx_enclave_id_t eid);
  // curl -d '{"id": 123, "jsonrpc": "2.0", "method": "status"}'  localhost:8123
  std::string attest() override;
  Json::Value status() override;
};

#endif //TOWN_CRIER_STATUSRPCSERVER_H
