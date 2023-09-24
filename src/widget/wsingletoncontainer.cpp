#include "widget/wsingletoncontainer.h"

#include <QLayout>

#include "moc_wsingletoncontainer.cpp"
#include "skin/legacy/skincontext.h"
#include "util/assert.h"
#include "widget/wlibrary.h"

WSingletonContainer::WSingletonContainer(QWidget* pParent)
        : WWidgetGroup(pParent), m_pWidget(nullptr), m_pLayout(nullptr) { }

void WSingletonContainer::setup(const QDomNode& node, const SkinContext& context) {
    setContentsMargins(0, 0, 0, 0);
    m_pLayout = new QVBoxLayout();
    m_pLayout->setSpacing(0);
    m_pLayout->setContentsMargins(0, 0, 0, 0);
    setLayout(m_pLayout);

    QDomElement containerNode = node.toElement();
    QString objectName;
    if (!context.hasNodeSelectString(node, "ObjectName", &objectName)) {
        SKIN_WARNING(node, context)
                << "Need objectName attribute for Singleton tag";
        return;
    }
    if (objectName.isEmpty()) {
        SKIN_WARNING(node, context)
                << "Singleton tag's ObjectName is empty";
        return;
    }
    m_pWidget = context.getSingletonWidget(objectName);
    if (m_pWidget == nullptr) {
        SKIN_WARNING(node, context)
                << "Asked for an unknown singleton widget:" << objectName;
    }
}

void WSingletonContainer::showEvent(QShowEvent* event) {
    Q_UNUSED(event);
    if (m_pWidget) {
        // The widget's current parent is some other SingletonContainer,
        // or some other widget in the skin if the widget has been newly
        // constructed.  (The widget will be hidden so it will never appear in
        // the place it was defined).
        // First confirm that the parentage is valid, and then
        // reparent the widget to our container.
        QWidget* parent = m_pWidget->parentWidget();
        if (parent == this) {
            // The widget is already owned by us, no need to reparent.
            return;
        }
        if (parent && parent->layout() && m_pLayout) {
            parent->layout()->removeWidget(m_pWidget);
            m_pLayout->addWidget(m_pWidget);
            m_pWidget->show();
        }
    }
}

void SingletonMap::insertSingleton(const QString& objectName, QWidget* widget) {
    if (m_singletons.contains(objectName)){
        qWarning() << "ERROR: Tried to insert a singleton with a name that has"
                   << "already been inserted:" << objectName;
        return;
    }
    m_singletons.insert(objectName, widget);
}

QWidget* SingletonMap::getSingletonWidget(const QString& objectName) const {
    return m_singletons.value(objectName, nullptr);
}
