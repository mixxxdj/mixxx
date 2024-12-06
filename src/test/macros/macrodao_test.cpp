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
    MacroAction action(mixxx::audio::FramePos(0), mixxx::audio::FramePos(7));
    Macro saved(QList<MacroAction>{action}, "Test", Macro::StateFlag::Looped);

    m_macroDAO.saveMacro(track, &saved, 1);
    EXPECT_EQ(m_macroDAO.loadMacros(TrackId(2)).size(), 0); // Sanity check

    QMap<int, MacroPointer> loaded = m_macroDAO.loadMacros(track);
    ASSERT_EQ(loaded.size(), 1);
    EXPECT_EQ(loaded.keys(), QList{1});

    MacroPointer macro = loaded.first();
    EXPECT_FALSE(macro->isEnabled());
    EXPECT_TRUE(macro->isLooped());
    EXPECT_EQ(macro->size(), 1);
    EXPECT_EQ(macro->getActions().first().getTargetPosition(), action.getTargetPosition());
    EXPECT_EQ(macro->getLabel(), "Test");

    // Change Macro slot
    m_macroDAO.saveMacro(track, macro.get(), 3);
    EXPECT_EQ(m_macroDAO.loadMacros(track).keys(), QList{3});
}
