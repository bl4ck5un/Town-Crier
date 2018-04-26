#include "env.h"

Environment *g_environment;

const string Environment::TC_ADDR_MAP_KEY = "TC_ADDRESS";

string get_env(const string& key) {
  return g_environment->get(key);
}

string getContractAddress() {
  return get_env(Environment::TC_ADDR_MAP_KEY);
}

void init(const char* tc_address) {
  g_environment = new Environment();
  g_environment->set(Environment::TC_ADDR_MAP_KEY, tc_address);
}

void set_env(const char* key, const char* value) {
  if (g_environment == nullptr) return;
  g_environment->set(key, value);
}

