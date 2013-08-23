#include "QItemSelection"

#include "dlghidden.h"
#include "library/hiddentablemodel.h"
#include "widget/wtracktableview.h"

DlgHidden::DlgHidden(QWidget* parent, ConfigObject<ConfigValue>* pConfig,
                     TrackCollection* pTrackCollection, MixxxKeyboard* pKeyboard)
         : QWidget(parent),
           Ui::DlgHidden(),
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

    connect(this, SIGNAL(activateButtons(bool)),
            this, SLOT(slotActivateButtons(bool)));
}

DlgHidden::~DlgHidden() {
    // Delete m_pTrackTableView before the table model. This is because the
    // table view saves the header state using the model.
    delete m_pTrackTableView;
    delete m_pHiddenTableModel;
}

void DlgHidden::onShow() {
    // tro's lambda idea
    m_pTrackCollection->callAsync(
                [this] (void) {
        m_pHiddenTableModel->select();
        // no buttons can be selected
        emit(activateButtons(false));
    });
}

void DlgHidden::onSearch(const QString& text) {
    // tro's lambda idea
    m_pTrackCollection->callAsync(
                [this, text] (void) {
        m_pHiddenTableModel->search(text);
    });
}

void DlgHidden::clicked() {
    // all marked tracks are gone now anyway
    onShow();
}

void DlgHidden::selectAll() {
    m_pTrackTableView->selectAll();
}

void DlgHidden::slotActivateButtons(bool enable) {
    btnPurge->setEnabled(enable);
    btnUnhide->setEnabled(enable);
}

void DlgHidden::selectionChanged(const QItemSelection &selected,
                                 const QItemSelection &deselected) {
    Q_UNUSED(deselected);
    slotActivateButtons(!selected.indexes().isEmpty());
}
