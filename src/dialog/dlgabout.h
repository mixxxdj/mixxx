#pragma once

#include <QDialog>

#include "dialog/ui_dlgaboutdlg.h"

class DlgAbout : public QDialog, public Ui::DlgAboutDlg {
    Q_OBJECT
  public:
    DlgAbout(QWidget* pParent);
};
