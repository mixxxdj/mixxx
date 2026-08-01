#pragma once

#include <QJSEngine>
#include <QObject>
#include <QQmlEngine>
#include <QString>

#include "preferences/interface.h"

namespace mixxx::qml {

class QmlDurationFormatter : public QObject {
    Q_OBJECT
    QML_NAMED_ELEMENT(DurationFormatter)
    QML_SINGLETON

  public:
    enum class Format {
        Traditional,
        TraditionalCoarse,
        Seconds,
        SecondsLong,
        KiloSeconds,
        HectoSeconds,
    };
    Q_ENUM(Format)

    explicit QmlDurationFormatter(QObject* parent = nullptr);

    static QmlDurationFormatter* create(QQmlEngine* pQmlEngine, QJSEngine* pJsEngine);

    Q_INVOKABLE QString format(
            double seconds,
            mixxx::qml::QmlDurationFormatter::Format format) const;
};

} // namespace mixxx::qml
