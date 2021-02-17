#include "library/dlgmissing.h"

#include "library/missingtablemodel.h"
#include "library/trackcollectionmanager.h"
#include "moc_dlgmissing.cpp"
#include "util/assert.h"
#include "widget/wlibrary.h"
#include "widget/wtracktableview.h"

DlgMissing::DlgMissing(
        WLibrary* parent,
        UserSettingsPointer pConfig,
        Library* pLibrary,
        KeyboardEventFilter* pKeyboard)
        : QWidget(parent),
          Ui::DlgMissing(),
          m_pTrackTableView(
                  new WTrackTableView(
                          this,
                          pConfig,
                          pLibrary,
                          parent->getTrackTableBackgroundColorOpacity(),
                          false)) {
    setupUi(this);
    m_pTrackTableView->installEventFilter(pKeyboard);

    // Install our own trackTable
    QBoxLayout* box = qobject_cast<QBoxLayout*>(layout());
    VERIFY_OR_DEBUG_ASSERT(box) { //Assumes the form layout is a QVBox/QHBoxLayout!
    } else {
        box->removeWidget(m_pTrackTablePlaceholder);
        m_pTrackTablePlaceholder->hide();
        box->insertWidget(1, m_pTrackTableView);
    }

    m_pMissingTableModel = new MissingTableModel(this, pLibrary->trackCollections());
    m_pTrackTableView->loadTrackModel(m_pMissingTableModel);

    connect(btnPurge, &QPushButton::clicked, m_pTrackTableView, &WTrackTableView::slotPurge);
    connect(btnPurge, &QPushButton::clicked, this, &DlgMissing::clicked);
    connect(btnSelect, &QPushButton::clicked, this, &DlgMissing::selectAll);
    connect(m_pTrackTableView->selectionModel(),
            &QItemSelectionModel::selectionChanged,
            this,
            &DlgMissing::selectionChanged);
    connect(m_pTrackTableView, &WTrackTableView::trackSelected, this, &DlgMissing::trackSelected);

    connect(pLibrary, &Library::setTrackTableFont, m_pTrackTableView, &WTrackTableView::setTrackTableFont);
    connect(pLibrary, &Library::setTrackTableRowHeight, m_pTrackTableView, &WTrackTableView::setTrackTableRowHeight);
    connect(pLibrary, &Library::setSelectedClick, m_pTrackTableView, &WTrackTableView::setSelectedClick);
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

QString DlgMissing::currentSearch() {
    return m_pMissingTableModel->currentSearch();
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

bool DlgMissing::hasFocus() const {
    return m_pTrackTableView->hasFocus();
}
