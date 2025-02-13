#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <QDomDocument>
#include <QTest>

#include "controllers/defs_controllers.h"
#include "controllers/legacycontrollermapping.h"
#include "controllers/legacycontrollermappingfilehandler.h"
#include "helpers/log_test.h"
#include "test/mixxxtest.h"
#include "util/time.h"

using ::testing::_;
using ::testing::FieldsAre;
using namespace std::chrono_literals;

class LegacyControllerMappingFileHandlerTest
        : public LegacyControllerMappingFileHandler,
          public MixxxTest {
  public:
    void SetUp() override {
        mixxx::Time::setTestMode(true);
        mixxx::Time::addTestTime(10ms);
        SETUP_LOG_CAPTURE();
    }

    void TearDown() override {
        mixxx::Time::setTestMode(false);
    }
    std::shared_ptr<LegacyControllerMapping> load(const QDomElement&,
            const QString&,
            const QDir&) override {
        throw std::runtime_error("not implemented");
    }
    static QFileInfo findLibraryPath(std::shared_ptr<LegacyControllerMapping>,
            const QString& dirname,
            const QDir&) {
        return QFileInfo(QDir("/dummy/path/").absoluteFilePath(dirname));
    }
    static QFileInfo findScriptFile(std::shared_ptr<LegacyControllerMapping>,
            const QString& filename,
            const QDir&) {
        return QFileInfo(QDir("/dummy/path/").absoluteFilePath(filename));
    }
};

class MockLegacyControllerMapping : public LegacyControllerMapping {
  public:
    MOCK_METHOD(void,
            addScriptFile,
            (LegacyControllerMapping::ScriptFileInfo info),
            (override));
    MOCK_METHOD(void,
            addScreenInfo,
            (LegacyControllerMapping::ScreenInfo info),
            (override));
    MOCK_METHOD(void, addModule, (const QFileInfo& dirinfo, bool builtin), (override));

    bool saveMapping(const QString&) const override {
        throw std::runtime_error("not implemented");
    }
    bool isMappable() const override {
        throw std::runtime_error("not implemented");
    }
};

TEST_F(LegacyControllerMappingFileHandlerTest, canParseSimpleMapping) {
    QDomDocument doc;
    doc.setContent(
            QByteArray(R"EOF(
        <controller id="DummyDevice">
            <scriptfiles>
                <file filename="DummyDeviceDefaultScreen.js" />
            </scriptfiles>
        </controller>
        )EOF"));

    auto mapping = std::make_shared<MockLegacyControllerMapping>();
    // This file always gets added
    EXPECT_CALL(*mapping,
            addScriptFile(FieldsAre(QString("common-controller-scripts.js"),
                    QString(""),
                    _, // gmock seems unable to assert QFileInfo
                    LegacyControllerMapping::ScriptFileInfo::Type::Javascript,
                    true)));

    EXPECT_CALL(*mapping,
            addScriptFile(FieldsAre(QString("DummyDeviceDefaultScreen.js"),
                    QString(""),
                    _, // gmock seems unable to assert QFileInfo
                    LegacyControllerMapping::ScriptFileInfo::Type::Javascript,
                    false)));
    EXPECT_CALL(*mapping, addScreenInfo(_)).Times(0);
    EXPECT_CALL(*mapping, addModule(_, _)).Times(0);

    addScriptFilesToMapping(
            doc.documentElement(),
            mapping,
            QDir());
}

TEST_F(LegacyControllerMappingFileHandlerTest, canParseScreenMapping) {
    QDomDocument doc;
    doc.setContent(QByteArray(R"EOF(
            <controller id="DummyDevice">
                <screens>
                    <screen identifier="main" width="480" height="360" targetFps="20" pixelType="RBGA" splashoff="2000" />
                </screens>
                <scriptfiles>
                    <file filename="DummyDeviceDefaultScreen.qml" />
                </scriptfiles>
                <qmllibraries>
                    <library path="foobar/" />
                </qmllibraries>
            </controller>
            )EOF"));

    auto mapping = std::make_shared<MockLegacyControllerMapping>();
    // This file always gets added
    EXPECT_CALL(*mapping,
            addScriptFile(FieldsAre(QString("common-controller-scripts.js"),
                    QString(""),
                    _, // gmock seems unable to assert QFileInfo
                    LegacyControllerMapping::ScriptFileInfo::Type::Javascript,
                    true)));

    EXPECT_CALL(*mapping,
            addScriptFile(FieldsAre(QString("DummyDeviceDefaultScreen.qml"),
                    QString(""),
                    QFileInfo("/dummy/path/DummyDeviceDefaultScreen.qml"),
                    LegacyControllerMapping::ScriptFileInfo::Type::Qml,
                    false)));
    EXPECT_CALL(*mapping,
            addScreenInfo(FieldsAre(QString("main"),
                    QSize(480, 360),
                    20,
                    1,
                    std::chrono::milliseconds(2000),
                    QImage::Format_RGBA8888,
                    LegacyControllerMapping::ScreenInfo::ColorEndian::Little,
                    false,
                    false)));
    EXPECT_CALL(*mapping, addModule(QFileInfo("/dummy/path/foobar"), false));

    addScriptFilesToMapping(
            doc.documentElement(),
            mapping,
            QDir());
}

TEST_F(LegacyControllerMappingFileHandlerTest, screenMappingTargetFPS) {
    QDomDocument doc;
    doc.setContent(
            QByteArray(R"EOF(
            <controller id="DummyDevice">
                <screens>
                    <screen identifier="main" width="1" height="1" targetFps="20" />
                </screens>
            </controller>
            )EOF"));

    auto mapping = std::make_shared<MockLegacyControllerMapping>();
    // This file always gets added
    EXPECT_CALL(*mapping,
            addScriptFile(FieldsAre(QString("common-controller-scripts.js"),
                    QString(""),
                    _, // gmock seems unable to assert QFileInfo
                    LegacyControllerMapping::ScriptFileInfo::Type::Javascript,
                    true)));
    EXPECT_CALL(*mapping, addScreenInfo(FieldsAre(_, _, 20, _, _, _, _, _, _)));

    addScriptFilesToMapping(
            doc.documentElement(),
            mapping,
            QDir());
    ASSERT_ALL_EXPECTED_MSG();

    doc.setContent(
            QByteArray(R"EOF(
            <controller id="DummyDevice">
                <screens>
                    <screen identifier="main" width="1" height="1" targetFps="0" />
                </screens>
            </controller>
            )EOF"));

    mapping = std::make_shared<MockLegacyControllerMapping>();
    // This file always gets added
    EXPECT_CALL(*mapping,
            addScriptFile(FieldsAre(QString("common-controller-scripts.js"),
                    QString(""),
                    _, // gmock seems unable to assert QFileInfo
                    LegacyControllerMapping::ScriptFileInfo::Type::Javascript,
                    true)));
    EXPECT_CALL(*mapping, addScreenInfo(_)).Times(0);
    EXPECT_LOG_MSG(QtWarningMsg,
            QString("Invalid target FPS. Target FPS must be between 1 and %0")
                    .arg(kMaxTargetFps));

    addScriptFilesToMapping(
            doc.documentElement(),
            mapping,
            QDir());
    ASSERT_ALL_EXPECTED_MSG();

    doc.setContent(
            QByteArray(R"EOF(
            <controller id="DummyDevice">
                <screens>
                    <screen identifier="main" width="1" height="1" targetFps="-10" />
                </screens>
            </controller>
            )EOF"));

    mapping = std::make_shared<MockLegacyControllerMapping>();
    // This file always gets added
    EXPECT_CALL(*mapping,
            addScriptFile(FieldsAre(QString("common-controller-scripts.js"),
                    QString(""),
                    _, // gmock seems unable to assert QFileInfo
                    LegacyControllerMapping::ScriptFileInfo::Type::Javascript,
                    true)));
    EXPECT_LOG_MSG(QtWarningMsg,
            QString("Unable to parse the field \"targetFps\" as an unsigned "
                    "integer in the screen definition."));

    EXPECT_CALL(*mapping, addScreenInfo(_)).Times(0);
    EXPECT_LOG_MSG(
            QtWarningMsg,
            QString("Invalid target FPS. Target FPS must be between 1 and %0")
                    .arg(kMaxTargetFps));

    addScriptFilesToMapping(
            doc.documentElement(),
            mapping,
            QDir());
    ASSERT_ALL_EXPECTED_MSG();

    doc.setContent(
            QByteArray(R"EOF(
            <controller id="DummyDevice">
                <screens>
                    <screen identifier="main" width="1" height="1" targetFps="9000" />
                </screens>
            </controller>
            )EOF"));

    mapping = std::make_shared<MockLegacyControllerMapping>();
    // This file always gets added
    EXPECT_CALL(*mapping,
            addScriptFile(FieldsAre(QString("common-controller-scripts.js"),
                    QString(""),
                    _, // gmock seems unable to assert QFileInfo
                    LegacyControllerMapping::ScriptFileInfo::Type::Javascript,
                    true)));

    EXPECT_CALL(*mapping, addScreenInfo(_)).Times(0);
    EXPECT_LOG_MSG(
            QtWarningMsg,
            QString("Invalid target FPS. Target FPS must be between 1 and %0")
                    .arg(kMaxTargetFps));

    addScriptFilesToMapping(
            doc.documentElement(),
            mapping,
            QDir());
    ASSERT_ALL_EXPECTED_MSG();

    doc.setContent(
            QByteArray(R"EOF(
            <controller id="DummyDevice">
                <screens>
                    <screen identifier="main" width="1" height="1" targetFps="bar" />
                </screens>
            </controller>
            )EOF"));

    mapping = std::make_shared<MockLegacyControllerMapping>();
    // This file always gets added
    EXPECT_CALL(*mapping,
            addScriptFile(FieldsAre(QString("common-controller-scripts.js"),
                    QString(""),
                    _, // gmock seems unable to assert QFileInfo
                    LegacyControllerMapping::ScriptFileInfo::Type::Javascript,
                    true)));
    EXPECT_LOG_MSG(QtWarningMsg,
            QString("Unable to parse the field \"targetFps\" as an unsigned "
                    "integer in the screen definition."));

    EXPECT_CALL(*mapping, addScreenInfo(_)).Times(0);
    EXPECT_LOG_MSG(
            QtWarningMsg,
            QString("Invalid target FPS. Target FPS must be between 1 and %0")
                    .arg(kMaxTargetFps));

    addScriptFilesToMapping(
            doc.documentElement(),
            mapping,
            QDir());
    ASSERT_ALL_EXPECTED_MSG();
}

TEST_F(LegacyControllerMappingFileHandlerTest, screenMappingSize) {
    QDomDocument doc;
    doc.setContent(
            QByteArray(R"EOF(
            <controller id="DummyDevice">
                <screens>
                    <screen identifier="main" width="10" height="10" />
                </screens>
            </controller>
            )EOF"));

    auto mapping = std::make_shared<MockLegacyControllerMapping>();
    // This file always gets added
    EXPECT_CALL(*mapping,
            addScriptFile(FieldsAre(QString("common-controller-scripts.js"),
                    QString(""),
                    _, // gmock seems unable to assert QFileInfo
                    LegacyControllerMapping::ScriptFileInfo::Type::Javascript,
                    true)));
    EXPECT_CALL(*mapping, addScreenInfo(FieldsAre(_, QSize(10, 10), _, _, _, _, _, _, _)));

    addScriptFilesToMapping(
            doc.documentElement(),
            mapping,
            QDir());
    ASSERT_ALL_EXPECTED_MSG();

    doc.setContent(
            QByteArray(R"EOF(
            <controller id="DummyDevice">
                <screens>
                    <screen identifier="main" width="100" height="0" />
                </screens>
            </controller>
            )EOF"));

    mapping = std::make_shared<MockLegacyControllerMapping>();
    // This file always gets added
    EXPECT_CALL(*mapping,
            addScriptFile(FieldsAre(QString("common-controller-scripts.js"),
                    QString(""),
                    _, // gmock seems unable to assert QFileInfo
                    LegacyControllerMapping::ScriptFileInfo::Type::Javascript,
                    true)));
    EXPECT_CALL(*mapping, addScreenInfo(_)).Times(0);
    EXPECT_LOG_MSG(
            QtWarningMsg,
            "Invalid screen size. Screen size must have a width and height above 1 pixel");

    addScriptFilesToMapping(
            doc.documentElement(),
            mapping,
            QDir());
    ASSERT_ALL_EXPECTED_MSG();

    doc.setContent(
            QByteArray(R"EOF(
            <controller id="DummyDevice">
                <screens>
                    <screen identifier="main" width="100" height="-1" />
                </screens>
            </controller>
            )EOF"));

    mapping = std::make_shared<MockLegacyControllerMapping>();
    // This file always gets added
    EXPECT_CALL(*mapping,
            addScriptFile(FieldsAre(QString("common-controller-scripts.js"),
                    QString(""),
                    _, // gmock seems unable to assert QFileInfo
                    LegacyControllerMapping::ScriptFileInfo::Type::Javascript,
                    true)));
    EXPECT_LOG_MSG(QtWarningMsg,
            QString("Unable to parse the field \"height\" as an unsigned "
                    "integer in the screen definition."));
    EXPECT_CALL(*mapping, addScreenInfo(_)).Times(0);
    EXPECT_LOG_MSG(
            QtWarningMsg,
            "Invalid screen size. Screen size must have a width and height above 1 pixel");

    addScriptFilesToMapping(
            doc.documentElement(),
            mapping,
            QDir());
    ASSERT_ALL_EXPECTED_MSG();

    doc.setContent(
            QByteArray(R"EOF(
            <controller id="DummyDevice">
                <screens>
                    <screen identifier="main" width="1" height="foo" />
                </screens>
            </controller>
            )EOF"));

    mapping = std::make_shared<MockLegacyControllerMapping>();
    // This file always gets added
    EXPECT_CALL(*mapping,
            addScriptFile(FieldsAre(QString("common-controller-scripts.js"),
                    QString(""),
                    _, // gmock seems unable to assert QFileInfo
                    LegacyControllerMapping::ScriptFileInfo::Type::Javascript,
                    true)));
    EXPECT_LOG_MSG(QtWarningMsg,
            QString("Unable to parse the field \"height\" as an unsigned "
                    "integer in the screen definition."));
    EXPECT_CALL(*mapping, addScreenInfo(_)).Times(0);
    EXPECT_LOG_MSG(
            QtWarningMsg,
            "Invalid screen size. Screen size must have a width and height above 1 pixel");

    addScriptFilesToMapping(
            doc.documentElement(),
            mapping,
            QDir());
    ASSERT_ALL_EXPECTED_MSG();

    doc.setContent(
            QByteArray(R"EOF(
            <controller id="DummyDevice">
                <screens>
                    <screen identifier="main" width="1" />
                </screens>
            </controller>
            )EOF"));

    mapping = std::make_shared<MockLegacyControllerMapping>();
    // This file always gets added
    EXPECT_CALL(*mapping,
            addScriptFile(FieldsAre(QString("common-controller-scripts.js"),
                    QString(""),
                    _, // gmock seems unable to assert QFileInfo
                    LegacyControllerMapping::ScriptFileInfo::Type::Javascript,
                    true)));
    EXPECT_CALL(*mapping, addScreenInfo(_)).Times(0);
    EXPECT_LOG_MSG(
            QtWarningMsg,
            "Invalid screen size. Screen size must have a width and height above 1 pixel");

    addScriptFilesToMapping(
            doc.documentElement(),
            mapping,
            QDir());
    ASSERT_ALL_EXPECTED_MSG();
}

TEST_F(LegacyControllerMappingFileHandlerTest, screenMappingBitFormatDefinition) {
    // pixelType
    // endian

    // No pixel type default to RGB 8-bits depth
    QDomDocument doc;
    doc.setContent(
            QByteArray(R"EOF(
            <controller id="DummyDevice">
                <screens>
                    <screen identifier="main" width="10" height="10" />
                </screens>
            </controller>
            )EOF"));

    auto mapping = std::make_shared<MockLegacyControllerMapping>();
    // This file always gets added
    EXPECT_CALL(*mapping,
            addScriptFile(FieldsAre(QString("common-controller-scripts.js"),
                    QString(""),
                    _, // gmock seems unable to assert QFileInfo
                    LegacyControllerMapping::ScriptFileInfo::Type::Javascript,
                    true)));
    EXPECT_CALL(*mapping, addScreenInfo(FieldsAre(_, _, _, _, _, QImage::Format_RGB888, _, _, _)));

    addScriptFilesToMapping(
            doc.documentElement(),
            mapping,
            QDir());

    doc.setContent(
            QByteArray(R"EOF(
            <controller id="DummyDevice">
                <screens>
                    <screen identifier="main" width="10" height="10" pixelType="RGB565" />
                </screens>
            </controller>
            )EOF"));

    mapping = std::make_shared<MockLegacyControllerMapping>();
    // This file always gets added
    EXPECT_CALL(*mapping,
            addScriptFile(FieldsAre(QString("common-controller-scripts.js"),
                    QString(""),
                    _, // gmock seems unable to assert QFileInfo
                    LegacyControllerMapping::ScriptFileInfo::Type::Javascript,
                    true)));
    EXPECT_CALL(*mapping, addScreenInfo(FieldsAre(_, _, _, _, _, QImage::Format_RGB16, _, _, _)));

    addScriptFilesToMapping(
            doc.documentElement(),
            mapping,
            QDir());

    doc.setContent(
            QByteArray(R"EOF(
            <controller id="DummyDevice">
                <screens>
                    <screen identifier="main" width="10" height="10" pixelType="FOOBAR" />
                </screens>
            </controller>
            )EOF"));

    mapping = std::make_shared<MockLegacyControllerMapping>();
    // This file always gets added
    EXPECT_CALL(*mapping,
            addScriptFile(FieldsAre(QString("common-controller-scripts.js"),
                    QString(""),
                    _, // gmock seems unable to assert QFileInfo
                    LegacyControllerMapping::ScriptFileInfo::Type::Javascript,
                    true)));
    EXPECT_CALL(*mapping, addScreenInfo(_)).Times(0);
    EXPECT_LOG_MSG(
            QtWarningMsg,
            "Unsupported pixel format \"FOOBAR\"");

    addScriptFilesToMapping(
            doc.documentElement(),
            mapping,
            QDir());
    ASSERT_ALL_EXPECTED_MSG();

    doc.setContent(
            QByteArray(R"EOF(
            <controller id="DummyDevice">
                <screens>
                    <screen identifier="main" width="10" height="10" />
                </screens>
            </controller>
            )EOF"));

    mapping = std::make_shared<MockLegacyControllerMapping>();
    // This file always gets added
    EXPECT_CALL(*mapping,
            addScriptFile(FieldsAre(QString("common-controller-scripts.js"),
                    QString(""),
                    _, // gmock seems unable to assert QFileInfo
                    LegacyControllerMapping::ScriptFileInfo::Type::Javascript,
                    true)));
    EXPECT_CALL(*mapping,
            addScreenInfo(FieldsAre(_,
                    _,
                    _,
                    _,
                    _,
                    _,
                    LegacyControllerMapping::ScreenInfo::ColorEndian::Little,
                    _,
                    _)));

    addScriptFilesToMapping(
            doc.documentElement(),
            mapping,
            QDir());

    doc.setContent(
            QByteArray(R"EOF(
            <controller id="DummyDevice">
                <screens>
                    <screen identifier="main" width="10" height="10" endian="little" />
                </screens>
            </controller>
            )EOF"));

    mapping = std::make_shared<MockLegacyControllerMapping>();
    // This file always gets added
    EXPECT_CALL(*mapping,
            addScriptFile(FieldsAre(QString("common-controller-scripts.js"),
                    QString(""),
                    _, // gmock seems unable to assert QFileInfo
                    LegacyControllerMapping::ScriptFileInfo::Type::Javascript,
                    true)));
    EXPECT_CALL(*mapping,
            addScreenInfo(FieldsAre(_,
                    _,
                    _,
                    _,
                    _,
                    _,
                    LegacyControllerMapping::ScreenInfo::ColorEndian::Little,
                    _,
                    _)));

    addScriptFilesToMapping(
            doc.documentElement(),
            mapping,
            QDir());

    doc.setContent(
            QByteArray(R"EOF(
            <controller id="DummyDevice">
                <screens>
                    <screen identifier="main" width="10" height="10" endian="big" />
                </screens>
            </controller>
            )EOF"));

    mapping = std::make_shared<MockLegacyControllerMapping>();
    // This file always gets added
    EXPECT_CALL(*mapping,
            addScriptFile(FieldsAre(QString("common-controller-scripts.js"),
                    QString(""),
                    _, // gmock seems unable to assert QFileInfo
                    LegacyControllerMapping::ScriptFileInfo::Type::Javascript,
                    true)));
    EXPECT_CALL(*mapping,
            addScreenInfo(FieldsAre(_,
                    _,
                    _,
                    _,
                    _,
                    _,
                    LegacyControllerMapping::ScreenInfo::ColorEndian::Big,
                    _,
                    _)));

    addScriptFilesToMapping(
            doc.documentElement(),
            mapping,
            QDir());

    doc.setContent(
            QByteArray(R"EOF(
            <controller id="DummyDevice">
                <screens>
                    <screen identifier="main" width="10" height="10" endian="enormous" />
                </screens>
            </controller>
            )EOF"));

    mapping = std::make_shared<MockLegacyControllerMapping>();
    // This file always gets added
    EXPECT_CALL(*mapping,
            addScriptFile(FieldsAre(QString("common-controller-scripts.js"),
                    QString(""),
                    _, // gmock seems unable to assert QFileInfo
                    LegacyControllerMapping::ScriptFileInfo::Type::Javascript,
                    true)));
    EXPECT_CALL(*mapping, addScreenInfo(_)).Times(0);
    EXPECT_LOG_MSG(
            QtWarningMsg,
            "Unknown endian format \"enormous\"");

    addScriptFilesToMapping(
            doc.documentElement(),
            mapping,
            QDir());
    ASSERT_ALL_EXPECTED_MSG();
}

TEST_F(LegacyControllerMappingFileHandlerTest, screenMappingExtraBoolPropertiesDefinition) {
    bool kExpectedWarning[] = {false, false, true, true, true, true, true};
    QStringList kFalseValue = {"false", "FALse", "no", "yes", "1", "nope", "maybe"};
    QStringList kTrueValue = {"true", "trUe", "TRUE "};
    QDomDocument doc;
    std::shared_ptr<MockLegacyControllerMapping> mapping;

    // reversed
    bool* expectedWarning = &kExpectedWarning[0];
    for (const QString& falseValue : std::as_const(kFalseValue)) {
        doc.setContent(
                QString(R"EOF(
                <controller id="DummyDevice">
                        <screens>
                        <screen identifier="main" width="10" height="10" reversed="%0"/>
                        </screens>
                </controller>
                )EOF")
                        .arg(falseValue)
                        .toUtf8());

        mapping = std::make_shared<MockLegacyControllerMapping>();
        // This file always gets added
        EXPECT_CALL(*mapping,
                addScriptFile(FieldsAre(QString("common-controller-scripts.js"),
                        QString(""),
                        _, // gmock seems unable to assert QFileInfo
                        LegacyControllerMapping::ScriptFileInfo::Type::Javascript,
                        true)));
        if (expectedWarning++) {
            EXPECT_LOG_MSG(QtWarningMsg,
                    QString("Unable to parse the field \"reversed\" as a "
                            "boolean in the screen definition."));
        }
        EXPECT_CALL(*mapping, addScreenInfo(FieldsAre(_, _, _, _, _, _, _, false, _)));

        addScriptFilesToMapping(
                doc.documentElement(),
                mapping,
                QDir());
    }
    for (const QString& falseValue : std::as_const(kTrueValue)) {
        doc.setContent(
                QString(R"EOF(
                <controller id="DummyDevice">
                        <screens>
                        <screen identifier="main" width="10" height="10" reversed="%0"/>
                        </screens>
                </controller>
                )EOF")
                        .arg(falseValue)
                        .toUtf8());

        mapping = std::make_shared<MockLegacyControllerMapping>();
        // This file always gets added
        EXPECT_CALL(*mapping,
                addScriptFile(FieldsAre(QString("common-controller-scripts.js"),
                        QString(""),
                        _, // gmock seems unable to assert QFileInfo
                        LegacyControllerMapping::ScriptFileInfo::Type::Javascript,
                        true)));
        EXPECT_CALL(*mapping, addScreenInfo(FieldsAre(_, _, _, _, _, _, _, true, _)));

        addScriptFilesToMapping(
                doc.documentElement(),
                mapping,
                QDir());
    }
    // raw
    expectedWarning = &kExpectedWarning[0];
    for (const QString& falseValue : std::as_const(kFalseValue)) {
        doc.setContent(
                QString(R"EOF(
                <controller id="DummyDevice">
                        <screens>
                        <screen identifier="main" width="10" height="10" raw="%0"/>
                        </screens>
                </controller>
                )EOF")
                        .arg(falseValue)
                        .toUtf8());

        mapping = std::make_shared<MockLegacyControllerMapping>();
        // This file always gets added
        EXPECT_CALL(*mapping,
                addScriptFile(FieldsAre(QString("common-controller-scripts.js"),
                        QString(""),
                        _, // gmock seems unable to assert QFileInfo
                        LegacyControllerMapping::ScriptFileInfo::Type::Javascript,
                        true)));
        EXPECT_CALL(*mapping, addScreenInfo(FieldsAre(_, _, _, _, _, _, _, _, false)));
        if (expectedWarning++) {
            EXPECT_LOG_MSG(QtWarningMsg,
                    QString("Unable to parse the field \"raw\" as a boolean in "
                            "the screen definition."));
        }

        addScriptFilesToMapping(
                doc.documentElement(),
                mapping,
                QDir());
    }
    for (const QString& falseValue : std::as_const(kTrueValue)) {
        doc.setContent(
                QString(R"EOF(
                <controller id="DummyDevice">
                        <screens>
                        <screen identifier="main" width="10" height="10" raw="%0"/>
                        </screens>
                </controller>
                )EOF")
                        .arg(falseValue)
                        .toUtf8());

        mapping = std::make_shared<MockLegacyControllerMapping>();
        // This file always gets added
        EXPECT_CALL(*mapping,
                addScriptFile(FieldsAre(QString("common-controller-scripts.js"),
                        QString(""),
                        _, // gmock seems unable to assert QFileInfo
                        LegacyControllerMapping::ScriptFileInfo::Type::Javascript,
                        true)));
        EXPECT_CALL(*mapping, addScreenInfo(FieldsAre(_, _, _, _, _, _, _, _, true)));

        addScriptFilesToMapping(
                doc.documentElement(),
                mapping,
                QDir());
    }
}

TEST_F(LegacyControllerMappingFileHandlerTest, screenMappingExtraIntPropertiesDefinition) {
    // splashoff
    QDomDocument doc;

    doc.setContent(
            QByteArray(R"EOF(
                <controller id="DummyDevice">
                        <screens>
                        <screen identifier="main" width="10" height="10"/>
                        </screens>
                </controller>
                )EOF"));

    auto mapping = std::make_shared<MockLegacyControllerMapping>();
    // This file always gets added
    EXPECT_CALL(*mapping,
            addScriptFile(FieldsAre(QString("common-controller-scripts.js"),
                    QString(""),
                    _, // gmock seems unable to assert QFileInfo
                    LegacyControllerMapping::ScriptFileInfo::Type::Javascript,
                    true)));
    EXPECT_CALL(*mapping,
            addScreenInfo(FieldsAre(
                    _, _, _, _, std::chrono::milliseconds(0), _, _, _, _)));

    addScriptFilesToMapping(
            doc.documentElement(),
            mapping,
            QDir());

    doc.setContent(
            QByteArray(R"EOF(
                <controller id="DummyDevice">
                        <screens>
                        <screen identifier="main" width="10" height="10" splashoff="500"/>
                        </screens>
                </controller>
                )EOF"));

    mapping = std::make_shared<MockLegacyControllerMapping>();
    // This file always gets added
    EXPECT_CALL(*mapping,
            addScriptFile(FieldsAre(QString("common-controller-scripts.js"),
                    QString(""),
                    _, // gmock seems unable to assert QFileInfo
                    LegacyControllerMapping::ScriptFileInfo::Type::Javascript,
                    true)));
    EXPECT_CALL(*mapping,
            addScreenInfo(FieldsAre(
                    _, _, _, _, std::chrono::milliseconds(500), _, _, _, _)));

    addScriptFilesToMapping(
            doc.documentElement(),
            mapping,
            QDir());

    doc.setContent(
            QByteArray(R"EOF(
                <controller id="DummyDevice">
                        <screens>
                        <screen identifier="main" width="10" height="10" splashoff="500000"/>
                        </screens>
                </controller>
                )EOF"));

    mapping = std::make_shared<MockLegacyControllerMapping>();
    // This file always gets added
    EXPECT_CALL(*mapping,
            addScriptFile(FieldsAre(QString("common-controller-scripts.js"),
                    QString(""),
                    _, // gmock seems unable to assert QFileInfo
                    LegacyControllerMapping::ScriptFileInfo::Type::Javascript,
                    true)));
    EXPECT_CALL(*mapping,
            addScreenInfo(FieldsAre(_,
                    _,
                    _,
                    _,
                    std::chrono::milliseconds(kMaxSplashOffDuration),
                    _,
                    _,
                    _,
                    _)));
    EXPECT_LOG_MSG(
            QtWarningMsg,
            QString("Invalid splashoff duration. Splashoff duration must "
                    "be between 0 and %0. Clamping to %1")
                    .arg(kMaxSplashOffDuration)
                    .arg(kMaxSplashOffDuration));

    addScriptFilesToMapping(
            doc.documentElement(),
            mapping,
            QDir());
    ASSERT_ALL_EXPECTED_MSG();

    doc.setContent(
            QByteArray(R"EOF(
                <controller id="DummyDevice">
                        <screens>
                        <screen identifier="main" width="10" height="10" splashoff="-1"/>
                        </screens>
                </controller>
                )EOF"));

    mapping = std::make_shared<MockLegacyControllerMapping>();
    // This file always gets added
    EXPECT_CALL(*mapping,
            addScriptFile(FieldsAre(QString("common-controller-scripts.js"),
                    QString(""),
                    _, // gmock seems unable to assert QFileInfo
                    LegacyControllerMapping::ScriptFileInfo::Type::Javascript,
                    true)));
    EXPECT_LOG_MSG(QtWarningMsg,
            QString("Unable to parse the field \"splashoff\" as an unsigned "
                    "integer in the screen definition."));
    EXPECT_CALL(*mapping,
            addScreenInfo(FieldsAre(
                    _, _, _, _, std::chrono::milliseconds(0), _, _, _, _)));

    addScriptFilesToMapping(
            doc.documentElement(),
            mapping,
            QDir());

    doc.setContent(
            QByteArray(R"EOF(
                <controller id="DummyDevice">
                        <screens>
                        <screen identifier="main" width="10" height="10" splashoff="foobar"/>
                        </screens>
                </controller>
                )EOF"));

    mapping = std::make_shared<MockLegacyControllerMapping>();
    // This file always gets added
    EXPECT_CALL(*mapping,
            addScriptFile(FieldsAre(QString("common-controller-scripts.js"),
                    QString(""),
                    _, // gmock seems unable to assert QFileInfo
                    LegacyControllerMapping::ScriptFileInfo::Type::Javascript,
                    true)));
    EXPECT_LOG_MSG(QtWarningMsg,
            QString("Unable to parse the field \"splashoff\" as an unsigned "
                    "integer in the screen definition."));
    EXPECT_CALL(*mapping,
            addScreenInfo(FieldsAre(
                    _, _, _, _, std::chrono::milliseconds(0), _, _, _, _)));

    addScriptFilesToMapping(
            doc.documentElement(),
            mapping,
            QDir());
}

TEST_F(LegacyControllerMappingFileHandlerTest, canParseHybridMapping) {
    QDomDocument doc;

    doc.setContent(QByteArray(R"EOF(
            <controller id="DummyDevice">
                <screens>
                    <screen identifier="main" width="480" height="360" targetFps="20" pixelType="RBGA" splashoff="2000" />
                </screens>
                <scriptfiles>
                    <file filename="DummyDeviceDefaultScreen.qml" />
                    <file filename="LegacyScript.js" />
                </scriptfiles>
                <qmllibraries>
                    <library path="foobar/" />
                </qmllibraries>
            </controller>
            )EOF"));

    auto mapping = std::make_shared<MockLegacyControllerMapping>();
    // This file always gets added
    EXPECT_CALL(*mapping,
            addScriptFile(FieldsAre(QString("common-controller-scripts.js"),
                    QString(""),
                    _, // gmock seems unable to assert QFileInfo
                    LegacyControllerMapping::ScriptFileInfo::Type::Javascript,
                    true)));

    EXPECT_CALL(*mapping,
            addScriptFile(FieldsAre(QString("DummyDeviceDefaultScreen.qml"),
                    QString(""),
                    QFileInfo("/dummy/path/DummyDeviceDefaultScreen.qml"),
                    LegacyControllerMapping::ScriptFileInfo::Type::Qml,
                    false)));
    EXPECT_CALL(*mapping,
            addScriptFile(FieldsAre(QString("LegacyScript.js"),
                    QString(""),
                    QFileInfo("/dummy/path/LegacyScript.js"),
                    LegacyControllerMapping::ScriptFileInfo::Type::Javascript,
                    false)));
    EXPECT_CALL(*mapping,
            addScreenInfo(FieldsAre(QString("main"),
                    QSize(480, 360),
                    20,
                    1,
                    std::chrono::milliseconds(2000),
                    QImage::Format_RGBA8888,
                    LegacyControllerMapping::ScreenInfo::ColorEndian::Little,
                    false,
                    false)));
    EXPECT_CALL(*mapping, addModule(QFileInfo("/dummy/path/foobar"), false));

    addScriptFilesToMapping(
            doc.documentElement(),
            mapping,
            QDir());
}
