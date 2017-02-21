//
// Created by sgx on 2/21/17.
//

#ifndef TOWN_CRIER_SCRAPER_H
#define TOWN_CRIER_SCRAPER_H

#include "../../Common/Constants.h"
#include "utils.h"

class Scraper {
public:
    Scraper();
    virtual err_code handler(uint8_t* req, int data_len, int *resp_data) = 0;

};


#endif //TOWN_CRIER_SCRAPER_H
