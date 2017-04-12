#ifndef TOWN_CRIER_EXCEPTION_H
#define TOWN_CRIER_EXCEPTION_H

#include <exception>
#include <string>

#include <sgx_error.h>
#include "utils.h"

namespace tc {
  class EcallException : public std::exception {
   private:
    sgx_status_t ecall_ret;
    std::string why;
   public:
    EcallException(sgx_status_t ecall_ret, std::string why) : ecall_ret(ecall_ret), why(why) {}
    virtual char const * what() const noexcept {
      why = std::string(sgx_error_message(ecall_ret)) + "; " + why;
      return why.c_str();
    }
  };
}

#endif //TOWN_CRIER_EXCEPTION_H
