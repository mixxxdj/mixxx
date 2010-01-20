#include <QWidget>
#include "widget/wpreparelibrarytableview.h"

WPrepareLibraryTableView::WPrepareLibraryTableView(QWidget* parent,
                                     ConfigObject<ConfigValue>* pConfig,
                                     ConfigKey headerStateKey,
                                     ConfigKey vScrollBarPosKey) :
                    WTrackTableView(parent, pConfig)
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

