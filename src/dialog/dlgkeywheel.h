#ifndef DLGKEYWHEEL_H
#define DLGKEYWHEEL_H

#include <QDialog>
#include <QSvgWidget>
#include <QEvent>
#include <QDomDocument>
#include "track/keyutils.h"

#include "dialog/ui_dlgkeywheel.h"


namespace Ui {
class DlgKeywheel;
}

class DlgKeywheel : public QDialog
{
    Q_OBJECT

public:
    explicit DlgKeywheel(QWidget *parent, UserSettingsPointer pConfig);
    void switchDisplay(int dir = 1);
    void updateDisplay();
    ~DlgKeywheel();

protected:
    bool eventFilter(QObject *obj, QEvent *event);

private:
  bool isHiddenNotation(KeyUtils::KeyNotation notation);
  KeyUtils::KeyNotation m_notation;
  QDomDocument m_domDocument;
  Ui::DlgKeywheel* ui;
  QSvgWidget* m_wheel;
  UserSettingsPointer m_pConfig;
};

#endif // DLGKEYWHEEL_H
