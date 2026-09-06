#pragma once

#include <QObject>
#include <QQmlEngine>
#include <QString>

#include "track/keyutils.h"

namespace mixxx {
namespace qml {

class QmlKeyUtils : public QObject {
    Q_OBJECT
    QML_NAMED_ELEMENT(KeyUtils)
    QML_SINGLETON

  public:
    explicit QmlKeyUtils(QObject* parent = nullptr);

    Q_INVOKABLE QString keyToString(double numericKey) const;
    Q_INVOKABLE QString keyToString(double numericKey, double notationValue) const;
    Q_INVOKABLE int keyToOpenKeyNumber(double numericKey) const;
    Q_INVOKABLE double scaleKeySteps(double numericKey, int steps) const;
    Q_INVOKABLE bool keyIsValid(double numericKey) const;

    static QmlKeyUtils* create(QQmlEngine* pQmlEngine, QJSEngine* pJsEngine);
};

} // namespace qml
} // namespace mixxx
