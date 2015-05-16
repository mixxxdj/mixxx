#include <gtest/gtest.h>
#include "util/console.h"


int main(int argc, char **argv) {
    Console console();
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
