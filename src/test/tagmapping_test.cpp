#include "tagging/tagmapping.h"

#include <gtest/gtest.h>

#include <QtDebug>

namespace mixxx {

class TagMappingTest : public testing::Test {
};

TEST_F(TagMappingTest, ExportTrack) {
    const auto config = TagMappingConfig(" - ", 0.5);

    TagVector tags = config.splitLabelIntoOrderedTags(
            "R&B/Soul - Pop - Hip-Hop/Rap - Rock'n'Roll - New Wave");

    EXPECT_EQ(5, tags.size());
    EXPECT_EQ(TagLabel("R&B/Soul"), tags[0].getLabel());
    EXPECT_EQ(1.0, tags[0].getScore());
    EXPECT_EQ(TagLabel("Pop"), tags[1].getLabel());
    EXPECT_EQ(0.5, tags[1].getScore());
    EXPECT_EQ(TagLabel("Hip-Hop/Rap"), tags[2].getLabel());
    EXPECT_EQ(0.25, tags[2].getScore());
    EXPECT_EQ(TagLabel("Rock'n'Roll"), tags[3].getLabel());
    EXPECT_EQ(0.125, tags[3].getScore());
    EXPECT_EQ(TagLabel("New Wave"), tags[4].getLabel());
    EXPECT_EQ(0.0625, tags[4].getScore());
}

TEST_F(TagMappingTest, ImportTrack) {
    auto config = TagMappingConfig(" ", 0.5);

    // Pre-ordered, the score is irrelevant
    TagVector tagsOrdered;
    tagsOrdered +=
            Tag(TagLabel("R&B/Soul"));
    tagsOrdered +=
            Tag(TagLabel("Pop"));
    tagsOrdered +=
            Tag(TagLabel("Hip-Hop/Rap"));
    tagsOrdered +=
            Tag(TagLabel("Rock'n'Roll"));
    tagsOrdered +=
            Tag(TagLabel("New Wave"));
    ASSERT_EQ(5, tagsOrdered.size());

    {
        // ambiguous result
        config.setLabelSeparator("-");
        const auto joinedLabel = config.joinLabelsOfOrderedTags(tagsOrdered);
        EXPECT_EQ(
                "R&B/Soul-Pop-Hip-Hop/Rap-Rock'n'Roll-New Wave",
                joinedLabel);
    }
    {
        // unambiguous result
        config.setLabelSeparator("  ");
        const auto joinedLabel = config.joinLabelsOfOrderedTags(tagsOrdered);
        EXPECT_EQ(
                "R&B/Soul  Pop  Hip-Hop/Rap  Rock'n'Roll  New Wave",
                joinedLabel);
        const auto splitTags = config.splitLabelIntoOrderedTags(joinedLabel);
        EXPECT_EQ(5, splitTags.size());
        EXPECT_EQ(TagLabel("R&B/Soul"), splitTags[0].getLabel());
        EXPECT_EQ(1.0, splitTags[0].getScore());
        EXPECT_EQ(TagLabel("Pop"), splitTags[1].getLabel());
        EXPECT_EQ(0.5, splitTags[1].getScore());
        EXPECT_EQ(TagLabel("Hip-Hop/Rap"), splitTags[2].getLabel());
        EXPECT_EQ(0.25, splitTags[2].getScore());
        EXPECT_EQ(TagLabel("Rock'n'Roll"), splitTags[3].getLabel());
        EXPECT_EQ(0.125, splitTags[3].getScore());
        EXPECT_EQ(TagLabel("New Wave"), splitTags[4].getLabel());
        EXPECT_EQ(0.0625, splitTags[4].getScore());
    }
}

} // namespace mixxx
