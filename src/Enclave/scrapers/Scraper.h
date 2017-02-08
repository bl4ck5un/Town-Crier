//
// Created by Oscar on 2/7/17.
//

#ifndef TOWN_CRIER_SCRAPER_H
#define TOWN_CRIER_SCRAPER_H


class Scraper {
public:

    Scraper();
    virtual err_code handler() = 0;
};


#endif //TOWN_CRIER_SCRAPER_H
