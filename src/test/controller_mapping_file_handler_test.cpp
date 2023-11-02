#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <QDomDocument>
#include <QTest>

#include "controllers/legacycontrollermappingfilehandler.h"
#include "helpers/log_test.h"
#include "test/mixxxtest.h"

using ::testing::_;

class LegacyControllerMappingFileHandlerTest
        : public LegacyControllerMappingFileHandler,
          public MixxxTest {
  public:
    void SetUp() override {
        mixxx::Time::setTestMode(true);
        mixxx::Time::setTestElapsedTime(mixxx::Duration::fromMillis(10));
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
            (const QString& name,
                    const QString& identifier,
                    const QFileInfo& file,
                    ScriptFileInfo::Type type,
                    bool builtin),
            (override));
    MOCK_METHOD(void,
            addScreenInfo,
            (const QString& identifier,
                    const QSize& size,
                    uint targetFps,
                    uint splashoff,
                    QImage::Format pixelFormat,
                    std::endian endian,
                    bool reversedColorse,
                    bool rawData),
            (override));
    MOCK_METHOD(void, addLibraryDirectory, (const QFileInfo& dirinfo, bool builtin), (override));

    std::shared_ptr<LegacyControllerMapping> clone() const override {
        throw std::runtime_error("not implemented");
    }
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
            addScriptFile(QString(REQUIRED_SCRIPT_FILE),
                    QString(""),
                    _, // gmock seems unable to assert QFileInfo
                    LegacyControllerMapping::ScriptFileInfo::Type::JAVASCRIPT,
                    true));

    EXPECT_CALL(*mapping,
            addScriptFile(QString("DummyDeviceDefaultScreen.js"),
                    QString(""),
                    _, // gmock seems unable to assert QFileInfo
                    LegacyControllerMapping::ScriptFileInfo::Type::JAVASCRIPT,
                    false));
    EXPECT_CALL(*mapping, addScreenInfo(_, _, _, _, _, _, _, _)).Times(0);
    EXPECT_CALL(*mapping, addLibraryDirectory(_, _)).Times(0);

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
            addScriptFile(QString(REQUIRED_SCRIPT_FILE),
                    QString(""),
                    _, // gmock seems unable to assert QFileInfo
                    LegacyControllerMapping::ScriptFileInfo::Type::JAVASCRIPT,
                    true));

    EXPECT_CALL(*mapping,
            addScriptFile(QString("DummyDeviceDefaultScreen.qml"),
                    QString(""),
                    QFileInfo("/dummy/path/DummyDeviceDefaultScreen.qml"),
                    LegacyControllerMapping::ScriptFileInfo::Type::QML,
                    false));
    EXPECT_CALL(*mapping,
            addScreenInfo(QString("main"),
                    QSize(480, 360),
                    20,
                    2000,
                    QImage::Format_RGBA8888,
                    std::endian::little,
                    false,
                    false));
    EXPECT_CALL(*mapping, addLibraryDirectory(QFileInfo("/dummy/path/foobar"), false));

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
            addScriptFile(QString(REQUIRED_SCRIPT_FILE),
                    QString(""),
                    _, // gmock seems unable to assert QFileInfo
                    LegacyControllerMapping::ScriptFileInfo::Type::JAVASCRIPT,
                    true));
    EXPECT_CALL(*mapping, addScreenInfo(_, _, 20, _, _, _, _, _));

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
            addScriptFile(QString(REQUIRED_SCRIPT_FILE),
                    QString(""),
                    _, // gmock seems unable to assert QFileInfo
                    LegacyControllerMapping::ScriptFileInfo::Type::JAVASCRIPT,
                    true));
    EXPECT_CALL(*mapping, addScreenInfo(_, _, _, _, _, _, _, _)).Times(0);
    EXPECT_LOG_MSG(QtWarningMsg,
            QString("Invalid target FPS. Target FPS must be between 1 and %0")
                    .arg(MAX_TARGET_FPS));

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
            addScriptFile(QString(REQUIRED_SCRIPT_FILE),
                    QString(""),
                    _, // gmock seems unable to assert QFileInfo
                    LegacyControllerMapping::ScriptFileInfo::Type::JAVASCRIPT,
                    true));

    EXPECT_CALL(*mapping, addScreenInfo(_, _, _, _, _, _, _, _)).Times(0);
    EXPECT_LOG_MSG(
            QtWarningMsg,
            QString("Invalid target FPS. Target FPS must be between 1 and %0")
                    .arg(MAX_TARGET_FPS));

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
            addScriptFile(QString(REQUIRED_SCRIPT_FILE),
                    QString(""),
                    _, // gmock seems unable to assert QFileInfo
                    LegacyControllerMapping::ScriptFileInfo::Type::JAVASCRIPT,
                    true));

    EXPECT_CALL(*mapping, addScreenInfo(_, _, _, _, _, _, _, _)).Times(0);
    EXPECT_LOG_MSG(
            QtWarningMsg,
            QString("Invalid target FPS. Target FPS must be between 1 and %0")
                    .arg(MAX_TARGET_FPS));

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
            addScriptFile(QString(REQUIRED_SCRIPT_FILE),
                    QString(""),
                    _, // gmock seems unable to assert QFileInfo
                    LegacyControllerMapping::ScriptFileInfo::Type::JAVASCRIPT,
                    true));

    EXPECT_CALL(*mapping, addScreenInfo(_, _, _, _, _, _, _, _)).Times(0);
    EXPECT_LOG_MSG(
            QtWarningMsg,
            QString("Invalid target FPS. Target FPS must be between 1 and %0")
                    .arg(MAX_TARGET_FPS));

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
            addScriptFile(QString(REQUIRED_SCRIPT_FILE),
                    QString(""),
                    _, // gmock seems unable to assert QFileInfo
                    LegacyControllerMapping::ScriptFileInfo::Type::JAVASCRIPT,
                    true));
    EXPECT_CALL(*mapping, addScreenInfo(_, QSize(10, 10), _, _, _, _, _, _));

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
            addScriptFile(QString(REQUIRED_SCRIPT_FILE),
                    QString(""),
                    _, // gmock seems unable to assert QFileInfo
                    LegacyControllerMapping::ScriptFileInfo::Type::JAVASCRIPT,
                    true));
    EXPECT_CALL(*mapping, addScreenInfo(_, _, _, _, _, _, _, _)).Times(0);
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
            addScriptFile(QString(REQUIRED_SCRIPT_FILE),
                    QString(""),
                    _, // gmock seems unable to assert QFileInfo
                    LegacyControllerMapping::ScriptFileInfo::Type::JAVASCRIPT,
                    true));
    EXPECT_CALL(*mapping, addScreenInfo(_, _, _, _, _, _, _, _)).Times(0);
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
            addScriptFile(QString(REQUIRED_SCRIPT_FILE),
                    QString(""),
                    _, // gmock seems unable to assert QFileInfo
                    LegacyControllerMapping::ScriptFileInfo::Type::JAVASCRIPT,
                    true));
    EXPECT_CALL(*mapping, addScreenInfo(_, _, _, _, _, _, _, _)).Times(0);
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
            addScriptFile(QString(REQUIRED_SCRIPT_FILE),
                    QString(""),
                    _, // gmock seems unable to assert QFileInfo
                    LegacyControllerMapping::ScriptFileInfo::Type::JAVASCRIPT,
                    true));
    EXPECT_CALL(*mapping, addScreenInfo(_, _, _, _, _, _, _, _)).Times(0);
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
            addScriptFile(QString(REQUIRED_SCRIPT_FILE),
                    QString(""),
                    _, // gmock seems unable to assert QFileInfo
                    LegacyControllerMapping::ScriptFileInfo::Type::JAVASCRIPT,
                    true));
    EXPECT_CALL(*mapping, addScreenInfo(_, _, _, _, QImage::Format_RGB888, _, _, _));

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
            addScriptFile(QString(REQUIRED_SCRIPT_FILE),
                    QString(""),
                    _, // gmock seems unable to assert QFileInfo
                    LegacyControllerMapping::ScriptFileInfo::Type::JAVASCRIPT,
                    true));
    EXPECT_CALL(*mapping, addScreenInfo(_, _, _, _, QImage::Format_RGB16, _, _, _));

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
            addScriptFile(QString(REQUIRED_SCRIPT_FILE),
                    QString(""),
                    _, // gmock seems unable to assert QFileInfo
                    LegacyControllerMapping::ScriptFileInfo::Type::JAVASCRIPT,
                    true));
    EXPECT_CALL(*mapping, addScreenInfo(_, _, _, _, _, _, _, _)).Times(0);
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
            addScriptFile(QString(REQUIRED_SCRIPT_FILE),
                    QString(""),
                    _, // gmock seems unable to assert QFileInfo
                    LegacyControllerMapping::ScriptFileInfo::Type::JAVASCRIPT,
                    true));
    EXPECT_CALL(*mapping, addScreenInfo(_, _, _, _, _, std::endian::little, _, _));

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
            addScriptFile(QString(REQUIRED_SCRIPT_FILE),
                    QString(""),
                    _, // gmock seems unable to assert QFileInfo
                    LegacyControllerMapping::ScriptFileInfo::Type::JAVASCRIPT,
                    true));
    EXPECT_CALL(*mapping, addScreenInfo(_, _, _, _, _, std::endian::little, _, _));

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
            addScriptFile(QString(REQUIRED_SCRIPT_FILE),
                    QString(""),
                    _, // gmock seems unable to assert QFileInfo
                    LegacyControllerMapping::ScriptFileInfo::Type::JAVASCRIPT,
                    true));
    EXPECT_CALL(*mapping, addScreenInfo(_, _, _, _, _, std::endian::big, _, _));

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
            addScriptFile(QString(REQUIRED_SCRIPT_FILE),
                    QString(""),
                    _, // gmock seems unable to assert QFileInfo
                    LegacyControllerMapping::ScriptFileInfo::Type::JAVASCRIPT,
                    true));
    EXPECT_CALL(*mapping, addScreenInfo(_, _, _, _, _, _, _, _)).Times(0);
    EXPECT_LOG_MSG(
            QtWarningMsg,
            "Unknown endiant format \"enormous\"");

    addScriptFilesToMapping(
            doc.documentElement(),
            mapping,
            QDir());
    ASSERT_ALL_EXPECTED_MSG();
}

TEST_F(LegacyControllerMappingFileHandlerTest, screenMappingExtraBoolPropertiesDefinition) {
    QStringList kFalseValue = {"false", "FALse", "no", "nope", "maybe"};
    QStringList kTrueValue = {"true", "trUe", "1", "yes"};
    QDomDocument doc;
    std::shared_ptr<MockLegacyControllerMapping> mapping;

    // reversed
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
                addScriptFile(QString(REQUIRED_SCRIPT_FILE),
                        QString(""),
                        _, // gmock seems unable to assert QFileInfo
                        LegacyControllerMapping::ScriptFileInfo::Type::JAVASCRIPT,
                        true));
        EXPECT_CALL(*mapping, addScreenInfo(_, _, _, _, _, _, false, _));

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
                addScriptFile(QString(REQUIRED_SCRIPT_FILE),
                        QString(""),
                        _, // gmock seems unable to assert QFileInfo
                        LegacyControllerMapping::ScriptFileInfo::Type::JAVASCRIPT,
                        true));
        EXPECT_CALL(*mapping, addScreenInfo(_, _, _, _, _, _, true, _));

        addScriptFilesToMapping(
                doc.documentElement(),
                mapping,
                QDir());
    }
    // raw
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
                addScriptFile(QString(REQUIRED_SCRIPT_FILE),
                        QString(""),
                        _, // gmock seems unable to assert QFileInfo
                        LegacyControllerMapping::ScriptFileInfo::Type::JAVASCRIPT,
                        true));
        EXPECT_CALL(*mapping, addScreenInfo(_, _, _, _, _, _, _, false));

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
                addScriptFile(QString(REQUIRED_SCRIPT_FILE),
                        QString(""),
                        _, // gmock seems unable to assert QFileInfo
                        LegacyControllerMapping::ScriptFileInfo::Type::JAVASCRIPT,
                        true));
        EXPECT_CALL(*mapping, addScreenInfo(_, _, _, _, _, _, _, true));

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
            addScriptFile(QString(REQUIRED_SCRIPT_FILE),
                    QString(""),
                    _, // gmock seems unable to assert QFileInfo
                    LegacyControllerMapping::ScriptFileInfo::Type::JAVASCRIPT,
                    true));
    EXPECT_CALL(*mapping, addScreenInfo(_, _, _, 0, _, _, _, _));

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
            addScriptFile(QString(REQUIRED_SCRIPT_FILE),
                    QString(""),
                    _, // gmock seems unable to assert QFileInfo
                    LegacyControllerMapping::ScriptFileInfo::Type::JAVASCRIPT,
                    true));
    EXPECT_CALL(*mapping, addScreenInfo(_, _, _, 500, _, _, _, _));

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
            addScriptFile(QString(REQUIRED_SCRIPT_FILE),
                    QString(""),
                    _, // gmock seems unable to assert QFileInfo
                    LegacyControllerMapping::ScriptFileInfo::Type::JAVASCRIPT,
                    true));
    EXPECT_CALL(*mapping, addScreenInfo(_, _, _, MAX_SPLASHOFF_DURATION, _, _, _, _));
    EXPECT_LOG_MSG(
            QtWarningMsg,
            QString("Invalid splashoff duration. Splashoff duration must "
                    "be between 0 and %0. Clamping to %1")
                    .arg(MAX_SPLASHOFF_DURATION)
                    .arg(MAX_SPLASHOFF_DURATION));

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
            addScriptFile(QString(REQUIRED_SCRIPT_FILE),
                    QString(""),
                    _, // gmock seems unable to assert QFileInfo
                    LegacyControllerMapping::ScriptFileInfo::Type::JAVASCRIPT,
                    true));
    EXPECT_CALL(*mapping, addScreenInfo(_, _, _, 0, _, _, _, _));

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
            addScriptFile(QString(REQUIRED_SCRIPT_FILE),
                    QString(""),
                    _, // gmock seems unable to assert QFileInfo
                    LegacyControllerMapping::ScriptFileInfo::Type::JAVASCRIPT,
                    true));
    EXPECT_CALL(*mapping, addScreenInfo(_, _, _, 0, _, _, _, _));

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
            addScriptFile(QString(REQUIRED_SCRIPT_FILE),
                    QString(""),
                    _, // gmock seems unable to assert QFileInfo
                    LegacyControllerMapping::ScriptFileInfo::Type::JAVASCRIPT,
                    true));

    EXPECT_CALL(*mapping,
            addScriptFile(QString("DummyDeviceDefaultScreen.qml"),
                    QString(""),
                    QFileInfo("/dummy/path/DummyDeviceDefaultScreen.qml"),
                    LegacyControllerMapping::ScriptFileInfo::Type::QML,
                    false));
    EXPECT_CALL(*mapping,
            addScriptFile(QString("LegacyScript.js"),
                    QString(""),
                    QFileInfo("/dummy/path/LegacyScript.js"),
                    LegacyControllerMapping::ScriptFileInfo::Type::JAVASCRIPT,
                    false));
    EXPECT_CALL(*mapping,
            addScreenInfo(QString("main"),
                    QSize(480, 360),
                    20,
                    2000,
                    QImage::Format_RGBA8888,
                    std::endian::little,
                    false,
                    false));
    EXPECT_CALL(*mapping, addLibraryDirectory(QFileInfo("/dummy/path/foobar"), false));

    addScriptFilesToMapping(
            doc.documentElement(),
            mapping,
            QDir());
}
