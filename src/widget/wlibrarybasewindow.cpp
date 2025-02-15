#include "widget/wlibrarybasewindow.h"

#include <QKeyEvent>
#include <QtDebug>

#include "library/libraryview.h"
#include "moc_wlibrarybasewindow.cpp"
#include "skin/legacy/skincontext.h"
#include "util/math.h"
#include "widget/wlibrary.h"
#include "widget/wlibrarypreparationwindow.h"
#include "widget/wtracktableview.h"

namespace {
const bool sDebug = false;
const QString windowName = QStringLiteral("[LIBRARYBASEWINDOW]");
} // namespace

WLibraryBaseWindow::WLibraryBaseWindow(
        QWidget* parent)
        : QStackedWidget(parent),
          // WBaseWidget(this),
          WBaseWidget(parent),
          m_callingParent("WLibraryBaseWindow"), // Default value for safety
          m_mutex(QT_RECURSIVE_MUTEX_INIT),
          m_trackTableBackgroundColorOpacity(kDefaultTrackTableBackgroundColorOpacity),
          m_bShowButtonText(true) {
    if (sDebug) {
        qDebug() << "Parent class set to:" << m_callingParent;
    }
}

void WLibraryBaseWindow::setup(const QDomNode& node, const SkinContext& context) {
    m_bShowButtonText =
            context.selectBool(
                    node,
                    "ShowButtonText",
                    true);
    m_trackTableBackgroundColorOpacity = math_clamp(
            context.selectDouble(
                    node,
                    "TrackTableBackgroundColorOpacity",
                    kDefaultTrackTableBackgroundColorOpacity),
            kMinTrackTableBackgroundColorOpacity,
            kMaxTrackTableBackgroundColorOpacity);
}

bool WLibraryBaseWindow::registerView(const QString& name, QWidget* pView) {
    if (sDebug) {
        qDebug() << windowName << " -> registerView toggled for " << name << " & " << pView;
    }
    if (m_callingParent == QStringLiteral("WLibraryPreparationWindow")) {
        // Do nothing for WLibraryPreparationWindow
        if (sDebug) {
            qDebug() << "[WLibraryPreparationWindow] -> Registering view in "
                        "PreparationWindow -> NO ACTION";
        }
        return false;
    } else {
        // Execute for WLibraryPreparationWindow
        if (sDebug) {
            qDebug() << "[WLibrary] -> Registering view in Library -> ACTION: " << name;
        }
        const auto lock = lockMutex(&m_mutex);
        if (m_viewMap.contains(name)) {
            return false;
        }
        if (dynamic_cast<LibraryView*>(pView) == nullptr) {
            if (sDebug) {
                qDebug() << "WARNING: Attempted to register view" << name
                         << "with WLibrary which does not implement the "
                            "LibraryView interface. Ignoring.";
            }
            return false;
        }
        addWidget(pView);
        m_viewMap[name] = pView;
        return true;
    }
}

bool WLibraryBaseWindow::registerViewInPreparationWindow(const QString& name, QWidget* pView) {
    if (sDebug) {
        qDebug() << windowName
                 << " -> registerViewInPreparationWindow toggled for " << name
                 << " & " << pView;
    }
    if (m_callingParent == QStringLiteral("WLibraryPreparationWindow")) {
        // Execute for WLibraryPreparationWindow
        if (sDebug) {
            qDebug() << "[WLibraryPreparationWindow] -> Registering view in "
                        "LibraryPreparationWindow -> ACTION: "
                     << name;
        }
        const auto lock = lockMutex(&m_mutex);
        if (m_viewMap.contains(name)) {
            return false;
        }
        if (dynamic_cast<LibraryView*>(pView) == nullptr) {
            if (sDebug) {
                qDebug() << "WARNING: Attempted to register view" << name
                         << "with PreparationWindow which does not implement "
                            "the LibraryView interface. Ignoring.";
            }
            return false;
        }
        addWidget(pView);
        m_viewMap[name] = pView;
        return true;
    } else {
        // Do nothing for WLibrary
        if (sDebug) {
            qDebug() << "[WLibrary] -> Registering view in WLibrary -> NO ACTION";
        }
        return false; // As an example, you could disable this function in PreparationWindow
    }
}

void WLibraryBaseWindow::switchToView(const QString& name) {
    if (sDebug) {
        qDebug() << windowName << " -> switchToView toggled for " << name;
    }
    if (m_callingParent == QStringLiteral("WLibraryPreparationWindow")) {
        // Do nothing for WLibraryPreparationWindow
        if (sDebug) {
            qDebug() << "[WLibraryPreparationWindow] -> Cannot switch to view "
                        "in PreparationWindow -> NO ACTION";
        }
    } else {
        // Execute for WLibrary
        if (sDebug) {
            qDebug() << "[WLibrary] -> Switching to view in Library -> ACTION: " << name;
        }
        const auto lock = lockMutex(&m_mutex);
        LibraryView* pOldLibraryView = dynamic_cast<LibraryView*>(currentWidget());
        QWidget* pWidget = m_viewMap.value(name, nullptr);
        if (pWidget != nullptr) {
            LibraryView* pLibraryView = dynamic_cast<LibraryView*>(pWidget);
            if (pLibraryView == nullptr) {
                if (sDebug) {
                    qDebug() << "WARNING: Attempted to switch to view" << name
                             << "with WLibrary which does not implement the "
                                "LibraryView interface. Ignoring.";
                }
                return;
            }
            if (currentWidget() != pWidget) {
                if (pOldLibraryView) {
                    pOldLibraryView->saveCurrentViewState();
                }
                setCurrentWidget(pWidget);
                pLibraryView->onShow();
                pLibraryView->restoreCurrentViewState();
            }
        } else {
            if (sDebug) {
                qDebug() << "[WLibrary] -> switchToViex -> pWidget " << pWidget << " = nullptr";
            }
        }
    }
}

void WLibraryBaseWindow::switchToViewInPreparationWindow(const QString& name) {
    if (sDebug) {
        qDebug() << windowName << " -> switchToViewInPreparationWindow toggled for " << name;
    }
    if (m_callingParent == QStringLiteral("WLibraryPreparationWindow")) {
        // Execute for WLibraryPreparationWindow
        if (sDebug) {
            qDebug() << "[WLibraryPreparationWindow] -> Switching to view in "
                        "PreparationWindow -> ACTION: "
                     << name;
        }
        const auto lock = lockMutex(&m_mutex);
        LibraryView* pOldLibraryView = dynamic_cast<LibraryView*>(currentWidget());
        QWidget* pWidget = m_viewMap.value(name, nullptr);
        if (pWidget != nullptr) {
            LibraryView* pLibraryView = dynamic_cast<LibraryView*>(pWidget);
            if (pLibraryView == nullptr) {
                if (sDebug) {
                    qDebug() << "WARNING: Attempted to switch to view" << name
                             << "with WLibrary which does not implement the "
                                "LibraryView interface. Ignoring.";
                }
                return;
            }
            if (currentWidget() != pWidget) {
                if (pOldLibraryView) {
                    pOldLibraryView->saveCurrentViewState();
                }
                setCurrentWidget(pWidget);
                pLibraryView->onShow();
                pLibraryView->restoreCurrentViewState();
            }
        } else {
            if (sDebug) {
                qDebug() << "[WLibraryPreparationWindow] -> "
                            "switchToViewInPreparationWindow -> pWidget "
                         << pWidget << " = nullptr";
            }
        }
    } else {
        // Do nothing for WLibrary
        if (sDebug) {
            qDebug() << "[WLibrary] -> Cannot switch to view in Library -> NO ACTION";
        }
    }
}

void WLibraryBaseWindow::pasteFromSidebar() {
    if (sDebug) {
        qDebug() << windowName << " -> pasteFromSidebar toggled";
    }
    if (m_callingParent == QStringLiteral("WLibraryPreparationWindow")) {
        // Do nothing for WLibraryPreparationWindow
        if (sDebug) {
            qDebug() << "[WLibraryPreparationWindow] -> Pasting from Sidebar "
                        "in PreparationWindow -> NO ACTION";
        }
    } else {
        // Execute for WLibrary
        if (sDebug) {
            qDebug() << "[WLibrary] -> Pasting from Sidebar in Library -> ACTION";
        }
        QWidget* pCurrent = currentWidget();
        LibraryView* pView = dynamic_cast<LibraryView*>(pCurrent);
        if (pView) {
            pView->pasteFromSidebar();
        }
    }
}

void WLibraryBaseWindow::pasteFromSidebarInPreparationWindow() {
    if (sDebug) {
        qDebug() << windowName << " -> pasteFromSidebarInPreparationWindow toggled";
    }
    if (m_callingParent == QStringLiteral("WLibraryPreparationWindow")) {
        // Execute for WLibraryPreparationWindow
        if (sDebug) {
            qDebug() << "[WLibraryPreparationWindow] -> Pasting from Sidebar in Library -> ACTION";
        }
        QWidget* pCurrent = currentWidget();
        LibraryView* pView = dynamic_cast<LibraryView*>(pCurrent);
        if (pView) {
            pView->pasteFromSidebar();
        }
    } else {
        // Do nothing for WLibrary
        if (sDebug) {
            qDebug() << "[WLibrary] -> Pasting from Sidebar in PreparationWindow -> NO ACTION";
        }
    }
}

void WLibraryBaseWindow::search(const QString& name) {
    if (sDebug) {
        qDebug() << windowName << " -> search toggled for " << name;
    }
    if (m_callingParent == QStringLiteral("WLibraryPreparationWindow")) {
        // Do nothing for WLibraryPreparationWindow
        if (sDebug) {
            qDebug() << "[WLibraryPreparationWindow] -> Search in PreparationWindow -> NO ACTION";
        }
    } else {
        // Execute for WLibrary
        if (sDebug) {
            qDebug() << "[WLibrary] -> Search in Library -> ACTION";
        }
        auto lock = lockMutex(&m_mutex);
        QWidget* pCurrent = currentWidget();
        LibraryView* pView = dynamic_cast<LibraryView*>(pCurrent);
        if (pView == nullptr) {
            if (sDebug) {
                qDebug() << "WARNING: Attempted to search in view" << name << "with WLibrary "
                         << "which does not implement the LibraryView interface. Ignoring.";
            }
            return;
        }
        lock.unlock();
        pView->onSearch(name);
    }
}

void WLibraryBaseWindow::searchInPreparationWindow(const QString& name) {
    if (sDebug) {
        qDebug() << windowName << " -> searchInPreparationWindow toggled for " << name;
    }
    if (m_callingParent == QStringLiteral("WLibraryPreparationWindow")) {
        // Execute for WLibraryPreparationWindow
        if (sDebug) {
            qDebug() << "[WLibraryPreparationWindow] -> Search in PreparationWindow -> ACTION";
        }
        auto lock = lockMutex(&m_mutex);
        QWidget* pCurrent = currentWidget();
        LibraryView* pView = dynamic_cast<LibraryView*>(pCurrent);
        if (pView == nullptr) {
            if (sDebug) {
                qDebug() << "WARNING: Attempted to search in view" << name << "with WLibrary "
                         << "which does not implement the LibraryView interface. Ignoring.";
            }
            return;
        }
        lock.unlock();
        pView->onSearch(name);
    } else {
        // Do nothing for WLibrary
        if (sDebug) {
            qDebug() << "[WLibrary] -> Search in Library -> NO ACTION";
        }
    }
}

LibraryView* WLibraryBaseWindow::getActiveView() const {
    return dynamic_cast<LibraryView*>(currentWidget());
}

WTrackTableView* WLibraryBaseWindow::getCurrentTrackTableView() const {
    qDebug() << windowName << " -> getCurrentTrackTableView toggled";
    QWidget* pCurrent = currentWidget();
    WTrackTableView* pTracksView = qobject_cast<WTrackTableView*>(pCurrent);
    if (!pTracksView) {
        // This view is not a tracks view, but possibly a special library view
        // with a controls row and a track view (DlgAutoDJ, DlgRecording etc.)?
        pTracksView = pCurrent->findChild<WTrackTableView*>();
    }
    return pTracksView; // might still be nullptr
}

// Eve -> please leave for eventual later work
//
//  WTrackTableView* WLibraryBaseWindow::getCurrentTrackTableViewInPreparationWindow() const {
//      qDebug() << windowName << " -> getCurrentTrackTableViewInPreparationWindow toggled";
//      QWidget* pCurrent = currentWidget();
//      WTrackTableView* pTracksView = qobject_cast<WTrackTableView*>(pCurrent);
//      if (!pTracksView) {
//          // This view is not a tracks view, but possibly a special library view
//          // with a controls row and a track view (DlgAutoDJ, DlgRecording etc.)?
//          pTracksView = pCurrent->findChild<WTrackTableView*>();
//      }
//      return pTracksView; // might still be nullptr
//  }

bool WLibraryBaseWindow::isTrackInCurrentView(const TrackId& trackId) {
    if (sDebug) {
        qDebug() << windowName << " -> isTrackInCurrentView toggled for " << trackId;
    }
    if (m_callingParent == QStringLiteral("WLibraryPreparationWindow")) {
        // Do nothing for WLibraryPreparationWindow
        if (sDebug) {
            qDebug() << "[WLibraryPreparationWindow] -> isTrackInCurrentView "
                        "in WLibraryPreparationWindow -> NO ACTION";
        }
        return false;
    } else {
        // Execute for WLibrary
        if (sDebug) {
            qDebug() << "[WLibrary] -> isTrackInCurrentView in WLibrary -> ACTION" << trackId;
        }
        VERIFY_OR_DEBUG_ASSERT(trackId.isValid()) {
            return false;
        }
        WTrackTableView* pTracksView = getCurrentTrackTableView();
        if (!pTracksView) {
            return false;
        }
        return pTracksView->isTrackInCurrentView(trackId);
    }
}

bool WLibraryBaseWindow::isTrackInCurrentViewInPreparationWindow(const TrackId& trackId) {
    if (sDebug) {
        qDebug() << windowName
                 << " -> isTrackInCurrentViewInPreparationWindow toggled for "
                 << trackId;
    }
    if (m_callingParent == QStringLiteral("WLibraryPreparationWindow")) {
        // Execute for WLibraryPreparationWindow
        if (sDebug) {
            qDebug() << "[WLibraryPreparationWindow] -> "
                        "isTrackInCurrentViewInPreparationWindow in "
                        "WLibraryPreparationWindow -> ACTION"
                     << trackId;
        }
        VERIFY_OR_DEBUG_ASSERT(trackId.isValid()) {
            return false;
        }
        WTrackTableView* pTracksView = getCurrentTrackTableView();
        if (!pTracksView) {
            return false;
        }
        return pTracksView->isTrackInCurrentView(trackId);
    } else {
        // Do nothing for WLibrary
        if (sDebug) {
            qDebug() << "[WLibrary] -> isTrackInCurrentViewInPreparationWindow "
                        "in WLibrary -> NO ACTION";
        }
        return false;
    }
}

void WLibraryBaseWindow::slotSelectTrackInActiveTrackView(const TrackId& trackId) {
    if (sDebug) {
        qDebug() << windowName << " -> slotSelectTrackInActiveTrackView toggled for " << trackId;
    }
    if (m_callingParent == QStringLiteral("WLibraryPreparationWindow")) {
        // Do nothing for WLibraryPreparationWindow
        if (sDebug) {
            qDebug() << "[WLibraryPreparationWindow] -> "
                        "slotSelectTrackInActiveTrackView in PreparationWindow "
                        "-> NO ACTION";
        }
    } else {
        // Execute for WLibrary
        if (sDebug) {
            qDebug() << "[WLibrary] -> slotSelectTrackInActiveTrackView in "
                        "Library -> ACTION"
                     << trackId;
        }
        if (!trackId.isValid()) {
            return;
        }
        WTrackTableView* pTracksView = getCurrentTrackTableView();
        if (!pTracksView) {
            return;
        }
        pTracksView->selectTrack(trackId);
    }
}

void WLibraryBaseWindow::slotSelectTrackInActiveTrackViewInPreparationWindow(
        const TrackId& trackId) {
    if (sDebug) {
        qDebug() << windowName
                 << " -> slotSelectTrackInActiveTrackViewInPreparationWindow "
                    "toggled for "
                 << trackId;
    }
    if (m_callingParent == QStringLiteral("WLibraryPreparationWindow")) {
        // Execute for WLibraryPreparationWindow
        if (sDebug) {
            qDebug() << "[WLibraryPreparationWindow] -> "
                        "slotSelectTrackInActiveTrackView -> ACTION"
                     << trackId;
        }
        if (!trackId.isValid()) {
            return;
        }
        WTrackTableView* pTracksView = getCurrentTrackTableView();
        if (!pTracksView) {
            return;
        }
        pTracksView->selectTrack(trackId);
    } else {
        // Do nothing for WLibrary
        if (sDebug) {
            qDebug() << "[WLibrary] ->  slotSelectTrackInActiveTrackView -> NO ACTION";
        }
    }
}

void WLibraryBaseWindow::saveCurrentViewState() const {
    if (sDebug) {
        qDebug() << windowName << " -> saveCurrentViewStatein toggled";
    }
    if (m_callingParent == QStringLiteral("WLibraryPreparationWindow")) {
        // Do nothing for WLibraryPreparationWindow
        if (sDebug) {
            qDebug() << "[WLibraryPreparationWindow] -> saveCurrentViewState "
                        "in PreparationWindow->NO ACTION ";
        }
    } else {
        // Execute for WLibrary
        if (sDebug) {
            qDebug() << "[WLibrary] -> saveCurrentViewState in Library->ACTION ";
        }
        WTrackTableView* pTracksView =
                getCurrentTrackTableView();
        if (!pTracksView) {
            return;
        }
        pTracksView->slotSaveCurrentViewState();
    }
}

void WLibraryBaseWindow::saveCurrentViewStateInPreparationWindow() const {
    if (sDebug) {
        qDebug() << windowName << " -> saveCurrentViewStateInPreparationWindow toggled";
    }
    // if (qobject_cast<WLibraryPreparationWindow*>(parent())) {
    if (m_callingParent == QStringLiteral("WLibraryPreparationWindow")) {
        // Execute for WLibraryPreparationWindow
        if (sDebug) {
            qDebug() << "[WLibraryPreparationWindow] -> saveCurrentViewState "
                        "in PreparationWindow -> ACTION";
        }
        WTrackTableView* pTracksView =
                getCurrentTrackTableView();
        if (!pTracksView) {
            return;
        }
        pTracksView->slotSaveCurrentViewState();
    } else {
        // Do nothing for WLibrary
        if (sDebug) {
            qDebug() << "[WLibrary] -> saveCurrentViewState in Library -> NO ACTION";
        }
    }
}

void WLibraryBaseWindow::restoreCurrentViewState() const {
    if (sDebug) {
        qDebug() << windowName << " -> restoreCurrentViewState toggled";
    }
    if (m_callingParent == QStringLiteral("WLibraryPreparationWindow")) {
        // Do nothing for WLibraryPreparationWindow
        if (sDebug) {
            qDebug()
                    << "[WLibraryPreparationWindow] -> restoreCurrentViewState "
                       "in PreparationWindow->NO ACTION ";
        }
    } else {
        // Execute for WLibrary
        if (sDebug) {
            qDebug() << "[WLibrary] -> restoreCurrentViewState in Library->ACTION ";
        }
        WTrackTableView* pTracksView =
                getCurrentTrackTableView();
        if (!pTracksView) {
            return;
        }
        pTracksView->slotRestoreCurrentViewState();
    }
}

void WLibraryBaseWindow::restoreCurrentViewStateInPreparationWindow() const {
    if (sDebug) {
        qDebug() << windowName << " -> restoreCurrentViewStateInPreparationWindow toggled";
    }
    // if (qobject_cast<WLibraryPreparationWindow*>(parent())) {
    if (m_callingParent == QStringLiteral("WLibraryPreparationWindow")) {
        if (sDebug) {
            qDebug() << "[WLibraryPreparationWindow] -> "
                        "restoreCurrentViewState in PreparationWindow->ACTION ";
        }
        // Execute for WLibraryPreparationWindow
        WTrackTableView* pTracksView =
                getCurrentTrackTableView();
        if (!pTracksView) {
            return;
        }
        pTracksView->slotRestoreCurrentViewState();
    } else {
        // Do nothing for WLibrary
        if (sDebug) {
            qDebug() << "[WLibrary] -> restoreCurrentViewState in Library->NO ACTION ";
        }
    }
}

bool WLibraryBaseWindow::event(QEvent* pEvent) {
    if (sDebug) {
        qDebug() << windowName << " -> event toggled for " << pEvent;
    }
    if (pEvent->type() == QEvent::ToolTip) {
        updateTooltip();
    }
    return QStackedWidget::event(pEvent);
}

// bool WLibraryBaseWindow::eventInPreparationWindow(QEvent* pEvent) {
//     qDebug() << windowName << " -> eventInPreparationWindow toggled";
//     if (pEvent->type() == QEvent::ToolTip) {
//         updateTooltip();
//     }
//     return QStackedWidget::event(pEvent);
// }

void WLibraryBaseWindow::keyPressEvent(QKeyEvent* pEvent) {
    if (sDebug) {
        qDebug() << windowName << " -> keyPressEvent toggled for " << pEvent;
    }
    if (pEvent->key() == Qt::Key_Left && pEvent->modifiers() & Qt::ControlModifier) {
        emit setLibraryFocus(FocusWidget::Sidebar);
    }
    QStackedWidget::keyPressEvent(pEvent);
}

// void WLibraryBaseWindow::keyPressEventInPreparationWindow(QKeyEvent* pEvent) {
//     qDebug() << windowName << " -> keyPressEventInPreparationWindow toggled";
//     if (pEvent->key() == Qt::Key_Left && pEvent->modifiers() & Qt::ControlModifier) {
//         emit setLibraryFocus(FocusWidget::Sidebar);
//     }
//     QStackedWidget::keyPressEvent(pEvent);
// }
