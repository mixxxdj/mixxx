#include "util/versionstore.h"
#ifdef USE_TEST_UI
#include <Spix/AnyRpcServer.h>
#include <Spix/QtQmlBot.h>

#include <QDir>
#include <QGuiApplication>
#include <QJsonDocument>
#include <QJsonObject>
#include <QQuickWindow>
#include <cstdlib>

#include "control/controlobject.h"
#include "coreservices.h"
#include "engine/engine.h"
#include "library/trackcollectionmanager.h"
#include "mixer/playermanager.h"
#include "qml/qmlapplication.h"
#include "test/qml/servertest.h"
#include "track/track.h"
#include "util/fileinfo.h"
#endif
#ifdef USE_BENCH
#include <benchmark/benchmark.h>
#endif

#include "errordialoghandler.h"
#include "mixxxtest.h"
#include "util/logging.h"

int main(int argc, char **argv) {
    // We never want to popup error dialogs when running tests.
    ErrorDialogHandler::setEnabled(false);

#ifdef USE_BENCH
    bool run_benchmarks = false;
#endif

#ifdef USE_TEST_UI
    bool run_ui = false;
    bool run_serve = false;
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
        } else if (strcmp(argv[i], "--serve") == 0) {
            run_serve = true;
            break;
        } else
#endif
                if (strcmp(argv[i], "--trace") == 0) {
            mixxx::Logging::setLogLevel(mixxx::LogLevel::Trace);
        }
    }

    // By default, render analyzer waveform tests to an offscreen buffer except
    // in RPC mode/UI testing
#ifdef USE_TEST_UI
    if (qEnvironmentVariableIsEmpty("QT_QPA_PLATFORM") && !run_serve) {
#else
    if (qEnvironmentVariableIsEmpty("QT_QPA_PLATFORM")) {
#endif
        qputenv("QT_QPA_PLATFORM", QByteArray("offscreen"));
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
#ifdef USE_BENCH
    if (run_benchmarks) {
        testing::InitGoogleTest(&argc, argv);
        MixxxTest::ApplicationScope applicationScope(argc, argv);
        benchmark::RunSpecifiedBenchmarks();
        return 0;
    } else
#endif
#ifdef USE_TEST_UI
            if (run_serve) {
        // auto* pApp = MixxxTest::application();

        // These need to be set early on (not sure how early) in order to trigger
        // logic in the OS X appstore support patch from QTBUG-16549.
        QCoreApplication::setOrganizationDomain("mixxx.org");
        QApplication::setAttribute(Qt::AA_ShareOpenGLContexts);

        QGuiApplication::setHighDpiScaleFactorRoundingPolicy(
                Qt::HighDpiScaleFactorRoundingPolicy::PassThrough);
        // Needed by Wayland compositors to set proper app_id and window icon
        QGuiApplication::setDesktopFileName(QStringLiteral("org.mixxx.Mixxx"));

        QCoreApplication::setApplicationName(VersionStore::applicationName());
        QCoreApplication::setApplicationVersion(VersionStore::version());

        // Construct a list of strings based on the command line arguments
        CmdlineArgs& args = CmdlineArgs::Instance();
        argv++;
        argc--;
        args.parse(argc, argv);
        args.parseForUserFeedback();

        // Set a unique thread object name
        //
        // This is used for a check within ErrorDialogHandler::errorDialog()
        // for earlier Qt versions
        QThread::currentThread()->setObjectName("Main");
        // args.parseForUserFeedback();

        MixxxApplication app(argc, argv);

        auto pCoreServices =
                std::make_shared<mixxx::CoreServices>(args, &app);

        mixxx::qml::QmlApplication qmlApplication(&app, pCoreServices);

        auto rootObjects = qmlApplication.qmlEngine()->rootObjects();
        for (auto* root : std::as_const(rootObjects)) {
            if (auto* window = qobject_cast<QQuickWindow*>(root)) {
                if (window->objectName().isEmpty()) {
                    window->setObjectName(root->objectName());
                }
            }
        }

        spix::AnyRpcServer server(9000);
        auto pPlayerManager = pCoreServices->getPlayerManager();

        server.setGenericCommandHandler([pCoreServices, pPlayerManager, &qmlApplication](
                                                const std::string& command,
                                                const std::string& payload) {
            if (command == "getControlValue") {
                QString rest = QString::fromStdString(payload);
                int comma = rest.indexOf(',');
                if (comma < 0) {
                    qWarning() << "getControlValue: invalid payload, expected "
                                  "'group,item', got:"
                               << rest;
                    return;
                }
                QString group = rest.left(comma);
                QString item = rest.mid(comma + 1);

                ConfigKey key(group, item);
                if (!ControlObject::exists(key)) {
                    qWarning() << "getControlValue: ControlObject does not exist:" << group << item;
                    return;
                }

                double value = ControlObject::get(key);

                qDebug() << "getControlValue:" << group << item << "=" << value;

                auto windows = QGuiApplication::topLevelWindows();
                for (auto* w : std::as_const(windows)) {
                    w->setProperty("lastControlValue", value);
                }
            } else if (command == "setControlValue") {
                QString rest = QString::fromStdString(payload);
                int firstComma = rest.indexOf(',');
                if (firstComma < 0) {
                    qWarning() << "setControlValue: invalid payload, expected "
                                  "'group,item,value', got:"
                               << rest;
                    return;
                }
                int secondComma = rest.indexOf(',', firstComma + 1);
                if (secondComma < 0) {
                    qWarning() << "setControlValue: invalid payload, expected "
                                  "'group,item,value', got:"
                               << rest;
                    return;
                }
                QString group = rest.left(firstComma);
                QString item = rest.mid(firstComma + 1, secondComma - firstComma - 1);
                QString valueStr = rest.mid(secondComma + 1);
                bool ok = false;
                double value = valueStr.toDouble(&ok);
                if (!ok) {
                    qWarning() << "setControlValue: invalid value:" << valueStr;
                    return;
                }
                ConfigKey key(group, item);
                if (!ControlObject::exists(key)) {
                    qWarning() << "setControlValue: ControlObject does not exist:" << group << item;
                    return;
                }
                ControlObject::set(key, value);
                qDebug() << "setControlValue:" << group << item << "=" << value;
            } else if (command == "loadTrack") {
                // payload: "deck,filepath" — loads a track file into a deck
                QString rest = QString::fromStdString(payload);
                int comma = rest.indexOf(',');
                if (comma < 0) {
                    qWarning() << "loadTrack: invalid payload, expected "
                                  "'deck,filepath', got:"
                               << rest;
                    return;
                }
                int deck = rest.left(comma).toInt();
                QString filePath = rest.mid(comma + 1);
                QString group = QString("[Channel%1]").arg(deck);
                pPlayerManager->slotLoadLocationToPlayer(filePath, group, false);
                qDebug() << "loadTrack: loaded" << filePath << "into" << group;
            } else if (command == "library") {
                QJsonDocument doc =
                        QJsonDocument::fromJson(
                                QByteArray::fromStdString(payload));
                QJsonObject obj = doc.object();
                QString action = obj["action"].toString();
                QString path = obj["path"].toString();
                bool scan = obj["scan"].toBool(false);

                if (action == "addDirectory") {
                    auto result = pCoreServices->getTrackCollectionManager()
                                          ->addDirectory(mixxx::FileInfo(path));
                    if (result == DirectoryDAO::AddResult::AlreadyWatching) {
                        qDebug() << "library: directory already watched:" << path;
                    } else if (result != DirectoryDAO::AddResult::Ok) {
                        qWarning() << "library addDirectory failed:"
                                   << static_cast<int>(result);
                        return;
                    }
                    if (scan) {
                        pCoreServices->getTrackCollectionManager()
                                ->startLibraryScan();
                    }
                    qDebug() << "library: added directory" << path
                             << "scan=" << scan;
                } else if (action == "removeDirectory") {
                    auto result = pCoreServices->getTrackCollectionManager()
                                          ->removeDirectory(mixxx::FileInfo(path));
                    if (result != DirectoryDAO::RemoveResult::Ok) {
                        qWarning() << "library removeDirectory failed:"
                                   << static_cast<int>(result);
                        return;
                    }
                    if (scan) {
                        pCoreServices->getTrackCollectionManager()
                                ->startLibraryScan();
                    }
                    qDebug() << "library: removed directory" << path
                             << "scan=" << scan;
                } else {
                    qWarning() << "library: unknown action" << action;
                }
            } else if (command == "reloadQml") {
                qmlApplication.loadQml(qmlApplication.mainFilePath());
                qDebug() << "reloadQml: QML engine reloaded";
            } else {
                qWarning() << "Unknown generic command:" << QString::fromStdString(command);
            }
        });

        auto* pBot = new spix::QtQmlBot();
        pBot->runTestServer(server);

        app.exec();
        delete pBot;
        return 0;
    } else if (run_ui) {
        testing::InitGoogleTest(&argc, argv);
        MixxxTest::ApplicationScope applicationScope(argc, argv);
        auto* pApp = MixxxTest::application();
        auto pCoreServices =
                std::make_shared<mixxx::CoreServices>(CmdlineArgs::Instance(), pApp);

        mixxx::qml::QmlApplication qmlApplication(pApp, pCoreServices);

        MixxxAppTest tests;
        auto* pBot = new spix::QtQmlBot();
        pBot->runTestServer(tests);

        pApp->exec();
        delete pBot;
        return tests.result;
    } else
#endif
    {
        testing::InitGoogleTest(&argc, argv);
        MixxxTest::ApplicationScope applicationScope(argc, argv);
        return RUN_ALL_TESTS();
    }
}
