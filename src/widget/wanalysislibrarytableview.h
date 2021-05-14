#pragma once

#include <QWidget>

#include "preferences/usersettings.h"
#include "widget/wtracktableview.h"

class WAnalysisLibraryTableView : public WTrackTableView {
  public:
    WAnalysisLibraryTableView(
            QWidget* parent,
            UserSettingsPointer pConfig,
            Library* pLibrary,
            double trackTableBackgroundColorOpacity);

    void onSearch(const QString& text) override;
};
