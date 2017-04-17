#include <stdio.h>
#include <stdlib.h>

#include "scrapers/UPS_Tracking.h"

int ups_self_test() {
  USPSScraper uspsScraper;
  int resp;
  uspsScraper.ups_tracking("1ZE331480394808282", &resp);

  return 0;
}