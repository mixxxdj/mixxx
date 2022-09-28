#pragma once

#include <QQuickItem>
#include <QSGNode>

namespace mixxx {
namespace qml {

class QmlRenderTest : public QQuickItem {
    Q_OBJECT

    int m_phase{0};

  public:
    QmlRenderTest(QQuickItem* parent = nullptr);
    ~QmlRenderTest() override = default;

    QSGNode* updatePaintNode(QSGNode* old, QQuickItem::UpdatePaintNodeData*) override;
};

} // namespace qml
} // namespace mixxx
