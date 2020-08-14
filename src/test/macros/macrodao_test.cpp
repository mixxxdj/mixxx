#include "macros/macrodao.h"

#include <database/schemamanager.h>
#include <gtest/gtest.h>

#include "test/mixxxdbtest.h"

class MacroDAOTest : public MixxxDbTest {
  public:
    MacroDAOTest()
            : m_macroDAO() {
        MixxxDb::initDatabaseSchema(dbConnection());
        m_macroDAO.initialize(dbConnection());
    }

    MacroDAO m_macroDAO;
};

TEST_F(MacroDAOTest, SaveAndLoadMacro) {
    TrackId track(1);
    m_macroDAO.saveMacro(track, QVector{MacroAction(0, 7)}, "Test", Macro::StateFlag::Looped);

    EXPECT_EQ(m_macroDAO.loadMacros(TrackId(2)).size(), 0);

    QMap<int, Macro> loaded = m_macroDAO.loadMacros(track);
    ASSERT_EQ(loaded.size(), 1);
    EXPECT_EQ(loaded.firstKey(), 1);

    Macro macro = loaded.first();
    EXPECT_EQ(macro.m_state.testFlag(Macro::StateFlag::Enabled), false);
    EXPECT_EQ(macro.m_state.testFlag(Macro::StateFlag::Looped), true);
    EXPECT_EQ(macro.m_actions.size(), 1);
    EXPECT_EQ(macro.m_actions.first().target, 7);
    EXPECT_EQ(macro.m_label, "Test");

    m_macroDAO.saveMacro(track, macro);
    loaded = m_macroDAO.loadMacros(track);
    EXPECT_EQ(loaded.size(), 2);
    auto expectedKeys = QList<int>{1, 2};
    EXPECT_EQ(loaded.keys(), expectedKeys);
    EXPECT_EQ(m_macroDAO.getFreeNumber(track), 3);
}
