#include "widget/wlibrary.h"

#include <QKeyEvent>
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

bool WLibrary::registerView(const QString& name, QWidget* pView) {
    //qDebug() << "WLibrary::registerView" << name;
    const auto lock = lockMutex(&m_mutex);
    if (m_viewMap.contains(name)) {
        return false;
    }
    if (dynamic_cast<LibraryView*>(pView) == nullptr) {
        qDebug() << "WARNING: Attempted to register view" << name << "with WLibrary "
                 << "which does not implement the LibraryView interface. "
                 << "Ignoring.";
        return false;
    }
    addWidget(pView);
    m_viewMap[name] = pView;
    return true;
}

void WLibrary::switchToView(const QString& name) {
    const auto lock = lockMutex(&m_mutex);
    //qDebug() << "WLibrary::switchToView" << name;

    LibraryView* pOldLibrartView = dynamic_cast<LibraryView*>(
            currentWidget());

    QWidget* pWidget = m_viewMap.value(name, nullptr);
    if (pWidget != nullptr) {
        LibraryView* pLibraryView = dynamic_cast<LibraryView*>(pWidget);
        if (pLibraryView == nullptr) {
            qDebug() << "WARNING: Attempted to switch to view" << name << "with WLibrary "
                     << "which does not implement the LibraryView interface. "
                     << "Ignoring.";
            return;
        }
        if (currentWidget() != pWidget) {
            if (pOldLibrartView) {
                pOldLibrartView->saveCurrentViewState();
            }
            //qDebug() << "WLibrary::setCurrentWidget" << name;
            setCurrentWidget(pWidget);
            pLibraryView->onShow();
            pLibraryView->restoreCurrentViewState();
        }
    }
}

void WLibrary::pasteFromSidebar() {
    QWidget* pCurrent = currentWidget();
    LibraryView* pView = dynamic_cast<LibraryView*>(pCurrent);
    if (pView) {
        pView->pasteFromSidebar();
    }
}

void WLibrary::search(const QString& name) {
    auto lock = lockMutex(&m_mutex);
    QWidget* pCurrent = currentWidget();
    LibraryView* pView = dynamic_cast<LibraryView*>(pCurrent);
    if (pView == nullptr) {
        qDebug() << "WARNING: Attempted to search in view" << name << "with WLibrary "
                 << "which does not implement the LibraryView interface. Ignoring.";
        return;
    }
    lock.unlock();
    pView->onSearch(name);
}

LibraryView* WLibrary::getActiveView() const {
    return dynamic_cast<LibraryView*>(currentWidget());
}

bool WLibrary::isTrackInCurrentView(const TrackId& trackId) {
    //qDebug() << "WLibrary::isTrackInCurrentView" << trackId;
    VERIFY_OR_DEBUG_ASSERT(trackId.isValid()) {
        return false;
    }
    QWidget* pCurrent = currentWidget();
    WTrackTableView* pTracksView = qobject_cast<WTrackTableView*>(pCurrent);
    if (!pTracksView) {
        // This view is no tracks view, but maybe a special tracks view with a
        // controls row (AutoDJ, Recording)?
        //qDebug() << "   view is no tracks view. look for tracks view child";
        pTracksView = pCurrent->findChild<WTrackTableView*>();
    }
    if (pTracksView) {
        //qDebug() << "   tracks view found";
        return pTracksView->isTrackInCurrentView(trackId);
    } else {
        // No tracks view, this is probably a root view WLibraryTextBrowser
        //qDebug() << "   no tracks view found";
        return false;
    }
}

void WLibrary::slotSelectTrackInActiveTrackView(const TrackId& trackId) {
    //qDebug() << "WLibrary::slotSelectTrackInActiveTrackView" << trackId;
    if (!trackId.isValid()) {
        return;
    }

    QWidget* pCurrent = currentWidget();
    WTrackTableView* pTracksView = qobject_cast<WTrackTableView*>(pCurrent);
    if (!pTracksView) {
        //qDebug() << "   view is no tracks view. look for tracks view child";
        pTracksView = pCurrent->findChild<WTrackTableView*>();
    }
    if (pTracksView) {
        //qDebug() << "   tracks view found";
        pTracksView->slotSelectTrack(trackId);
    } else {
        //qDebug() << "   no tracks view found";
    }
}

bool WLibrary::event(QEvent* pEvent) {
    if (pEvent->type() == QEvent::ToolTip) {
        updateTooltip();
    }
    return QStackedWidget::event(pEvent);
}

void WLibrary::keyPressEvent(QKeyEvent* pEvent) {
    if (pEvent->key() == Qt::Key_Left && pEvent->modifiers() & Qt::ControlModifier) {
        emit setLibraryFocus(FocusWidget::Sidebar);
    }
    QStackedWidget::keyPressEvent(pEvent);
}
