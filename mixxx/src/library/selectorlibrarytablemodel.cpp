// Created 3/17/2012 by Keith Salisbury (keithsalisbury@gmail.com)

#include <QObject>

#include "selectorlibrarytablemodel.h"
#include "library/trackcollection.h"
#include "basetrackplayer.h"
#include "playerinfo.h"

#include "controlobjectthreadmain.h"
#include "controlobject.h"
#include "trackinfoobject.h"


const QString GENRE_FILTER = "Genre == '%1'";


SelectorLibraryTableModel::SelectorLibraryTableModel(QObject* parent,
                                                   TrackCollection* pTrackCollection)
        : LibraryTableModel(parent, pTrackCollection,
                            "mixxx.db.model.prepare") {

    connect(this, SIGNAL(doSearch(const QString&)),
            this, SLOT(slotSearch(const QString&)));

    connect(this, SIGNAL(doFilterByGenre(const QString&)),
            this, SLOT(slotFilterByGenre(const QString&)));

    connect(this, SIGNAL(doFilterByBpm(const float&)),
            this, SLOT(slotFilterByBpm(const float&)));

    filterByGenre();

}


SelectorLibraryTableModel::~SelectorLibraryTableModel() {
}

bool SelectorLibraryTableModel::isColumnInternal(int column) {
    return LibraryTableModel::isColumnInternal(column);
}

void SelectorLibraryTableModel::search(const QString& searchText) {
    // qDebug() << "SelectorLibraryTableModel::search()" << searchText
    //          << QThread::currentThread();
    emit(doSearch(searchText));
}

void SelectorLibraryTableModel::slotSearch(const QString& searchText) {
    BaseSqlTableModel::search(searchText, QString());
}

void SelectorLibraryTableModel::slotFilterByGenre(const QString& genre) {
    BaseSqlTableModel::search("", QString("Genre == '%1'").arg(genre));
}

void SelectorLibraryTableModel::slotFilterByBpm(const float& bpm) {
    BaseSqlTableModel::search("", QString("Bpm == '%1'").arg(bpm));
}

QString SelectorLibraryTableModel::currentGenre() const {
    /*int decks = ControlObject::getControl(
        ConfigKey("[Master]", "num_decks"))->get();
    // check if file is loaded to a deck
    for (int i = 1; i <= decks; ++i) {
        TrackPointer loaded_track = PlayerInfo::Instance().getTrackInfo(
            QString("[Channel%1]").arg(i));
        if (loaded_track) {
            return QString("Got Here");
        }
    }
    return QString();
    */
    // Just use deck 1 for now
    TrackPointer loadedTrack =
                PlayerInfo::Instance().getTrackInfo("[Channel1]");

    if (loadedTrack) {
      QString TrackGenre = loadedTrack->getGenre();
      return TrackGenre;
    }
    return QString();
}

void SelectorLibraryTableModel::filterByGenre() {
  emit(doFilterByGenre(currentGenre()));
}

float SelectorLibraryTableModel::currentBpm() const {
    /*int decks = ControlObject::getControl(
        ConfigKey("[Master]", "num_decks"))->get();
    // check if file is loaded to a deck
    for (int i = 1; i <= decks; ++i) {
        TrackPointer loaded_track = PlayerInfo::Instance().getTrackInfo(
            QString("[Channel%1]").arg(i));
        if (loaded_track) {
            return QString("Got Here");
        }
    }
    return QString();
    */
    // Just use deck 1 for now
    TrackPointer loadedTrack =
                PlayerInfo::Instance().getTrackInfo("[Channel1]");

    if (loadedTrack) {
      float TrackBpm = loadedTrack->getBpm();
      return TrackBpm;
    }
    return float();
}

void SelectorLibraryTableModel::filterByBpm() {
  emit(doFilterByBpm(currentBpm()));
}
