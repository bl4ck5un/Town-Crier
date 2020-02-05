#include "env.h"

Environment *g_environment;

const string Environment::TC_ADDR_MAP_KEY = "TC_ADDRESS";

string get_env(const string& key) {
  return g_environment->get(key);
}

string getContractAddress() {
  return get_env(Environment::TC_ADDR_MAP_KEY);
}

void init_enclave_kv_store(const char* tc_address)
{
  g_environment = new Environment();
  g_environment->set(Environment::TC_ADDR_MAP_KEY, tc_address);
}

void set_enclave_kv(const char* key, const char* value)
{
  if (g_environment == nullptr) return;
  g_environment->set(key, value);
}

