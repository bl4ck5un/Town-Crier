#ifndef TOWN_CRIER_EXCEPTION_H
#define TOWN_CRIER_EXCEPTION_H

#include <exception>
#include <string>

#include <sgx_error.h>

namespace tc {
  class EcallException : public std::exception {
   private:
    std::string why;
   public:
    EcallException(sgx_status_t ecall_ret, std::string why) : why(why) {}
    EcallException() : why("Ecall failed") {}
    virtual char const * what() const noexcept {
      return why.c_str();
    }
  };
}

#endif //TOWN_CRIER_EXCEPTION_H
