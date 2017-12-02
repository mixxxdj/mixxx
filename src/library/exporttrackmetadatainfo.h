#pragma once

#include <QMessageBox>


namespace mixxx {

class ExportTrackMetadataInfo: private QDialog {
  public:
    static void showMessageBox();
};

} // namespace mixxx
