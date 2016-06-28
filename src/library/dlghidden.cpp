#include "QItemSelection"

#include "library/dlghidden.h"
#include "library/hiddentablemodel.h"
#include "widget/wtracktableview.h"
#include "util/assert.h"

DlgHidden::DlgHidden(QWidget* parent, TrackCollection* pTrackCollection)
         : QWidget(parent),
           Ui::DlgHidden(),
           m_focusedPane(-1) {
    setupUi(this);
    
    connect(btnPurge, SIGNAL(clicked()), this, SLOT(clicked()));
    connect(btnSelect, SIGNAL(clicked()), this, SLOT(selectAll()));

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

void DlgHidden::setTrackTable(Library* pLibrary, WTrackTableView *pTrackTableView, int paneId) {
    connect(btnUnhide, SIGNAL(clicked()),
            pTrackTableView, SLOT(slotUnhide()));
    connect(btnUnhide, SIGNAL(clicked()),
            this, SLOT(clicked()));
    connect(btnPurge, SIGNAL(clicked()),
            pTrackTableView, SLOT(slotPurge()));
    connect(pTrackTableView->selectionModel(),
            SIGNAL(selectionChanged(const QItemSelection&, const QItemSelection&)),
            this, 
            SLOT(selectionChanged(const QItemSelection&, const QItemSelection&)));
    connect(pTrackTableView, SIGNAL(trackSelected(TrackPointer)),
            this, SIGNAL(trackSelected(TrackPointer)));
    connect(pLibrary, SIGNAL(setTrackTableFont(QFont)),
            pTrackTableView, SLOT(setTrackTableFont(QFont)));
    connect(pLibrary, SIGNAL(setTrackTableRowHeight(int)),
            pTrackTableView, SLOT(setTrackTableRowHeight(int)));
    pTrackTableView->loadTrackModel(m_pHiddenTableModel);
    
    m_trackTableView[paneId] = pTrackTableView;
}

void DlgHidden::clicked() {
    // all marked tracks are gone now anyway
    onShow();
}

void DlgHidden::selectAll() {
    if (m_trackTableView.contains(m_focusedPane)) {
        m_trackTableView[m_focusedPane]->selectAll();
    }
}

void DlgHidden::activateButtons(bool enable) {
    btnPurge->setEnabled(enable);
    btnUnhide->setEnabled(enable);
}

void DlgHidden::selectionChanged(const QItemSelection& selected,
                                 const QItemSelection&) {
    activateButtons(!selected.indexes().isEmpty());
}
