#include "customitem.h"

#include <QSGFlatColorMaterial>
#include <QtQuick/QSGGeometryNode>
#include <QtQuick/QSGMaterial>
#include <QtQuick/QSGRectangleNode>
#include <QtQuick/QSGTexture>
#include <QtQuick/QSGTextureProvider>

#include "examplenode.h"
#include "rendergraph/context.h"
#include "rendergraph/geometrynode.h"
#include "rendergraph/material/unicolormaterial.h"
#include "rendergraph/node.h"
#include "rendergraph/vertexupdaters/vertexupdater.h"

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
        bgNode->setColor(QColor(0, 0, 255, 255));
        bgNode->setRect(boundingRect());

        QSGGeometry* geometry = new QSGGeometry(QSGGeometry::defaultAttributes_Point2D(), 3);

        geometry->vertexDataAsPoint2D()[0].set(10, 10);
        geometry->vertexDataAsPoint2D()[1].set(100, 10);
        geometry->vertexDataAsPoint2D()[2].set(10, 100);

        QSGFlatColorMaterial* material = new QSGFlatColorMaterial;
        material->setColor(QColor(255, 0, 0));

        rendergraph::Context context(window());
        auto pExampleNode = std::make_unique<rendergraph::ExampleNode>(&context);

        bgNode->appendChildNode(pExampleNode.release());

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
