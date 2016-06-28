#include "library/dlgmissing.h"

#include "library/missingtablemodel.h"
#include "widget/wtracktableview.h"
#include "util/assert.h"

DlgMissing::DlgMissing(QWidget* parent, UserSettingsPointer pConfig,
                       Library* pLibrary,
                       TrackCollection* pTrackCollection, KeyboardEventFilter* pKeyboard)
         : QWidget(parent),
           Ui::DlgMissing(),
           m_pTrackTableView(
               new WTrackTableView(this, pConfig, pTrackCollection, false)) {
    setupUi(this);    
    m_pMissingTableModel = new MissingTableModel(this, pTrackCollection);
 
    connect(btnPurge, SIGNAL(clicked()), this, SLOT(clicked()));
    connect(btnSelect, SIGNAL(clicked()), this, SLOT(selectAll()));
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

void DlgMissing::clicked() {
    // all marked tracks are gone now anyway
    onShow();
}

void DlgMissing::onSearch(const QString& text) {
    m_pMissingTableModel->search(text);
}

void DlgMissing::setTrackTable(WTrackTableView* pTrackTableView, int paneId) {
    connect(btnPurge, SIGNAL(clicked()),
            pTrackTableView, SLOT(slotPurge()));
    connect(pTrackTableView->selectionModel(),
            SIGNAL(selectionChanged(const QItemSelection&, const QItemSelection&)),
            this,
            SLOT(selectionChanged(const QItemSelection&, const QItemSelection&)));
    connect(pLibrary, SIGNAL(setTrackTableFont(QFont)),
            pTrackTableView, SLOT(setTrackTableFont(QFont)));
    connect(pLibrary, SIGNAL(setTrackTableRowHeight(int)),
            pTrackTableView, SLOT(setTrackTableRowHeight(int)));

    connect(pTrackTableView, SIGNAL(trackSelected(TrackPointer)),
            this, SIGNAL(trackSelected(TrackPointer)));
    
    pTrackTableView->loadTrackModel(m_pMissingTableModel);
    m_trackTableView[paneId] = pTrackTableView;
}

void DlgMissing::selectAll() {    
    if (m_trackTableView.contains(m_focusedPane)) {
        m_trackTableView[m_focusedPane]->selectAll();
    }
}

void DlgMissing::activateButtons(bool enable) {
    btnPurge->setEnabled(enable);
}

void DlgMissing::selectionChanged(const QItemSelection &selected,
                                  const QItemSelection &deselected) {
    Q_UNUSED(deselected);
    activateButtons(!selected.indexes().isEmpty());
}
