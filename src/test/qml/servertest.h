#pragma once

#include <Spix/TestServer.h>
#include <gtest/gtest.h>

#define UI_TEST_ONLY                                                       \
    if (MixxxAppTest::instance == nullptr) {                               \
        GTEST_SKIP() << "Skipping UI test since no UI setup is available"; \
    }

class MixxxAppTest : public spix::TestServer {
  protected:
    void executeTest() override {
        instance = this;
        wait(std::chrono::milliseconds(1000));
        ::testing::GTEST_FLAG(filter) = "UITest.*";
        result = RUN_ALL_TESTS();
        quit();
    }

  public:
    static MixxxAppTest* instance;
    int result{-1};
};
