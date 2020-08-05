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
    m_macroDAO.saveMacro(TrackId(1), "Test", QVector{MacroAction(0, 7)}, Macro::StateFlag::Looped);

    EXPECT_EQ(m_macroDAO.loadMacros(TrackId(2)).size(), 0);

    QList<Macro> loaded = m_macroDAO.loadMacros(TrackId(1));
    EXPECT_EQ(loaded.size(), 1);

    Macro macro = loaded.first();
    EXPECT_EQ(macro.m_state.testFlag(Macro::StateFlag::Enabled), false);
    EXPECT_EQ(macro.m_state.testFlag(Macro::StateFlag::Looped), true);
    EXPECT_EQ(macro.m_actions.size(), 1);
    EXPECT_EQ(macro.m_actions.first().target, 7);
    EXPECT_EQ(macro.m_label, "Test");
}
