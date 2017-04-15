//
// Created by fanz on 4/15/17.
//

#ifndef TOWN_CRIER_GMTIME_H
#define TOWN_CRIER_GMTIME_H

namespace tc {
namespace ext {
struct date {
  int year;
  int month;
  int day;
};
}
}

void gmtime_tc(const time_t *timer, tc::ext::date& date);

#endif //TOWN_CRIER_GMTIME_H
