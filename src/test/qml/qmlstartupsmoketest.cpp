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

#include "coreservices.h"
#include "preferences/configobject.h"
#include "qml/qmlapplication.h"
#include "skin/qml/qmlskin.h"
#include "test/mixxxtest.h"
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
#if defined(__WINDOWS__)
    if (skin.useNewUi) {
        GTEST_SKIP() << "NewUi QML startup smoke test is temporarily disabled on Windows";
    }
#endif
    QTemporaryDir profile;
    ASSERT_TRUE(profile.isValid());
    const QString settingsPath = profile.path() + QLatin1Char('/');
    writeProfile(settingsPath, skin);

    SettingsPathGuard settingsPathGuard(CmdlineArgs::Instance());
    CmdlineArgs::Instance().setSettingsPath(settingsPath);
    const auto args = makeCmdlineArgs(settingsPath);
    auto coreServices = std::make_shared<mixxx::CoreServices>(args, application());

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
    }
    application()->removeEventFilter(&rejectFileDialogs);

    EXPECT_TRUE(startupReady)
            << "Mixxx failed to start the QML skin '" << skin.name << "'"
            << "\nQML entry point: "
            << (mainQmlFilePath.isEmpty() ? "<default>" : qPrintable(mainQmlFilePath))
            << "\nSettings path: " << qPrintable(settingsPath);
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
