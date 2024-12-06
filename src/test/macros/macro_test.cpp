#include "track/macro.h"

#include <gtest/gtest.h>

TEST(MacroTest, SerializeMacroActions) {
    QList<MacroAction> actions{MacroAction(mixxx::audio::FramePos(0), mixxx::audio::FramePos(1))};

    QString filename(QDir::currentPath() % "/src/test/macros/macro_proto");
    ASSERT_TRUE(QFile::exists(filename));
    QFile file(filename);
    file.open(QIODevice::OpenModeFlag::ReadOnly);
    QByteArray content = file.readAll();
    QByteArray serialized = Macro::serialize(actions);
    EXPECT_EQ(serialized.length(), content.length());
    EXPECT_EQ(serialized, content);
    QList<MacroAction> deserialized = Macro::deserialize(serialized);
    EXPECT_EQ(deserialized.size(), 1);
    EXPECT_EQ(deserialized, actions);
}

TEST(MacroTest, CreateMacroAndChangeLabel) {
    Macro macro;
    EXPECT_FALSE(macro.isDirty());
    EXPECT_TRUE(macro.isEmpty());
    macro.clear();
    const Macro macro2;
    EXPECT_EQ(macro, macro2);

    macro.setLabel("hello");
    EXPECT_TRUE(macro.isDirty());
    EXPECT_NE(macro, macro2);
}
