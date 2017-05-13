//
// Created by fanz on 5/12/17.
//

#include "env.h"
#include "init.h"

Environment *g_environment;

void init() {
  g_environment = new Environment();
}

void set_env(const char* key, const char* value) {
  if (g_environment == NULL) return;
  g_environment->set(key, value);
}