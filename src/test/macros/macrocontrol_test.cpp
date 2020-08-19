#include "engine/controls/macrocontrol.h"

#include <gtest/gtest.h>

TEST(MacroControl, Test) {
    QString group("[Channel1]");
    MacroControl macroControl(group, nullptr, 2);
    ControlProxy COStatus(ConfigKey(group, "macro_2_status"));
    EXPECT_EQ(COStatus.get(), MacroControl::Status::NoTrack);

    // TODO(xerus) ensure valid macro on load
    //macroControl.trackLoaded(Track::newTemporary());
    //EXPECT_EQ(COStatus.get(), MacroControl::Status::Empty);

    auto pTrack = Track::newTemporary();
    pTrack->setMacros({{2, std::make_shared<Macro>(QList{MacroAction()}, "test")}});
    macroControl.trackLoaded(pTrack);
    EXPECT_EQ(COStatus.get(), MacroControl::Status::Recorded);
}
