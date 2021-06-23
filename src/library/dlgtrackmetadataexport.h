#pragma once

#include <QDialog>


namespace mixxx {

// This is just an ugly hack to avoid duplicate code and to define
// translatable strings only once. Inheritance from QDialog is
// needed for i18n message strings. The class contains mutable
// static data and must only be used within the UI thread.
class DlgTrackMetadataExport: private QDialog {
    Q_OBJECT
  public:
    static void showMessageBoxOncePerSession();

  private:
    static bool s_bShownDuringThisSession;
};

} // namespace mixxx
