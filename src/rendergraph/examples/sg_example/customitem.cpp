#include "customitem.h"

#include <QtQuick/QSGGeometryNode>
#include <QtQuick/QSGMaterial>
#include <QtQuick/QSGRectangleNode>
#include <QtQuick/QSGTexture>
#include <QtQuick/QSGTextureProvider>

#include "examplenodes.h"
#include "rendergraph/context.h"
#include "rendergraph/engine.h"
#include "rendergraph/node.h"

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

        rendergraph::Context context(window());

        auto pTopNode = std::make_unique<rendergraph::ExampleTopNode>(context);
        bgNode->appendChildNode(pTopNode->backendNode());

        m_pEngine = std::make_unique<rendergraph::Engine>(std::move(pTopNode));
        m_pEngine->initialize();

        node = bgNode;
    } else {
        bgNode = static_cast<QSGRectangleNode*>(node);
    }

    if (m_geometryChanged) {
        bgNode->setRect(boundingRect());
        m_pEngine->resize(boundingRect().width(), boundingRect().height());
        m_geometryChanged = false;
    }

    return node;
}
