#ifndef WPREPARELIBRARYTABLEVIEW_H
#define WPREPARELIBRARYTABLEVIEW_H

#include "configobject.h"
#include "wtracktableview.h"
class QWidget;

class WPrepareLibraryTableView : public WTrackTableView 
{
    public:
        WPrepareLibraryTableView(QWidget* parent, ConfigObject<ConfigValue>* pConfig,
                            ConfigKey headerStateKey,
                            ConfigKey vScrollBarPosKey);
        ~WPrepareLibraryTableView();

        virtual void onSearchStarting();
        virtual void onSearchCleared();
        virtual void onSearch(const QString& text);
};

#endif
