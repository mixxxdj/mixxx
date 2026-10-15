#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>

#include "ai/aimixplanner.h"

using ::testing::ElementsAre;

namespace {

using mixxx::ai::AiCandidateTrack;
using mixxx::ai::AiMixPlanner;
using mixxx::ai::AiMixSettings;
using mixxx::ai::extractServiceErrorMessage;

/// Builds a minimal chat completions response carrying the given content.
QByteArray chatCompletion(const QString& content) {
    QJsonObject message;
    message.insert(QStringLiteral("content"), content);
    QJsonObject choice;
    choice.insert(QStringLiteral("message"), message);
    QJsonArray choices;
    choices.append(choice);
    QJsonObject root;
    root.insert(QStringLiteral("choices"), choices);
    return QJsonDocument(root).toJson(QJsonDocument::Compact);
}

QByteArray candidateTracks(int count) {
    QJsonArray tracks;
    for (int i = 0; i < count; ++i) {
        AiCandidateTrack track;
        track.index = i;
        track.title = QStringLiteral("Track %1").arg(i);
        tracks.append(track.toJson());
    }
    return QJsonDocument(tracks).toJson(QJsonDocument::Compact);
}

class AiMixPlannerTest : public testing::Test {
};

TEST_F(AiMixPlannerTest, ParsesAPlainJsonArray) {
    QString error;
    const QList<int> order = AiMixPlanner::parseTrackOrder(
            chatCompletion(QStringLiteral("[12, 4, 27]")), 30, &error);
    EXPECT_TRUE(error.isEmpty());
    EXPECT_THAT(order, ElementsAre(12, 4, 27));
}

TEST_F(AiMixPlannerTest, ParsesAnArrayWrappedInProseAndCodeFence) {
    QString error;
    const QList<int> order = AiMixPlanner::parseTrackOrder(
            chatCompletion(QStringLiteral("Sure!\n```json\n[3, 1]\n```\nEnjoy.")),
            10,
            &error);
    EXPECT_TRUE(error.isEmpty());
    EXPECT_THAT(order, ElementsAre(3, 1));
}

TEST_F(AiMixPlannerTest, ParsesAnObjectContainingTheTrackArray) {
    QString error;
    const QList<int> order = AiMixPlanner::parseTrackOrder(
            chatCompletion(QStringLiteral("{\"tracks\": [5, 2]}")), 10, &error);
    EXPECT_TRUE(error.isEmpty());
    EXPECT_THAT(order, ElementsAre(5, 2));
}

TEST_F(AiMixPlannerTest, DropsOutOfRangeAndDuplicateIndices) {
    QString error;
    const QList<int> order = AiMixPlanner::parseTrackOrder(
            chatCompletion(QStringLiteral("[2, 2, 99, -1, 0]")), 3, &error);
    EXPECT_TRUE(error.isEmpty());
    // 99 and -1 are outside the candidate list, the second 2 is a repeat.
    EXPECT_THAT(order, ElementsAre(2, 0));
}

TEST_F(AiMixPlannerTest, ReportsTheMessageSentByTheService) {
    const QByteArray body = QByteArrayLiteral(
            "{\"error\":{\"message\":\"Invalid API key\"}}");
    QString error;
    EXPECT_TRUE(AiMixPlanner::parseTrackOrder(body, 10, &error).isEmpty());
    EXPECT_EQ(QStringLiteral("Invalid API key"), error);
}

TEST_F(AiMixPlannerTest, RejectsAnEmptySelection) {
    QString error;
    EXPECT_TRUE(AiMixPlanner::parseTrackOrder(
                            chatCompletion(QStringLiteral("[]")), 10, &error)
                    .isEmpty());
    EXPECT_FALSE(error.isEmpty());
}

TEST_F(AiMixPlannerTest, RejectsAResponseThatIsNotJson) {
    QString error;
    EXPECT_TRUE(AiMixPlanner::parseTrackOrder(
                            QByteArrayLiteral("<html>oops</html>"), 10, &error)
                    .isEmpty());
    EXPECT_FALSE(error.isEmpty());
}

TEST_F(AiMixPlannerTest, RejectsAnEmptyCompletion) {
    QString error;
    EXPECT_TRUE(AiMixPlanner::parseTrackOrder(chatCompletion(QString()), 10, &error).isEmpty());
    EXPECT_FALSE(error.isEmpty());
}

TEST_F(AiMixPlannerTest, RejectsACompletionWithoutAnyTrackList) {
    QString error;
    EXPECT_TRUE(AiMixPlanner::parseTrackOrder(
                            chatCompletion(QStringLiteral("I cannot help with that.")),
                            10,
                            &error)
                    .isEmpty());
    EXPECT_FALSE(error.isEmpty());
}

TEST_F(AiMixPlannerTest, ExtractsOnlyRealServiceErrors) {
    QString message;
    EXPECT_FALSE(extractServiceErrorMessage(
            QByteArrayLiteral("{\"choices\":[]}"), &message));
    EXPECT_FALSE(extractServiceErrorMessage(
            QByteArrayLiteral("{\"error\":\"plain string\"}"), &message));
    EXPECT_FALSE(extractServiceErrorMessage(QByteArrayLiteral("nonsense"), &message));
    EXPECT_TRUE(extractServiceErrorMessage(
            QByteArrayLiteral("{\"error\":{\"message\":\"Quota exceeded\"}}"), &message));
    EXPECT_EQ(QStringLiteral("Quota exceeded"), message);
}

TEST_F(AiMixPlannerTest, BuildsARequestPayloadCarryingModelInstructionAndCandidates) {
    AiMixSettings settings;
    settings.provider = AiMixSettings::Provider::DeepSeek;
    settings.model = QStringLiteral("deepseek-chat");

    const QJsonObject payload = AiMixPlanner::buildRequestPayload(
            settings,
            QStringLiteral("one hour of techno"),
            QJsonDocument::fromJson(candidateTracks(3)).array());

    EXPECT_EQ(QStringLiteral("deepseek-chat"),
            payload.value(QStringLiteral("model")).toString());
    EXPECT_FALSE(payload.value(QStringLiteral("stream")).toBool());

    const QJsonArray messages = payload.value(QStringLiteral("messages")).toArray();
    ASSERT_EQ(2, messages.size());
    EXPECT_EQ(QStringLiteral("system"),
            messages.at(0).toObject().value(QStringLiteral("role")).toString());
    EXPECT_EQ(QStringLiteral("user"),
            messages.at(1).toObject().value(QStringLiteral("role")).toString());

    const QString userContent = messages.at(1)
                                        .toObject()
                                        .value(QStringLiteral("content"))
                                        .toString();
    EXPECT_TRUE(userContent.contains(QStringLiteral("one hour of techno")));
    EXPECT_TRUE(userContent.contains(QStringLiteral("Track 0")));
    EXPECT_TRUE(userContent.contains(QStringLiteral("Track 2")));
}

TEST_F(AiMixPlannerTest, CandidateTracksCarryTheMetadataTheServiceNeeds) {
    AiCandidateTrack track;
    track.index = 7;
    track.title = QStringLiteral("Acid Rain");
    track.artist = QStringLiteral("Someone");
    track.genre = QStringLiteral("Techno");
    track.bpm = 138.5;
    track.key = QStringLiteral("8A");
    track.durationSeconds = 372;

    const QJsonObject json = track.toJson();
    EXPECT_EQ(7, json.value(QStringLiteral("index")).toInt());
    EXPECT_EQ(QStringLiteral("Acid Rain"), json.value(QStringLiteral("title")).toString());
    EXPECT_EQ(QStringLiteral("Techno"), json.value(QStringLiteral("genre")).toString());
    EXPECT_DOUBLE_EQ(138.5, json.value(QStringLiteral("bpm")).toDouble());
    EXPECT_EQ(QStringLiteral("8A"), json.value(QStringLiteral("key")).toString());
    EXPECT_EQ(372, json.value(QStringLiteral("duration")).toInt());
}

TEST_F(AiMixPlannerTest, BuildsTheChatCompletionsUrlFromTheConfiguredBaseUrl) {
    AiMixSettings settings;
    settings.baseUrl = QStringLiteral("https://api.openai.com/v1");
    EXPECT_EQ(QStringLiteral("https://api.openai.com/v1/chat/completions"),
            settings.chatCompletionsUrl().toString());

    // A trailing slash must not produce a double separator.
    settings.baseUrl = QStringLiteral("https://api.deepseek.com/v1/");
    EXPECT_EQ(QStringLiteral("https://api.deepseek.com/v1/chat/completions"),
            settings.chatCompletionsUrl().toString());
}

TEST_F(AiMixPlannerTest, ProviderDefaultsPointAtOpenAiCompatibleEndpoints) {
    EXPECT_EQ(QStringLiteral("https://api.deepseek.com/v1"),
            AiMixSettings::defaultBaseUrl(AiMixSettings::Provider::DeepSeek));
    EXPECT_EQ(QStringLiteral("https://api.openai.com/v1"),
            AiMixSettings::defaultBaseUrl(AiMixSettings::Provider::OpenAI));
    // The local default keeps the library metadata on the user's machine.
    EXPECT_EQ(QStringLiteral("http://localhost:11434/v1"),
            AiMixSettings::defaultBaseUrl(AiMixSettings::Provider::Custom));

    EXPECT_EQ(QStringLiteral("deepseek-chat"),
            AiMixSettings::defaultModel(AiMixSettings::Provider::DeepSeek));
    EXPECT_FALSE(AiMixSettings::defaultModel(AiMixSettings::Provider::OpenAI).isEmpty());

    EXPECT_EQ(QStringLiteral("DeepSeek"),
            AiMixSettings::providerId(AiMixSettings::Provider::DeepSeek));
    EXPECT_EQ(QStringLiteral("OpenAI"),
            AiMixSettings::providerId(AiMixSettings::Provider::OpenAI));
    EXPECT_EQ(QStringLiteral("Custom"),
            AiMixSettings::providerId(AiMixSettings::Provider::Custom));
}

} // namespace
