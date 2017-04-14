#ifndef TOWN_CRIER_EXCEPTION_H
#define TOWN_CRIER_EXCEPTION_H

#include <exception>
#include <string>

#include <sgx_error.h>
#include "utils.h"

using std::string;

namespace tc {
class EcallException : public std::exception {
 private:
  sgx_status_t ecall_ret;
  std::string why;
 public:
  EcallException(sgx_status_t ecall_ret, std::string _why) : ecall_ret(ecall_ret) {
    this->why = sgx_error_message(ecall_ret) + "; " + _why;
  }
  virtual char const *what() const noexcept {
    return why.c_str();
  }
};
}

#endif //TOWN_CRIER_EXCEPTION_H
