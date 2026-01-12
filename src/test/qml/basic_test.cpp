#include <gtest/gtest.h>

#include "test/qml/servertest.h"

class UITest : public testing::Test {
  public:
    void SetUp() override {
        UI_TEST_ONLY
    }

    void TearDown() override {
        if (MixxxAppTest::instance == nullptr) {
            return;
        }
        MixxxAppTest::instance->wait(std::chrono::milliseconds(500));
        const auto& currentTest = testing::UnitTest::GetInstance()->current_test_info();

        MixxxAppTest::instance->takeScreenshot(spix::ItemPath("mainWindow"),
                std::string("./screenshots/") + currentTest->name() + ".png");
    }
};

TEST_F(UITest, LibraryFullScreen) {
    MixxxAppTest::instance->mouseClick(spix::ItemPath("mainWindow/library"));
    MixxxAppTest::instance->wait(std::chrono::milliseconds(250));
    MixxxAppTest::instance->mouseClick(spix::ItemPath("mainWindow/library"));
    MixxxAppTest::instance->wait(std::chrono::milliseconds(250));
}

TEST_F(UITest, Show4Decks) {
    MixxxAppTest::instance->mouseClick(spix::ItemPath("mainWindow/show4DecksButton"));
    MixxxAppTest::instance->wait(std::chrono::milliseconds(500));

    EXPECT_TRUE(MixxxAppTest::instance->existsAndVisible(
            spix::ItemPath("mainWindow/deck3waveform")));
    EXPECT_TRUE(MixxxAppTest::instance->existsAndVisible(
            spix::ItemPath("mainWindow/deck4waveform")));
    EXPECT_TRUE(MixxxAppTest::instance->existsAndVisible(spix::ItemPath("mainWindow/decks34")));

    MixxxAppTest::instance->mouseClick(spix::ItemPath("mainWindow/show4DecksButton"));
    MixxxAppTest::instance->wait(std::chrono::milliseconds(500));

    EXPECT_FALSE(MixxxAppTest::instance->existsAndVisible(
            spix::ItemPath("mainWindow/deck3waveform")));
    EXPECT_FALSE(MixxxAppTest::instance->existsAndVisible(
            spix::ItemPath("mainWindow/deck4waveform")));
    EXPECT_FALSE(MixxxAppTest::instance->existsAndVisible(spix::ItemPath("mainWindow/decks34")));
}
