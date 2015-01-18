#include "widget/wsingletoncontainer.h"

#include <QLayout>

#include "util/assert.h"
#include "skin/skincontext.h"
#include "widget/wlibrary.h"


WSingletonContainer::WSingletonContainer(QWidget* pParent)
        : WWidgetGroup(pParent), m_pWidget(NULL), m_pLayout(NULL) { }

void WSingletonContainer::setup(QDomNode node, const SkinContext& context) {
    setContentsMargins(0, 0, 0, 0);
    m_pLayout = new QVBoxLayout();
    m_pLayout->setSpacing(0);
    m_pLayout->setContentsMargins(0, 0, 0, 0);
    setLayout(m_pLayout);

    QDomElement containerNode = node.toElement();
    if (!context.hasNode(node, "ObjectName")) {
        SKIN_WARNING(node, context)
                << "Need objectName attribute for Singleton tag";
        return;
    }
    QString objectName = context.selectString(node, "ObjectName");
    m_pWidget = context.getSingletonWidget(objectName);
    if (m_pWidget == NULL) {
        SKIN_WARNING(node, context)
                << "Asked for an unknown singleton widget:" << objectName;
    }
}

void WSingletonContainer::showEvent(QShowEvent* event) {
    Q_UNUSED(event);
    if (m_pWidget) {
        QWidget* parent = m_pWidget->parentWidget();
        if (parent && parent->layout() && m_pLayout) {
            parent->layout()->removeWidget(m_pWidget);
            m_pLayout->addWidget(m_pWidget);
            m_pWidget->show();
        }
    }
}

void SingletonMap::defineSingleton(QString objectName, QWidget* widget) {
    if (m_singletons.contains(objectName)){
        qWarning() << "ERROR: Tried to define a singleton with a name that has"
                   << "already been defined:" << objectName;
        return;
    }
    m_singletons.insert(objectName, widget);
}

QWidget* SingletonMap::getSingletonWidget(QString objectName) const {
    WidgetMap::const_iterator widget_it = m_singletons.find(objectName);
    if (widget_it == m_singletons.end()) {
        return NULL;
    }
    return *widget_it;
}
