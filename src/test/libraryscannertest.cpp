#include <string>
#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "mixxxtest.h"
#include "library/scanner/libraryscanner.h"

class LibraryScannerTest : public MixxxTest {
  protected:
    virtual void SetUp() {
        m_pLibraryScanner = new LibraryScanner(NULL, config());
    }

    virtual void TearDown() {
        delete m_pLibraryScanner;
    }

    LibraryScanner* m_pLibraryScanner;

    FRIEND_TEST(FooTest, BarReturnsZeroOnNull);
};

TEST_F(LibraryScannerTest, ScannerRoundtrip) {
    // Normal flow:
    EXPECT_EQ(m_pLibraryScanner->m_state, LibraryScanner::IDLE);
    m_pLibraryScanner->changeScannerState(LibraryScanner::STARTING);
    EXPECT_EQ(m_pLibraryScanner->m_state, LibraryScanner::STARTING);
    m_pLibraryScanner->changeScannerState(LibraryScanner::SCANNING);
    EXPECT_EQ(m_pLibraryScanner->m_state, LibraryScanner::SCANNING);
    m_pLibraryScanner->changeScannerState(LibraryScanner::FINISHED);
    EXPECT_EQ(m_pLibraryScanner->m_state, LibraryScanner::IDLE);

    // No Tracks:
    EXPECT_EQ(m_pLibraryScanner->m_state, LibraryScanner::IDLE);
    m_pLibraryScanner->changeScannerState(LibraryScanner::STARTING);
    EXPECT_EQ(m_pLibraryScanner->m_state, LibraryScanner::STARTING);
    m_pLibraryScanner->changeScannerState(LibraryScanner::IDLE);
    EXPECT_EQ(m_pLibraryScanner->m_state, LibraryScanner::IDLE);

    // Cancel during scaning:
    EXPECT_EQ(m_pLibraryScanner->m_state, LibraryScanner::IDLE);
    m_pLibraryScanner->changeScannerState(LibraryScanner::STARTING);
    EXPECT_EQ(m_pLibraryScanner->m_state, LibraryScanner::STARTING);
    m_pLibraryScanner->changeScannerState(LibraryScanner::SCANNING);
    EXPECT_EQ(m_pLibraryScanner->m_state, LibraryScanner::SCANNING);
    m_pLibraryScanner->changeScannerState(LibraryScanner::CANCELING);
    EXPECT_EQ(m_pLibraryScanner->m_state, LibraryScanner::CANCELING);
    m_pLibraryScanner->changeScannerState(LibraryScanner::FINISHED);
    EXPECT_EQ(m_pLibraryScanner->m_state, LibraryScanner::IDLE);
    m_pLibraryScanner->changeScannerState(LibraryScanner::IDLE);
    EXPECT_EQ(m_pLibraryScanner->m_state, LibraryScanner::IDLE);

    // restart during canceling :
    EXPECT_EQ(m_pLibraryScanner->m_state, LibraryScanner::IDLE);
    m_pLibraryScanner->changeScannerState(LibraryScanner::CANCELING);
    EXPECT_EQ(m_pLibraryScanner->m_state, LibraryScanner::CANCELING);
    m_pLibraryScanner->changeScannerState(LibraryScanner::STARTING);
    EXPECT_EQ(m_pLibraryScanner->m_state, LibraryScanner::CANCELING);
    m_pLibraryScanner->changeScannerState(LibraryScanner::IDLE);
    EXPECT_EQ(m_pLibraryScanner->m_state, LibraryScanner::IDLE);
}
