#include "controllers/dlgprefnocontrollers.h"

DlgPrefNoControllers::DlgPrefNoControllers(QWidget * parent,
                                           ConfigObject<ConfigValue> * _config)
        : QWidget(parent),
          Ui::DlgPrefNoControllersDlg() {
    setupUi(this);
}

DlgPrefNoControllers::~DlgPrefNoControllers() {
}
