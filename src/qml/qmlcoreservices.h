#pragma once

#include <QJSEngine>
#include <QObject>
#include <QQmlEngine>
#include <QString>
#include <utility>

#include "util/singleton.h"

namespace mixxx {
namespace qml {

class QmlCoreServices : public QObject, public Singleton<QmlCoreServices> {
    Q_OBJECT
    Q_PROPERTY(bool ready READ ready NOTIFY readyChanged)
    Q_PROPERTY(int initializationProgress READ initializationProgress NOTIFY
                    initializationProgressChanged)
    Q_PROPERTY(QString initializationService READ initializationService NOTIFY
                    initializationServiceChanged)
    Q_PROPERTY(QString colorScheme READ colorScheme CONSTANT)
    QML_NAMED_ELEMENT(Core)
    QML_SINGLETON
  public:
    explicit QmlCoreServices(QString colorScheme, QObject* parent)
            : QObject(parent),
              m_colorScheme(std::move(colorScheme)) {
    }
    ~QmlCoreServices() override = default;

    static QmlCoreServices* create(QQmlEngine* pQmlEngine, QJSEngine* pJsEngine);

    bool ready() const {
        return m_ready;
    }

    int initializationProgress() const {
        return m_initializationProgress;
    }

    QString initializationService() const {
        return m_initializationService;
    }

    QString colorScheme() const {
        return m_colorScheme;
    }

    void setReady();
    void setInitializationProgress(int progress, const QString& service);

  signals:
    void readyChanged();
    void initializationProgressChanged();
    void initializationServiceChanged();

  private:
    bool m_ready{false};
    int m_initializationProgress{0};
    QString m_initializationService;
    const QString m_colorScheme;
};

} // namespace qml
} // namespace mixxx
