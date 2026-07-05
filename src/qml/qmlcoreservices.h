#pragma once

#include <QJSEngine>
#include <QObject>
#include <QQmlEngine>
#include <QString>

#include "util/singleton.h"

class RecordingManager;

namespace mixxx {
namespace qml {

class QmlCoreServices : public QObject, public Singleton<QmlCoreServices> {
    Q_OBJECT
    Q_PROPERTY(bool ready READ ready NOTIFY readyChanged)
    QML_NAMED_ELEMENT(Core)
    QML_SINGLETON
  public:
    explicit QmlCoreServices(QObject* parent)
            : QObject(parent) {
    }
    ~QmlCoreServices() override = default;

    static QmlCoreServices* create(QQmlEngine* pQmlEngine, QJSEngine* pJsEngine);

    bool ready() const {
        return m_ready;
    }

    void setReady();

  signals:
    void readyChanged();

  private:
    bool m_ready{false};
};

} // namespace qml
} // namespace mixxx
