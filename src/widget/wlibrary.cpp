#include "widget/wlibrary.h"

#include <QMutexLocker>
#include <QtDebug>

#include "controllers/keyboard/keyboardeventfilter.h"
#include "library/libraryview.h"
#include "moc_wlibrary.cpp"
#include "util/math.h"
#include "widget/wtracktableview.h"

WLibrary::WLibrary(QWidget* parent)
        : QStackedWidget(parent),
          WBaseWidget(this),
          m_mutex(QMutex::Recursive),
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
    QMutexLocker lock(&m_mutex);
    if (m_viewMap.contains(name)) {
        return false;
    }
    if (dynamic_cast<LibraryView*>(view) == nullptr) {
        qDebug() << "WARNING: Attempted to register a view with WLibrary "
                 << "that does not implement the LibraryView interface. "
                 << "Ignoring.";
        return false;
    }
    addWidget(view);
    m_viewMap[name] = view;
    return true;
}

void WLibrary::switchToView(const QString& name) {
    QMutexLocker lock(&m_mutex);
    //qDebug() << "WLibrary::switchToView" << name;

    WTrackTableView* ttView = qobject_cast<WTrackTableView*>(
            currentWidget());

    if (ttView != nullptr){
        //qDebug("trying to save position");
        ttView->saveCurrentVScrollBarPos();
    }

    QWidget* widget = m_viewMap.value(name, nullptr);
    if (widget != nullptr) {
        LibraryView * lview = dynamic_cast<LibraryView*>(widget);
        if (lview == nullptr) {
            qDebug() << "WARNING: Attempted to register a view with WLibrary "
                     << "that does not implement the LibraryView interface. "
                     << "Ignoring.";
            return;
        }
        if (currentWidget() != widget) {
            //qDebug() << "WLibrary::setCurrentWidget" << name;
            setCurrentWidget(widget);
            lview->onShow();
        }

        WTrackTableView* ttWidgetView = qobject_cast<WTrackTableView*>(
                widget);

        if (ttWidgetView != nullptr){
            qDebug("trying to restore position");
            ttWidgetView->restoreCurrentVScrollBarPos();
        }
    }
}

void WLibrary::search(const QString& name) {
    QMutexLocker lock(&m_mutex);
    QWidget* current = currentWidget();
    LibraryView* view = dynamic_cast<LibraryView*>(current);
    if (view == nullptr) {
        qDebug() << "WARNING: Attempted to register a view with WLibrary "
          << "that does not implement the LibraryView interface. Ignoring.";
        return;
    }
    lock.unlock();
    view->onSearch(name);
}

LibraryView* WLibrary::getActiveView() const {
    return dynamic_cast<LibraryView*>(currentWidget());
}

bool WLibrary::event(QEvent* pEvent) {
    if (pEvent->type() == QEvent::ToolTip) {
        updateTooltip();
    }
    return QStackedWidget::event(pEvent);
}
