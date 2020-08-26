#include "library/dao/macrodao.h"

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

TEST_F(MacroDAOTest, LoadMacros) {
    EXPECT_EQ(m_macroDAO.loadMacros(TrackId(1)).size(), kMacrosPerTrack);
}

TEST_F(MacroDAOTest, SaveAndLoadMacro) {
    TrackId track(1);
    MacroAction action(0, 7);
    Macro saved(QList<MacroAction>{action}, "Test", Macro::StateFlag::Looped);

    m_macroDAO.saveMacro(track, &saved);
    EXPECT_EQ(m_macroDAO.getFreeSlot(track), 2);

    MacroPtr macro = m_macroDAO.loadMacros(track).first();
    EXPECT_EQ(macro->isEnabled(), false);
    EXPECT_EQ(macro->isLooped(), true);
    EXPECT_EQ(macro->size(), 1);
    EXPECT_EQ(macro->getActions().first().target, action.target);
    EXPECT_EQ(macro->getLabel(), "Test");

    // Change Macro slot
    m_macroDAO.saveMacro(track, macro.get(), 3);
    EXPECT_EQ(m_macroDAO.getFreeSlot(track), 1);
    EXPECT_EQ(*m_macroDAO.loadMacros(track)[3], *macro);
}
