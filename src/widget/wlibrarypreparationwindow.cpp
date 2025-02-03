#include <QKeyEvent>
#include <QtDebug>

#include "library/libraryview.h"
#include "moc_wlibrarypreparationwindow.cpp"
#include "skin/legacy/skincontext.h"
#include "util/math.h"
#include "widget/wlibrary.h"
#include "widget/wpreparationwindowtracktableview.h"

WLibraryPreparationWindow::WLibraryPreparationWindow(QWidget* parent)
        : QStackedWidget(parent),
          WBaseWidget(this),
          m_mutex(QT_RECURSIVE_MUTEX_INIT),
          m_trackTableBackgroundColorOpacity(kDefaultTrackTableBackgroundColorOpacity),
          m_bShowButtonText(true) {
}

void WLibraryPreparationWindow::setup(const QDomNode& node, const SkinContext& context) {
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

bool WLibraryPreparationWindow::registerView(const QString& name, QWidget* pView) {
    // qDebug() << "WLibraryPreparationWindow::registerView" << name;
    const auto lock = lockMutex(&m_mutex);
    if (m_viewMap.contains(name)) {
        return false;
    }
    if (dynamic_cast<LibraryView*>(pView) == nullptr) {
        qDebug() << "WARNING: Attempted to register view" << name
                 << "with WLibraryPreparationWindow "
                 << "which does not implement the LibraryView interface. "
                 << "Ignoring.";
        return false;
    }
    addWidget(pView);
    m_viewMap[name] = pView;
    return true;
}

void WLibraryPreparationWindow::switchToView(const QString& name) {
    name == "Auto DJ";
    const auto lock = lockMutex(&m_mutex);
    // qDebug() << "WLibraryPreparationWindow::switchToView" << name;

    LibraryView* pOldLibrartView = dynamic_cast<LibraryView*>(
            currentWidget());

    // QWidget* pWidget = m_viewMap.value(name, nullptr);
    QWidget* pWidget = m_viewMap.value("Auto DJ", nullptr);
    if (pWidget != nullptr) {
        LibraryView* pLibraryView = dynamic_cast<LibraryView*>(pWidget);
        if (pLibraryView == nullptr) {
            qDebug() << "WARNING: Attempted to switch to view" << name
                     << "with WLibraryPreparationWindow "
                     << "which does not implement the LibraryView interface. "
                     << "Ignoring.";
            return;
        }
        if (currentWidget() != pWidget) {
            if (pOldLibrartView) {
                pOldLibrartView->saveCurrentViewState();
            }
            // qDebug() << "WLibraryPreparationWindow::setCurrentWidget" << name;
            setCurrentWidget(pWidget);
            pLibraryView->onShow();
            pLibraryView->restoreCurrentViewState();
        }
    }
}

void WLibraryPreparationWindow::pasteFromSidebar() {
    QWidget* pCurrent = currentWidget();
    LibraryView* pView = dynamic_cast<LibraryView*>(pCurrent);
    if (pView) {
        //    pView->pasteFromSidebar();
    }
}

void WLibraryPreparationWindow::search(const QString& name) {
    //    auto lock = lockMutex(&m_mutex);
    //    QWidget* pCurrent = currentWidget();
    //    LibraryView* pView = dynamic_cast<LibraryView*>(pCurrent);
    //    if (pView == nullptr) {
    //        qDebug() << "WARNING: Attempted to search in view" << name <<
    //        "with WLibraryPreparationWindow "
    //                 << "which does not implement the LibraryView interface.
    //                 Ignoring.";
    //        return;
    //    }
    //    lock.unlock();
    //    pView->onSearch(name);
}

LibraryView* WLibraryPreparationWindow::getActiveView() const {
    return dynamic_cast<LibraryView*>(currentWidget());
}

WPreparationWindowTrackTableView* WLibraryPreparationWindow::getCurrentTrackTableView() const {
    QWidget* pCurrent = currentWidget();
    WPreparationWindowTrackTableView* pTracksView =
            qobject_cast<WPreparationWindowTrackTableView*>(pCurrent);
    if (!pTracksView) {
        // This view is not a tracks view, but possibly a special library view
        // with a controls row and a track view (DlgAutoDJ, DlgRecording etc.)?
        pTracksView = pCurrent->findChild<WPreparationWindowTrackTableView*>();
    }
    return pTracksView; // might still be nullptr
}

bool WLibraryPreparationWindow::isTrackInCurrentView(const TrackId& trackId) {
    // qDebug() << "WLibraryPreparationWindow::isTrackInCurrentView" << trackId;
    VERIFY_OR_DEBUG_ASSERT(trackId.isValid()) {
        return false;
    }
    WPreparationWindowTrackTableView* pTracksView = getCurrentTrackTableView();
    if (!pTracksView) {
        return false;
    }

    return pTracksView->isTrackInCurrentView(trackId);
}

void WLibraryPreparationWindow::slotSelectTrackInActiveTrackView(const TrackId& trackId) {
    // qDebug() << "WLibraryPreparationWindow::slotSelectTrackInActiveTrackView" << trackId;
    if (!trackId.isValid()) {
        return;
    }
    WPreparationWindowTrackTableView* pTracksView = getCurrentTrackTableView();
    if (!pTracksView) {
        return;
    }
    pTracksView->selectTrack(trackId);
}

void WLibraryPreparationWindow::saveCurrentViewState() const {
    WPreparationWindowTrackTableView* pTracksView = getCurrentTrackTableView();
    if (!pTracksView) {
        return;
    }
    pTracksView->slotSaveCurrentViewState();
}

void WLibraryPreparationWindow::restoreCurrentViewState() const {
    WPreparationWindowTrackTableView* pTracksView = getCurrentTrackTableView();
    if (!pTracksView) {
        return;
    }
    pTracksView->slotRestoreCurrentViewState();
}

bool WLibraryPreparationWindow::event(QEvent* pEvent) {
    if (pEvent->type() == QEvent::ToolTip) {
        updateTooltip();
    }
    return QStackedWidget::event(pEvent);
}

void WLibraryPreparationWindow::keyPressEvent(QKeyEvent* pEvent) {
    if (pEvent->key() == Qt::Key_Left && pEvent->modifiers() & Qt::ControlModifier) {
        emit setLibraryFocus(FocusWidget::Sidebar);
    }
    QStackedWidget::keyPressEvent(pEvent);
}
