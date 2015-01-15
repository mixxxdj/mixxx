#include "widget/wsingletoncontainer.h"

#include <QLayout>

#include "util/assert.h"
#include "widget/wlibrary.h"

// static
WSingletonContainer::WidgetMap WSingletonContainer::m_singletons;

// static
WSingletonContainer* WSingletonContainer::getSingleton(
        QString objectname, QWidget* pParent) {
   WidgetMap::const_iterator widget_it = m_singletons.find(objectname);
    if (widget_it == m_singletons.end()) {
        qWarning() << "ERROR: Asked for an unknown singleton widget:"
                   << objectname;
        return NULL;
    }
    return new WSingletonContainer(objectname, pParent);
}

WSingletonContainer::WSingletonContainer(QString objectname, QWidget* pParent)
        : WWidgetGroup(pParent), m_pWidget(NULL) {
    WidgetMap::const_iterator widget_it = m_singletons.find(objectname);
    DEBUG_ASSERT(widget_it != m_singletons.end());

    m_pWidget = *widget_it;
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
    }
}

// static
void WSingletonContainer::defineSingleton(QString objectname, QWidget* widget) {
    WidgetMap::const_iterator widget_it = m_singletons.find(objectname);
    if (widget_it != m_singletons.end()) {
        qWarning() << "ERROR: Tried to define a singleton with a name that has"
                   << "already been defined:" << objectname;
        return;
    }
    m_singletons[objectname] = widget;
}
