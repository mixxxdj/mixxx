#include <qjsondocument.h>
#include <qjsonobject.h>

#include <chrono>
#include <memory>

#include "analyzer/analyzerbeats.h"
#include "analyzer/constants.h"
#include "sources/audiosourcestereoproxy.h"
#include "util/timer.h"
#ifdef USE_BENCH
#include <benchmark/benchmark.h>
#endif
#include "errordialoghandler.h"
#include "mixxxtest.h"
#include "test/analyzer_benchmark.cpp"
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
    bool run_analyser_report = false;
    for (int i = 0; i < argc; ++i) {
        if (strcmp(argv[i], "--benchmark") == 0) {
            run_benchmarks = true;
            break;
        } else if (strcmp(argv[i], "--analyser") == 0) {
            run_analyser_report = true;
            break;
        } else if (strcmp(argv[i], "--trace") == 0) {
            mixxx::Logging::setLogLevel(mixxx::LogLevel::Trace);
        }
    }

    if (run_analyser_report) {
        MixxxTest::ApplicationScope applicationScope(argc, argv);

        if (!SoundSourceProxy::registerProviders()) {
            qCritical() << "Failed to register any SoundSource providers";
            return 1;
        }

        AnalyzerBenchmark benchmark;
        auto* path = argv[argc - 1];

        Timer timer("runtime");

        timer.start();
        auto pTrack = std::make_shared<TrackWrapper>(path);
        auto source = benchmark.run(pTrack);
        auto duration = timer.elapsed(false);
        ;
        if (!source) {
            qCritical() << "Could not analyze";
            return 1;
        }
        auto result = pTrack->result();

        auto duration_ms =
                std::chrono::duration_cast<std::chrono::milliseconds>(
                        duration.toStdDuration())
                        .count();

        QJsonObject output;
        output.insert("path", path);
        output.insert("track_duration", source->getDuration());
        output.insert("track_samplerate",
                static_cast<int>(source->getSignalInfo().getSampleRate() /
                        source->getSignalInfo().getChannelCount()));
        output.insert("runtime", static_cast<int>(duration_ms));
        output.insert("first_beat", result->firstBeat().value());
        output.insert("bpm", result->getLastMarkerBpm().value());

        std::cout << QJsonDocument(output).toJson().data();

        return 0;
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
