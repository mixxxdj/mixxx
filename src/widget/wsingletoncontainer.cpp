#include "widget/wsingletoncontainer.h"

#include <QLayout>

#include "util/assert.h"
#include "skin/skincontext.h"
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

void SingletonMap::insertSingleton(QString objectName, QWidget* widget) {
    if (m_singletons.contains(objectName)){
        qWarning() << "ERROR: Tried to insert a singleton with a name that has"
                   << "already been inserted:" << objectName;
        return;
    }
    m_singletons.insert(objectName, widget);
}

QWidget* SingletonMap::getSingletonWidget(QString objectName) const {
    return m_singletons.value(objectName, nullptr);
}

#include <QHBoxLayout>
#include <skin/skincontext.h>
#include <skin/legacyskinparser.h>

WSingleton::WSingleton(
        QSharedPointer<LegacySkinParser> pParser,
        QDomNode childrenNode,
        QWidget* pParent)
    : QWidget(pParent),
      m_pParser(pParser),
      m_childrenNode(childrenNode) {
}

void WSingleton::setVisible(bool visible) {
    if (!m_pWidget && visible) {


        // Descend chilren, taking the first valid element.
        QDomNode child_node;
        QDomNodeList children = m_childrenNode.childNodes();
        for (int i = 0; i < children.count(); ++i) {
            child_node = children.at(i);
            if (child_node.isElement()) {
                break;
            }
        }

        if (child_node.isNull()) {
            /*
            SKIN_WARNING(node, *m_pContext)
                    << "SingletonDefinition Children node is empty";
            */
            return;
        }

        QDomElement element = child_node.toElement();
        QList<QWidget*> child_widgets = m_pParser->parseNode(element);
        if (child_widgets.empty()) {
            /*
            SKIN_WARNING(node, *m_pContext)
                    << "SingletonDefinition child produced no widget.";
            */
            return;
        } else if (child_widgets.size() > 1) {
            /*
            SKIN_WARNING(node, *m_pContext)
                    << "SingletonDefinition child produced multiple widgets."
                    << "All but the first are ignored.";
            */
        }

        m_pWidget = child_widgets[0];
        if (!m_pWidget) {
            /*
            SKIN_WARNING(node, *m_pContext)
                    << "SingletonDefinition child widget is NULL";
            */
            return;
        }

        m_pWidget->setParent(this);
        setLayout(new QHBoxLayout(m_pWidget));
        layout()->setContentsMargins(0, 0, 0, 0);
        layout()->addWidget(m_pWidget);
    }
    QWidget::setVisible(visible);
}

