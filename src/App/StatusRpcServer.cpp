#include <iostream>
#include "StatusRpcServer.h"
#include "attestation.h"
#include "Converter.h"

StatusRpcServer::StatusRpcServer(AbstractServerConnector &connector, sgx_enclave_id_t eid)
    : AbstractStatusServer(connector), eid(eid) {
}

std::string StatusRpcServer::attest() {
  try {
    std::vector<uint8_t> attestation = get_attestation(this->eid);
    return bufferToHex(attestation);
  }
  catch (std::runtime_error& e) {
    return e.what();
  }
}

Json::Value StatusRpcServer::status() {
  Json::Value status;
  status["numberOfBlocks"] = 0;
  return status;
}
