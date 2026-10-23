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

    // Hotcues indices by position should now be 1 3 5 7. The two hotcues
    // at position 200 retain their existing order (indices 1 and 7),
    // i.e. none of them is swallowed or reassigned to a duplicate index.
    EXPECT_EQ(pHotcue1->getHotCue(), 1);
    EXPECT_EQ(pHotcue2->getHotCue(), 3);
    EXPECT_EQ(pHotcue3->getHotCue(), 5);
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

    // Hotcues indices by position should now be 0 1 2 3. The two hotcues
    // at position 200 retain their existing order (indices 1 and 7),
    // i.e. none of them is swallowed or reassigned to a duplicate index.
    EXPECT_EQ(pHotcue1->getHotCue(), 0);
    EXPECT_EQ(pHotcue2->getHotCue(), 1);
    EXPECT_EQ(pHotcue3->getHotCue(), 2);
    EXPECT_EQ(pHotcue4->getHotCue(), 3);
}

TEST_F(TrackHotcueOrderByPosTest, orderHotcuesMixedTypesDuplicatePositionsKeepOffsets) {
    auto pTrack = newTestTrack();
    pTrack->markClean();

    // create a hotcue, a loop and a jump cue at the same position, with
    // indices defining an existing order that differs from the type order
    CuePointer pHotcue = pTrack->createAndAddCue(mixxx::CueType::HotCue,
            9,
            mixxx::audio::FramePos(100),
            mixxx::audio::kInvalidFramePos);
    CuePointer pLoop = pTrack->createAndAddCue(mixxx::CueType::Loop,
            7,
            mixxx::audio::FramePos(100),
            mixxx::audio::kInvalidFramePos);
    CuePointer pJump = pTrack->createAndAddCue(mixxx::CueType::Jump,
            8,
            mixxx::audio::FramePos(100),
            mixxx::audio::kInvalidFramePos);

    pTrack->setHotcueIndicesSortedByPosition(HotcueSortMode::KeepOffsets);

    // The cues retain their existing order at the same position, i.e.
    // each of them keeps its current index: loop 7, jump 8, hotcue 9
    EXPECT_EQ(pLoop->getHotCue(), 7);
    EXPECT_EQ(pJump->getHotCue(), 8);
    EXPECT_EQ(pHotcue->getHotCue(), 9);
}

TEST_F(TrackHotcueOrderByPosTest, orderHotcuesMixedTypesDuplicatePositionsRemoveOffsets) {
    auto pTrack = newTestTrack();
    pTrack->markClean();

    // create a hotcue, a loop and a jump cue at the same position, with
    // indices defining an existing order that differs from the type order
    CuePointer pHotcue = pTrack->createAndAddCue(mixxx::CueType::HotCue,
            9,
            mixxx::audio::FramePos(100),
            mixxx::audio::kInvalidFramePos);
    CuePointer pLoop = pTrack->createAndAddCue(mixxx::CueType::Loop,
            7,
            mixxx::audio::FramePos(100),
            mixxx::audio::kInvalidFramePos);
    CuePointer pJump = pTrack->createAndAddCue(mixxx::CueType::Jump,
            8,
            mixxx::audio::FramePos(100),
            mixxx::audio::kInvalidFramePos);

    pTrack->setHotcueIndicesSortedByPosition(HotcueSortMode::RemoveOffsets);

    // The consecutive indices 0 1 2 are assigned keeping the existing
    // order at the same position: loop, jump, hotcue
    EXPECT_EQ(pLoop->getHotCue(), 0);
    EXPECT_EQ(pJump->getHotCue(), 1);
    EXPECT_EQ(pHotcue->getHotCue(), 2);
}
