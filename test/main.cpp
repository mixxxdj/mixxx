#include <benchmark/benchmark.h>

#include "errordialoghandler.h"
#include "mixxxtest.h"
#include "util/logging.h"

int main(int argc, char **argv) {
    // We never want to popup error dialogs when running tests.
    ErrorDialogHandler::setEnabled(false);

    bool run_benchmarks = false;
    for (int i = 0; i < argc; ++i) {
        if (strcmp(argv[i], "--benchmark") == 0) {
            run_benchmarks = true;
            break;
        } else if (strcmp(argv[i], "--trace") == 0) {
            mixxx::Logging::setLogLevel(mixxx::LogLevel::Trace);
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
