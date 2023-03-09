#pragma once

#include <QWidget>

#include "preferences/configobject.h"
#include "preferences/usersettings.h"
#include "widget/wtracktableview.h"

class WAnalysisLibraryTableView : public WTrackTableView {
    Q_OBJECT
  public:
    WAnalysisLibraryTableView(
            QWidget* parent,
            UserSettingsPointer pConfig,
            ConfigObject<ConfigValueKbd>* pKbdConfig,
            Library* pLibrary,
            double trackTableBackgroundColorOpacity);

    void onSearch(const QString& text) override;
};
