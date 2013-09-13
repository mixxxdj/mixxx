#include "dlgmissing.h"

#include "library/missingtablemodel.h"
#include "widget/wtracktableview.h"

DlgMissing::DlgMissing(QWidget* parent, ConfigObject<ConfigValue>* pConfig,
                     TrackCollection* pTrackCollection, MixxxKeyboard* pKeyboard)
         : QWidget(parent),
           Ui::DlgMissing(),
           m_pTrackCollection(pTrackCollection),
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
    // tro's lambda idea, this code calls synchronously!
    m_pTrackCollection->callSync(
                [this] (void) {
        m_pMissingTableModel->init();
    }, __PRETTY_FUNCTION__);
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

    connect(this, SIGNAL(activateButtons(bool)),
            this, SLOT(slotActivateButtons(bool)), Qt::BlockingQueuedConnection);
}

DlgMissing::~DlgMissing() {
    // Delete m_pTrackTableView before the table model. This is because the
    // table view saves the header state using the model.
    delete m_pTrackTableView;
    delete m_pMissingTableModel;
}

void DlgMissing::onShow() {
    m_pMissingTableModel->select();
    // no buttons can be selected
    MainExecuter::callAsync([this](void) {
        slotActivateButtons(false);
    });
}

void DlgMissing::clicked() {
    // all marked tracks are gone now anyway
    // tro's lambda idea. This code calls asynchronously!
    m_pTrackCollection->callAsync(
                [this] (void) {
        onShow();
    }, __PRETTY_FUNCTION__);
}

void DlgMissing::onSearch(const QString& text) {
    m_pMissingTableModel->search(text);
}

void DlgMissing::selectAll() {
    m_pTrackTableView->selectAll();
}

void DlgMissing::slotActivateButtons(bool enable) {
    btnPurge->setEnabled(enable);
}

void DlgMissing::selectionChanged(const QItemSelection &selected,
                                  const QItemSelection &deselected) {
    Q_UNUSED(deselected);
    slotActivateButtons(!selected.indexes().isEmpty());
}
