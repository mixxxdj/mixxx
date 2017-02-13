#include <QString>

#include "test/mixxxtest.h"
#include "preferences/configobject.h"

namespace {

class ConfigObjectTest : public MixxxTest {};

TEST_F(ConfigObjectTest, SetValue_Overwrite) {
    auto ck = ConfigKey("[Test]", "test");
    config()->setValue(ck, QString("asdf"));
    config()->setValue(ck, QString("zxcv"));
    EXPECT_QSTRING_EQ("zxcv", config()->getValue<QString>(ck));
}

TEST_F(ConfigObjectTest, SetValue_QString) {
    auto ck = ConfigKey("[Test]", "test");
    config()->setValue(ck, QString("asdf"));
    EXPECT_QSTRING_EQ("asdf", config()->getValue<QString>(ck));
}

TEST_F(ConfigObjectTest, SetValue_QString_NullVsEmpty) {
    auto ck = ConfigKey("[Test]", "test");
    EXPECT_TRUE(config()->getValue<QString>(ck).isNull());

    // Setting it to an empty string returns an empty, not null string. Empty
    // strings are counted as present by the default value logic.
    config()->setValue(ck, QString(""));
    EXPECT_TRUE(config()->getValue<QString>(ck, QString("asdf")).isEmpty());
    EXPECT_FALSE(config()->getValue<QString>(ck, QString("asdf")).isNull());

    // And it persists across restarts.
    saveAndReloadConfig();
    EXPECT_TRUE(config()->getValue<QString>(ck).isEmpty());
    EXPECT_FALSE(config()->getValue<QString>(ck).isNull());
}

TEST_F(ConfigObjectTest, GetValue_QString) {
    auto ck = ConfigKey("[Test]", "test");
    EXPECT_QSTRING_EQ("zxcv", config()->getValue<QString>(ck, "zxcv"));
    config()->setValue(ck, QString("asdf"));
    EXPECT_QSTRING_EQ("asdf", config()->getValue<QString>(ck, "zxcv"));
}

TEST_F(ConfigObjectTest, SetValue_Integer) {
    auto ck = ConfigKey("[Test]", "test");
    config()->setValue(ck, 5);
    EXPECT_QSTRING_EQ("5", config()->getValue<QString>(ck));
}

TEST_F(ConfigObjectTest, GetValue_Integer) {
    auto ck = ConfigKey("[Test]", "test");

    // Not present.
    EXPECT_EQ(5, config()->getValue(ck, 5));

    // Empty
    config()->setValue(ck, QString(""));
    EXPECT_EQ(5, config()->getValue(ck, 5));

    // Malformatted.
    config()->setValue(ck, QString("asdf"));
    EXPECT_EQ(5, config()->getValue(ck, 5));

    // Overflow 32-bit int.
    config()->setValue(ck, QString("2147483648"));
    EXPECT_EQ(5, config()->getValue(ck, 5));
    config()->setValue(ck, QString("-2147483649"));
    EXPECT_EQ(5, config()->getValue(ck, 5));

    // Ok.
    config()->setValue(ck, QString("4"));
    EXPECT_EQ(4, config()->getValue(ck, 5));
    config()->setValue(ck, QString("-4"));
    EXPECT_EQ(-4, config()->getValue(ck, 5));
}

TEST_F(ConfigObjectTest, SetValue_Bool) {
    auto ck = ConfigKey("[Test]", "test");
    config()->setValue(ck, true);
    EXPECT_QSTRING_EQ("1", config()->getValue<QString>(ck));
    config()->setValue(ck, false);
    EXPECT_QSTRING_EQ("0", config()->getValue<QString>(ck));
}

TEST_F(ConfigObjectTest, GetValue_Bool) {
    auto ck = ConfigKey("[Test]", "test");

    // Not present.
    EXPECT_TRUE(config()->getValue(ck, true));

    // Empty
    config()->setValue(ck, QString(""));
    EXPECT_TRUE(config()->getValue(ck, true));

    // Malformatted.
    config()->setValue(ck, QString("asdf"));
    EXPECT_TRUE(config()->getValue(ck, true));

    // Ok.
    config()->setValue(ck, QString("0"));
    EXPECT_FALSE(config()->getValue(ck, true));

    // Not just 0 and 1.
    config()->setValue(ck, QString("5"));
    EXPECT_TRUE(config()->getValue(ck, false));
}

TEST_F(ConfigObjectTest, Exists) {
    auto ck = ConfigKey("[Test]", "test");
    EXPECT_FALSE(config()->exists(ck));
    config()->setValue(ck, 5);
    EXPECT_TRUE(config()->exists(ck));
}

TEST_F(ConfigObjectTest, GetValueString) {
    auto ck = ConfigKey("[Test]", "foo");
    auto ck2 = ConfigKey("[Test]", "bar");
    config()->setValue(ck, 5);
    EXPECT_QSTRING_EQ("5", config()->getValueString(ck));
    EXPECT_QSTRING_EQ("6", config()->getValue(ck2, "6"));
}

TEST_F(ConfigObjectTest, Save) {
    for (int i = 0; i < 10; ++i) {
        config()->setValue(ConfigKey(QString("[Test%1]").arg(i),
                                     QString("control%1").arg(i)), i);
    }

    m_pConfig->save();
    m_pConfig = UserSettingsPointer(
            new UserSettings(getTestDataDir().filePath("test.cfg")));

    for (int i = 0; i < 10; ++i) {
        EXPECT_EQ(i, config()->getValue<int>(
            ConfigKey(QString("[Test%1]").arg(i),
                      QString("control%1").arg(i)), -1));
    }
}

}  // namespace
