#ifndef DIALOG_DLGABOUT_H
#define DIALOG_DLGABOUT_H

#include <QWidget>

#include "dialog/ui_dlgaboutdlg.h"
#include "preferences/usersettings.h"

class DlgAbout : public QDialog, public Ui::DlgAboutDlg {
    Q_OBJECT
  public:
    DlgAbout(QWidget* parent);
};

#endif  // DIALOG_DLGABOUT_H
