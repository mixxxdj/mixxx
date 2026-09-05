#pragma once

#include <QObject>
#include <QQmlAbstractUrlInterceptor>
#include <atomic>

#include "util/autofilereloader.h"

namespace mixxx {
namespace qml {

class QmlAutoReload : public QObject, public QQmlAbstractUrlInterceptor {
    Q_OBJECT
  public:
    explicit QmlAutoReload();

    QUrl intercept(const QUrl& url, QQmlAbstractUrlInterceptor::DataType type) override;

    void clear() {
        ++m_generation;
        m_autoReloader.clear();
    }

  signals:
    void triggered();

  private:
    AutoFileReloader m_autoReloader;
    std::atomic<quint64> m_generation{0};
};

} // namespace qml
} // namespace mixxx
