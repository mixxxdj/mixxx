#pragma once

#include <QObject>
#include <QString>

namespace mixxx {
namespace qml {

class QmlConfigProxyBase : public QObject {
    Q_OBJECT
  public:
    explicit QmlConfigProxyBase(QObject* pParent = nullptr)
            : QObject(pParent) {
    }
    virtual ~QmlConfigProxyBase() override = default;

    virtual void setConfigScheme(const QString& scheme) = 0;

    static inline QmlConfigProxyBase* s_pInstance = nullptr;

  signals:
    void configSchemeChanged();
};

} // namespace qml
} // namespace mixxx
