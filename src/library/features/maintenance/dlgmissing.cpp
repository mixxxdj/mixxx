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
}

void DlgMissing::onShow() {
    VERIFY_OR_DEBUG_ASSERT(!m_pMissingTableModel.isNull())
        return;
    
    m_pMissingTableModel->select();
    // no buttons can be selected
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
