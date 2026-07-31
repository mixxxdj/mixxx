#include <Spix/AnyRpcServer.h>
#include <Spix/QtQmlBot.h>

#include <QApplication>
#include <QCoreApplication>
#include <QDebug>
#include <QEventLoop>
#include <QGuiApplication>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QQuickWindow>
#include <QThread>

#include "control/controlobject.h"
#include "coreservices.h"
#include "library/trackcollectionmanager.h"
#include "mixer/playermanager.h"
#include "mixxxapplication.h"
#include "mixxxtest.h"
#include "qml/qmlapplication.h"
#include "soundio/soundmanager.h"
#include "track/track.h"
#include "util/cmdlineargs.h"
#include "util/fileinfo.h"
#include "util/versionstore.h"

int runServeMode(int argc, char** argv) {
    QCoreApplication::setOrganizationDomain("mixxx.org");
    QApplication::setAttribute(Qt::AA_ShareOpenGLContexts);

    QGuiApplication::setHighDpiScaleFactorRoundingPolicy(
            Qt::HighDpiScaleFactorRoundingPolicy::PassThrough);
    QGuiApplication::setDesktopFileName(QStringLiteral("org.mixxx.Mixxx"));

    QCoreApplication::setApplicationName(VersionStore::applicationName());
    QCoreApplication::setApplicationVersion(VersionStore::version());

    CmdlineArgs& args = CmdlineArgs::Instance();
    args.parse(argc, argv);

    QThread::currentThread()->setObjectName("Main");

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
        } else if (command == "getConfigValue") {
            QString rest = QString::fromStdString(payload);
            int comma = rest.indexOf(',');
            if (comma < 0) {
                qWarning() << "getConfigValue: invalid payload, expected "
                              "'group,key', got:"
                           << rest;
                return;
            }
            QString group = rest.left(comma);
            QString key = rest.mid(comma + 1);

            QString value = pCoreServices->getSettings()->getValueString(
                    ConfigKey(group, key));

            qDebug() << "getConfigValue:" << group << key << "=" << value;

            auto windows = QGuiApplication::topLevelWindows();
            for (auto* w : std::as_const(windows)) {
                w->setProperty("lastConfigValue", value);
            }
        } else if (command == "setConfigValue") {
            QString rest = QString::fromStdString(payload);
            int firstComma = rest.indexOf(',');
            if (firstComma < 0) {
                qWarning() << "setConfigValue: invalid payload, expected "
                              "'group,key,value', got:"
                           << rest;
                return;
            }
            int secondComma = rest.indexOf(',', firstComma + 1);
            if (secondComma < 0) {
                qWarning() << "setConfigValue: invalid payload, expected "
                              "'group,key,value', got:"
                           << rest;
                return;
            }
            QString group = rest.left(firstComma);
            QString key = rest.mid(firstComma + 1, secondComma - firstComma - 1);
            QString valueStr = rest.mid(secondComma + 1);
            pCoreServices->getSettings()->setValue(ConfigKey(group, key), valueStr);
            qDebug() << "setConfigValue:" << group << key << "=" << valueStr;
        } else if (command == "loadTrack") {
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
                    auto* pManager = pCoreServices->getTrackCollectionManager().get();
                    QEventLoop loop;
                    QObject::connect(pManager,
                            &TrackCollectionManager::libraryScanFinished,
                            &loop,
                            &QEventLoop::quit);
                    pManager->startLibraryScan();
                    loop.exec();
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
                    auto* pManager = pCoreServices->getTrackCollectionManager().get();
                    QEventLoop loop;
                    QObject::connect(pManager,
                            &TrackCollectionManager::libraryScanFinished,
                            &loop,
                            &QEventLoop::quit);
                    pManager->startLibraryScan();
                    loop.exec();
                }
                qDebug() << "library: removed directory" << path
                         << "scan=" << scan;
            } else {
                qWarning() << "library: unknown action" << action;
            }
        } else if (command == "registerMockDevices") {
            QJsonDocument doc =
                    QJsonDocument::fromJson(
                            QByteArray::fromStdString(payload));
            QJsonObject obj = doc.object();
            QJsonArray devices = obj["devices"].toArray();
            pCoreServices->getSoundManager()->registerMockDevices(devices);
            qDebug() << "registerMockDevices: injected"
                     << devices.size() << "devices";
        } else if (command == "clearMockDevices") {
            pCoreServices->getSoundManager()->clearMockDevices();
            auto config = pCoreServices->getSoundManager()->getConfig();
            config.clearOutputs();
            config.clearInputs();
            pCoreServices->getSoundManager()->setConfig(config);
            qDebug() << "clearMockDevices: cleared";
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
}
