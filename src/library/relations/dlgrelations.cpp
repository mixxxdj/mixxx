#include "library/relations/dlgrelations.h"

#include <QItemSelection>

#include "controllers/keyboard/keyboardeventfilter.h"
#include "library/library.h"
#include "library/relations/relationstablemodel.h"
#include "moc_dlgrelations.cpp"
#include "util/assert.h"
#include "widget/wlibrary.h"
#include "widget/wrelationtableview.h"

DlgRelations::DlgRelations(
        WLibrary* parent,
        UserSettingsPointer pConfig,
        Library* pLibrary,
        KeyboardEventFilter* pKeyboard,
        bool relationPairView)
        : QWidget(parent),
          Ui::DlgRelations(),
          m_pRelationTableView(
                  new WRelationTableView(
                          this,
                          pConfig,
                          pLibrary,
                          parent->getTrackTableBackgroundColorOpacity(),
                          relationPairView)),
          m_bRelationPairView(relationPairView) {
    setupUi(this);
    m_pRelationTableView->installEventFilter(pKeyboard);

    // Install modified tracktable
    QBoxLayout* box = qobject_cast<QBoxLayout*>(layout());
    VERIFY_OR_DEBUG_ASSERT(box) { // Assumes the form layout is a QVBox/QHBoxLayout!
    }
    else {
        box->removeWidget(m_pRelationTablePlaceholder);
        m_pRelationTablePlaceholder->hide();
        box->insertWidget(1, m_pRelationTableView);
    }

    m_pRelationTableModel = new RelationsTableModel(this, pLibrary->trackCollectionManager());
    m_pRelationTableView->loadTrackModel(m_pRelationTableModel);

    connect(m_pRelationTableView,
            &WRelationTableView::trackSelected,
            this,
            &DlgRelations::trackSelected);
    connect(m_pRelationTableView,
            &WRelationTableView::loadTrack,
            this,
            &DlgRelations::loadTrack);
    connect(m_pRelationTableView,
            &WRelationTableView::loadTrackToPlayer,
            this,
            &DlgRelations::loadTrackToPlayer);

    connect(pLibrary,
            &Library::setTrackTableFont,
            m_pRelationTableView,
            &WRelationTableView::setTrackTableFont);
    connect(pLibrary,
            &Library::setTrackTableRowHeight,
            m_pRelationTableView,
            &WRelationTableView::setTrackTableRowHeight);
    connect(pLibrary,
            &Library::setSelectedClick,
            m_pRelationTableView,
            &WRelationTableView::setSelectedClick);
}

DlgRelations::~DlgRelations() {
    delete m_pRelationTableView;
    delete m_pRelationTableModel;
}

void DlgRelations::onShow() {
    m_pRelationTableModel->select();
}

void DlgRelations::onSearch(const QString& text) {
    m_pRelationTableModel->search(text);
}

QString DlgRelations::currentSearch() {
    return m_pRelationTableModel->currentSearch();
}

void DlgRelations::showAllRelations() {
    m_pRelationTableModel->showAllRelations();
}

void DlgRelations::showRelatedTracks(TrackPointer pTrack) {
    m_pRelationTableModel->showRelatedTracks(pTrack);
}

bool DlgRelations::hasFocus() const {
    return m_pRelationTableView->hasFocus();
}

void DlgRelations::saveCurrentViewState() {
    m_pRelationTableView->saveCurrentViewState();
}

bool DlgRelations::restoreCurrentViewState() {
    return m_pRelationTableView->restoreCurrentViewState();
}

void DlgRelations::setFocus() {
    m_pRelationTableView->setFocus();
}

void DlgRelations::installEventFilter(QObject* pFilter) {
    QWidget::installEventFilter(pFilter);
    m_pRelationTableView->installEventFilter(pFilter);
}
