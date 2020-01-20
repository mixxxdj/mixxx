#include "library/dlghidden.h"

#include "library/hiddentablemodel.h"
#include "library/trackcollectionmanager.h"
#include "moc_dlghidden.cpp"
#include "util/assert.h"
#include "widget/wlibrary.h"
#include "widget/wtracktableview.h"

DlgHidden::DlgHidden(
        WLibrary* parent,
        UserSettingsPointer pConfig,
        Library* pLibrary,
        KeyboardEventFilter* pKeyboard)
        : QWidget(parent),
          Ui::DlgHidden(),
          m_pTrackTableView(
                  new WTrackTableView(
                          this,
                          pConfig,
                          pLibrary->trackCollections(),
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

    m_pHiddenTableModel = new HiddenTableModel(this, pLibrary->trackCollections());
    m_pTrackTableView->loadTrackModel(m_pHiddenTableModel);

    connect(btnUnhide,
            &QPushButton::clicked,
            m_pTrackTableView,
            &WTrackTableView::slotUnhide);
    connect(btnUnhide,
            &QPushButton::clicked,
            this,
            &DlgHidden::clicked);
    connect(btnPurge,
            &QPushButton::clicked,
            m_pTrackTableView,
            &WTrackTableView::slotPurge);
    connect(btnPurge,
            &QPushButton::clicked,
            this,
            &DlgHidden::clicked);
    connect(btnSelect,
            &QPushButton::clicked,
            this,
            &DlgHidden::selectAll);
    connect(m_pTrackTableView->selectionModel(),
            &QItemSelectionModel::selectionChanged,
            this,
            &DlgHidden::selectionChanged);
    connect(m_pTrackTableView,
            &WTrackTableView::trackSelected,
            this,
            &DlgHidden::trackSelected);

    connect(pLibrary,
            &Library::setTrackTableFont,
            m_pTrackTableView,
            &WTrackTableView::setTrackTableFont);
    connect(pLibrary,
            &Library::setTrackTableRowHeight,
            m_pTrackTableView,
            &WTrackTableView::setTrackTableRowHeight);
    connect(pLibrary,
            &Library::setSelectedClick,
            m_pTrackTableView,
            &WTrackTableView::setSelectedClick);
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

QString DlgHidden::currentSearch() {
    return m_pHiddenTableModel->currentSearch();
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

bool DlgHidden::hasFocus() const {
    return m_pTrackTableView->hasFocus();
}
