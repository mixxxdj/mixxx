#include <string>
#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "mixxxtest.h"
#include "library/scanner/libraryscanner.h"

class LibraryScannerTest : public MixxxTest {
  protected:
    virtual void SetUp() {
        m_pParent = new QWidget();
        m_pLibraryScanner = new LibraryScanner(m_pParent, NULL, config());
    }

    virtual void TearDown() {
        delete m_pLibraryScanner;
    }

    LibraryScanner* m_pLibraryScanner;
    QWidget* m_pParent;

    FRIEND_TEST(FooTest, BarReturnsZeroOnNull);
};

TEST_F(LibraryScannerTest, ScannerRoundtrip) {
    EXPECT_EQ(m_pLibraryScanner->m_state, LibraryScanner::IDLE);


    /*
    bool LibraryScanner::changeScannerState(ScannerState newState) {
        // Allowed State transitions:
        // IDLE -> STARTING
        // STARTING -> IDLE
        // STARTING -> SCANNING
        // SCANNING -> FINISHED
        // FINISHED -> IDLE
        // every state can change to CANCELING
        // CANCELING -> IDLE
        switch (newState) {
        case IDLE:
            // we are leaving STARTING  or CANCELING state
            // m_state is already IDLE if a scan was canceled
            m_state = IDLE;
            m_stateMutex.unlock();
            return true;
        case STARTING:
            // we need to lock the mutex during the STARTING state
            // to prevent loosing cancel commands or start the scanner
            // twice
            if (m_stateMutex.tryLock()) {
                if (m_state != IDLE) {
                    qDebug() << "LibraryScanner: Scan already in progress.";
                    m_stateMutex.unlock();
                    return false;
                }
                m_state = STARTING;
                return true;
            } else {
                qDebug() << "LibraryScanner: mutex locked, state =" << m_state;
                return false;
            }
        case SCANNING:
            DEBUG_ASSERT(m_state == STARTING);
            // Transition protected by the mutex is over now
            // Allow canceling
            m_state = SCANNING;
            m_stateMutex.unlock();
            return true;
        case CANCELING:
            // canceling is always possible, but wait
            // until there is no scan starting.
            m_stateMutex.lock();
            m_state = CANCELING;
            return true;
        case FINISHED:
            // we must not lock the mutex here, because
            // the mutex is already locked in case we
            // are canceling.
            // There is no race condition, since the state
            // is set to IDLE after canceling as well
            m_state = IDLE;
            return true;
        default:
            DEBUG_ASSERT(false);
            return false;
        }
        */

}

