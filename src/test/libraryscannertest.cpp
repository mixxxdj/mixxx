#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "test/librarytest.h"

#include "library/scanner/libraryscanner.h"

class LibraryScannerTest : public LibraryTest {
  protected:
    LibraryScannerTest()
        : m_libraryScanner(dbConnectionPool(), collection(), config()) {
    }
    LibraryScanner m_libraryScanner;
};

TEST_F(LibraryScannerTest, ScannerRoundtrip) {
    // Normal flow:
    EXPECT_EQ(m_libraryScanner.m_state, LibraryScanner::IDLE);
    m_libraryScanner.changeScannerState(LibraryScanner::STARTING);
    EXPECT_EQ(m_libraryScanner.m_state, LibraryScanner::STARTING);
    m_libraryScanner.changeScannerState(LibraryScanner::SCANNING);
    EXPECT_EQ(m_libraryScanner.m_state, LibraryScanner::SCANNING);
    m_libraryScanner.changeScannerState(LibraryScanner::FINISHED);
    EXPECT_EQ(m_libraryScanner.m_state, LibraryScanner::IDLE);

    // No Tracks:
    EXPECT_EQ(m_libraryScanner.m_state, LibraryScanner::IDLE);
    m_libraryScanner.changeScannerState(LibraryScanner::STARTING);
    EXPECT_EQ(m_libraryScanner.m_state, LibraryScanner::STARTING);
    m_libraryScanner.changeScannerState(LibraryScanner::IDLE);
    EXPECT_EQ(m_libraryScanner.m_state, LibraryScanner::IDLE);

    // Cancel during scaning:
    EXPECT_EQ(m_libraryScanner.m_state, LibraryScanner::IDLE);
    m_libraryScanner.changeScannerState(LibraryScanner::STARTING);
    EXPECT_EQ(m_libraryScanner.m_state, LibraryScanner::STARTING);
    m_libraryScanner.changeScannerState(LibraryScanner::SCANNING);
    EXPECT_EQ(m_libraryScanner.m_state, LibraryScanner::SCANNING);
    m_libraryScanner.changeScannerState(LibraryScanner::CANCELING);
    EXPECT_EQ(m_libraryScanner.m_state, LibraryScanner::CANCELING);
    m_libraryScanner.changeScannerState(LibraryScanner::FINISHED);
    EXPECT_EQ(m_libraryScanner.m_state, LibraryScanner::IDLE);
    m_libraryScanner.changeScannerState(LibraryScanner::IDLE);
    EXPECT_EQ(m_libraryScanner.m_state, LibraryScanner::IDLE);

    // restart during canceling :
    EXPECT_EQ(m_libraryScanner.m_state, LibraryScanner::IDLE);
    m_libraryScanner.changeScannerState(LibraryScanner::CANCELING);
    EXPECT_EQ(m_libraryScanner.m_state, LibraryScanner::CANCELING);
    m_libraryScanner.changeScannerState(LibraryScanner::STARTING);
    EXPECT_EQ(m_libraryScanner.m_state, LibraryScanner::CANCELING);
    m_libraryScanner.changeScannerState(LibraryScanner::IDLE);
    EXPECT_EQ(m_libraryScanner.m_state, LibraryScanner::IDLE);
}
