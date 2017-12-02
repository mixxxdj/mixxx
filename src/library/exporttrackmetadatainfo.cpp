#include "library/exporttrackmetadatainfo.h"

#include <QMessageBox>


namespace mixxx {

//static
bool ExportTrackMetadataInfo::s_bShownOncePerSession = false;

void ExportTrackMetadataInfo::showMessageBox() {
    if (!s_bShownOncePerSession) {
        QMessageBox::information(
                nullptr,
                tr("Export Modified Track Metadata"),
                tr("Mixxx may wait to modify files until it is certain that writing into those files will not cause audible glitches. "
                        "If you do not see changed metadata in other programs, close Mixxx to modify the files immediately."));
        s_bShownOncePerSession = true;
    }
}

} // namespace mixxx
