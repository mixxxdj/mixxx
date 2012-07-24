
#include "library/trackcollection.h"
#include "widget/wselectorlibrarytableview.h"

WSelectorLibraryTableView::WSelectorLibraryTableView(QWidget* parent,
                                                   ConfigObject<ConfigValue>* pConfig,
                                                   TrackCollection* pTrackCollection,
                                                   ConfigKey headerStateKey,
                                                   ConfigKey vScrollBarPosKey)
        : WTrackTableView(parent, pConfig, pTrackCollection)
{
    setDragDropMode(QAbstractItemView::DragOnly);
    setDragEnabled(true); //Always enable drag for now (until we have a model that doesn't support this.)

}

WSelectorLibraryTableView::~WSelectorLibraryTableView()
{
}

void WSelectorLibraryTableView::onSearchStarting() {
}
void WSelectorLibraryTableView::onSearchCleared() {
}
void WSelectorLibraryTableView::onSearch(const QString& text) {
}

