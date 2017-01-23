#include <iostream>
#include "StatusRpcServer.h"

StatusRpcServer::StatusRpcServer(AbstractServerConnector &connector) : AbstractStatusServer(connector) {
}

std::string StatusRpcServer::attest() {
  return "attestation";
}

Json::Value StatusRpcServer::status() {
  Json::Value status;
  status["numberOfBlocks"] = 100;
  return status;
}
