
#include "library/trackcollection.h"
#include "widget/wpreparelibrarytableview.h"

WPrepareLibraryTableView::WPrepareLibraryTableView(QWidget* parent,
                                                   ConfigObject<ConfigValue>* pConfig,
                                                   TrackCollection* pTrackCollection,
                                                   ConfigKey headerStateKey,
                                                   ConfigKey vScrollBarPosKey)
        : WTrackTableView(parent, pConfig, pTrackCollection)
{
    setDragDropMode(QAbstractItemView::DragOnly);
    setDragEnabled(true); //Always enable drag for now (until we have a model that doesn't support this.)

}

WPrepareLibraryTableView::~WPrepareLibraryTableView()
{
}

void WPrepareLibraryTableView::onSearchStarting() {
}
void WPrepareLibraryTableView::onSearchCleared() {
}
void WPrepareLibraryTableView::onSearch(const QString& text) {
}

