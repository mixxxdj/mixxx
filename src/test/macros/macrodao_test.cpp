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

TEST_F(MacroDAOTest, SaveAndLoadMacro) {
    TrackId track(1);
    MacroAction action(0, 7);
    Macro saved(QList<MacroAction>{action}, "Test", Macro::StateFlag::Looped);

    m_macroDAO.saveMacro(track, &saved);
    EXPECT_EQ(m_macroDAO.getFreeSlot(track), 2);
    // Sanity check
    EXPECT_EQ(m_macroDAO.loadMacros(TrackId(2)).size(), 0);

    QMap<int, MacroPtr> loaded = m_macroDAO.loadMacros(track);
    ASSERT_EQ(loaded.size(), 1);
    EXPECT_EQ(loaded.keys(), QList{1});

    MacroPtr macro = loaded.first();
    EXPECT_FALSE(macro->isEnabled());
    EXPECT_TRUE(macro->isLooped());
    EXPECT_EQ(macro->size(), 1);
    EXPECT_EQ(macro->getActions().first().target, action.target);
    EXPECT_EQ(macro->getLabel(), "Test");

    // Change Macro slot
    m_macroDAO.saveMacro(track, macro.get(), 3);
    EXPECT_EQ(m_macroDAO.getFreeSlot(track), 1);

    loaded = m_macroDAO.loadMacros(track);
    EXPECT_EQ(loaded.size(), 1);
    EXPECT_EQ(loaded.keys(), QList{3});
}
