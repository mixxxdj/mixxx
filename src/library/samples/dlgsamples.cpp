#include "library/samples/dlgsamples.h"

#include <QBoxLayout>
#include <QFileInfo>

#include "controllers/keyboard/keyboardeventfilter.h"
#include "library/library.h"
#include "library/trackcollectionmanager.h"
#include "moc_dlgsamples.cpp"
#include "util/assert.h"
#include "widget/wlibrary.h"
#include "widget/wtracktableview.h"

DlgSamples::DlgSamples(
        WLibrary* parent,
        UserSettingsPointer pConfig,
        Library* pLibrary,
        KeyboardEventFilter* pKeyboard)
        : QWidget(parent),
          m_pConfig(pConfig),
          m_pTrackTableView(
                  new WTrackTableView(
                          this,
                          pConfig,
                          pLibrary,
                          parent->getTrackTableBackgroundColorOpacity())),
          m_browseModel(this, pLibrary->trackCollectionManager(), nullptr),
          m_proxyModel(&m_browseModel, true) {
    // Set up layout with just the track table
    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(m_pTrackTableView);

    m_pTrackTableView->installEventFilter(pKeyboard);

    connect(m_pTrackTableView,
            &WTrackTableView::loadTrack,
            this,
            &DlgSamples::loadTrack);
    connect(m_pTrackTableView,
            &WTrackTableView::loadTrackToPlayer,
            this,
            &DlgSamples::loadTrackToPlayer);
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
    connect(&m_browseModel,
            &BrowseTableModel::restoreModelState,
            m_pTrackTableView,
            &WTrackTableView::restoreCurrentViewState);

    m_proxyModel.setFilterCaseSensitivity(Qt::CaseInsensitive);
    m_proxyModel.setSortCaseSensitivity(Qt::CaseInsensitive);

    refreshBrowseModel();
    m_pTrackTableView->loadTrackModel(&m_proxyModel);
}

DlgSamples::~DlgSamples() {
}

bool DlgSamples::hasFocus() const {
    return m_pTrackTableView->hasFocus();
}

void DlgSamples::setFocus() {
    m_pTrackTableView->setFocus();
}

void DlgSamples::refreshBrowseModel() {
    saveCurrentViewState();
    // Use the app resource path to find samples (works on all platforms including Android)
    QString path = m_pConfig->getResourcePath() + QStringLiteral("samples/");
    QFileInfo fi(path);
    if (!fi.exists() || !fi.isDir()) {
        // Fallback for development builds
        path = QStringLiteral("res/samples/");
    }
    m_browseModel.setPath(mixxx::FileAccess(mixxx::FileInfo(path)));
}

void DlgSamples::onSearch(const QString& text) {
    m_proxyModel.search(text);
}

void DlgSamples::slotRestoreSearch() {
    emit restoreSearch(currentSearch());
}

void DlgSamples::saveCurrentViewState() {
    m_pTrackTableView->saveCurrentViewState();
}

bool DlgSamples::restoreCurrentViewState() {
    return m_pTrackTableView->restoreCurrentViewState();
}
