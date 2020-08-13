#include <QtDebug>
#include "preferences/dialog/dlgprefnovinyl.h"

DlgPrefNoVinyl::DlgPrefNoVinyl(QWidget * parent, SoundManager * soundman,
                               UserSettingsPointer  _config)
        : DlgPreferencePage(parent) {
    Q_UNUSED(soundman);
    Q_UNUSED(_config);
    setupUi(this);
}

DlgPrefNoVinyl::~DlgPrefNoVinyl() {
}
