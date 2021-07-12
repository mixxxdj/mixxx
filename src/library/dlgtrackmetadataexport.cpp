#include "library/dlgtrackmetadataexport.h"

#include <QMessageBox>

#include "moc_dlgtrackmetadataexport.cpp"

namespace mixxx {

//static
bool DlgTrackMetadataExport::s_bShownDuringThisSession = false;

void DlgTrackMetadataExport::showMessageBoxOncePerSession() {
    if (!s_bShownDuringThisSession) {
        QMessageBox::information(
                nullptr,
                tr("Export Modified Track Metadata"),
                tr("Mixxx may wait to modify files until they are not loaded to any decks or samplers. "
                        "If you do not see changed metadata in other programs immediately, "
                        "eject the track from all decks and samplers or shutdown Mixxx."));
        s_bShownDuringThisSession = true;
    }
}

} // namespace mixxx
