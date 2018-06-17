#pragma once

#include "preferences/dlgpreferencepage.h"
#include "preferences/dialog/ui_dlgprefmetadatadlg.h"

class DlgPrefMetadata : public DlgPreferencePage, public Ui::DlgPrefMetadataDlg {
public:
    DlgPrefMetadata(QWidget *pParent);

};



