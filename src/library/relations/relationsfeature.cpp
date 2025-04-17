#include "library/relations/relationsfeature.h"

#include <QAction>
#include <QFileInfo>
#include <QMenu>
#include <QPushButton>
#include <QStandardPaths>
#include <QStringList>

#include "controllers/keyboard/keyboardeventfilter.h"
#include "library/library.h"
#include "library/relations/dlgrelations.h"
#include "library/treeitem.h"
#include "mixer/playerinfo.h"
#include "mixer/playermanager.h"
#include "moc_relationsfeature.cpp"
#include "widget/wlibrary.h"
#include "widget/wlibrarysidebar.h"
#include "widget/wlibrarytextbrowser.h"

namespace {

const QString kRootViewName = QStringLiteral("RELATIONSHOME");
const QString kEmptyDeckViewName = QStringLiteral("EMPTYDECK");

const QString kAppGroup = QStringLiteral("[App]");

} // anonymous namespace

RelationsFeature::RelationsFeature(Library* pLibrary, UserSettingsPointer pConfig)
        : BaseTrackSetFeature(pLibrary, pConfig, kRootViewName, QStringLiteral("relations")),
          m_pNumDecks(kAppGroup, QStringLiteral("num_decks")) {
    std::unique_ptr<TreeItem> pRootItem = TreeItem::newRoot(this);

    int iNumDecks = static_cast<int>(m_pNumDecks.get());
    for (int i = 1; i <= iNumDecks; ++i) {
        m_DeckRelationItemList.append(pRootItem->appendChild(
                tr("Deck %1").arg(i), PlayerManager::groupForDeck(i - 1)));
    }
    m_pSidebarModel->setRootItem(std::move(pRootItem));
}

QVariant RelationsFeature::title() {
    return tr("Relations");
}

void RelationsFeature::bindLibraryWidget(WLibrary* pLibraryWidget, KeyboardEventFilter* pKeyboard) {
    // Register view for all relations
    m_pRelationView = new DlgRelations(pLibraryWidget, m_pConfig, m_pLibrary, pKeyboard);

    connect(m_pRelationView,
            &DlgRelations::trackSelected,
            this,
            &RelationsFeature::trackSelected);

    m_pRelationView->installEventFilter(pKeyboard);

    pLibraryWidget->registerView(kRootViewName, m_pRelationView);

    // Register view for empty decks
    // WLibraryTextBrowser* editEmptyDeck = new WLibraryTextBrowser(pLibraryWidget);
    // editEmptyDeck->setHtml(getEmptyDeckViewHtml());
    // pLibraryWidget->registerView(kEmptyDeckViewName, editEmptyDeck);
}

void RelationsFeature::bindSidebarWidget(WLibrarySidebar* pSidebarWidget) {
    m_pSidebarWidget = pSidebarWidget;
}

TreeItemModel* RelationsFeature::sidebarModel() const {
    return m_pSidebarModel;
}

void RelationsFeature::activate() {
    m_lastRightClickedIndex = QModelIndex();
    emit saveModelState();
    m_pRelationView->showAllRelations();
    emit switchToView(kRootViewName);
    emit enableCoverArtDisplay(true);
}

/* DECK SPECIFIC
QString RelationsFeature::deckGroupFromIndex(const QModelIndex& index) const {
    TreeItem* item = static_cast<TreeItem*>(index.internalPointer());
    QString deckGroup = item->getData().toString();
    return deckGroup;
}

void RelationsFeature::activateChild(const QModelIndex& index) {
    QString deckGroup = deckGroupFromIndex(index);
    m_deckGroup = deckGroup;
    TrackPointer pTrack = PlayerInfo::instance().getTrackInfo(deckGroup);
    if (!pTrack) {
        emit switchToView(kEmptyDeckViewName);
        return;
    }
    m_lastClickedIndex = index;
    m_lastRightClickedIndex = QModelIndex();
    emit saveModelState();
    m_relationsTableModel.displayRelatedTracks(pTrack);
    emit showTrackModel(&m_relationsTableModel);
    emit enableCoverArtDisplay(true);
}

QString RelationsFeature::getEmptyDeckViewHtml() const {
    QString browseTitle = tr("Selected Deck is Empty");
    QString browseSummary = tr(
            "Load a track to the selected deck to see manually set "
            "relations to the track.");

    QString html;
    html.append(QString("<h2>%1</h2>").arg(browseTitle));
    html.append(QString("<p>%1</p>").arg(browseSummary));
    return html;
}
*/
