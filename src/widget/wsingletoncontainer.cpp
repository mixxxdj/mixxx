#include "widget/wsingletoncontainer.h"

#include <QLayout>

#include "util/assert.h"
#include "widget/wlibrary.h"


WSingletonContainer::WSingletonContainer(QWidget* widget, QWidget* pParent)
        : WWidgetGroup(pParent), m_pWidget(widget) {
    setContentsMargins(0, 0, 0, 0);
    m_pLayout = new QVBoxLayout();
    m_pLayout->setSpacing(0);
    m_pLayout->setContentsMargins(0, 0, 0, 0);
    m_pLayout->setAlignment(Qt::AlignCenter);
    setLayout(m_pLayout);
}

void WSingletonContainer::showEvent(QShowEvent* event) {
    Q_UNUSED(event);
    QWidget* parent = m_pWidget->parentWidget();
    if (parent && parent->layout()) {
        parent->layout()->removeWidget(m_pWidget);
        m_pLayout->addWidget(m_pWidget);
        m_pWidget->show();
    }
}

void SingletonMap::defineSingleton(QString objectName, QWidget* widget) {
    WidgetMap::const_iterator widget_it = m_singletons.find(objectName);
    if (widget_it != m_singletons.end()) {
        qWarning() << "ERROR: Tried to define a singleton with a name that has"
                   << "already been defined:" << objectName;
        return;
    }
    m_singletons.insert(objectName, widget);
}

WSingletonContainer* SingletonMap::getSingleton(
        QString objectName, QWidget* pParent) {
    WidgetMap::const_iterator widget_it = m_singletons.find(objectName);
    if (widget_it == m_singletons.end()) {
        qWarning() << "ERROR: Asked for an unknown singleton widget:"
                   << objectName;
        return NULL;
    }
    return new WSingletonContainer(*widget_it, pParent);
}
