#include "qml/qmlfontscale.h"

#include <QCoreApplication>
#include <QDebug>
#include <QEvent>
#include <QtMath>

#include "moc_qmlfontscale.cpp"

namespace mixxx {
namespace qml {

namespace {

qreal extractValidPointSize(const QFont& font) {
    const qreal pointSize = font.pointSizeF();
    return (pointSize > 0.0) ? pointSize : QmlFontScale::kFallbackPointSize;
}

} // namespace

QmlFontScale* QmlFontScale::s_pInstance = nullptr;

QmlFontScale::QmlFontScale(QObject* parent)
        : QObject(parent),
          m_basePointSize(extractValidPointSize(QGuiApplication::font())) {
    s_pInstance = this;
    qDebug() << "QmlFontScale: initialized with basePointSize =" << m_basePointSize << "pt";

    if (auto* app = QCoreApplication::instance()) {
        app->installEventFilter(this);
    }
}

QmlFontScale::~QmlFontScale() {
    if (auto* app = QCoreApplication::instance()) {
        app->removeEventFilter(this);
    }
    if (s_pInstance == this) {
        s_pInstance = nullptr;
    }
}

// static
QmlFontScale* QmlFontScale::instance() {
    return s_pInstance;
}

// static
QmlFontScale* QmlFontScale::create(
        QQmlEngine* pQmlEngine,
        [[maybe_unused]] QJSEngine* pJsEngine) {
    if (!s_pInstance) {
        s_pInstance = new QmlFontScale(pQmlEngine);
    }
    return s_pInstance;
}

bool QmlFontScale::eventFilter(QObject* watched, QEvent* event) {
    if (event->type() == QEvent::ApplicationFontChange) {
        updateFont(QGuiApplication::font());
    }
    return QObject::eventFilter(watched, event);
}

void QmlFontScale::updateFont(const QFont& font) {
    const qreal pointSize = extractValidPointSize(font);
    if (!qFuzzyCompare(m_basePointSize, pointSize)) {
        qDebug() << "QmlFontScale: OS font changed to" << pointSize << "pt";
        m_basePointSize = pointSize;
        emit basePointSizeChanged();
    }
}

} // namespace qml
} // namespace mixxx
