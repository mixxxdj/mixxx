#include "qml/qmlkeyutils.h"

#include <QQmlEngine>

#include "moc_qmlkeyutils.cpp"
#include "track/keyutils.h"

namespace mixxx {
namespace qml {

QmlKeyUtils::QmlKeyUtils(QObject* parent)
        : QObject(parent) {
}

QString QmlKeyUtils::keyToString(double numericKey) const {
    const auto key = KeyUtils::keyFromNumericValue(numericKey);
    return KeyUtils::keyToString(key);
}

QString QmlKeyUtils::keyToString(double numericKey, double notationValue) const {
    const auto key = KeyUtils::keyFromNumericValue(numericKey);
    const auto notation = KeyUtils::keyNotationFromNumericValue(notationValue);
    return KeyUtils::keyToString(key, notation);
}

int QmlKeyUtils::keyToOpenKeyNumber(double numericKey) const {
    const auto key = KeyUtils::keyFromNumericValue(numericKey);
    return KeyUtils::keyToOpenKeyNumber(key);
}

double QmlKeyUtils::scaleKeySteps(double numericKey, int steps) const {
    const auto key = KeyUtils::keyFromNumericValue(numericKey);
    const auto scaledKey = KeyUtils::scaleKeySteps(key, steps);
    return KeyUtils::keyToNumericValue(scaledKey);
}

bool QmlKeyUtils::keyIsValid(double numericKey) const {
    const auto key = KeyUtils::keyFromNumericValue(numericKey);
    return key != mixxx::track::io::key::INVALID;
}

// static
QmlKeyUtils* QmlKeyUtils::create(QQmlEngine* pQmlEngine, [[maybe_unused]] QJSEngine* pJsEngine) {
    return new QmlKeyUtils(pQmlEngine);
}

} // namespace qml
} // namespace mixxx
