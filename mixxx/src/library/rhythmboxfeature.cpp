#include <QMessageBox>
#include <QtDebug>
#include <QStringList>

#include "library/proxytrackmodel.h"
#include "library/rhythmboxtrackmodel.h"
#include "library/rhythmboxplaylistmodel.h"
#include "library/rhythmboxfeature.h"
#include "treeitem.h"

RhythmboxFeature::RhythmboxFeature(QObject* parent, TrackCollection* pTrackCollection)
    : LibraryFeature(parent),
      m_pTrackCollection(pTrackCollection),
      m_database(m_pTrackCollection->getDatabase()) {

    m_pRhythmboxTrackModel = new RhythmboxTrackModel(this, m_pTrackCollection);
    m_pRhythmboxPlaylistModel = new RhythmboxPlaylistModel(this, m_pTrackCollection);
    m_isActivated =  false;
}

RhythmboxFeature::~RhythmboxFeature() {

}

bool RhythmboxFeature::isSupported() {
    return (QFile::exists(QDir::homePath() + "/.gnome2/rhythmbox/rhythmdb.xml") ||
            QFile::exists(QDir::homePath() + "/.local/share/rhythmbox/rhythmdb.xml"));
}

QVariant RhythmboxFeature::title() {
    return tr("Rhythmbox");
}

QIcon RhythmboxFeature::getIcon() {
    return QIcon(":/images/library/ic_library_rhythmbox.png");
}

TreeItemModel* RhythmboxFeature::getChildModel() {
    return &m_childModel;
}

void RhythmboxFeature::activate() {
     qDebug() << "RhythmboxFeature::activate()";
    
    if(!m_isActivated){
        if (QMessageBox::question(
            NULL,
            tr("Load Rhythmbox Library?"),
            tr("Would you like to load your Rhythmbox library?"),
            QMessageBox::Ok,
            QMessageBox::Cancel)
            == QMessageBox::Cancel) {
            return;
        }
        if(importMusicCollection() && importPlaylists())
            m_isActivated =  true;
    }
    emit(showTrackModel(m_pRhythmboxTrackModel));
}

void RhythmboxFeature::activateChild(const QModelIndex& index) {
    //qDebug() << "RhythmboxFeature::activateChild()" << index;
    QString playlist = index.data().toString();
    qDebug() << "Activating " << playlist;
    m_pRhythmboxPlaylistModel->setPlaylist(playlist);
    emit(showTrackModel(m_pRhythmboxPlaylistModel));
}

void RhythmboxFeature::onRightClick(const QPoint& globalPos) {
}

void RhythmboxFeature::onRightClickChild(const QPoint& globalPos, QModelIndex index) {
}

bool RhythmboxFeature::dropAccept(QUrl url) {
    return false;
}

bool RhythmboxFeature::dropAcceptChild(const QModelIndex& index, QUrl url) {
    return false;
}

bool RhythmboxFeature::dragMoveAccept(QUrl url) {
    return false;
}

bool RhythmboxFeature::dragMoveAcceptChild(const QModelIndex& index, QUrl url) {
    return false;
}
bool RhythmboxFeature::importMusicCollection()
{
    /*
     * Try and open the Rhythmbox DB. An API call which tells us where
     * the file is would be nice.
     */
    QFile db(QDir::homePath() + "/.gnome2/rhythmbox/rhythmdb.xml");
    if ( ! db.exists()) {
        db.setFileName(QDir::homePath() + "/.local/share/rhythmbox/rhythmdb.xml");
        if ( ! db.exists())
            return false;
    }

    if (!db.open(QIODevice::ReadOnly | QIODevice::Text))
        return false;

    return true;

}
bool RhythmboxFeature::importPlaylists()
{
    QFile db(QDir::homePath() + "/.gnome2/rhythmbox/playlists.xml");
    if ( ! db.exists()) {
        db.setFileName(QDir::homePath() + "/.local/share/rhythmbox/playlists.xml");
        if ( ! db.exists())
            return false;
    }
}

