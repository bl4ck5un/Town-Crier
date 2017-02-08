file(REMOVE_RECURSE
  "Enclave_u.c"
  " Enclave_u.h"
)

# Per-language clean rules from dependency scanning.
foreach(lang )
  include(CMakeFiles/enclave.dir/cmake_clean_${lang}.cmake OPTIONAL)
endforeach()
