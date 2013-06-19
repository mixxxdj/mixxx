#ifndef WPREPARELIBRARYTABLEVIEW_H
#define WPREPARELIBRARYTABLEVIEW_H

#include <QWidget>

#include "configobject.h"
#include "widget/wtracktableview.h"

class TrackCollection;

class WPrepareLibraryTableView : public WTrackTableView
{
    public:
        WPrepareLibraryTableView(QWidget* parent, ConfigObject<ConfigValue>* pConfig,
                                 TrackCollection* pTrackCollection);
        ~WPrepareLibraryTableView();

        virtual void onSearchStarting();
        virtual void onSearchCleared();
        virtual void onSearch(const QString& text);
};

#endif
