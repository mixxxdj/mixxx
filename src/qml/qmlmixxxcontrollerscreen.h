#pragma once

#include <QQuickItem>

#include "controllers/rendering/controllerrenderingengine.h"

namespace mixxx {
namespace qml {

class QmlMixxxControllerScreen : public QQuickItem {
    Q_OBJECT
    QML_NAMED_ELEMENT(ControllerScreen)
    Q_PROPERTY(QJSValue init READ getInit WRITE setInit
                    NOTIFY initChanged REQUIRED);
    Q_PROPERTY(QJSValue shutdown READ getShutdown WRITE setShutdown
                    NOTIFY shutdownChanged REQUIRED);
    Q_PROPERTY(QJSValue transformFrame READ getTransform WRITE setTransform
                    NOTIFY transformChanged REQUIRED);

  public:
    explicit QmlMixxxControllerScreen(QQuickItem* parent = nullptr);

    void setInit(const QJSValue& value);
    void setShutdown(const QJSValue& value);
    void setTransform(const QJSValue& value);

    QJSValue getInit() const {
        return m_initFunc;
    }

    QJSValue getShutdown() const {
        return m_shutdownFunc;
    }

    QJSValue getTransform() const {
        return m_transformFunc;
    }

  signals:
    void initChanged();
    void shutdownChanged();
    void transformChanged();

  private:
    QJSValue m_initFunc;
    QJSValue m_shutdownFunc;
    QJSValue m_transformFunc;
};

} // namespace qml
} // namespace mixxx
