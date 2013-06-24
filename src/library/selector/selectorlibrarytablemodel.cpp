// Created 3/17/2012 by Keith Salisbury (keithsalisbury@gmail.com)

#include <QObject>

#include "selectorlibrarytablemodel.h"
#include "library/trackcollection.h"
#include "basetrackplayer.h"
#include "playerinfo.h"

#include "controlobjectthreadmain.h"
#include "controlobject.h"
#include "trackinfoobject.h"

#include "track/keyutils.h"

using mixxx::track::io::key::ChromaticKey;
using mixxx::track::io::key::ChromaticKey_IsValid;

const bool sDebug = true;

SelectorLibraryTableModel::SelectorLibraryTableModel(QObject* parent,
                                                   TrackCollection* pTrackCollection)
        : LibraryTableModel(parent, pTrackCollection,
                            "mixxx.db.model.selector") {

    // Getting info on current decks playing etc
    connect(&PlayerInfo::Instance(), SIGNAL(currentPlayingDeckChanged(int)),
           this, SLOT(slotPlayingDeckChanged(int)));

    connect(this, SIGNAL(filtersChanged()),
            this, SLOT(slotFiltersChanged()));

    m_bFilterGenre = false;
    m_bFilterBpm = false;
    m_iFilterBpmRange = 0;
    m_bFilterYear = false;
    m_bFilterRating = false;
    m_bFilterKey = false;
    m_bFilterKey4th = false;
    m_bFilterKey5th = false;
    m_bFilterKeyRelative = false;
    m_filterString = "";
    m_channelBpm = NULL;
    m_bActive = false;
    m_rate = 0;
    
    m_sCurrentTrackGenre = "";
    m_fCurrentTrackBpm = 0;
    m_sCurrentTrackYear = "";
    m_iCurrentTrackRating = 0;
    m_currentTrackKey = mixxx::track::io::key::INVALID;

}

SelectorLibraryTableModel::~SelectorLibraryTableModel() {
}

bool SelectorLibraryTableModel::isColumnInternal(int column) {
    return LibraryTableModel::isColumnInternal(column);
}

int SelectorLibraryTableModel::rowCount() {
    return BaseSqlTableModel::rowCount();
}

void SelectorLibraryTableModel::active(bool value) {
    m_bActive = value;
}

bool SelectorLibraryTableModel::currentTrackGenreExists() {
    return m_sCurrentTrackGenre != QString();
}

bool SelectorLibraryTableModel::currentTrackBpmExists() {
    return m_fCurrentTrackBpm > 0;
}

bool SelectorLibraryTableModel::currentTrackYearExists() {
    return m_sCurrentTrackYear != QString();
}

bool SelectorLibraryTableModel::currentTrackRatingExists() {
    return m_iCurrentTrackRating > 0;
}

bool SelectorLibraryTableModel::currentTrackKeyExists() {
    return ChromaticKey_IsValid(m_currentTrackKey) &&
               m_currentTrackKey != mixxx::track::io::key::INVALID;
}

void SelectorLibraryTableModel::filterByGenre(bool value) {
    m_bFilterGenre = value;
    updateFilterText();
}

void SelectorLibraryTableModel::filterByBpm(bool value, int range) {
    m_bFilterBpm = value;
    m_iFilterBpmRange = range;
    updateFilterText();
}

void SelectorLibraryTableModel::filterByYear(bool value) {
    m_bFilterYear = value;
    updateFilterText();
}

void SelectorLibraryTableModel::filterByRating(bool value) {
    m_bFilterRating = value;
    updateFilterText();
}

void SelectorLibraryTableModel::filterByKey(bool value) {
    m_bFilterKey = value;
    updateFilterText();
}

void SelectorLibraryTableModel::filterByKey4th(bool value) {
    m_bFilterKey4th = value;
    updateFilterText();
}

void SelectorLibraryTableModel::filterByKey5th(bool value) {
    m_bFilterKey5th = value;
    updateFilterText();
}

void SelectorLibraryTableModel::filterByKeyRelative(bool value) {
    m_bFilterKeyRelative = value;
    updateFilterText();
}

// PRIVATE SLOTS

void SelectorLibraryTableModel::slotPlayingDeckChanged(int deck) {
    if (deck > 0) {
        m_pChannel = QString("[Channel%1]").arg(deck);

        // disconnect the old pitch slider
    //    if (m_channelBpm) {
    //        disconnect(m_channelBpm, 0, this, 0);
    //    }
        // get the new pitch slider object
    //    m_channelBpm = new ControlObjectThreadMain(
    //                ControlObject::getControl(ConfigKey(m_pChannel, "bpm")));

        // listen for slider change events
    //    connect(m_channelBpm, SIGNAL(valueChanged(double)), this,
    //        SLOT(slotChannel1BpmChanged(double)));

        m_pLoadedTrack = PlayerInfo::Instance().getTrackInfo(m_pChannel);
        if (m_pLoadedTrack) {
            m_sCurrentTrackGenre = m_pLoadedTrack->getGenre();
            m_fCurrentTrackBpm = m_pLoadedTrack->getBpm();
            m_sCurrentTrackYear = m_pLoadedTrack->getYear();
            m_iCurrentTrackRating = m_pLoadedTrack->getRating();
            m_currentTrackKey = m_pLoadedTrack->getKey();
        } else {
            qDebug() << "Called with deck " << deck << " and no track playing.";
        }
        emit(currentTrackInfoChanged());

        setRate();
        updateFilterText();
    }

}

void SelectorLibraryTableModel::slotChannel1BpmChanged(double value) {
    qDebug() << "BPM changed to " << value;
    setRate();
    updateFilterText();
}

void SelectorLibraryTableModel::slotFiltersChanged() {
	if (!m_bActive) return;
    search("");
}

void SelectorLibraryTableModel::search(const QString& text) {
    setSearch(text, m_filterString);
    select();
}

// PRIVATE METHODS

void SelectorLibraryTableModel::updateFilterText() {
	if (!m_bActive) return;
    if (m_pLoadedTrack) {
		QStringList filters;

        // Genre
        if (m_bFilterGenre) {
            QString TrackGenre = m_sCurrentTrackGenre;
            if (TrackGenre != "")
                filters << QString("Genre == '%1'").arg(TrackGenre);
        }

        // Year
        if (m_bFilterYear) {
            QString TrackYear = m_sCurrentTrackYear;
            if (TrackYear!="")
                filters << QString("Year == '%1'").arg(TrackYear);
        }

        // Rating
        if (m_bFilterRating) {
            int TrackRating = m_iCurrentTrackRating;
            if (TrackRating > 0)
                filters << QString("Rating >= %1").arg(TrackRating);
        }

        // calculate the current BPM
        float trackBpm = m_fCurrentTrackBpm;
        float currentBpm = trackBpm * m_rate;

        // Bpm
        if (m_bFilterBpm) {
            //float trackBpm = pChannel1Bpm->get();
            if (currentBpm > 0) 
                filters << QString("(Bpm > %1 AND Bpm < %2)").arg(
                    floor(currentBpm - m_iFilterBpmRange)).arg(
                    ceil(currentBpm + m_iFilterBpmRange));
        } 

        // Keys

        QList<ChromaticKey> hKeys = getHarmonicKeys(m_currentTrackKey);

        if (!hKeys.isEmpty()) {
            // string business is a hack until filters use ChromaticKey directly
            QStringList keyNames;

            while (!hKeys.isEmpty()) {
                ChromaticKey key = hKeys.takeFirst();
                keyNames.append(QString("'%1'").arg(KeyUtils::keyToString(key)));
            }
            if (!keyNames.isEmpty()) {
                QString keyString = keyNames.join(",");
                qDebug() << "Match keys " << keyString;
                filters << QString("Key in (%1)").arg(keyString);
            }
        }
                    
        QString filterString = filters.join(" AND ");
        if (m_filterString != filterString) {
            m_filterString = filterString;
            emit(filtersChanged());
        }

    }

}

QList<ChromaticKey> SelectorLibraryTableModel::getHarmonicKeys(ChromaticKey key) {
    QList<ChromaticKey> keys;

    if (currentTrackKeyExists()) {
        if (m_bFilterKey) {
            keys.append(key);
        }
        if (m_bFilterKey4th) {
            keys.append(KeyUtils::scaleKeySteps(key, 5));
        }
        if (m_bFilterKey5th) {
            keys.append(KeyUtils::scaleKeySteps(key, 7));
        }
        if (m_bFilterKeyRelative) {
            keys.append(KeyUtils::keyToRelativeMajorOrMinor(key));
        }
    }

    return keys;
}

void SelectorLibraryTableModel::setRate() {
        // get pitch slider value (deck rate)
    ControlObjectThreadMain* rateSlider = new ControlObjectThreadMain(
        ControlObject::getControl(ConfigKey(m_pChannel, "rate")));
    ControlObjectThreadMain* rateRange = new ControlObjectThreadMain(
        ControlObject::getControl(ConfigKey(m_pChannel, "rateRange")));
    ControlObjectThreadMain* rateDirection = new ControlObjectThreadMain(
        ControlObject::getControl(ConfigKey(m_pChannel, "rate_dir")));

    if (rateSlider != NULL && rateRange != NULL && rateDirection != NULL) {
        m_rate = (1 + rateSlider->get() * rateRange->get() * rateDirection->get());        
    }
}
