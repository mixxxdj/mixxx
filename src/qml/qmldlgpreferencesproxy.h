#pragma once

#include <QDialog>
#include <QObject>
#include <QtQml>
#include <memory>

namespace mixxx {
namespace qml {

class QmlDlgPreferencesProxy : public QObject {
    Q_OBJECT
    QML_NAMED_ELEMENT(PreferencesDialog)
    QML_SINGLETON
  public:
    explicit QmlDlgPreferencesProxy(
            std::shared_ptr<QDialog> pDlgPreferences,
            QObject* parent = nullptr);

    Q_INVOKABLE void show();

    static QmlDlgPreferencesProxy* create(QQmlEngine* pQmlEngine, QJSEngine* pJsEngine);
    static inline QmlDlgPreferencesProxy* s_pInstance = nullptr;

  private:
    static inline QJSEngine* s_pJsEngine = nullptr;
    std::shared_ptr<QDialog> m_pDlgPreferences;
};

} // namespace qml
} // namespace mixxx
