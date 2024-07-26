#include "mixxxscreen.h"

#include "controllers/scripting/controllerscriptenginebase.h"
#include "util/assert.h"

namespace mixxx {
namespace qml {

void MixxxScreen::componentComplete() {
    auto* const context = QQmlEngine::contextForObject(this);
    VERIFY_OR_DEBUG_ASSERT(context) {
        return;
    }

    m_engine = context->engine();
    const MixxxControllerEngineInterface controllerEngineInterface =
            qobject_cast<MixxxControllerEngineInterface>(
                    m_engine->property("controllerEngineInterface"));
    VERIFY_OR_DEBUG_ASSERT(controllerEngineInterface) {
        return;
    }
    controllerEngineInterface.declareScreen(this);
}

QString MixxxScreen::screenId() {
    return m_screenId;
}

int MixxxScreen::width() {
    return m_size.width();
}

void MixxxScreen::setWidth(int value) {
    m_size = QSize(value, m_size.height());
}

int MixxxScreen::height() {
    return m_size.height();
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

QJSValue MixxxScreen::jsTransformFrame() {
    return m_transformFunc;
}

void MixxxScreen::setJsTransformFrame(QJSValue value) {
    if (!value.isCallable()) {
        qWarning() << "transformFrame is not a valid function";
        return;
    }

    m_transformFunc = value;
    emit jsTransformFrameChanged();
}

const std::unique_ptr<QByteArray> MixxxScreen::transform(
        const QByteArray& frame, QDateTime timestamp, QRect area) {
    VERIFY_OR_DEBUG_ASSERT(m_engine) {
        return std::make_unique<QByteArray>(frame);
    }

    if (m_transformFunc.isUndefined()) { // Default implementation
        return std::make_unique<QByteArray>(frame);
    }
    const auto transformResult =
            m_transformFunc.call(QJSValueList{m_engine->toScriptValue(frame),
                    m_engine->toScriptValue(timestamp),
                    m_engine->toScriptValue(area)});
    if (!transformResult.isVariant()) {
        qWarning() << "transformFrame did not return a valid result";
        return std::make_unique<QByteArray>(frame);
    }
    const auto result = transformResult.toVariant().toByteArray();
    qDebug() << "transformFrame returned a result of size" << result.length();
    return std::make_unique<QByteArray>(result);
}

} // namespace qml
} // namespace mixxx
