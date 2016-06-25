#include <QDebug>
#include <QMutexLocker>
#include <QEvent>
#include <QStyle>

#include "wbaselibrary.h"

WBaseLibrary::WBaseLibrary(QWidget* parent)
        : QStackedWidget(parent),
          WBaseWidget(this),
          m_mutex(QMutex::Recursive),
          m_showFocus(0) {

}

bool WBaseLibrary::registerView(QString name, QWidget* view) {
    QMutexLocker lock(&m_mutex);
    if (m_viewMap.contains(name)) {
        return false;
    }

    view->installEventFilter(this);
    int index = addWidget(view);
    setCurrentIndex(index);
    m_currentViewName = name;
    m_viewMap[name] = view;
    return true;
}

QString WBaseLibrary::getCurrentViewName() {
    return m_currentViewName;
}

int WBaseLibrary::getShowFocus() {
    return m_showFocus;
}

void WBaseLibrary::setShowFocus(int sFocus) {
    //qDebug() << "WBaseLibrary::setShowFocus" << sFocus << this;
    m_showFocus = sFocus;

    style()->unpolish(this);
    style()->polish(this);
    update();
}

void WBaseLibrary::switchToView(const QString& name) {
    QMutexLocker lock(&m_mutex);
    //qDebug() << "WBaseLibrary::switchToView" << name;
    QWidget* widget = m_viewMap.value(name, nullptr);
    if (widget != nullptr && currentWidget() != widget) {
        //qDebug() << "WBaseLibrary::setCurrentWidget" << name;
        m_currentViewName = name;
        setCurrentWidget(widget);
    }
}

bool WBaseLibrary::eventFilter(QObject*, QEvent* pEvent) {
    if (pEvent->type() == QEvent::FocusIn) {
        //qDebug() << "WBaseLibrary::eventFilter FocusIn";
        emit(focused());
    }
    return false;
}

bool WBaseLibrary::event(QEvent* pEvent) {
    if (pEvent->type() == QEvent::ToolTip) {
        updateTooltip();
    }

    return QStackedWidget::event(pEvent);
}

