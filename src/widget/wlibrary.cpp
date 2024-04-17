#include "widget/wlibrary.h"

#include <QtDebug>

#include "library/libraryview.h"
#include "moc_wlibrary.cpp"
#include "skin/legacy/skincontext.h"
#include "util/math.h"
#include "widget/wtracktableview.h"

WLibrary::WLibrary(QWidget* parent)
        : QStackedWidget(parent),
          WBaseWidget(this),
          m_mutex(QT_RECURSIVE_MUTEX_INIT),
          m_trackTableBackgroundColorOpacity(kDefaultTrackTableBackgroundColorOpacity),
          m_bShowButtonText(true) {
}

void WLibrary::setup(const QDomNode& node, const SkinContext& context) {
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

bool WLibrary::registerView(const QString& name, QWidget* view) {
    //qDebug() << "WLibrary::registerView" << name;
    const auto lock = lockMutex(&m_mutex);
    if (m_viewMap.contains(name)) {
        return false;
    }
    if (dynamic_cast<LibraryView*>(view) == nullptr) {
        qDebug() << "WARNING: Attempted to register view" << name << "with WLibrary "
                 << "which does not implement the LibraryView interface. "
                 << "Ignoring.";
        return false;
    }
    addWidget(view);
    m_viewMap[name] = view;
    return true;
}

void WLibrary::switchToView(const QString& name) {
    const auto lock = lockMutex(&m_mutex);
    //qDebug() << "WLibrary::switchToView" << name;

    LibraryView* oldLibraryView = dynamic_cast<LibraryView*>(
            currentWidget());

    QWidget* widget = m_viewMap.value(name, nullptr);
    if (widget != nullptr) {
        LibraryView * lview = dynamic_cast<LibraryView*>(widget);
        if (lview == nullptr) {
            qDebug() << "WARNING: Attempted to switch to view" << name << "with WLibrary "
                     << "which does not implement the LibraryView interface. "
                     << "Ignoring.";
            return;
        }
        if (currentWidget() != widget) {
            if (oldLibraryView) {
                oldLibraryView->saveCurrentViewState();
            }
            //qDebug() << "WLibrary::setCurrentWidget" << name;
            setCurrentWidget(widget);
            lview->onShow();
            lview->restoreCurrentViewState();
        }
    }
}

void WLibrary::search(const QString& name) {
    auto lock = lockMutex(&m_mutex);
    QWidget* current = currentWidget();
    LibraryView* view = dynamic_cast<LibraryView*>(current);
    if (view == nullptr) {
        qDebug() << "WARNING: Attempted to search in view" << name << "with WLibrary "
                 << "which does not implement the LibraryView interface. Ignoring.";
        return;
    }
    lock.unlock();
    view->onSearch(name);
}

LibraryView* WLibrary::getActiveView() const {
    return dynamic_cast<LibraryView*>(currentWidget());
}

WTrackTableView* WLibrary::getCurrentTrackTableView() const {
    QWidget* pCurrent = currentWidget();
    WTrackTableView* pTracksView = qobject_cast<WTrackTableView*>(pCurrent);
    if (!pTracksView) {
        // This view is no tracks view, but maybe a special tracks view with a
        // controls row (DlgAutoDJ, DlgRecording)?
        // qDebug() << "   view is no tracks view. look for tracks view child";
        pTracksView = pCurrent->findChild<WTrackTableView*>();
    }
    return pTracksView; // might be nullptr
}

bool WLibrary::isTrackInCurrentView(const TrackId& trackId) {
    // qDebug() << "WLibrary::isTrackInCurrentView" << trackId;
    VERIFY_OR_DEBUG_ASSERT(trackId.isValid()) {
        return false;
    }
    WTrackTableView* pTracksView = getCurrentTrackTableView();
    if (!pTracksView) {
        return false;
    }

    return pTracksView->isTrackInCurrentView(trackId);
}

void WLibrary::slotSelectTrackInActiveTrackView(const TrackId& trackId) {
    //qDebug() << "WLibrary::slotSelectTrackInActiveTrackView" << trackId;
    if (!trackId.isValid()) {
        return;
    }
    WTrackTableView* pTracksView = getCurrentTrackTableView();
    if (!pTracksView) {
        return;
    }
    pTracksView->slotSelectTrack(trackId);
}

void WLibrary::saveCurrentViewState() const {
    WTrackTableView* pTracksView = getCurrentTrackTableView();
    if (!pTracksView) {
        return;
    }
    pTracksView->slotSaveCurrentViewState();
}

void WLibrary::restoreCurrentViewState() const {
    WTrackTableView* pTracksView = getCurrentTrackTableView();
    if (!pTracksView) {
        return;
    }
    pTracksView->slotRestoreCurrentViewState();
}

bool WLibrary::event(QEvent* pEvent) {
    if (pEvent->type() == QEvent::ToolTip) {
        updateTooltip();
    }
    return QStackedWidget::event(pEvent);
}
