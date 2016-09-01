#include "library/features/maintenance/dlgmissing.h"

#include "library/features/maintenance/missingtablemodel.h"
#include "widget/wtracktableview.h"
#include "util/assert.h"

DlgMissing::DlgMissing(QWidget* parent)
         : QFrame(parent),
           Ui::DlgMissing() {
    setupUi(this);    
 
    connect(btnPurge, SIGNAL(clicked()), this, SIGNAL(purge()));
    connect(btnSelect, SIGNAL(clicked()), this, SIGNAL(selectAll()));
}

DlgMissing::~DlgMissing() {
    // Delete m_pTrackTableView before the table model. This is because the
    // table view saves the header state using the model.
    delete m_pMissingTableModel;
}

void DlgMissing::onShow() {
    m_pMissingTableModel->select();
    activateButtons(false);
}

void DlgMissing::setSelectedIndexes(const QModelIndexList& selectedIndexes) {
    activateButtons(!selectedIndexes.isEmpty());
}

void DlgMissing::setTableModel(MissingTableModel* pTableModel) {
    m_pMissingTableModel = pTableModel;
}

void DlgMissing::activateButtons(bool enable) {
    btnPurge->setEnabled(enable);
}
