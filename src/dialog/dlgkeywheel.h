#ifndef DLGKEYWHEEL_H
#define DLGKEYWHEEL_H

#include <QDialog>
#include <QDomDocument>
#include <QEvent>
#include <QSvgWidget>

#include "dialog/ui_dlgkeywheel.h"
#include "track/keyutils.h"

namespace Ui {
class DlgKeywheel;
}

class DlgKeywheel : public QDialog {
    Q_OBJECT

  public:
    explicit DlgKeywheel(QWidget* parent, UserSettingsPointer pConfig);
    void switchDisplay(int dir = 1);
    void updateDisplay();
    ~DlgKeywheel();

  protected:
    bool eventFilter(QObject* obj, QEvent* event);
    void resizeEvent(QResizeEvent* ev) override;

  private:
    bool isHiddenNotation(KeyUtils::KeyNotation notation);
    KeyUtils::KeyNotation m_notation;
    QDomDocument m_domDocument;
    Ui::DlgKeywheel* ui;
    QSvgWidget* m_wheel;
    UserSettingsPointer m_pConfig;
};

#endif // DLGKEYWHEEL_H
