#include "QItemSelection"

#include "library/dlghidden.h"
#include "library/hiddentablemodel.h"
#include "widget/wtracktableview.h"
#include "util/assert.h"

DlgHidden::DlgHidden(QWidget* parent, TrackCollection* pTrackCollection)
         : QWidget(parent),
           Ui::DlgHidden() {
    setupUi(this);
    
    connect(btnPurge, SIGNAL(clicked()), this, SLOT(onShow()));
    connect(btnSelect, SIGNAL(clicked()), this, SIGNAL(selectAll()));

    m_pHiddenTableModel = new HiddenTableModel(this, pTrackCollection);
}

DlgHidden::~DlgHidden() {
    // Delete m_pTrackTableView before the table model. This is because the
    // table view saves the header state using the model.
    delete m_pHiddenTableModel;
}

void DlgHidden::onShow() {
    m_pHiddenTableModel->select();
    // no buttons can be selected
    activateButtons(false);
}

void DlgHidden::setTrackTable(WTrackTableView *pTrackTableView) {
    pTrackTableView->loadTrackModel(m_pHiddenTableModel);
    
    connect(btnUnhide, SIGNAL(clicked()), pTrackTableView, SLOT(slotUnhide()));
    connect(btnPurge, SIGNAL(clicked()), pTrackTableView, SLOT(slotPurge()));
}

void DlgHidden::setSelectedIndexes(const QModelIndexList& selectedIndexes) {
    activateButtons(!selectedIndexes.empty());
}

void DlgHidden::activateButtons(bool enable) {
    btnPurge->setEnabled(enable);
    btnUnhide->setEnabled(enable);
}
