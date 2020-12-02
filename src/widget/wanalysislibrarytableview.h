#ifndef WANALYSISLIBRARYTABLEVIEW_H
#define WANALYSISLIBRARYTABLEVIEW_H

#include <QString>
#include <QWidget>

#include "preferences/usersettings.h"
#include "widget/wtracktableview.h"

class QWidget;
class TrackCollectionManager;

class WAnalysisLibraryTableView : public WTrackTableView {
  public:
    WAnalysisLibraryTableView(
            QWidget* parent,
            UserSettingsPointer pConfig,
            TrackCollectionManager* pTrackCollectionManager,
            double trackTableBackgroundColorOpacity);

    void onSearch(const QString& text) override;
};

#endif
