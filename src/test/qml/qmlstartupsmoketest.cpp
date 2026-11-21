#include <gtest/gtest.h>

#include <QDialog>
#include <QDir>
#include <QEvent>
#include <QFile>
#include <QFileDialog>
#include <QString>
#include <QTemporaryDir>
#include <QTimer>
#include <memory>
#include <ostream>
#include <string>

#include "control/controlobject.h"
#include "coreservices.h"
#include "library/autodj/autodjprocessor.h"
#include "library/library.h"
#include "library/playlisttablemodel.h"
#include "library/trackcollectionmanager.h"
#include "preferences/configobject.h"
#include "qml/qmlapplication.h"
#include "skin/qml/qmlskin.h"
#include "test/mixxxtest.h"
#include "track/track.h"
#include "util/cmdlineargs.h"
#include "util/versionstore.h"

namespace {

struct QmlSkin {
    const char* name;
    bool useNewUi;
};

void PrintTo(const QmlSkin& skin, std::ostream* stream) {
    *stream << skin.name;
}

class RejectFileDialogs final : public QObject {
  protected:
    bool eventFilter(QObject* watched, QEvent* event) override {
        if (event->type() == QEvent::Show) {
            if (auto* dialog = qobject_cast<QFileDialog*>(watched)) {
                QTimer::singleShot(0, dialog, &QDialog::reject);
            }
        }
        return false;
    }
};

class SettingsPathGuard final {
  public:
    explicit SettingsPathGuard(CmdlineArgs& args)
            : m_args(args),
              m_previousArgs(args) {
    }

    ~SettingsPathGuard() {
        m_args = m_previousArgs;
    }

  private:
    CmdlineArgs& m_args;
    const CmdlineArgs m_previousArgs;
};

class QmlStartupSmokeTest : public MixxxTest,
                            public testing::WithParamInterface<QmlSkin> {
  protected:
    static CmdlineArgs makeCmdlineArgs(const QString& settingsPath) {
        CmdlineArgs args;
        args.setSettingsPath(settingsPath);
        return args;
    }

    static void writeProfile(const QString& settingsPath,
            const QmlSkin& skin,
            bool startAutoDJ,
            bool randomQueue) {
        auto settings = UserSettingsPointer(new UserSettings(settingsPath + "mixxx.cfg"));
        settings->setValue(ConfigKey("[Config]", "Version"), VersionStore::FUTURE_UNSTABLE);
        if (startAutoDJ) {
            settings->setValue(ConfigKey("[Auto DJ]", "StartOnLaunch"), true);
        }
        if (randomQueue) {
            settings->setValue(ConfigKey("[Auto DJ]", "EnableRandomQueue"), true);
            settings->setValue(ConfigKey("[Auto DJ]", "RandomQueueMinimumAllowed"), 1);
        }
        if (!skin.useNewUi) {
            settings->setValue(
                    ConfigKey("[Config]", "ResizableSkin"),
                    QString::fromLatin1(skin.name));
        }
        settings->save();

        QFile soundConfig(settingsPath + "soundconfig.xml");
        ASSERT_TRUE(soundConfig.open(QIODevice::WriteOnly | QIODevice::Truncate));
        ASSERT_GT(soundConfig.write("<SoundManagerConfig api=\"None\"/>\n"), 0);
    }

    void verifyStartup(bool startAutoDJ, bool addTrack, bool randomQueue);
};

void QmlStartupSmokeTest::verifyStartup(bool startAutoDJ, bool addTrack, bool randomQueue) {
    const auto& skin = GetParam();
    QTemporaryDir profile;
    ASSERT_TRUE(profile.isValid());
    const QString settingsPath = profile.path() + QLatin1Char('/');
    writeProfile(settingsPath, skin, startAutoDJ, randomQueue);

    SettingsPathGuard settingsPathGuard(CmdlineArgs::Instance());
    CmdlineArgs::Instance().setSettingsPath(settingsPath);
    const auto args = makeCmdlineArgs(settingsPath);
    auto coreServices = std::make_shared<mixxx::CoreServices>(args, application());

    AutoDJProcessor* processor = nullptr;
    int emptyQueueErrors = 0;
    bool requestedPlayback = false;
    QObject observation;
    QObject::connect(coreServices.get(),
            &mixxx::CoreServices::initializationProgressUpdate,
            &observation,
            [&](int progress, const QString&) {
                if (progress != 60) {
                    return;
                }
                processor = coreServices->getLibrary()->findChild<AutoDJProcessor*>();
                ASSERT_NE(processor, nullptr);
                QObject::connect(processor,
                        &AutoDJProcessor::autoDJError,
                        &observation,
                        [&](AutoDJProcessor::AutoDJError error) {
                            if (error == AutoDJProcessor::ADJ_QUEUE_EMPTY) {
                                ++emptyQueueErrors;
                            }
                        });
                QObject::connect(processor,
                        &AutoDJProcessor::loadTrackToPlayer,
                        &observation,
                        [&](TrackPointer, const QString&,
#ifdef __STEM__
                                mixxx::StemChannelSelection,
#endif
                                bool play) {
                            requestedPlayback |= play;
                        });
                if (addTrack) {
                    const auto track = coreServices->getTrackCollectionManager()->getOrAddTrack(
                            TrackRef::fromFilePath(getTestFile(QStringLiteral("-png.mp3"))));
                    ASSERT_TRUE(track);
                    ASSERT_TRUE(track->getId().isValid());
                    if (!randomQueue) {
                        ASSERT_TRUE(processor->getTableModel()->appendTrack(track->getId()));
                    }
                }
            });

    QString mainQmlFilePath;
    if (!skin.useNewUi) {
        ASSERT_QSTRING_EQ(
                QString::fromLatin1(skin.name),
                coreServices->getSettings()->getValueString(
                        ConfigKey("[Config]", "ResizableSkin")));
        const auto qmlSkin = mixxx::skin::qml::QmlSkin::fromDirectory(
                QDir(QStringLiteral(RESOURCE_FOLDER "/skins/LateNightQML")));
        ASSERT_TRUE(qmlSkin);
        ASSERT_EQ(mixxx::skin::SkinType::QML, qmlSkin->type());
        mainQmlFilePath = qmlSkin->mainQmlFilePath();
    }

    RejectFileDialogs rejectFileDialogs;
    application()->installEventFilter(&rejectFileDialogs);
    bool startupReady = false;
    {
        mixxx::qml::QmlApplication qmlApplication(application(), coreServices, mainQmlFilePath);
        startupReady = qmlApplication.isReady();
        ASSERT_NE(processor, nullptr);
        EXPECT_EQ(startAutoDJ && addTrack,
                ControlObject::toBool(ConfigKey("[AutoDJ]", "enabled")));
        EXPECT_EQ(startAutoDJ && !addTrack ? 1 : 0, emptyQueueErrors);
        EXPECT_EQ(startAutoDJ && addTrack, requestedPlayback);
        if (startAutoDJ && addTrack) {
            EXPECT_EQ(AutoDJProcessor::ADJ_ENABLE_P1LOADED, processor->getState());
        }
    }
    application()->removeEventFilter(&rejectFileDialogs);

    EXPECT_TRUE(startupReady)
            << "Mixxx failed to start the QML skin '" << skin.name << "'"
            << "\nQML entry point: "
            << (mainQmlFilePath.isEmpty() ? "<default>" : qPrintable(mainQmlFilePath))
            << "\nSettings path: " << qPrintable(settingsPath);
}

TEST_P(QmlStartupSmokeTest, Starts) {
    verifyStartup(false, false, false);
}

TEST_P(QmlStartupSmokeTest, StartsWithoutAutoDJByDefault) {
    verifyStartup(false, true, false);
}

TEST_P(QmlStartupSmokeTest, StartsAutoDJWithEmptyQueue) {
    verifyStartup(true, false, false);
}

TEST_P(QmlStartupSmokeTest, StartsAutoDJWithQueuedTrack) {
    verifyStartup(true, true, false);
}

TEST_P(QmlStartupSmokeTest, StartsAutoDJWithRandomTrack) {
    verifyStartup(true, true, true);
}

INSTANTIATE_TEST_SUITE_P(
        QmlSkins,
        QmlStartupSmokeTest,
        testing::Values(
                QmlSkin{"LateNightQML", false},
                QmlSkin{"NewUi", true}),
        [](const testing::TestParamInfo<QmlSkin>& info) {
            return std::string(info.param.name);
        });

} // namespace
