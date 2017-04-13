//
// Created by fanz on 4/12/17.
// TODO: add license info
//


#ifndef TOWN_CRIER_YAHOO_YQL_H
#define TOWN_CRIER_YAHOO_YQL_H

#include "Scraper.h"
#include <string>

using namespace std;

class YahooYQL {
 public:
  enum Format {
    JSON,
    XML
  };

 private:
  string q;
  Format format;
  string env;
  string buildQuery();

 public:
  YahooYQL(string q): q(q), format(XML), env("") {}
  YahooYQL(string q, Format f, string env): q(q), format(f), env(env) {}
  err_code execute(string&);
};

#endif //TOWN_CRIER_YAHOO_YQL_H
