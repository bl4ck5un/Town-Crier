//
// Created by Oscar on 2/21/17.
// Virtual class providing an interface for the Event handler to use.
//

#ifndef TOWN_CRIER_SCRAPER_H
#define TOWN_CRIER_SCRAPER_H

#include "../../Common/Constants.h"
#include "utils.h"

class Scraper {
public:
    virtual err_code handler(uint8_t* req, int data_len, int *resp_data) = 0;

    int dummy(int i){
        return i;
    }
};


#endif //TOWN_CRIER_SCRAPER_H
