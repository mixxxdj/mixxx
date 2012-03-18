#ifndef WSELECTORLIBRARYTABLEVIEW_H
#define WSELECTORLIBRARYTABLEVIEW_H

#include <QWidget>

#include "configobject.h"
#include "widget/wtracktableview.h"

class TrackCollection;

class WSelectorLibraryTableView : public WTrackTableView
{
    public:
        WSelectorLibraryTableView(QWidget* parent, ConfigObject<ConfigValue>* pConfig,
                                 TrackCollection* pTrackCollection,
                                 ConfigKey headerStateKey,
                                 ConfigKey vScrollBarPosKey);
        ~WSelectorLibraryTableView();

        virtual void onSearchStarting();
        virtual void onSearchCleared();
        virtual void onSearch(const QString& text);
};

#endif
