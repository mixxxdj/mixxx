#include <QDebug>
#include <QMutexLocker>
#include <QEvent>

#include "wbaselibrary.h"

WBaseLibrary::WBaseLibrary(QWidget* parent)
        : QStackedWidget(parent),
          WBaseWidget(this),
          m_mutex(QMutex::Recursive) {

}

bool WBaseLibrary::registerView(QString name, QWidget* view) {
    QMutexLocker lock(&m_mutex);
    if (m_viewMap.contains(name)) {
        return false;
    }

    int index = addWidget(view);
    setCurrentIndex(index);
    m_viewMap[name] = view;
    return true;
}

void WBaseLibrary::switchToView(const QString& name) {
    QMutexLocker lock(&m_mutex);
    qDebug() << "WBaseLibrary::switchToView" << name;
    QWidget* widget = m_viewMap.value(name, nullptr);
    if (widget != nullptr && currentWidget() != widget) {
        qDebug() << "WBaseLibrary::setCurrentWidget" << name;
        setCurrentWidget(widget);
    }
}

bool WBaseLibrary::event(QEvent* pEvent) {
    if (pEvent->type() == QEvent::ToolTip) {
        updateTooltip();
    }

    return QStackedWidget::event(pEvent);
}

