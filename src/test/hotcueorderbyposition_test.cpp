// #include "library/coverart.h"
// #include "sources/soundsourceproxy.h"
// #include "test/soundsourceproviderregistration.h"
#include "test/mixxxtest.h"
#include "track/cue.h"
#include "track/track.h"

// Test for updating track metadata and cover art from files.
class TrackHotcueOrderByPosTest : public MixxxTest {
  protected:
    static TrackPointer newTestTrack() {
        return Track::newTemporary(
                QDir(MixxxTest::getOrInitTestDir().filePath(QStringLiteral("track-test-data"))),
                "THOBP.mp3");
    }
};

TEST_F(TrackHotcueOrderByPosTest, orderHotcuesKeepOffsets) {
    auto pTrack = newTestTrack();
    pTrack->markClean();

    // create hotcues with ascending position but unordered indices
    CuePointer pHotcue1 = pTrack->createAndAddCue(mixxx::CueType::HotCue,
            2,
            mixxx::audio::FramePos(100),
            mixxx::audio::kInvalidFramePos);
    CuePointer pHotcue2 = pTrack->createAndAddCue(mixxx::CueType::HotCue,
            1,
            mixxx::audio::FramePos(200),
            mixxx::audio::kInvalidFramePos);
    CuePointer pHotcue3 = pTrack->createAndAddCue(mixxx::CueType::HotCue,
            7,
            mixxx::audio::FramePos(300),
            mixxx::audio::kInvalidFramePos,
            mixxx::PredefinedColorPalettes::kDefaultCueColor);
    CuePointer pHotcue4 = pTrack->createAndAddCue(mixxx::CueType::HotCue,
            5,
            mixxx::audio::FramePos(400),
            mixxx::audio::kInvalidFramePos);

    pTrack->setHotcueIndicesSortedByPosition(HotcueSortMode::KeepOffsets);

    // Hotcues indices by position should now be 1 2 5 7
    EXPECT_EQ(pHotcue1->getHotCue(), 1);
    EXPECT_EQ(pHotcue2->getHotCue(), 2);
    EXPECT_EQ(pHotcue3->getHotCue(), 5);
    EXPECT_EQ(pHotcue4->getHotCue(), 7);
}

TEST_F(TrackHotcueOrderByPosTest, orderHotcuesRemoveOffsets) {
    auto pTrack = newTestTrack();
    pTrack->markClean();

    // create hotcues with ascending position but unordered indices
    CuePointer pHotcue1 = pTrack->createAndAddCue(mixxx::CueType::HotCue,
            2,
            mixxx::audio::FramePos(100),
            mixxx::audio::kInvalidFramePos);
    CuePointer pHotcue2 = pTrack->createAndAddCue(mixxx::CueType::HotCue,
            1,
            mixxx::audio::FramePos(200),
            mixxx::audio::kInvalidFramePos);
    CuePointer pHotcue3 = pTrack->createAndAddCue(mixxx::CueType::HotCue,
            7,
            mixxx::audio::FramePos(300),
            mixxx::audio::kInvalidFramePos,
            mixxx::PredefinedColorPalettes::kDefaultCueColor);
    CuePointer pHotcue4 = pTrack->createAndAddCue(mixxx::CueType::HotCue,
            5,
            mixxx::audio::FramePos(400),
            mixxx::audio::kInvalidFramePos);

    pTrack->setHotcueIndicesSortedByPosition(HotcueSortMode::RemoveOffsets);

    // Hotcues indices by position should now be 0 1 2 3
    EXPECT_EQ(pHotcue1->getHotCue(), 0);
    EXPECT_EQ(pHotcue2->getHotCue(), 1);
    EXPECT_EQ(pHotcue3->getHotCue(), 2);
    EXPECT_EQ(pHotcue4->getHotCue(), 3);
}

TEST_F(TrackHotcueOrderByPosTest, orderHotcuesDuplicatePositionsKeepOffsets) {
    auto pTrack = newTestTrack();
    pTrack->markClean();

    // create hotcues with ascending positions, two of them sharing
    // the same position
    CuePointer pHotcue1 = pTrack->createAndAddCue(mixxx::CueType::HotCue,
            3,
            mixxx::audio::FramePos(100),
            mixxx::audio::kInvalidFramePos);
    CuePointer pHotcue2 = pTrack->createAndAddCue(mixxx::CueType::HotCue,
            1,
            mixxx::audio::FramePos(200),
            mixxx::audio::kInvalidFramePos);
    CuePointer pHotcue3 = pTrack->createAndAddCue(mixxx::CueType::HotCue,
            7,
            mixxx::audio::FramePos(200),
            mixxx::audio::kInvalidFramePos);
    CuePointer pHotcue4 = pTrack->createAndAddCue(mixxx::CueType::HotCue,
            5,
            mixxx::audio::FramePos(400),
            mixxx::audio::kInvalidFramePos);

    pTrack->setHotcueIndicesSortedByPosition(HotcueSortMode::KeepOffsets);

    // Hotcues indices by position should now be 1 3 5 7, i.e. the two
    // hotcues at position 200 must not swallow each other and none of
    // them may be reassigned to index 0
    EXPECT_EQ(pHotcue1->getHotCue(), 1);
    EXPECT_TRUE(pHotcue2->getHotCue() == 3 || pHotcue2->getHotCue() == 5);
    EXPECT_TRUE(pHotcue3->getHotCue() == 3 || pHotcue3->getHotCue() == 5);
    EXPECT_NE(pHotcue2->getHotCue(), pHotcue3->getHotCue());
    EXPECT_EQ(pHotcue4->getHotCue(), 7);
}

TEST_F(TrackHotcueOrderByPosTest, orderHotcuesDuplicatePositionsRemoveOffsets) {
    auto pTrack = newTestTrack();
    pTrack->markClean();

    // create hotcues with ascending positions, two of them sharing
    // the same position
    CuePointer pHotcue1 = pTrack->createAndAddCue(mixxx::CueType::HotCue,
            3,
            mixxx::audio::FramePos(100),
            mixxx::audio::kInvalidFramePos);
    CuePointer pHotcue2 = pTrack->createAndAddCue(mixxx::CueType::HotCue,
            1,
            mixxx::audio::FramePos(200),
            mixxx::audio::kInvalidFramePos);
    CuePointer pHotcue3 = pTrack->createAndAddCue(mixxx::CueType::HotCue,
            7,
            mixxx::audio::FramePos(200),
            mixxx::audio::kInvalidFramePos);
    CuePointer pHotcue4 = pTrack->createAndAddCue(mixxx::CueType::HotCue,
            5,
            mixxx::audio::FramePos(400),
            mixxx::audio::kInvalidFramePos);

    pTrack->setHotcueIndicesSortedByPosition(HotcueSortMode::RemoveOffsets);

    // Hotcues indices by position should now be 0 1 2 3, i.e. the two
    // hotcues at position 200 must not swallow each other and none of
    // them may be reassigned a duplicate index
    EXPECT_EQ(pHotcue1->getHotCue(), 0);
    EXPECT_TRUE(pHotcue2->getHotCue() == 1 || pHotcue2->getHotCue() == 2);
    EXPECT_TRUE(pHotcue3->getHotCue() == 1 || pHotcue3->getHotCue() == 2);
    EXPECT_NE(pHotcue2->getHotCue(), pHotcue3->getHotCue());
    EXPECT_EQ(pHotcue4->getHotCue(), 3);
}
