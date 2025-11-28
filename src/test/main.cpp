
#ifdef USE_TEST_UI
#include <Spix/QtQmlBot.h>

#include "coreservices.h"
#include "qml/qmlapplication.h"
#include "test/qml/servertest.h"
#endif
#ifdef USE_BENCH
#include <benchmark/benchmark.h>
#endif

#include "errordialoghandler.h"
#include "mixxxtest.h"
#include "util/logging.h"

int main(int argc, char **argv) {
    // By default, render analyzer waveform tests to an offscreen buffer
    if (qEnvironmentVariableIsEmpty("QT_QPA_PLATFORM")) {
        qputenv("QT_QPA_PLATFORM", QByteArray("offscreen"));
    }

    // We never want to popup error dialogs when running tests.
    ErrorDialogHandler::setEnabled(false);

#ifdef USE_BENCH
    bool run_benchmarks = false;
#endif

#ifdef USE_TEST_UI
    bool run_ui = false;
#endif
    for (int i = 0; i < argc; ++i) {
#ifdef USE_BENCH
        if (strcmp(argv[i], "--benchmark") == 0) {
            run_benchmarks = true;
            break;
        } else
#endif
#ifdef USE_TEST_UI
                if (strcmp(argv[i], "--ui") == 0) {
            run_ui = true;
            break;
        } else
#endif
                if (strcmp(argv[i], "--trace") == 0) {
            mixxx::Logging::setLogLevel(mixxx::LogLevel::Trace);
        }
    }
#ifdef USE_BENCH
    if (run_benchmarks) {
        benchmark::Initialize(&argc, argv);
        MixxxTest::ApplicationScope applicationScope(argc, argv);
        benchmark::RunSpecifiedBenchmarks();
        return 0;
    }
#endif

    // Otherwise, run the test suite:
    testing::InitGoogleTest(&argc, argv);
    MixxxTest::ApplicationScope applicationScope(argc, argv);

#ifdef USE_BENCH
    if (run_benchmarks) {
        benchmark::RunSpecifiedBenchmarks();
        return 0;
    } else
#endif
#ifdef USE_TEST_UI
            if (run_ui) {
        auto* pApp = MixxxTest::application();

        mixxx::qml::QmlApplication qmlApplication(pApp, CmdlineArgs::Instance());

        MixxxAppTest tests;
        auto* pBot = new spix::QtQmlBot();
        pBot->runTestServer(tests);

        pApp->exec();
        delete pBot;
        return tests.result;
    } else
#endif
    {
        return RUN_ALL_TESTS();
    }
}
