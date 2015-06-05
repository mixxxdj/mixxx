#include "mixxxtest.h"
#include "util/console.h"

int main(int argc, char **argv) {
    Console console();

    MixxxTest::ApplicationScope applicationScope(argc, argv);

    return RUN_ALL_TESTS();
}
