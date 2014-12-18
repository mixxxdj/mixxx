#include "QItemSelection"

#include "dlghidden.h"
#include "library/hiddentablemodel.h"
#include "widget/wtracktableview.h"
#include "util/assert.h"

DlgHidden::DlgHidden(QWidget* parent, ConfigObject<ConfigValue>* pConfig,
                     TrackCollection* pTrackCollection, MixxxKeyboard* pKeyboard)
         : QWidget(parent),
           Ui::DlgHidden(),
           m_pTrackTableView(
               new WTrackTableView(this, pConfig, pTrackCollection, false)) {
    setupUi(this);
    m_pTrackTableView->installEventFilter(pKeyboard);

    // Install our own trackTable
    QBoxLayout* box = dynamic_cast<QBoxLayout*>(layout());
    DEBUG_ASSERT_AND_HANDLE(box) { //Assumes the form layout is a QVBox/QHBoxLayout!
    } else {
        box->removeWidget(m_pTrackTablePlaceholder);
        m_pTrackTablePlaceholder->hide();
        box->insertWidget(1, m_pTrackTableView);
    }

    m_pHiddenTableModel = new HiddenTableModel(this, pTrackCollection);
    m_pTrackTableView->loadTrackModel(m_pHiddenTableModel);

    connect(btnUnhide, SIGNAL(clicked()),
            m_pTrackTableView, SLOT(slotUnhide()));
    connect(btnUnhide, SIGNAL(clicked()),
            this, SLOT(clicked()));
    connect(btnPurge, SIGNAL(clicked()),
            m_pTrackTableView, SLOT(slotPurge()));
    connect(btnPurge, SIGNAL(clicked()),
            this, SLOT(clicked()));
    connect(btnSelect, SIGNAL(clicked()),
            this, SLOT(selectAll()));
    connect(m_pTrackTableView->selectionModel(),
            SIGNAL(selectionChanged(const QItemSelection&, const QItemSelection&)),
            this,
            SLOT(selectionChanged(const QItemSelection&, const QItemSelection&)));

    connect(m_pTrackTableView, SIGNAL(trackSelected(TrackPointer)),
            this, SIGNAL(trackSelected(TrackPointer)));
}

DlgHidden::~DlgHidden() {
    // Delete m_pTrackTableView before the table model. This is because the
    // table view saves the header state using the model.
    delete m_pTrackTableView;
    delete m_pHiddenTableModel;
}

void DlgHidden::onShow() {
    m_pHiddenTableModel->select();
    // no buttons can be selected
    activateButtons(false);
}

void DlgHidden::onSearch(const QString& text) {
    m_pHiddenTableModel->search(text);
}

void DlgHidden::clicked() {
    // all marked tracks are gone now anyway
    onShow();
}

void DlgHidden::selectAll() {
    m_pTrackTableView->selectAll();
}

void DlgHidden::activateButtons(bool enable) {
    btnPurge->setEnabled(enable);
    btnUnhide->setEnabled(enable);
}

void DlgHidden::selectionChanged(const QItemSelection &selected,
                                 const QItemSelection &deselected) {
    Q_UNUSED(deselected);
    activateButtons(!selected.indexes().isEmpty());
}
