#include "recording/macromanager.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "mixxxtest.h"

class MacroManagerTest : public MixxxTest {
};

TEST(MacrosTest, CreateMacro) {
    auto macro = new Macro();
    ASSERT_EQ(macro->m_length, 0);
}