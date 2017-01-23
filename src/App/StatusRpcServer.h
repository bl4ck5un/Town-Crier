#ifndef TOWN_CRIER_STATUSRPCSERVER_H
#define TOWN_CRIER_STATUSRPCSERVER_H

#include "abstractstatusserver.h"

#include <jsonrpccpp/server/connectors/httpserver.h>

using namespace ::jsonrpc;

class StatusRpcServer : public AbstractStatusServer {
 public:
  StatusRpcServer(AbstractServerConnector &connector);
  // curl -d '{"id": 123, "jsonrpc": "2.0", "method": "status"}'  localhost:8123
  std::string attest() override;
  Json::Value status() override;
};

#endif //TOWN_CRIER_STATUSRPCSERVER_H
