#include <benchmark/benchmark.h>

#include "mixxxtest.h"
#include "util/console.h"
#include "errordialoghandler.h"

int main(int argc, char **argv) {
    Console console;
    // We never want to popup error dialogs when running tests.
    ErrorDialogHandler::setEnabled(false);

    bool run_benchmarks = false;
    for (int i = 0; i < argc; ++i) {
        if (strcmp(argv[i], "--benchmark") == 0) {
            run_benchmarks = true;
            break;
        }
    }

    if (run_benchmarks) {
        benchmark::Initialize(&argc, argv);
    } else {
        testing::InitGoogleTest(&argc, argv);
    }

    // Otherwise, run the test suite:
    MixxxTest::ApplicationScope applicationScope(argc, argv);

    if (run_benchmarks) {
        benchmark::RunSpecifiedBenchmarks();
        return 0;
    } else {
        return RUN_ALL_TESTS();
    }
}
