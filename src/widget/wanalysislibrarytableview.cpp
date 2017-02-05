#include "library/trackcollection.h"
#include "widget/wanalysislibrarytableview.h"

WAnalysisLibraryTableView::WAnalysisLibraryTableView(QWidget* parent,
                                                   UserSettingsPointer pConfig,
                                                   TrackCollection* pTrackCollection)
        : WTrackTableView(parent, pConfig, pTrackCollection) {
    setDragDropMode(QAbstractItemView::DragOnly);
    setDragEnabled(true); //Always enable drag for now (until we have a model that doesn't support this.)
}

void WAnalysisLibraryTableView::onSearchStarting() {
}

void WAnalysisLibraryTableView::onSearchCleared() {
}

void WAnalysisLibraryTableView::onSearch(const QString& text) {
    Q_UNUSED(text);
}
