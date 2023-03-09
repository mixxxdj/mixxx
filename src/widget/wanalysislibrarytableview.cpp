#include "library/trackcollection.h"
#include "preferences/configobject.h"
#include "preferences/keyboardconfig.h"
#include "widget/wanalysislibrarytableview.h"

WAnalysisLibraryTableView::WAnalysisLibraryTableView(
        QWidget* parent,
        UserSettingsPointer pConfig,
        KeyboardConfigPointer pKbdConfig,
        Library* pLibrary,
        double trackTableBackgroundColorOpacity)
        : WTrackTableView(parent,
                  pConfig,
                  pKbdConfig,
                  pLibrary,
                  trackTableBackgroundColorOpacity,
                  true) {
    setDragDropMode(QAbstractItemView::DragOnly);
    setDragEnabled(true); //Always enable drag for now (until we have a model that doesn't support this.)
}

void WAnalysisLibraryTableView::onSearch(const QString& text) {
    Q_UNUSED(text);
}
