#pragma once

#include <QDialog>
#include <QPushButton>

#include "dialog/ui_dlgreplacecolordlg.h"
#include "preferences/usersettings.h"

class DlgReplaceColor : public QDialog, public Ui::DlgReplaceColor {
    Q_OBJECT
  public:
    DlgReplaceColor(QWidget* pParent);

  private:
    void slotSelectColor(QPushButton* button);
    void slotApply();
};
