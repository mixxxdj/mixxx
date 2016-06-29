#include "library/dlgmissing.h"

#include "library/missingtablemodel.h"
#include "widget/wtracktableview.h"
#include "util/assert.h"

DlgMissing::DlgMissing(QWidget* parent, TrackCollection *pTrackCollection)
         : QWidget(parent),
           Ui::DlgMissing() {
    setupUi(this);    
    m_pMissingTableModel = new MissingTableModel(this, pTrackCollection);
 
    connect(btnPurge, SIGNAL(clicked()), this, SLOT(onShow()));
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

void DlgMissing::setTrackTable(WTrackTableView* pTrackTableView) {
    pTrackTableView->loadTrackModel(m_pMissingTableModel);
    
    connect(btnPurge, SIGNAL(clicked()), pTrackTableView, SLOT(slotPurge()));
}

void DlgMissing::setSelectedIndexes(const QModelIndexList& selectedIndexes) {
    activateButtons(!selectedIndexes.isEmpty());
}

void DlgMissing::activateButtons(bool enable) {
    btnPurge->setEnabled(enable);
}
