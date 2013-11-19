#include "dlgmissing.h"

#include "library/missingtablemodel.h"
#include "widget/wtracktableview.h"

DlgMissing::DlgMissing(QWidget* parent, ConfigObject<ConfigValue>* pConfig,
                     TrackCollection* pTrackCollection, MixxxKeyboard* pKeyboard)
         : QWidget(parent),
           Ui::DlgMissing(),
           m_pTrackTableView(
               new WTrackTableView(this,pConfig,pTrackCollection, false)) {
    setupUi(this);
    m_pTrackTableView->installEventFilter(pKeyboard);

    // Install our own trackTable
    QBoxLayout* box = dynamic_cast<QBoxLayout*>(layout());
    Q_ASSERT(box); //Assumes the form layout is a QVBox/QHBoxLayout!
    box->removeWidget(m_pTrackTablePlaceholder);
    m_pTrackTablePlaceholder->hide();
    box->insertWidget(1, m_pTrackTableView);

    m_pMissingTableModel = new MissingTableModel(this, pTrackCollection);
    m_pTrackTableView->loadTrackModel(m_pMissingTableModel);

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
}

DlgMissing::~DlgMissing() {
    // Delete m_pTrackTableView before the table model. This is because the
    // table view saves the header state using the model.
    delete m_pTrackTableView;
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

void DlgMissing::selectAll() {
    m_pTrackTableView->selectAll();
}

void DlgMissing::activateButtons(bool enable) {
    btnPurge->setEnabled(enable);
}

void DlgMissing::selectionChanged(const QItemSelection &selected,
                                  const QItemSelection &deselected) {
    Q_UNUSED(deselected);
    activateButtons(!selected.indexes().isEmpty());
}
