#include <gtest/gtest.h>

#include <QEvent>
#include <QFile>
#include <QFileDialog>
#include <QString>
#include <QTemporaryDir>
#include <QTimer>
#include <memory>
#include <ostream>
#include <string>

#include "coreservices.h"
#include "preferences/configobject.h"
#include "qml/qmlapplication.h"
#include "skin/skinloader.h"
#include "test/mixxxtest.h"
#include "util/cmdlineargs.h"
#include "util/versionstore.h"

namespace {

struct QmlSkin {
    const char* name;
    bool useNewUi;
};

void PrintTo(const QmlSkin& skin, std::ostream* stream) {
    *stream << skin.name << (skin.useNewUi ? " (--new-ui)" : " (--developer)");
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

class QmlStartupSmokeTest : public MixxxTest,
                            public testing::WithParamInterface<QmlSkin> {
  protected:
    static CmdlineArgs makeCmdlineArgs(const QString& settingsPath) {
        CmdlineArgs args;
        args.setSettingsPath(settingsPath);
        return args;
    }

    static void writeProfile(const QString& settingsPath, const QmlSkin& skin) {
        auto settings = UserSettingsPointer(new UserSettings(settingsPath + "mixxx.cfg"));
        settings->setValue(ConfigKey("[Config]", "Version"), VersionStore::FUTURE_UNSTABLE);
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
};

TEST_P(QmlStartupSmokeTest, Starts) {
    const auto& skin = GetParam();
    QTemporaryDir profile;
    ASSERT_TRUE(profile.isValid());
    const QString settingsPath = profile.path() + QLatin1Char('/');
    writeProfile(settingsPath, skin);

    CmdlineArgs::Instance().setSettingsPath(settingsPath);
    const auto args = makeCmdlineArgs(settingsPath);
    auto coreServices = std::make_shared<mixxx::CoreServices>(args, application());

    QString mainQmlFilePath;
    if (!skin.useNewUi) {
        mixxx::skin::SkinLoader skinLoader(coreServices->getSettings());
        const auto configuredSkin = skinLoader.getConfiguredSkin();
        ASSERT_TRUE(configuredSkin);
        ASSERT_EQ(mixxx::skin::SkinType::QML, configuredSkin->type());
        mainQmlFilePath = configuredSkin->mainQmlFilePath();
    }

    RejectFileDialogs rejectFileDialogs;
    application()->installEventFilter(&rejectFileDialogs);
    mixxx::qml::QmlApplication qmlApplication(application(), coreServices, mainQmlFilePath);
    application()->removeEventFilter(&rejectFileDialogs);

    EXPECT_TRUE(qmlApplication.isReady())
            << "Mixxx failed to start the QML skin '" << skin.name << "'";
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
