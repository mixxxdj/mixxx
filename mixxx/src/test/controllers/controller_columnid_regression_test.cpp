/*
 This test case is used to ensure that hardcoded CO value in the the settings
 definition matches with Mixxx value and will help detecting regression if they
 are ever updated.

 Currently, the S4 MK3 is referencing library column ID in its setting, so this
 test ensure that the value always matches with the Mixxx spec. New controllers
 can be added by duplicated the `ensureS4MK3` case and adapt as needed
*/
#include "controllers/legacycontrollermapping.h"
#include "controllers/legacycontrollermappingfilehandler.h"
#include "library/trackmodel.h"
#include "test/mixxxtest.h"
#include "util/time.h"

using namespace std::chrono_literals;

class ControllerLibraryColumnIDRegressionTest : public MixxxTest {
  protected:
    void SetUp() override {
        mixxx::Time::setTestMode(true);
        mixxx::Time::addTestTime(10ms);
    }

    void TearDown() override {
        mixxx::Time::setTestMode(false);
    }

    static QHash<QString, TrackModel::SortColumnId> COLUMN_MAPPING;
};

QHash<QString, TrackModel::SortColumnId>
        ControllerLibraryColumnIDRegressionTest::COLUMN_MAPPING = {
                {"Artist", TrackModel::SortColumnId::Artist},
                {"Title", TrackModel::SortColumnId::Title},
                {"Album", TrackModel::SortColumnId::Album},
                {"Album Artist", TrackModel::SortColumnId::AlbumArtist},
                {"Year", TrackModel::SortColumnId::Year},
                {"Genre", TrackModel::SortColumnId::Genre},
                {"Composer", TrackModel::SortColumnId::Composer},
                {"Grouping", TrackModel::SortColumnId::Grouping},
                {"Track Number", TrackModel::SortColumnId::TrackNumber},
                {"File Type", TrackModel::SortColumnId::FileType},
                {"Native Location", TrackModel::SortColumnId::NativeLocation},
                {"Comment", TrackModel::SortColumnId::Comment},
                {"Duration", TrackModel::SortColumnId::Duration},
                {"Bitrate", TrackModel::SortColumnId::BitRate},
                {"BPM", TrackModel::SortColumnId::Bpm},
                {"Replay Gain", TrackModel::SortColumnId::ReplayGain},
                {"Datetime Added", TrackModel::SortColumnId::DateTimeAdded},
                {"Times Played", TrackModel::SortColumnId::TimesPlayed},
                {"Rating", TrackModel::SortColumnId::Rating},
                {"Key", TrackModel::SortColumnId::Key},
                // More mapping can be added here if needed.
                // NOTE: If some of the missing value are referenced in a
                // controller setting, test case will fail.
};

#ifdef __HID__
TEST_F(ControllerLibraryColumnIDRegressionTest, ensureS4MK3) {
    QDir systemMappingsPath = getTestDir().filePath(QStringLiteral("../../res/controllers/"));
    std::shared_ptr<LegacyControllerMapping> pMapping =
            LegacyControllerMappingFileHandler::loadMapping(
                    QFileInfo(systemMappingsPath.filePath(
                            QStringLiteral("Traktor Kontrol S4 MK3.hid.xml"))),
                    systemMappingsPath);
    ASSERT_TRUE(pMapping);
    auto settings = pMapping->getSettings();
    EXPECT_TRUE(!settings.isEmpty());

    const int expectedSettingCount = 6; // Number of settings using library count.
    int count = 0;
    for (const auto& setting : settings) {
        if (!setting->variableName().startsWith("librarySortableColumns")) {
            continue;
        }
        auto pEnum = std::dynamic_pointer_cast<LegacyControllerEnumSetting>(setting);
        EXPECT_TRUE(pEnum);
        for (const auto& opt : pEnum->options()) {
            EXPECT_EQ(static_cast<int>(COLUMN_MAPPING[opt.value]), opt.label.toInt());
        }
        count++;
    }
    EXPECT_EQ(count, expectedSettingCount);
}
#endif
