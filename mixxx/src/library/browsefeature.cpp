// browsefeature.cpp
// Created 9/8/2009 by RJ Ryan (rryan@mit.edu)

#include <QStringList>
#include <QTreeView>
#include <QDirModel>
#include <QStringList>

#include "trackinfoobject.h"
#include "library/browsefeature.h"
#include "library/browsefilter.h"
#include "library/libraryview.h"
#include "library/trackcollection.h"
#include "library/dao/trackdao.h"
#include "widget/wwidget.h"
#include "widget/wskincolor.h"
#include "widget/wlibrary.h"
#include "widget/wlibrarysidebar.h"
#include "widget/wlibrarytableview.h"
#include "widget/wbrowsetableview.h"

BrowseFeature::BrowseFeature(QObject* parent, ConfigObject<ConfigValue>* pConfig, TrackCollection* pTrackCollection)
        : LibraryFeature(parent),
          m_pConfig(pConfig),
          m_browseModel(this),
          m_proxyModel(this),
          m_pTrackCollection(pTrackCollection) {
    m_proxyModel.setSourceModel(&m_browseModel);
    connect(this, SIGNAL(setRootIndex(const QModelIndex&)),
            &m_proxyModel, SLOT(setProxyParent(const QModelIndex&)));
}

BrowseFeature::~BrowseFeature() {

}

QVariant BrowseFeature::title() {
    return QVariant(tr("Browse"));
}

QIcon BrowseFeature::getIcon() {
    return QIcon(":/images/library/ic_library_browse.png");
}

QAbstractItemModel* BrowseFeature::getChildModel() {
    return &m_childModel;
}

bool BrowseFeature::dropAccept(QUrl url) {
    return false;
}

bool BrowseFeature::dropAcceptChild(const QModelIndex& index, QUrl url) {
    return false;
}

bool BrowseFeature::dragMoveAccept(QUrl url) {
    return false;
}

bool BrowseFeature::dragMoveAcceptChild(const QModelIndex& index, QUrl url) {
    return false;
}

void BrowseFeature::bindWidget(WLibrarySidebar* sidebarWidget,
                               WLibrary* libraryWidget,
                               MixxxKeyboard* keyboard) {
    WBrowseTableView* pBrowseView = new WBrowseTableView(libraryWidget,
                                                         m_pConfig);

    connect(pBrowseView, SIGNAL(activated(const QModelIndex &)),
            this, SLOT(onFileActivate(const QModelIndex &)));
    connect(pBrowseView, SIGNAL(loadToPlayer(const QModelIndex&, int)),
            this, SLOT(loadToPlayer(const QModelIndex&, int)));
    connect(this, SIGNAL(setRootIndex(const QModelIndex&)),
            pBrowseView, SLOT(setRootIndex(const QModelIndex&)));
    connect(pBrowseView, SIGNAL(search(const QString&)),
            this, SLOT(search(const QString&)));
    connect(pBrowseView, SIGNAL(searchCleared()),
            this, SLOT(searchCleared()));
    connect(pBrowseView, SIGNAL(searchStarting()),
            this, SLOT(searchStarting()));

    pBrowseView->setDragEnabled(true);
    pBrowseView->setDragDropMode(QAbstractItemView::DragDrop);
    pBrowseView->setAcceptDrops(false);
    pBrowseView->setModel(&m_proxyModel);

    QString startPath = m_pConfig->getValueString(ConfigKey("[Playlist]","Directory"));
    m_browseModel.setRootPath(startPath);
    QModelIndex startIndex = m_browseModel.index(startPath);
    QModelIndex proxyIndex = m_proxyModel.mapFromSource(startIndex);
    emit(setRootIndex(proxyIndex));

    libraryWidget->registerView("BROWSE", pBrowseView);
}

void BrowseFeature::activate() {
    emit(switchToView("BROWSE"));
    emit(restoreSearch(m_currentSearch));
}

void BrowseFeature::activateChild(const QModelIndex&) {
}

void BrowseFeature::onRightClick(const QPoint& globalPos) {
}

void BrowseFeature::onRightClickChild(const QPoint& globalPos, QModelIndex index) {
}

void BrowseFeature::onFileActivate(const QModelIndex& index) {
    QModelIndex sourceIndex = m_proxyModel.mapToSource(index);
    QString path = m_browseModel.filePath(sourceIndex);
    QFileInfo info(path);
    QString absPath = info.absoluteFilePath();
    qDebug() << "activate()" << path;

    if (m_browseModel.isDir(sourceIndex)) {
        m_browseModel.setRootPath(absPath);
        QModelIndex absIndex = m_browseModel.index(absPath);
        QModelIndex absIndexProxy = m_proxyModel.mapFromSource(absIndex);
        emit(setRootIndex(absIndexProxy));
    } else {
        TrackDAO& trackDao = m_pTrackCollection->getTrackDAO();
        TrackInfoObject* track = trackDao.getTrack(trackDao.getTrackId(absPath));

        // The track doesn't exist in the database.
        if (track == NULL) {
            track = new TrackInfoObject(info);
        }

        emit(loadTrack(track));
    }
}

void BrowseFeature::loadToPlayer(const QModelIndex& index, int player) {
    QModelIndex sourceIndex = m_proxyModel.mapToSource(index);
    QString path = m_browseModel.filePath(sourceIndex);
    QFileInfo info(path);
    QString absPath = info.absoluteFilePath();

    if (!m_browseModel.isDir(sourceIndex)) {
        TrackDAO& trackDao = m_pTrackCollection->getTrackDAO();
        TrackInfoObject* track = trackDao.getTrack(trackDao.getTrackId(absPath));

        // The track doesn't exist in the database.
        if (track == NULL) {
            track = new TrackInfoObject(info);
        }

        emit(loadTrackToPlayer(track, player));
    }
}

void BrowseFeature::searchStarting() {
    m_currentSearch = "";
}

void BrowseFeature::search(const QString& text) {
    m_currentSearch = text;
    m_proxyModel.setFilterFixedString(text);
}

void BrowseFeature::searchCleared() {
    m_currentSearch = "";
    m_proxyModel.setFilterFixedString("");
}
