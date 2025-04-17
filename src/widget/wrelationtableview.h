#pragma once

#include "preferences/usersettings.h"
#include "widget/wtracktableview.h"

class WRelationTableView : public WTrackTableView {
    Q_OBJECT
  public:
    WRelationTableView(
            QWidget* parent,
            UserSettingsPointer pConfig,
            Library* pLibrary,
            double trackTableBackgroundColorOpacity);

  protected:
    void paintEvent(QPaintEvent* event) override;
};
