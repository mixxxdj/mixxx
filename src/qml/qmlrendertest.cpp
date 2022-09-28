#include "qml/qmlrendertest.h"

#include <QSGSimpleRectNode>
#include <QSGVertexColorMaterial>
#include <cmath>
#include <iostream>

using namespace mixxx::qml;

QmlRenderTest::QmlRenderTest(QQuickItem* parent)
        : QQuickItem(parent), m_phase((std::rand() & 16) * 100) {
    setFlag(QQuickItem::ItemHasContents, true);
}

QSGNode* QmlRenderTest::updatePaintNode(QSGNode* old, QQuickItem::UpdatePaintNodeData*) {
    auto* bgNode = dynamic_cast<QSGSimpleRectNode*>(old);
    QSGGeometry* geometry;
    QSGGeometryNode* geometryNode;
    QSGVertexColorMaterial* material;
    if (!bgNode) {
        bgNode = new QSGSimpleRectNode();
        geometryNode = new QSGGeometryNode();

        geometry = new QSGGeometry(QSGGeometry::defaultAttributes_ColoredPoint2D(), 0);
        geometry->setDrawingMode(QSGGeometry::DrawLines);
        geometryNode->setGeometry(geometry);
        geometryNode->setFlag(QSGNode::OwnsGeometry);

        material = new QSGVertexColorMaterial();
        geometryNode->setMaterial(material);
        geometryNode->setFlag(QSGNode::OwnsMaterial);
        bgNode->appendChildNode(geometryNode);
    } else {
        geometryNode = dynamic_cast<QSGGeometryNode*>(bgNode->childAtIndex(0));
        material = dynamic_cast<QSGVertexColorMaterial*>(geometryNode->material());
        geometry = geometryNode->geometry();
    }
    bgNode->setRect(QRect(0, 0, width(), height()));
    bgNode->setColor(QColor(0, 0, 0));

    geometry->allocate(width() * 2);
    QSGGeometry::ColoredPoint2D* vertices = geometry->vertexDataAsColoredPoint2D();

    constexpr float twopi = M_PI * 2.f;
    int j = 0;

    const float h = height();

    for (int i = 0; i < width(); i++) {
        float x = static_cast<float>(i);
        float f = twopi * 3.f * 4.f * 5.f * 7.f * ((m_phase + i) & 4095) / 4096.f;
        uchar r = static_cast<uchar>(std::cos(f / 3.0) * 127.f + 127.f);
        uchar g = static_cast<uchar>(std::cos(f / 4.0) * 127.f + 127.f);
        uchar b = static_cast<uchar>(std::cos(f / 5.0) * 127.f + 127.f);
        vertices[j++].set(x, std::cos(f / 7.f) * h * 0.25 + h * 0.75, r, g, b, 255);
        vertices[j++].set(x, std::cos(f / 5.f) * h * 0.25 + h * 0.25, r, g, b, 255);
    }

    m_phase += 5;

    bgNode->markDirty(QSGNode::DirtyGeometry);
    geometryNode->markDirty(QSGNode::DirtyGeometry);
    update();

    return bgNode;
}
