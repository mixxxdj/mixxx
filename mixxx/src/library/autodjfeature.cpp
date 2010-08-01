// autodjfeature.cpp
// FORK FORK FORK on 11/1/2009 by Albert Santoni (alberts@mixxx.org)
// Created 8/23/2009 by RJ Ryan (rryan@mit.edu)

#include <QtDebug>

#include "library/autodjfeature.h"
#include "library/playlisttablemodel.h"
#include "library/proxytrackmodel.h"
#include "library/trackcollection.h"
#include "dlgautodj.h"
#include "widget/wlibrary.h"
#include "widget/wlibrarysidebar.h"
#include "mixxxkeyboard.h"

const QString AutoDJFeature::m_sAutoDJViewName = QString("Auto DJ");

AutoDJFeature::AutoDJFeature(QObject* parent,
                             ConfigObject<ConfigValue>* pConfig,
                             TrackCollection* pTrackCollection)
        : LibraryFeature(parent),
          m_pConfig(pConfig),
          m_pTrackCollection(pTrackCollection),
          m_playlistDao(pTrackCollection->getPlaylistDAO()) {
}

AutoDJFeature::~AutoDJFeature() {
}

QVariant AutoDJFeature::title() {
    return tr("Auto DJ");
}

QIcon AutoDJFeature::getIcon() {
    return QIcon(":/images/library/ic_library_autodj.png");
}

void AutoDJFeature::bindWidget(WLibrarySidebar* sidebarWidget,
                               WLibrary* libraryWidget,
                               MixxxKeyboard* keyboard) {

    DlgAutoDJ* pAutoDJView = new DlgAutoDJ(libraryWidget,
                                           m_pConfig,
                                           m_pTrackCollection);
    pAutoDJView->installEventFilter(keyboard);
    libraryWidget->registerView(m_sAutoDJViewName, pAutoDJView);
    connect(pAutoDJView, SIGNAL(loadTrack(TrackInfoObject*)),
            this, SIGNAL(loadTrack(TrackInfoObject*)));
    connect(pAutoDJView, SIGNAL(loadTrackToPlayer(TrackInfoObject*, int)),
            this, SIGNAL(loadTrackToPlayer(TrackInfoObject*, int)));
}

QAbstractItemModel* AutoDJFeature::getChildModel() {
    return &m_childModel;
}

void AutoDJFeature::activate() {
    //qDebug() << "AutoDJFeature::activate()";
    //emit(showTrackModel(m_pAutoDJTableModelProxy));
    emit(switchToView("Auto DJ"));
}

void AutoDJFeature::activateChild(const QModelIndex& index) {

}

void AutoDJFeature::onRightClick(const QPoint& globalPos) {
}

void AutoDJFeature::onRightClickChild(const QPoint& globalPos,
                                            QModelIndex index) {
}

bool AutoDJFeature::dropAccept(QUrl url) {

    //TODO: Filter by supported formats regex and reject anything that doesn't match.

    TrackDAO &trackDao = m_pTrackCollection->getTrackDAO();

    //If a track is dropped onto a playlist's name, but the track isn't in the library,
    //then add the track to the library before adding it to the playlist.
    QString location = url.toLocalFile();
    if (!trackDao.trackExistsInDatabase(location))
    {
        trackDao.addTrack(location);
    }
    //Get id of track
    int trackId = trackDao.getTrackId(location);

    int playlistId = m_playlistDao.getPlaylistIdFromName(AUTODJ_TABLE);
    m_playlistDao.appendTrackToPlaylist(trackId, playlistId);
    return true;

}

bool AutoDJFeature::dropAcceptChild(const QModelIndex& index, QUrl url) {
    return false;
}

bool AutoDJFeature::dragMoveAccept(QUrl url) {
    return true;
}

bool AutoDJFeature::dragMoveAcceptChild(const QModelIndex& index,
                                              QUrl url) {
    return false;
}
