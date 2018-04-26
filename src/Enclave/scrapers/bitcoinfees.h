//
// Created by fanz on 4/25/18.
//

#ifndef TOWN_CRIER_BITCOINFEES_H
#define TOWN_CRIER_BITCOINFEES_H

#include "Scraper.h"

struct RecommendedFees {
  int fastestFee;
  int halfHourFee;
  int hourFee;
};

class BitcoinFees : Scraper {
private:
  const std::string HOSTNAME = "bitcoinfees.earn.com";
  const std::string URL = "/api/v1/fees/recommended";
public:
  err_code handle(const uint8_t *req, size_t len, int *resp_data) final;
  void get_recommended_fees(RecommendedFees*);
private:
};

#endif //TOWN_CRIER_BITCOINFEES_H
