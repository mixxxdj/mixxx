#pragma once

#include <QFont>
#include <QGuiApplication>
#include <QJSEngine>
#include <QObject>
#include <QQmlEngine>

namespace mixxx {
namespace qml {

class QmlFontScale : public QObject {
    Q_OBJECT
    Q_PROPERTY(qreal basePointSize READ basePointSize NOTIFY basePointSizeChanged)
    QML_NAMED_ELEMENT(FontScale)
    QML_SINGLETON

  public:
    static constexpr qreal kFallbackPointSize = 10.0;

    explicit QmlFontScale(QObject* parent = nullptr);
    ~QmlFontScale() override;

    qreal basePointSize() const {
        return m_basePointSize;
    }

    static QmlFontScale* create(QQmlEngine* pQmlEngine, [[maybe_unused]] QJSEngine* pJsEngine);
    static QmlFontScale* instance();

  signals:
    void basePointSizeChanged();

  private slots:
    void onFontChanged(const QFont& font);

  private:
    void updateFont(const QFont& font);

    static QmlFontScale* s_pInstance;
    qreal m_basePointSize;
};

} // namespace qml
} // namespace mixxx
