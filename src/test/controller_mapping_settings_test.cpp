
#include <gtest/gtest.h>

#include <QDomDocument>

// FIXME (acolombier): here we need the CPP file so the templated methods gets
// built. AFAIK, there is an alternate solution which is to include the cpp file
// in the CMake build list
#include "controllers/legacycontrollersettings.cpp"
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

TEST_F(LegacyControllerMappingSettingsTest, booleanSettingParsing) {
    QDomDocument doc;
    doc.setContent(QString(kValidBoolean).arg("false").toLatin1());

    EXPECT_TRUE(LegacyControllerBooleanSetting::match(doc.documentElement()));
    LegacyControllerBooleanSetting* setting = (LegacyControllerBooleanSetting*)
            LegacyControllerBooleanSetting::createFrom(doc.documentElement());
    EXPECT_TRUE(setting->valid()) << "Unable to create a boolean setting";

    EXPECT_EQ(setting->variableName(), "myToggle1");
    EXPECT_EQ(setting->label(), "Test label");
    EXPECT_EQ(setting->description(), "Test description");

    EXPECT_FALSE(setting->isDirty());
    EXPECT_TRUE(setting->isDefault());
    EXPECT_EQ(setting->stringify(), "false");
    EXPECT_TRUE(setting->valid());

    delete setting;

    doc.setContent(QString(kValidBoolean).arg("true").toLatin1());

    setting = (LegacyControllerBooleanSetting*)
            LegacyControllerBooleanSetting::createFrom(doc.documentElement());
    EXPECT_TRUE(setting->valid()) << "Unable to create a boolean setting";

    EXPECT_EQ(setting->stringify(), "true");

    delete setting;
}

TEST_F(LegacyControllerMappingSettingsTest, booleanSettingEditing) {
    QDomDocument doc;
    doc.setContent(
            QByteArray("<option variable=\"myToggle1\" type=\"boolean\" "
                       "default=\"false\" label=\"Test label\"/>"));

    LegacyControllerBooleanSetting* setting = (LegacyControllerBooleanSetting*)
            LegacyControllerBooleanSetting::createFrom(doc.documentElement());
    EXPECT_TRUE(setting->valid()) << "Unable to create a boolean setting";

    setting->m_dirtyValue = true;
    EXPECT_TRUE(setting->isDirty());
    setting->save();
    EXPECT_FALSE(setting->isDirty());
    EXPECT_EQ(setting->m_currentValue, true);

    bool ok;
    setting->parse("true", &ok);
    EXPECT_TRUE(ok);
    EXPECT_FALSE(setting->isDirty());
    EXPECT_EQ(setting->stringify(), "true");
    EXPECT_FALSE(setting->isDefault());
    setting->parse("1", &ok);
    EXPECT_TRUE(ok);
    setting->parse("0", &ok);
    EXPECT_TRUE(ok);
    EXPECT_FALSE(setting->isDirty());
    EXPECT_EQ(setting->stringify(), "false");
    setting->parse("TRUE", &ok);
    EXPECT_TRUE(ok);
    EXPECT_EQ(setting->stringify(), "true");
    setting->reset();
    EXPECT_EQ(setting->stringify(), "false");
    EXPECT_TRUE(setting->isDefault());
}

TEST_F(LegacyControllerMappingSettingsTest, integerSettingParsing) {
    QDomDocument doc;
    doc.setContent(QString(kValidInteger).arg("42", "1", "99", "1").toLatin1());

    EXPECT_TRUE(LegacyControllerIntegerSetting::match(doc.documentElement()));
    LegacyControllerIntegerSetting* setting = (LegacyControllerIntegerSetting*)
            LegacyControllerIntegerSetting::createFrom(doc.documentElement());
    EXPECT_TRUE(setting->valid()) << "Unable to create an integer setting";

    EXPECT_EQ(setting->variableName(), "myInteger1");
    EXPECT_EQ(setting->label(), "Test label");
    EXPECT_EQ(setting->description(), "Test description");

    EXPECT_FALSE(setting->isDirty());
    EXPECT_TRUE(setting->isDefault());
    EXPECT_EQ(setting->stringify(), "42");
    EXPECT_TRUE(setting->valid());

    delete setting;

    doc.setContent(QString(kValidInteger).arg("18", "1", "99", "1").toLatin1());
    setting = (LegacyControllerIntegerSetting*)
            LegacyControllerIntegerSetting::createFrom(doc.documentElement());
    EXPECT_TRUE(setting->valid()) << "Unable to create an integer setting";
    EXPECT_EQ(setting->stringify(), "18");
    EXPECT_TRUE(setting->valid());

    delete setting;

    // Invalid if default is out of range
    doc.setContent(QString(kValidInteger).arg("10", "20", "30", "1").toLatin1());
    setting = (LegacyControllerIntegerSetting*)
            LegacyControllerIntegerSetting::createFrom(doc.documentElement());
    EXPECT_EQ(setting->stringify(), "10");
    EXPECT_FALSE(setting->valid());

    delete setting;

    doc.setContent(QString(kValidInteger).arg("10", "1", "5", "1").toLatin1());
    setting = (LegacyControllerIntegerSetting*)
            LegacyControllerIntegerSetting::createFrom(doc.documentElement());
    EXPECT_EQ(setting->stringify(), "10");
    EXPECT_FALSE(setting->valid());

    delete setting;

    // Invalid if step is zero
    doc.setContent(QString(kValidInteger).arg("10", "0", "30", "0").toLatin1());
    setting = (LegacyControllerIntegerSetting*)
            LegacyControllerIntegerSetting::createFrom(doc.documentElement());
    EXPECT_EQ(setting->stringify(), "10");
    EXPECT_FALSE(setting->valid());

    delete setting;
}

TEST_F(LegacyControllerMappingSettingsTest, integerSettingEditing) {
    // TODO (acolombier) Add test for setting edition
}

TEST_F(LegacyControllerMappingSettingsTest, doubleSettingParsing) {
    QDomDocument doc;
    doc.setContent(QString(kValidDouble).arg("42", "1", "99", "1").toLatin1());

    EXPECT_TRUE(LegacyControllerRealSetting::match(doc.documentElement()));
    LegacyControllerRealSetting* setting = (LegacyControllerRealSetting*)
            LegacyControllerRealSetting::createFrom(doc.documentElement());
    EXPECT_TRUE(setting->valid()) << "Unable to create a real setting";

    EXPECT_EQ(setting->variableName(), "myReal1");
    EXPECT_EQ(setting->label(), "myReal1");
    EXPECT_TRUE(setting->description().isEmpty());

    EXPECT_FALSE(setting->isDirty());
    EXPECT_TRUE(setting->isDefault());
    EXPECT_EQ(setting->stringify(), "42");
    EXPECT_TRUE(setting->valid());

    delete setting;

    doc.setContent(QString(kValidInteger).arg("18", "1", "99", "1").toLatin1());
    setting = (LegacyControllerRealSetting*)
            LegacyControllerRealSetting::createFrom(doc.documentElement());
    EXPECT_EQ(setting->stringify(), "18");
    EXPECT_TRUE(setting->valid());

    delete setting;

    // Invalid if default is out of range
    doc.setContent(QString(kValidInteger).arg("10", "20", "30", "1").toLatin1());
    setting = (LegacyControllerRealSetting*)
            LegacyControllerRealSetting::createFrom(doc.documentElement());
    EXPECT_EQ(setting->stringify(), "10");
    EXPECT_FALSE(setting->valid());

    delete setting;

    doc.setContent(QString(kValidInteger).arg("10", "1", "5", "1").toLatin1());
    setting = (LegacyControllerRealSetting*)
            LegacyControllerRealSetting::createFrom(doc.documentElement());
    EXPECT_EQ(setting->stringify(), "10");
    EXPECT_FALSE(setting->valid());

    delete setting;

    // Invalid if step is zero
    doc.setContent(QString(kValidInteger).arg("10", "0", "30", "0").toLatin1());
    setting = (LegacyControllerRealSetting*)
            LegacyControllerRealSetting::createFrom(doc.documentElement());
    EXPECT_EQ(setting->stringify(), "10");
    EXPECT_FALSE(setting->valid());

    delete setting;
}

TEST_F(LegacyControllerMappingSettingsTest, doubleSettingEditing) {
    // TODO (acolombier) Add test for setting edition
}
