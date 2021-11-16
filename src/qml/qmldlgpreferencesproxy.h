#pragma once
#include <QObject>

#include "preferences/dialog/dlgpreferences.h"

namespace mixxx {
namespace qml {

class QmlDlgPreferencesProxy : public QObject {
    Q_OBJECT
  public:
    explicit QmlDlgPreferencesProxy(
            std::shared_ptr<DlgPreferences> pDlgPreferences,
            QObject* parent = nullptr);

    Q_INVOKABLE void show();

  private:
    std::shared_ptr<DlgPreferences> m_pDlgPreferences;
};

} // namespace qml
} // namespace mixxx
