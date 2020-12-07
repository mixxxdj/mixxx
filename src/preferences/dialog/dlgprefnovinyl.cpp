#include "preferences/dialog/dlgprefnovinyl.h"

#include <QtDebug>

#include "moc_dlgprefnovinyl.cpp"

DlgPrefNoVinyl::DlgPrefNoVinyl(QWidget * parent, SoundManager * soundman,
                               UserSettingsPointer  _config)
        : DlgPreferencePage(parent) {
    Q_UNUSED(soundman);
    Q_UNUSED(_config);
    setupUi(this);
}

DlgPrefNoVinyl::~DlgPrefNoVinyl() {
}
