#include "customitem.h"

#include <QtQuick/QSGGeometryNode>
#include <QtQuick/QSGMaterial>
#include <QtQuick/QSGRectangleNode>
#include <QtQuick/QSGTexture>
#include <QtQuick/QSGTextureProvider>

#include "examplenodes.h"
#include "rendergraph/context.h"
#include "rendergraph/node.h"
#include "rendergraph/scenegraph.h"

CustomItem::CustomItem(QQuickItem* parent)
        : QQuickItem(parent) {
    setFlag(ItemHasContents, true);
}

CustomItem::~CustomItem() = default;

void CustomItem::geometryChange(const QRectF& newGeometry, const QRectF& oldGeometry) {
    m_geometryChanged = true;
    update();
    QQuickItem::geometryChange(newGeometry, oldGeometry);
}

QSGNode* CustomItem::updatePaintNode(QSGNode* node, UpdatePaintNodeData*) {
    QSGRectangleNode* bgNode;
    if (!node) {
        bgNode = window()->createRectangleNode();
        bgNode->setColor(QColor(0, 0, 0, 255));
        bgNode->setRect(boundingRect());

        m_node = std::make_unique<rendergraph::Node>();
        m_node->Node::appendChildNode(std::make_unique<rendergraph::ExampleNode1>());
        m_node->Node::appendChildNode(std::make_unique<rendergraph::ExampleNode2>());
        m_node->Node::appendChildNode(std::make_unique<rendergraph::ExampleNode3>());

        {
            QImage img(":/example/images/test.png");
            auto context = rendergraph::createSgContext(window());
            static_cast<rendergraph::ExampleNode3*>(m_node->Node::lastChild())
                    ->setTexture(std::make_unique<rendergraph::Texture>(
                            *context, img));
        }

        bgNode->appendChildNode(m_node->sgNode());

        node = bgNode;
    } else {
        bgNode = static_cast<QSGRectangleNode*>(node);
    }

    if (m_geometryChanged) {
        bgNode->setRect(boundingRect());
        m_geometryChanged = false;
    }

    return node;
}
