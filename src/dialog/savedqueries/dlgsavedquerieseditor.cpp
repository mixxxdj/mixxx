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
    auto pSaveModel = make_parented<SavedQueriesTableModel>(m_pFeature, 
            m_pTrackCollection->getSavedQueriesDAO(), parent);
    tableView->setModel(pSaveModel.get());
    int numColumns = static_cast<int>(SavedQueryColumns::NUM_COLUMNS);
    for (int i = 0; i < numColumns; ++i) {
        tableView->setColumnHidden(i, pSaveModel->isColumnInternal(i));
    }
    
    tableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    tableView->setSelectionMode(QAbstractItemView::SingleSelection);
    tableView->verticalHeader()->hide();
    
    connect(pushRemove, SIGNAL(pressed()), this, SLOT(removeQuery()));
}

void DlgSavedQueriesEditor::accept() {
    tableView->model()->submit();
    QDialog::accept();
}

void DlgSavedQueriesEditor::removeQuery() {
    auto model = tableView->selectionModel();
    if (model == nullptr) return;
    
    QModelIndexList selected = model->selectedRows();
    
    QSet<int> removedRows;
    for (const auto& index : selected) {
        removedRows << index.row();
    }
    
    for (int row : removedRows) {
        tableView->model()->removeRow(row);
    }
}
