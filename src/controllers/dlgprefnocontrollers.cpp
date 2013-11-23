#include "controllers/dlgprefnocontrollers.h"

DlgPrefNoControllers::DlgPrefNoControllers(QWidget * parent,
                                           ConfigObject<ConfigValue> * _config)
        : QWidget(parent),
          Ui::DlgPrefNoControllersDlg() {
    Q_UNUSED(_config);
    setupUi(this);
}

DlgPrefNoControllers::~DlgPrefNoControllers() {
}
