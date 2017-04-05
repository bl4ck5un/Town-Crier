#include "gtest/gtest.h"

#define LOGURU_IMPLEMENTATION 1
#include "Log.h"

int main(int argc, char **argv) {
  loguru::g_stderr_verbosity = LOG_LEVEL_INFO;
  loguru::init(argc, (const char **) argv);

  ::testing::InitGoogleTest(&argc, argv);
  int ret = RUN_ALL_TESTS();
  return ret;
}
