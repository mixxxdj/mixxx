#include "mixxxscreen.h"

namespace mixxx {
namespace qml {

void MixxxScreen::classBegin() {
}

void MixxxScreen::componentComplete() {
    const auto* const meta = metaObject();
    int typedIdx = meta->indexOfMethod(kScreenTransformFunctionTypedSignature);
    int untypedIdx = meta->indexOfMethod(kScreenTransformFunctionUntypedSignature);
    if (typedIdx >= 0) {
        auto transformMethod = meta->method(typedIdx);
        if (!transformMethod.isValid()) {
            // TODO
        } else {
            transform = [transformMethod, this](const QByteArray input,
                                const QDateTime& timestamp) -> QVariant {
                return transform(transformMethod, input, timestamp, true);
            };
        }
    } else if (untypedIdx >= 0) {
        auto transformMethod = meta->method(untypedIdx);
        if (!transformMethod.isValid()) {
            // TODO
        } else {
            transform = [transformMethod, this](const QByteArray input,
                                const QDateTime& timestamp) -> QVariant {
                return transform(transformMethod, input, timestamp, false);
            };
        }
    } else {
    }
}

int MixxxScreen::width() {
    return m_size.width();
}

void MixxxScreen::setWidth(int value) {
    m_size = QSize(value, m_size.height());
}

int MixxxScreen::height() {
    return m_size.width();
}

void MixxxScreen::setHeight(int value) {
    m_size = QSize(m_size.width(), value);
}

uint MixxxScreen::splashOff() {
    return m_splashOff.count();
}

void MixxxScreen::setSplashOff(uint value) {
    m_splashOff = std::chrono::milliseconds(value);
}
QVariant MixxxScreen::transform(QMetaMethod transformMethod,
        const QByteArray input,
        const QDateTime& timestamp,
        bool typed) {
    QVariant returnedValue;
    const bool isSuccessful = transformMethod.invoke(
            &m_item,
            Qt::DirectConnection,
            Q_RETURN_ARG(QVariant, returnedValue),
            typed ? Q_ARG(QVariant, QVariant::fromValue(input)) : Q_ARG(QByteArray, input),
            Q_ARG(QVariant, timestamp));
    if (!isSuccessful) {
        // TODO
        return {};
    } else if (returnedValue.isValid()) {
        // TODO
        return {};
    }

    if (returnedValue.canView<QByteArray>()) {
        return QVariant(returnedValue.view<QByteArray>());
    } else if (returnedValue.canConvert<QByteArray>()) {
        return QVariant(returnedValue.toByteArray());
    } else {
        // TODO
        return {};
    }
}

} // namespace qml
} // namespace mixxx
