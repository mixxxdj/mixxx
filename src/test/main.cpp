#ifdef USE_BENCH
#include <benchmark/benchmark.h>
#endif

#include <QDir>

#include "errordialoghandler.h"
#include "mixxxtest.h"
#include "util/logging.h"

int main(int argc, char **argv) {
    // By default, render analyzer waveform tests to an offscreen buffer
    if (qEnvironmentVariableIsEmpty("QT_QPA_PLATFORM")) {
        qputenv("QT_QPA_PLATFORM", QByteArray("offscreen"));
    }

#ifdef MIXXX_TEST_LV2_PATH
    // On Windows the vcpkg-provided lilv library does not know where the
    // LV2 specification bundles of the build environment live, and this
    // machine has no LV2 bundles in lilv's default search directories.
    // Without a single specification on its search path,
    // lilv_world_load_all() corrupts the heap while scanning, which
    // blocks or aborts test setup (e.g. PlayerManagerTest::SetUp) in
    // debug builds. Default LV2_PATH to the bundles of the build
    // environment unless the caller provided their own.
    if (qEnvironmentVariableIsEmpty("LV2_PATH")) {
        QByteArray lv2Path = QByteArrayLiteral(MIXXX_TEST_LV2_PATH);
        lv2Path.replace('/', '\\');
        if (QDir(QString::fromUtf8(lv2Path)).exists()) {
            qputenv("LV2_PATH", lv2Path);
        }
    }
#endif

    // We never want to popup error dialogs when running tests.
    ErrorDialogHandler::setEnabled(false);

#ifdef USE_BENCH
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
        MixxxTest::ApplicationScope applicationScope(argc, argv);
        benchmark::RunSpecifiedBenchmarks();
        return 0;
    }

    // Otherwise, run the test suite:
#endif
    testing::InitGoogleTest(&argc, argv);
    MixxxTest::ApplicationScope applicationScope(argc, argv);
    return RUN_ALL_TESTS();
}
