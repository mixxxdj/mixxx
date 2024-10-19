#ifndef CUSTOMITEM_H
#define CUSTOMITEM_H

#include <QQuickItem>
#include <memory>

namespace rendergraph {
class Node;
}

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

  private:
    std::unique_ptr<rendergraph::Node> m_pExampleNode;
};

#endif // CUSTOMITEM_H
