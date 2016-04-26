#ifndef WANALYSISLIBRARYTABLEVIEW_H
#define WANALYSISLIBRARYTABLEVIEW_H

#include <QWidget>

#include "preferences/usersettings.h"
#include "widget/wtracktableview.h"

class TrackCollection;

class WAnalysisLibraryTableView : public WTrackTableView
{
    public:
        WAnalysisLibraryTableView(QWidget* parent, UserSettingsPointer pConfig,
                                 TrackCollection* pTrackCollection);

        virtual void onSearchStarting();
        virtual void onSearchCleared();
        void onSearch(const QString& text) override;
};

#endif
