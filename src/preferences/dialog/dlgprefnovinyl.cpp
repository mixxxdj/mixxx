#include "preferences/dialog/dlgprefnovinyl.h"

#include <QtDebug>

#include "defs_urls.h"
#include "moc_dlgprefnovinyl.cpp"

DlgPrefNoVinyl::DlgPrefNoVinyl(QWidget * parent, SoundManager * soundman,
                               UserSettingsPointer  _config)
        : DlgPreferencePage(parent) {
    Q_UNUSED(soundman);
    Q_UNUSED(_config);
    setupUi(this);
    // Create text color for the cue mode link "?" to the manual
    createLinkColor();
    noVinylControlHint->setText(
            // TODO That link is kinda pointless as it does not clarify
            // why VC is not available
            tr("This version of Mixxx does not support vinyl control.\n"
               "Please visit %1 for more information.")
                    .arg(coloredLinkString(
                            m_pLinkColor,
                            MIXXX_WEBSITE_SHORT_URL,
                            MIXXX_WEBSITE_URL)));
}

DlgPrefNoVinyl::~DlgPrefNoVinyl() {
}
