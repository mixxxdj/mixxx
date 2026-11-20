#include "widget/wsearchlineedit.h"

#include <gtest/gtest.h>

#include <QSignalSpy>
#include <QTest>

#include "test/mixxxtest.h"

class WSearchLineEditTest : public MixxxTest {
  protected:
    void SetUp() override {
        WSearchLineEdit::setDebouncingTimeoutMillis(
                WSearchLineEdit::kMinDebouncingTimeoutMillis);
        m_pSearch = std::make_unique<WSearchLineEdit>(nullptr);
        // Enable the search box like a library feature does
        m_pSearch->slotRestoreSearch(QString());
    }

    void TearDown() override {
        m_pSearch.reset();
        WSearchLineEdit::setDebouncingTimeoutMillis(
                WSearchLineEdit::kDefaultDebouncingTimeoutMillis);
    }

    // Waits until a pending debounced search would have been emitted
    static void waitForDebouncedSearch() {
        QTest::qWait(WSearchLineEdit::kMinDebouncingTimeoutMillis * 3);
    }

    // The query is saved in the history kSaveTimeoutMillis after typing
    void saveSearch() {
        ASSERT_TRUE(QMetaObject::invokeMethod(m_pSearch.get(), "slotSaveSearch"));
    }

    std::unique_ptr<WSearchLineEdit> m_pSearch;
};

TEST_F(WSearchLineEditTest, SavingQueryWithTrailingSpaceDoesNotSearchAgain) {
    QSignalSpy searchSpy(m_pSearch.get(), &WSearchLineEdit::search);
    m_pSearch->setCurrentText(QStringLiteral("-stems "));
    waitForDebouncedSearch();
    ASSERT_EQ(1, searchSpy.count());
    EXPECT_EQ(QStringLiteral("-stems "), searchSpy.takeFirst().front().toString());

    saveSearch();
    waitForDebouncedSearch();

    // The history entry is trimmed, the edited text is kept
    EXPECT_EQ(QStringLiteral("-stems"), m_pSearch->itemText(0));
    EXPECT_EQ(QStringLiteral("-stems "), m_pSearch->currentText());
    // Saving must not trigger another search, which would restore the
    // selected track in the library view and scroll to it, and save
    // the query again after kSaveTimeoutMillis, endlessly.
    EXPECT_EQ(0, searchSpy.count());

    // Saving the same query again doesn't search either
    saveSearch();
    waitForDebouncedSearch();
    EXPECT_EQ(0, searchSpy.count());
    EXPECT_EQ(1, m_pSearch->count());
}

TEST_F(WSearchLineEditTest, SavingQueryWithoutTrailingSpaceDoesNotSearchAgain) {
    QSignalSpy searchSpy(m_pSearch.get(), &WSearchLineEdit::search);
    m_pSearch->setCurrentText(QStringLiteral("-stems"));
    waitForDebouncedSearch();
    ASSERT_EQ(1, searchSpy.count());
    searchSpy.clear();

    saveSearch();
    waitForDebouncedSearch();

    EXPECT_EQ(QStringLiteral("-stems"), m_pSearch->itemText(0));
    EXPECT_EQ(QStringLiteral("-stems"), m_pSearch->currentText());
    EXPECT_EQ(0, searchSpy.count());
}
