// gtest-nano.h implements very minimalistic GTest-compatible API that can be used to run tests in older
// (C++98-compatible) environments.

#include <cmath>
#include <cstring>
#include <iostream>
#include <vector>

namespace testing {
    struct TestInfo {
        const char *suite;
        const char *name;
        void (*testFunc)();
    };

    std::vector<TestInfo> g_tests;
    bool g_testPass;
    bool g_allPass;

    bool FLAGS_gtest_also_run_disabled_tests = false;

    const char DISABLED_TEST_PREFIX[] = "DISABLED_";
    const std::size_t STRLEN_DISABLED_TEST_PREFIX = sizeof(DISABLED_TEST_PREFIX) - 1; // `- 1` due to the null terminator

    void InitGoogleTest(const int *argc, char **argv) {
        for (int i = 1; i < *argc; i++) {
            if (std::strcmp(argv[i], "--gtest_also_run_disabled_tests") == 0) {
                FLAGS_gtest_also_run_disabled_tests = true;
            } else {
                std::cerr
                        << "Warning: unrecognized argument " << argv[i]
                        << " (the only supported flag is --gtest_also_run_disabled_tests)\n";
            }
        }

        std::cout << "[----------] gtest-nano: starting up\n";
    }

    int runAllTests() {
        std::cout << "[==========] Running " << g_tests.size() << " tests.\n";

        g_allPass = true;
        for (std::vector<TestInfo>::const_iterator it = g_tests.begin(); it != g_tests.end(); ++it) {
            const TestInfo &test = *it;
            bool is_disabled = false;
            if (std::strncmp(test.suite, DISABLED_TEST_PREFIX, STRLEN_DISABLED_TEST_PREFIX) == 0) {
                is_disabled = true;
            }
            if (std::strncmp(test.name, DISABLED_TEST_PREFIX, STRLEN_DISABLED_TEST_PREFIX) == 0) {
                is_disabled = true;
            }
            if (is_disabled && !FLAGS_gtest_also_run_disabled_tests) {
                std::cout << "[ DISABLED ] " << test.suite << "." << test.name << "\n";
                continue;
            }
            g_testPass = true;
            std::cout << "[ RUN      ] " << test.suite << "." << test.name << "\n";
            test.testFunc();
            std::cout << (g_testPass ? "[       OK ] " : "[  FAILED  ] ") <<  test.suite << "." << test.name << "\n";
            g_allPass &= g_testPass;
        }

        return g_allPass ? 0 : 1;
    }
}

// Defines test function and registers it in global list of tests
#define TEST(suite, name)                                    \
    void Test_##suite##_##name();                            \
    namespace {                                              \
        struct Register_##suite##_##name {                   \
            Register_##suite##_##name() {                    \
                ::testing::TestInfo info = {                 \
                    #suite,                                  \
                    #name,                                   \
                    Test_##suite##_##name                    \
                };                                           \
                ::testing::g_tests.push_back(info);          \
            }                                                \
        };                                                   \
        Register_##suite##_##name register_##suite##_##name; \
    }                                                        \
    void Test_##suite##_##name()

// Assertion macro
#define EXPECT_EQ(a, b)                           \
    do {                                          \
        if ((a) == (b)) {                         \
        } else {                                  \
            ::testing::g_testPass = false;        \
        }                                         \
    } while (false)

// Floating point comparison macro
#define EXPECT_FLOAT_EQ(a, b)                     \
    do {                                          \
        if (std::fabs(a - b) < 1e-6) {            \
        } else {                                  \
            ::testing::g_testPass = false;        \
        }                                         \
    } while (false)

#define EXPECT_DOUBLE_EQ(a, b) EXPECT_FLOAT_EQ(a, b)

// Failure macro
#define FAIL()                     \
    ::testing::g_testPass = false; \
    std::cerr

#define RUN_ALL_TESTS() ::testing::runAllTests()
