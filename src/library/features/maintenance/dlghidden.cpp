#include "QItemSelection"

#include "library/features/maintenance/dlghidden.h"
#include "library/features/maintenance/hiddentablemodel.h"
#include "widget/wtracktableview.h"
#include "util/assert.h"

DlgHidden::DlgHidden(QWidget* parent)
         : QFrame(parent),
           Ui::DlgHidden() {
    setupUi(this);
    
    connect(btnSelect, SIGNAL(clicked()), this, SIGNAL(selectAll()));
    connect(btnPurge, SIGNAL(clicked()), this, SIGNAL(purge()));
    connect(btnUnhide, SIGNAL(clicked()), this, SIGNAL(unhide()));
}

DlgHidden::~DlgHidden() {
}

void DlgHidden::onShow() {
    VERIFY_OR_DEBUG_ASSERT (!m_pHiddenTableModel.isNull())
        return;
    
    m_pHiddenTableModel->select();
    // no buttons can be selected
    activateButtons(false);
}

void DlgHidden::setSelectedIndexes(const QModelIndexList& selectedIndexes) {
    activateButtons(!selectedIndexes.empty());
}

void DlgHidden::setTableModel(HiddenTableModel* pTableModel) {
    m_pHiddenTableModel = pTableModel;
}

void DlgHidden::activateButtons(bool enable) {
    btnPurge->setEnabled(enable);
    btnUnhide->setEnabled(enable);
}
