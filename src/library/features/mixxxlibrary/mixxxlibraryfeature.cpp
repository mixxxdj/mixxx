// mixxxlibraryfeature.cpp
// Created 8/23/2009 by RJ Ryan (rryan@mit.edu)

#include <QDebug>
#include <QMenu>
#include <QStringList>
#include <QVBoxLayout>

#include "library/features/mixxxlibrary/mixxxlibraryfeature.h"

#include "library/basetrackcache.h"
#include "library/librarytablemodel.h"
#include "library/parser.h"
#include "library/queryutil.h"
#include "library/trackcollection.h"
#include "sources/soundsourceproxy.h"
#include "util/dnd.h"
#include "widget/wlibrarysidebar.h"
#include "widget/wlibrarystack.h"
#include "widget/wtracktableview.h"

const QString MixxxLibraryFeature::kLibraryTitle = tr("Library");

const QStringList MixxxLibraryFeature::kGroupingText = 
    QStringList::fromStdList({
        tr("Artist > Album"),
        tr("Album"),
        tr("Genre > Artist > Album"),
        tr("Genre > Album")
});

const QList<QStringList> MixxxLibraryFeature::kGroupingOptions =
    QList<QStringList>::fromStdList({
        QStringList::fromStdList({ LIBRARYTABLE_ARTIST, LIBRARYTABLE_ALBUM }),
        QStringList::fromStdList({ LIBRARYTABLE_ALBUM }),
        QStringList::fromStdList({ LIBRARYTABLE_GENRE, LIBRARYTABLE_ARTIST, 
                                   LIBRARYTABLE_ALBUM }),
        QStringList::fromStdList({ LIBRARYTABLE_GENRE, LIBRARYTABLE_ALBUM })
});

MixxxLibraryFeature::MixxxLibraryFeature(UserSettingsPointer pConfig,
                                         Library* pLibrary,
                                         QObject* parent,
                                         TrackCollection* pTrackCollection)
        : LibraryFeature(pConfig, pLibrary, pTrackCollection, parent),
          m_trackDao(pTrackCollection->getTrackDAO()) {

    m_pBaseTrackCache = pTrackCollection->getTrackSource();
    connect(&m_trackDao, SIGNAL(trackDirty(TrackId)),
            m_pBaseTrackCache.data(), SLOT(slotTrackDirty(TrackId)));
    connect(&m_trackDao, SIGNAL(trackClean(TrackId)),
            m_pBaseTrackCache.data(), SLOT(slotTrackClean(TrackId)));
    connect(&m_trackDao, SIGNAL(trackChanged(TrackId)),
            m_pBaseTrackCache.data(), SLOT(slotTrackChanged(TrackId)));
    connect(&m_trackDao, SIGNAL(tracksAdded(QSet<TrackId>)),
            m_pBaseTrackCache.data(), SLOT(slotTracksAdded(QSet<TrackId>)));
    connect(&m_trackDao, SIGNAL(tracksRemoved(QSet<TrackId>)),
            m_pBaseTrackCache.data(), SLOT(slotTracksRemoved(QSet<TrackId>)));
    connect(&m_trackDao, SIGNAL(dbTrackAdded(TrackPointer)),
            m_pBaseTrackCache.data(), SLOT(slotDbTrackAdded(TrackPointer)));

    setChildModel(new MixxxLibraryTreeModel(this, m_pTrackCollection, m_pConfig));
    m_pLibraryTableModel = new LibraryTableModel(this, pTrackCollection, "mixxx.db.model.library");
}

MixxxLibraryFeature::~MixxxLibraryFeature() {
    delete m_pChildModel;
    delete m_pLibraryTableModel;
}

QVariant MixxxLibraryFeature::title() {
    return kLibraryTitle;
}

QString MixxxLibraryFeature::getIconPath() {
    return ":/images/library/ic_library_library.png";
}

QString MixxxLibraryFeature::getSettingsName() const {
    return "MixxxLibraryFeature";
}

TreeItemModel* MixxxLibraryFeature::getChildModel() {
    return m_pChildModel;
}

QWidget* MixxxLibraryFeature::createInnerSidebarWidget(KeyboardEventFilter* pKeyboard) {
    m_pSidebar = createLibrarySidebarWidget(pKeyboard);
    m_pSidebar->setIconSize(m_pChildModel->getDefaultIconSize());    
    m_pChildModel->reloadTree();
    return m_pSidebar;
}

void MixxxLibraryFeature::refreshLibraryModels() {
    if (m_pLibraryTableModel) {
        m_pLibraryTableModel->select();
    }
}

void MixxxLibraryFeature::onSearch(const QString&) {
    showBreadCrumb();
    if (!m_pSidebar.isNull()) {
        m_pSidebar->clearSelection();
    }
}

void MixxxLibraryFeature::setChildModel(TreeItemModel* pChild) {
    if (!m_pChildModel.isNull()) {
        delete m_pChildModel;
    }
    
    m_pChildModel = pChild;
    connect(&m_trackDao, SIGNAL(trackChanged(TrackId)),
            m_pChildModel, SLOT(reloadTree()));
    connect(&m_trackDao, SIGNAL(tracksRemoved(QSet<TrackId>)),
            m_pChildModel, SLOT(reloadTree()));
    connect(&m_trackDao, SIGNAL(tracksAdded(QSet<TrackId>)),
            m_pChildModel, SLOT(reloadTree()));
}

void MixxxLibraryFeature::activate() {
    if (m_lastClickedIndex.isValid()) {
        activateChild(m_lastClickedIndex);
        return;
    }
    
    //qDebug() << "MixxxLibraryFeature::activate()";
    showTrackModel(m_pLibraryTableModel);
    restoreSearch("");
    showBreadCrumb();
    
}

void MixxxLibraryFeature::activateChild(const QModelIndex& index) {
    m_lastClickedIndex = index;
    if (!index.isValid()) return;

    QString query = index.data(AbstractRole::RoleQuery).toString();
    //qDebug() << "MixxxLibraryFeature::activateChild" << query;
    
    if (query == "$groupingSettings$") {
        // Act as right click
        onRightClickChild(QCursor::pos(), QModelIndex());
        return;
    }
    
    m_pLibraryTableModel->search(query);
    switchToFeature();
    showBreadCrumb(index.data(AbstractRole::RoleBreadCrumb).toString(), getIcon());
    restoreSearch(query);
}

void MixxxLibraryFeature::invalidateChild() {
    m_lastClickedIndex = QModelIndex();
}

void MixxxLibraryFeature::onRightClickChild(const QPoint& pos, 
                                            const QModelIndex&) {
    
    // Create the sort menu
    QMenu menu;    
    QVariant varSort = m_pChildModel->data(QModelIndex(), 
                                           AbstractRole::RoleSettings);
    QStringList currentSort = varSort.toStringList();
    
    QActionGroup* orderGroup = new QActionGroup(&menu);
    for (int i = 0; i < kGroupingOptions.size(); ++i) {
        QAction* action = menu.addAction(kGroupingText.at(i));
        action->setActionGroup(orderGroup);
        action->setData(kGroupingOptions.at(i));
        action->setCheckable(true);
        action->setChecked(currentSort == kGroupingOptions.at(i));
    }
    
    QAction* selected = menu.exec(pos);
    if (selected == nullptr) {
        return;
    }
    if (!m_pGroupingCombo.isNull()) {
        int index = kGroupingOptions.indexOf(selected->data().toStringList());
        m_pGroupingCombo->setCurrentIndex(index);
    }
    setTreeSettings(selected->data());
}

bool MixxxLibraryFeature::dropAccept(QList<QUrl> urls, QObject* pSource) {
    if (pSource) {
        return false;
    } else {
        QList<QFileInfo> files = 
                DragAndDropHelper::supportedTracksFromUrls(urls, false, true);

        // Adds track, does not insert duplicates, handles unremoving logic.
        QList<TrackId> trackIds = m_trackDao.addMultipleTracks(files, true);
        m_pChildModel->reloadTree();
        return trackIds.size() > 0;
    }
}

bool MixxxLibraryFeature::dragMoveAccept(QUrl url) {
    return SoundSourceProxy::isUrlSupported(url) ||
            Parser::isPlaylistFilenameSupported(url.toLocalFile());
}

void MixxxLibraryFeature::setTreeSettings(const QVariant& settings) {
    if (m_pChildModel.isNull()) {
        return;
    }
    m_pChildModel->setData(QModelIndex(), settings, AbstractRole::RoleSettings);
    m_pChildModel->reloadTree();
}
