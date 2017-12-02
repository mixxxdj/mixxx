#include "library/exporttrackmetadatainfo.h"

#include <QMessageBox>


namespace mixxx {

void ExportTrackMetadataInfo::showMessageBox() {
    QMessageBox::information(
            nullptr,
            tr("Export Modified Track Metadata"),
            tr("Mixxx may wait to modify files until it is certain that writing into those files will not cause audible glitches. "
                    "If you do not see changed metadata in other programs, close Mixxx to modify the files immediately."));
}

} // namespace mixxx
