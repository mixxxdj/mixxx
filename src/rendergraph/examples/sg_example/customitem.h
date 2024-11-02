#pragma once

#include <QQuickItem>
#include <memory>

#include "rendergraph/node.h"

class CustomItem : public QQuickItem {
    Q_OBJECT
    QML_ELEMENT

  public:
    explicit CustomItem(QQuickItem* parent = nullptr);
    ~CustomItem();

  protected:
    QSGNode* updatePaintNode(QSGNode*, UpdatePaintNodeData*) override;
    void geometryChange(const QRectF& newGeometry, const QRectF& oldGeometry) override;

    bool m_geometryChanged{};
};
