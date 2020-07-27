#include "macros/macromanager.h"

#include <database/schemamanager.h>
#include <gtest/gtest.h>

#include "test/mixxxdbtest.h"

class MacroManagerTest : public MixxxDbTest {
  public:
    MacroManagerTest()
            : m_macroManager(dbConnectionPooler(), nullptr) {
        MixxxDb::initDatabaseSchema(dbConnection());
    }

    MacroManager m_macroManager;
};

TEST_F(MacroManagerTest, SaveAndLoadMacro) {
    m_macroManager.saveMacro(TrackId(1), "Test", QVector{MacroAction(0, 7)});
    EXPECT_EQ(m_macroManager.loadMacros(TrackId(2)).size(), 0);
    QList<Macro> loaded = m_macroManager.loadMacros(TrackId(1));
    EXPECT_EQ(loaded.size(), 1);
    Macro macro = loaded.first();
    EXPECT_EQ(macro.m_loop, false);
    EXPECT_EQ(macro.m_enabled, false);
    EXPECT_EQ(macro.m_actions.size(), 1);
    EXPECT_EQ(macro.m_actions.first().target, 7);
    EXPECT_EQ(macro.m_label, "Test");
}