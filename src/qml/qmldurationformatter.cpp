#include "qml/qmldurationformatter.h"

#include <cmath>

#include "moc_qmldurationformatter.cpp"
#include "util/duration.h"

namespace mixxx::qml {

QmlDurationFormatter::QmlDurationFormatter(QObject* parent)
        : QObject(parent) {
}

QmlDurationFormatter* QmlDurationFormatter::create(
        QQmlEngine* pQmlEngine,
        QJSEngine* pJsEngine) {
    Q_UNUSED(pJsEngine);
    auto* pFormatter = new QmlDurationFormatter(pQmlEngine);
    QQmlEngine::setObjectOwnership(pFormatter, QQmlEngine::CppOwnership);
    return pFormatter;
}

QString QmlDurationFormatter::format(
        double seconds,
        mixxx::qml::QmlDurationFormatter::Format format) const {
    if (!std::isfinite(seconds) || seconds < 0.0) {
        return Duration::kInvalidDurationString;
    }

    switch (format) {
    case Format::Traditional:
        return Duration::formatTime(seconds, Duration::Precision::CENTISECONDS);
    case Format::TraditionalCoarse:
        return Duration::formatTime(seconds, Duration::Precision::SECONDS);
    case Format::Seconds:
        return Duration::formatSeconds(seconds, Duration::Precision::CENTISECONDS);
    case Format::SecondsLong:
        return Duration::formatSecondsLong(seconds, Duration::Precision::CENTISECONDS);
    case Format::KiloSeconds:
        return Duration::formatKiloSeconds(seconds, Duration::Precision::CENTISECONDS);
    case Format::HectoSeconds:
        return QStringLiteral("???");
    }

    return Duration::kInvalidDurationString;
}

} // namespace mixxx::qml
