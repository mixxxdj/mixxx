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
          m_pFeature(pFeature) {
    setupUi(this);
    SavedQueriesTableModel* pTableModel = 
            new SavedQueriesTableModel(m_pFeature, 
                                       m_pTrackCollection->getSavedQueriesDAO(),
                                       parent);
    tableView->setModel(pTableModel);
    for (int i = 0; i < SavedQueryColumns::NUM_COLUMNS; ++i) {
        tableView->setColumnHidden(i, pTableModel->isColumnInternal(i));
    }
}

void DlgSavedQueriesEditor::accept() {
    tableView->model()->submit();
    QDialog::accept();
}
