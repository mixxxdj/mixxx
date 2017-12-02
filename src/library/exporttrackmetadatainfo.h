#pragma once

#include <QMessageBox>


namespace mixxx {

// This is just an ugly hack to avoid duplicate code and to define
// translatable strings only once.
class ExportTrackMetadataInfo: private QDialog {
  public:
    static void showMessageBox();

  private:
    static bool s_bShownOncePerSession;
};

} // namespace mixxx
