//
// Created by fanz on 5/12/17.
//

#ifndef TOWN_CRIER_ENV_H
#define TOWN_CRIER_ENV_H

#include <map>
#include <string>

using std::string;
using std::map;

/*!
 * a class for
 */
class Environment {
 private:
  map<string, string> env;
 public:
  void set(string key, string value) {
    env[key] = value;
  }
  string get(string key) {
    map<string, string>::iterator search = env.find(key);
    if (search == env.end()) {
      return "";
    }

    return env[key];
  }
};

extern Environment *g_environment;

#endif //TOWN_CRIER_ENV_H
