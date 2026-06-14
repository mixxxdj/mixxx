#include <gtest/gtest.h>

#include <QVector>

#include "audio/frame.h"
#include "track/phrases.h"

using namespace mixxx;

namespace {

Phrase makePhrase(double start, double end, PhraseType type) {
    return Phrase(audio::FramePos(start), audio::FramePos(end), type);
}

TEST(PhrasesTest, ConstructorSortsByStart) {
    QVector<Phrase> input;
    input.append(makePhrase(200, 300, PhraseType::Verse));
    input.append(makePhrase(0, 100, PhraseType::Intro));
    Phrases phrases(std::move(input));
    ASSERT_EQ(phrases.size(), 2);
    EXPECT_EQ(phrases.phrases().at(0).type(), PhraseType::Intro);
    EXPECT_EQ(phrases.phrases().at(1).type(), PhraseType::Verse);
}

TEST(PhrasesTest, TryAddPhraseSucceeds) {
    Phrases phrases;
    const auto result = phrases.tryAddPhrase(makePhrase(0, 100, PhraseType::Intro));
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ((*result)->size(), 1);
    // The original instance is immutable and unchanged.
    EXPECT_TRUE(phrases.isEmpty());
}

TEST(PhrasesTest, TryAddPhraseRejectsOverlapButAllowsAdjacent) {
    const Phrases phrases(QVector<Phrase>{makePhrase(0, 100, PhraseType::Intro)});
    EXPECT_FALSE(phrases.tryAddPhrase(makePhrase(50, 150, PhraseType::Verse)).has_value());
    EXPECT_TRUE(phrases.tryAddPhrase(makePhrase(100, 200, PhraseType::Verse)).has_value());
}

TEST(PhrasesTest, TryRemovePhrase) {
    const Phrases phrases(QVector<Phrase>{
            makePhrase(0, 100, PhraseType::Intro),
            makePhrase(100, 200, PhraseType::Verse)});
    const auto result = phrases.tryRemovePhrase(0);
    ASSERT_TRUE(result.has_value());
    ASSERT_EQ((*result)->size(), 1);
    EXPECT_EQ((*result)->phrases().at(0).type(), PhraseType::Verse);
    EXPECT_FALSE(phrases.tryRemovePhrase(5).has_value());
    EXPECT_FALSE(phrases.tryRemovePhrase(-1).has_value());
}

TEST(PhrasesTest, TrySetPhraseType) {
    const Phrases phrases(QVector<Phrase>{makePhrase(0, 100, PhraseType::Intro)});
    const auto result = phrases.trySetPhraseType(0, PhraseType::Drop);
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ((*result)->phrases().at(0).type(), PhraseType::Drop);
    EXPECT_FALSE(phrases.trySetPhraseType(3, PhraseType::Drop).has_value());
}

TEST(PhrasesTest, FindPhraseAtPosition) {
    const Phrases phrases(QVector<Phrase>{
            makePhrase(0, 100, PhraseType::Intro),
            makePhrase(100, 200, PhraseType::Verse)});
    const Phrase* pIntro = phrases.findPhraseAtPosition(audio::FramePos(50));
    ASSERT_NE(pIntro, nullptr);
    EXPECT_EQ(pIntro->type(), PhraseType::Intro);
    // Start is inclusive, end exclusive: position 100 belongs to the second phrase.
    const Phrase* pBoundary = phrases.findPhraseAtPosition(audio::FramePos(100));
    ASSERT_NE(pBoundary, nullptr);
    EXPECT_EQ(pBoundary->type(), PhraseType::Verse);
    EXPECT_EQ(phrases.findPhraseAtPosition(audio::FramePos(500)), nullptr);
}

TEST(PhrasesTest, ProtobufRoundTrip) {
    const Phrases original(QVector<Phrase>{
            makePhrase(0, 12345, PhraseType::Intro),
            Phrase(audio::FramePos(12345),
                    audio::FramePos(20000),
                    PhraseType::Drop,
                    QStringLiteral("My Drop"))});
    const QByteArray data = original.toByteArray();
    const PhrasesPointer restored = Phrases::fromByteArray(data);
    ASSERT_NE(restored, nullptr);
    ASSERT_EQ(restored->size(), 2);
    EXPECT_EQ(*restored, original);
    EXPECT_EQ(restored->phrases().at(1).label(), QStringLiteral("My Drop"));
    EXPECT_EQ(restored->phrases().at(1).type(), PhraseType::Drop);
}

TEST(PhrasesTest, EmptyRoundTrip) {
    const Phrases empty;
    EXPECT_TRUE(empty.isEmpty());
    const PhrasesPointer restored = Phrases::fromByteArray(empty.toByteArray());
    ASSERT_NE(restored, nullptr);
    EXPECT_TRUE(restored->isEmpty());
}

} // namespace
