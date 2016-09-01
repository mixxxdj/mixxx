#include <QSqlTableModel>

#include "dialog/savedqueries/dlgsavedquerieseditor.h"
#include "dialog/savedqueries/savedqueriestablemodel.h"
#include "library/libraryfeature.h"
#include "library/trackcollection.h"

DlgSavedQueriesEditor::DlgSavedQueriesEditor(LibraryFeature* pFeature,
                                             TrackCollection* pTrackCollection,
                                             QWidget* parent)
        : QDialog(parent),
          m_pTrackCollection(pTrackCollection),
          m_savedDAO(m_pTrackCollection->getSavedQueriesDAO()),
          m_pFeature(pFeature) {
    setupUi(this);
    SavedQueriesTableModel* pTableModel = 
            new SavedQueriesTableModel(m_pFeature, parent, 
                                       m_pTrackCollection->getDatabase());
    tableView->setModel(pTableModel);
    for (int i = 0; i < SavedQueryColums::NUM_COLUMNS; ++i) {
        tableView->setColumnHidden(i, pTableModel->isColumnInternal(i));
    }
}
