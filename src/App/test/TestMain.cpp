//
// Created by fanz on 6/11/16.
//

#include "gtest/gtest.h"

#define LOGURU_IMPLEMENTATION 1
#include "Log.h"

int main(int argc, char **argv) {
  // init logging
  loguru::init(argc, (const char**) argv);
  loguru::g_stderr_verbosity = LOG_LEVEL_DEBUG;


  ::testing::InitGoogleTest(&argc, argv);
  int ret = RUN_ALL_TESTS();
  return ret;
}