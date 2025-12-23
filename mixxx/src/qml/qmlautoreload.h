#pragma once

#include <QObject>
#include <QQmlAbstractUrlInterceptor>

#include "util/autofilereloader.h"

namespace mixxx {
namespace qml {

class QmlAutoReload : public QObject, public QQmlAbstractUrlInterceptor {
    Q_OBJECT
  public:
    explicit QmlAutoReload();

    QUrl intercept(const QUrl& url, QQmlAbstractUrlInterceptor::DataType type) override;

    void clear() {
        m_autoReloader.clear();
    }

  signals:
    void triggered();

  private:
    AutoFileReloader m_autoReloader;
};

} // namespace qml
} // namespace mixxx
