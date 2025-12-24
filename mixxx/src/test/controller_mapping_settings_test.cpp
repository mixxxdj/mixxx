
#include <gtest/gtest.h>

#include <QDomDocument>

#include "controllers/legacycontrollermapping.h"
#include "controllers/legacycontrollermappingfilehandler.h"
#include "controllers/legacycontrollersettings.h"
#include "test/mixxxtest.h"

class LegacyControllerMappingSettingsTest : public MixxxTest {
};

const char* const kValidBoolean =
        "<option variable=\"myToggle1\" type=\"boolean\" default=\"%1\" "
        "label=\"Test label\"><description>Test description</description>"
        "</option>";

const char* const kValidInteger =
        "<option variable=\"myInteger1\" type=\"integer\" default=\"%1\" "
        "min=\"%2\" max=\"%3\" step=\"%4\" label=\"Test label\"><description>Test description<"
        "/description></option>";

// This setting has purposfully no custom "label" and description
const char* const kValidDouble =
        "<option variable=\"myReal1\" type=\"real\" default=\"%1\" min=\"%"
        "2\" max=\"%3\" step=\"%4\" precision=\"2\"/>";

const char* const kValidEnum =
        " <option variable=\"myEnum1\" type=\"enum\" label=\"Test "
        "label\">%3<description>Test description</description></option>";
const char* const kValidEnumOption = "<value label=\"%1\">%2</value>";

TEST_F(LegacyControllerMappingSettingsTest, booleanSettingParsing) {
    QDomDocument doc;
    {
        doc.setContent(QString(kValidBoolean).arg("false").toLatin1());

        EXPECT_TRUE(LegacyControllerBooleanSetting::match(doc.documentElement()));
        LegacyControllerBooleanSetting setting(doc.documentElement());
        EXPECT_TRUE(setting.valid()) << "Unable to create a boolean setting";

        EXPECT_EQ(setting.variableName(), "myToggle1");
        EXPECT_EQ(setting.label(), "Test label");
        EXPECT_EQ(setting.description(), "Test description");

        EXPECT_FALSE(setting.isDirty());
        EXPECT_TRUE(setting.isDefault());
        EXPECT_EQ(setting.stringify(), "false");
        EXPECT_TRUE(setting.valid());
    }

    {
        doc.setContent(QString(kValidBoolean).arg("true").toLatin1());

        LegacyControllerBooleanSetting setting(doc.documentElement());
        EXPECT_TRUE(setting.valid()) << "Unable to create a boolean setting";

        EXPECT_EQ(setting.stringify(), "true");
    }
}

TEST_F(LegacyControllerMappingSettingsTest, booleanSettingEditing) {
    QDomDocument doc;
    doc.setContent(
            QByteArray("<option variable=\"myToggle1\" type=\"boolean\" "
                       "default=\"false\" label=\"Test label\"/>"));

    LegacyControllerBooleanSetting setting(doc.documentElement());
    EXPECT_TRUE(setting.valid()) << "Unable to create a boolean setting";

    setting.m_editedValue = true;
    EXPECT_TRUE(setting.isDirty());
    setting.save();
    EXPECT_FALSE(setting.isDirty());
    EXPECT_EQ(setting.m_savedValue, true);

    bool ok;
    setting.parse("true", &ok);
    EXPECT_TRUE(ok);
    EXPECT_FALSE(setting.isDirty());
    EXPECT_EQ(setting.stringify(), "true");
    EXPECT_TRUE(setting.value().isBool());
    EXPECT_TRUE(setting.value().toBool());
    EXPECT_FALSE(setting.isDefault());
    setting.parse("1", &ok);
    EXPECT_TRUE(ok);
    setting.parse("0", &ok);
    EXPECT_TRUE(ok);
    EXPECT_FALSE(setting.isDirty());
    EXPECT_EQ(setting.stringify(), "false");
    setting.parse("TRUE", &ok);
    EXPECT_TRUE(ok);
    EXPECT_EQ(setting.stringify(), "true");
    EXPECT_TRUE(setting.value().isBool());
    EXPECT_TRUE(setting.value().toBool());
    setting.reset();
    EXPECT_TRUE(setting.isDirty());
    setting.save();
    EXPECT_EQ(setting.stringify(), "false");
    EXPECT_TRUE(setting.isDefault());

    EXPECT_TRUE(setting.value().isBool());
    EXPECT_FALSE(setting.value().toBool());
}

TEST_F(LegacyControllerMappingSettingsTest, integerSettingParsing) {
    QDomDocument doc;
    {
        doc.setContent(QString(kValidInteger).arg("42", "1", "99", "1").toLatin1());

        EXPECT_TRUE(LegacyControllerIntegerSetting::match(doc.documentElement()));
        LegacyControllerIntegerSetting setting(doc.documentElement());
        EXPECT_TRUE(setting.valid()) << "Unable to create an integer setting";

        EXPECT_EQ(setting.variableName(), "myInteger1");
        EXPECT_EQ(setting.label(), "Test label");
        EXPECT_EQ(setting.description(), "Test description");

        EXPECT_FALSE(setting.isDirty());
        EXPECT_TRUE(setting.isDefault());
        EXPECT_EQ(setting.stringify(), "42");
        EXPECT_TRUE(setting.valid());
    }
    {
        doc.setContent(QString(kValidInteger).arg("18", "1", "99", "1").toLatin1());
        LegacyControllerIntegerSetting setting(doc.documentElement());
        EXPECT_TRUE(setting.valid()) << "Unable to create an integer setting";
        EXPECT_EQ(setting.stringify(), "18");
        EXPECT_TRUE(setting.valid());
    }
    {
        // Invalid if default is out of range
        doc.setContent(QString(kValidInteger).arg("10", "20", "30", "1").toLatin1());
        LegacyControllerIntegerSetting setting(doc.documentElement());
        EXPECT_EQ(setting.stringify(), "10");
        EXPECT_FALSE(setting.valid());
    }
    {
        doc.setContent(QString(kValidInteger).arg("10", "1", "5", "1").toLatin1());
        LegacyControllerIntegerSetting setting(doc.documentElement());
        EXPECT_EQ(setting.stringify(), "10");
        EXPECT_FALSE(setting.valid());
    }
    {
        // Invalid if step is zero
        doc.setContent(QString(kValidInteger).arg("10", "0", "30", "0").toLatin1());
        LegacyControllerIntegerSetting setting(doc.documentElement());
        EXPECT_EQ(setting.stringify(), "10");
        EXPECT_FALSE(setting.valid());
    }
}

TEST_F(LegacyControllerMappingSettingsTest, integerSettingEditing) {
    QDomDocument doc;
    doc.setContent(
            QByteArray("<option variable=\"myInteger1\" type=\"integer\" label=\"Test label\"/>"));

    LegacyControllerIntegerSetting setting(doc.documentElement());
    EXPECT_TRUE(setting.valid()) << "Unable to create an integer setting";

    setting.m_editedValue = true;
    EXPECT_TRUE(setting.isDirty());
    setting.save();
    EXPECT_FALSE(setting.isDirty());
    EXPECT_EQ(setting.m_savedValue, true);

    bool ok;
    setting.parse("42", &ok);
    EXPECT_TRUE(ok);
    EXPECT_FALSE(setting.isDirty());
    EXPECT_EQ(setting.stringify(), "42");
    EXPECT_TRUE(setting.value().isNumber());
    EXPECT_EQ(setting.value().toInt(), 42);
    EXPECT_FALSE(setting.isDefault());
    setting.parse("-15", &ok);
    EXPECT_TRUE(ok);
    setting.parse("0", &ok);
    EXPECT_TRUE(ok);
    EXPECT_FALSE(setting.isDirty());
    EXPECT_EQ(setting.stringify(), "0");
    setting.parse("30 ", &ok);
    EXPECT_TRUE(ok);
    EXPECT_EQ(setting.stringify(), "30");
    EXPECT_TRUE(setting.value().isNumber());
    EXPECT_EQ(setting.value().toInt(), 30);
    setting.reset();
    EXPECT_TRUE(setting.isDirty());
    setting.save();
    EXPECT_EQ(setting.stringify(), "0");
    EXPECT_TRUE(setting.isDefault());
    setting.parse("abc ", &ok);
    EXPECT_FALSE(ok);
    EXPECT_TRUE(setting.value().isNumber());
    EXPECT_EQ(setting.value().toInt(), 0);
}

TEST_F(LegacyControllerMappingSettingsTest, doubleSettingParsing) {
    QDomDocument doc;
    {
        doc.setContent(QString(kValidDouble).arg("42", "1", "99", "1").toLatin1());

        EXPECT_TRUE(LegacyControllerRealSetting::match(doc.documentElement()));
        LegacyControllerIntegerSetting setting(doc.documentElement());
        EXPECT_TRUE(setting.valid()) << "Unable to create a real setting";

        EXPECT_EQ(setting.variableName(), "myReal1");
        EXPECT_EQ(setting.label(), "myReal1");
        EXPECT_TRUE(setting.description().isEmpty());

        EXPECT_FALSE(setting.isDirty());
        EXPECT_TRUE(setting.isDefault());
        EXPECT_EQ(setting.stringify(), "42");
        EXPECT_TRUE(setting.valid());
    }
    {
        doc.setContent(QString(kValidInteger).arg("18", "1", "99", "1").toLatin1());
        LegacyControllerIntegerSetting setting(doc.documentElement());
        EXPECT_EQ(setting.stringify(), "18");
        EXPECT_TRUE(setting.valid());
    }
    {
        // Invalid if default is out of range
        doc.setContent(QString(kValidInteger).arg("10", "20", "30", "1").toLatin1());
        LegacyControllerIntegerSetting setting(doc.documentElement());
        EXPECT_EQ(setting.stringify(), "10");
        EXPECT_FALSE(setting.valid());
    }
    {
        doc.setContent(QString(kValidInteger).arg("10", "1", "5", "1").toLatin1());
        LegacyControllerIntegerSetting setting(doc.documentElement());
        EXPECT_EQ(setting.stringify(), "10");
        EXPECT_FALSE(setting.valid());
    }
    {
        // Invalid if step is zero
        doc.setContent(QString(kValidInteger).arg("10", "0", "30", "0").toLatin1());
        LegacyControllerIntegerSetting setting(doc.documentElement());
        EXPECT_EQ(setting.stringify(), "10");
        EXPECT_FALSE(setting.valid());
    }
}

TEST_F(LegacyControllerMappingSettingsTest, doubleSettingEditing) {
    QDomDocument doc;
    doc.setContent(
            QByteArray("<option variable=\"myReal1\" type=\"real\" label=\"Test label\"/>"));

    LegacyControllerRealSetting setting(doc.documentElement());
    EXPECT_TRUE(setting.valid()) << "Unable to create a double setting";

    setting.m_editedValue = 1.0;
    EXPECT_TRUE(setting.isDirty());
    setting.save();
    EXPECT_FALSE(setting.isDirty());
    EXPECT_EQ(setting.m_savedValue, true);

    bool ok;
    setting.parse("0.001", &ok);
    EXPECT_TRUE(ok);
    EXPECT_FALSE(setting.isDirty());
    EXPECT_EQ(setting.stringify(), "0.001");
    EXPECT_TRUE(setting.value().isNumber());
    EXPECT_EQ(setting.value().toNumber(), 0.001);
    EXPECT_FALSE(setting.isDefault());
    setting.parse("-15", &ok);
    EXPECT_TRUE(ok);
    setting.parse("0", &ok);
    EXPECT_TRUE(ok);
    EXPECT_FALSE(setting.isDirty());
    EXPECT_EQ(setting.stringify(), "0");
    setting.parse("30.0 ", &ok);
    EXPECT_TRUE(ok);
    EXPECT_EQ(setting.stringify(), "30");
    EXPECT_TRUE(setting.value().isNumber());
    EXPECT_EQ(setting.value().toNumber(), 30.0);
    setting.reset();
    EXPECT_TRUE(setting.isDirty());
    setting.save();
    EXPECT_EQ(setting.stringify(), "0");
    EXPECT_TRUE(setting.isDefault());
    setting.parse("abc ", &ok);
    EXPECT_FALSE(ok);
    EXPECT_TRUE(setting.value().isNumber());
    EXPECT_EQ(setting.value().toNumber(), .0);
}

TEST_F(LegacyControllerMappingSettingsTest, enumSettingParsing) {
    QDomDocument doc;
    {
        doc.setContent(QString(kValidEnum)
                        .arg(QList({QString(kValidEnumOption)
                                                   .arg("My option label",
                                                           "myOptionValue")})
                                        .join(""))
                        .toLatin1());

        EXPECT_TRUE(LegacyControllerEnumSetting::match(doc.documentElement()));
        LegacyControllerEnumSetting setting(doc.documentElement());
        EXPECT_TRUE(setting.valid()) << "Unable to create an enum setting";

        EXPECT_EQ(setting.variableName(), "myEnum1");
        EXPECT_EQ(setting.label(), "Test label");
        EXPECT_EQ(setting.description(), "Test description");

        EXPECT_FALSE(setting.isDirty());
        EXPECT_TRUE(setting.isDefault());
        EXPECT_EQ(setting.stringify(), "myOptionValue");
        EXPECT_TRUE(setting.valid());
    }
    {
        doc.setContent(QString(kValidEnum).arg("").toLatin1());
        LegacyControllerEnumSetting setting(doc.documentElement());
        EXPECT_FALSE(setting.valid());
    }
}

TEST_F(LegacyControllerMappingSettingsTest, enumSettingEditing) {
    QDomDocument doc;
    doc.setContent(QString(kValidEnum)
                           .arg(QList({
                                              QString(kValidEnumOption)
                                                      .arg("My option label",
                                                              "myOptionValue1"),
                                              QString(kValidEnumOption)
                                                      .arg("My option label",
                                                              "myOptionValue2"),
                                              QString(kValidEnumOption)
                                                      .arg("My option label",
                                                              "myOptionValue3"),
                                      })
                                           .join(""))
                           .toLatin1());

    EXPECT_TRUE(LegacyControllerEnumSetting::match(doc.documentElement()));
    LegacyControllerEnumSetting setting(doc.documentElement());
    EXPECT_TRUE(setting.valid()) << "Unable to create an enum setting";

    setting.m_editedValue = 2;
    EXPECT_TRUE(setting.isDirty());
    setting.save();
    EXPECT_FALSE(setting.isDirty());
    EXPECT_EQ(setting.m_savedValue, 2);
    EXPECT_EQ(setting.stringify(), "myOptionValue3");
    EXPECT_TRUE(setting.value().isString());
    EXPECT_EQ(setting.value().toString(), "myOptionValue3");

    bool ok;
    setting.parse("myOptionValue2", &ok);
    EXPECT_TRUE(ok);
    EXPECT_FALSE(setting.isDirty());
    EXPECT_EQ(setting.stringify(), "myOptionValue2");
    EXPECT_TRUE(setting.value().isString());
    EXPECT_EQ(setting.value().toString(), "myOptionValue2");
    EXPECT_FALSE(setting.isDefault());
    setting.reset();
    EXPECT_TRUE(setting.isDirty());
    setting.save();
    EXPECT_EQ(setting.stringify(), "myOptionValue1");
    EXPECT_TRUE(setting.isDefault());
    setting.parse("abc ", &ok);
    EXPECT_FALSE(ok);
    EXPECT_TRUE(setting.value().isString());
    EXPECT_EQ(setting.value().toString(), "myOptionValue1");
}

class LegacyDummyMapping : public LegacyControllerMapping {
  public:
    LegacyDummyMapping() {
    }

    bool saveMapping(const QString&) const override {
        return false;
    }

    bool isMappable() const override {
        return false;
    }
};

class LegacyDummyMappingFileHandler : public LegacyControllerMappingFileHandler {
  public:
    LegacyDummyMappingFileHandler(){};
    virtual ~LegacyDummyMappingFileHandler(){};

    static std::shared_ptr<LegacyControllerMapping> loadDummyMapping(
            const QDomElement& root, const QString& filePath) {
        LegacyDummyMappingFileHandler handler;
        return handler.load(root, filePath, QDir());
    }

  private:
    virtual std::shared_ptr<LegacyControllerMapping> load(const QDomElement& root,
            const QString& filePath,
            const QDir&) {
        auto pMapping = std::make_shared<LegacyDummyMapping>();
        pMapping->setFilePath(filePath);
        parseMappingSettings(root, pMapping.get());
        return pMapping;
    }
};

TEST_F(LegacyControllerMappingSettingsTest, parseSimpleSettingBlock) {
    QDomDocument doc;
    QString dom;
    QTextStream(&dom)
            << QString(kValidBoolean).arg("true")
            << QString(kValidInteger).arg("42", "1", "99", "1")
            << QString(kValidDouble).arg("42", "1", "99", "1")
            << QString(kValidEnum)
                       .arg(QList({
                                          QString(kValidEnumOption)
                                                  .arg("My option label",
                                                          "myOptionValue1"),
                                          QString(kValidEnumOption)
                                                  .arg("My option label",
                                                          "myOptionValue2"),
                                          QString(kValidEnumOption)
                                                  .arg("My option label",
                                                          "myOptionValue3"),
                                  })
                                       .join(""));
    doc.setContent(
            QString("<?xml version=\"1.0\" "
                    "encoding=\"utf-8\"?><MixxxControllerPreset><settings>%1</"
                    "settings></MixxxControllerPreset>")
                    .arg(dom));

    auto pMapping = LegacyDummyMappingFileHandler::loadDummyMapping(
            doc.documentElement(), "/fake/path");

    const auto& settings = pMapping->getSettings();

    ASSERT_EQ(settings.size(), 4);
    ASSERT_EQ(settings.at(0)->variableName(), "myToggle1");
    ASSERT_EQ(settings.at(1)->variableName(), "myInteger1");
    ASSERT_EQ(settings.at(2)->variableName(), "myReal1");
    ASSERT_EQ(settings.at(3)->variableName(), "myEnum1");
}

TEST_F(LegacyControllerMappingSettingsTest, discardDuplicateSettings) {
    QDomDocument doc;
    QString dom;
    QTextStream(&dom)
            << QString(kValidBoolean).arg("true")
            << QString(kValidBoolean).arg("false")
            << QString(kValidInteger).arg("50", "0", "100", "1")
            << QString(kValidInteger).arg("500", "0", "1000", "10")
            << QString(kValidEnum)
                       .arg(QList({
                                          QString(kValidEnumOption)
                                                  .arg("My option label",
                                                          "myOptionValue1"),
                                  })
                                       .join(""))
            << QString(kValidEnum)
                       .arg(QList({
                                          QString(kValidEnumOption)
                                                  .arg("My option label",
                                                          "myOtherOptionValue1"),
                                  })
                                       .join(""));
    doc.setContent(
            QString("<?xml version=\"1.0\" "
                    "encoding=\"utf-8\"?><MixxxControllerPreset><settings>%1</"
                    "settings></MixxxControllerPreset>")
                    .arg(dom));

    auto pMapping = LegacyDummyMappingFileHandler::loadDummyMapping(
            doc.documentElement(), "/fake/path");

    const auto& settings = pMapping->getSettings();

    ASSERT_EQ(settings.size(), 3);
    ASSERT_EQ(settings.at(0)->variableName(), "myToggle1");
    ASSERT_EQ(settings.at(1)->variableName(), "myInteger1");
    ASSERT_EQ(settings.at(2)->variableName(), "myEnum1");

    ASSERT_TRUE(settings.at(0)->value().toBool());
    ASSERT_EQ(settings.at(1)->value().toNumber(), 50);
    ASSERT_EQ(settings.at(2)->value().toString(), "myOptionValue1");
}

TEST_F(LegacyControllerMappingSettingsTest, handleNumberWithNegativeRange) {
    QDomDocument doc;
    QString dom;
    QTextStream(&dom)
            << QString(kValidInteger).arg("0", "-500", "0", "1");
    doc.setContent(
            QString("<?xml version=\"1.0\" "
                    "encoding=\"utf-8\"?><MixxxControllerPreset><settings>%1</"
                    "settings></MixxxControllerPreset>")
                    .arg(dom));

    auto pMapping = LegacyDummyMappingFileHandler::loadDummyMapping(
            doc.documentElement(), "/fake/path");

    ASSERT_EQ(pMapping->getSettings().size(), 1);
    ASSERT_EQ(pMapping->getSettings().at(0)->variableName(), "myInteger1");
    ASSERT_EQ(pMapping->getSettings().at(0)->value().toNumber(), 0);

    dom.clear();
    QTextStream(&dom)
            << QString(kValidInteger).arg("20", "-100", "100", "50");
    doc.setContent(
            QString("<?xml version=\"1.0\" "
                    "encoding=\"utf-8\"?><MixxxControllerPreset><settings>%1</"
                    "settings></MixxxControllerPreset>")
                    .arg(dom));

    pMapping = LegacyDummyMappingFileHandler::loadDummyMapping(
            doc.documentElement(), "/fake/path");

    ASSERT_EQ(pMapping->getSettings().size(), 1);
    ASSERT_EQ(pMapping->getSettings().at(0)->variableName(), "myInteger1");
    ASSERT_EQ(pMapping->getSettings().at(0)->value().toNumber(), 20);
}
