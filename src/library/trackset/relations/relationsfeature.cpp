#include "library/trackset/relations/relationsfeature.h"

#include <QAction>
#include <QFileInfo>
#include <QMenu>
#include <QPushButton>
#include <QStandardPaths>
#include <QStringList>

#include "library/library.h"
#include "library/treeitem.h"
#include "moc_relationsfeature.cpp"
#include "widget/wlibrary.h"
#include "widget/wlibrarysidebar.h"
#include "widget/wlibrarytextbrowser.h"

namespace {

const QString kViewName = QStringLiteral("RELATIONSHOME");

} // anonymous namespace

RelationsFeature::RelationsFeature(Library* pLibrary, UserSettingsPointer pConfig)
        : BaseTrackSetFeature(pLibrary, pConfig, kViewName, QStringLiteral("relations")) {
    // The invisible root item of the child model
    std::unique_ptr<TreeItem> pRootItem = TreeItem::newRoot(this);

    m_pAllRelationsItem = pRootItem->appendChild(tr("All Relations"), ALL_RELATIONS_NODE);
    m_pDeck1RelationsItem = pRootItem->appendChild(tr("Deck 1", DECK_1_NODE));
    m_pDeck2RelationsItem = pRootItem->appendChild(tr("Deck 2", DECK_2_NODE));
    m_pDeck3RelationsItem = pRootItem->appendChild(tr("Deck 3", DECK_3_NODE));
    m_pDeck4RelationsItem = pRootItem->appendChild(tr("Deck 4", DECK_4_NODE));

    // initialize the model
    m_pSidebarModel->setRootItem(std::move(pRootItem));
}

QVariant RelationsFeature::title() {
    return tr("Relations");
}

void RelationsFeature::bindLibraryWidget(WLibrary* libraryWidget, KeyboardEventFilter* keyboard) {
    Q_UNUSED(keyboard);
    WLibraryTextBrowser* edit = new WLibraryTextBrowser(libraryWidget);
    edit->setHtml(getRootViewHtml());
    libraryWidget->registerView(kViewName, edit);
}

void RelationsFeature::bindSidebarWidget(WLibrarySidebar* pSidebarWidget) {
    m_pSidebarWidget = pSidebarWidget;
}

TreeItemModel* RelationsFeature::sidebarModel() const {
    return m_pSidebarModel;
}

void RelationsFeature::activate() {
    emit switchToView(kViewName);
}

QString RelationsFeature::getRootViewHtml() const {
    QString browseTitle = tr("Relations");
    QString browseSummary = tr(
            "\"Relations\" allows you to browse all "
            "relations or those of a track loaded in a specific deck");

    QString html;
    html.append(QString("<h2>%1</h2>").arg(browseTitle));
    html.append(QString("<p>%1</p>").arg(browseSummary));
    return html;
}
