// Created 3/17/2012 by Keith Salisbury (keithsalisbury@gmail.com)

#include <QObject>

#include "selectorlibrarytablemodel.h"
#include "library/trackcollection.h"
#include "basetrackplayer.h"
#include "playerinfo.h"

#include "controlobjectthreadmain.h"
#include "controlobject.h"
#include "trackinfoobject.h"

const bool sDebug = true;

SelectorLibraryTableModel::SelectorLibraryTableModel(QObject* parent,
                                                   TrackCollection* pTrackCollection)
        : LibraryTableModel(parent, pTrackCollection,
                            "mixxx.db.model.selector") {

    connect(this, SIGNAL(doSearch(const QString&)),
            this, SLOT(slotSearch(const QString&)));

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
    m_bFilterHarmonicKey = false;
    m_filtersText = "";
    m_channelBpm = NULL;
    m_rate = 0;

    initializeHarmonicsData();

}

SelectorLibraryTableModel::~SelectorLibraryTableModel() {
}

bool SelectorLibraryTableModel::isColumnInternal(int column) {
    return LibraryTableModel::isColumnInternal(column);
}

int SelectorLibraryTableModel::rowCount() {
    return BaseSqlTableModel::rowCount();
}

void SelectorLibraryTableModel::filterByGenre(bool value) {
    m_bFilterGenre = value;
    updateFilterText();
}

void SelectorLibraryTableModel::filterByBpm(bool value, int range) {
    //qDebug() << "filterByBpm bpm = " << value << " range = " << range;
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

void SelectorLibraryTableModel::filterByHarmonicKey(bool value) {
    m_bFilterHarmonicKey = value;
    updateFilterText();
}

// PRIVATE SLOTS

void SelectorLibraryTableModel::slotPlayingDeckChanged(int deck) {

    m_pChannel = QString("[Channel%1]").arg(deck);

    // disconnect the old pitch slider
    if (m_channelBpm) {
        m_channelBpm->disconnect(this);
    }
    // get the new pitch slider object
    m_channelBpm = new ControlObjectThreadMain(
        ControlObject::getControl(ConfigKey(m_pChannel, "bpm")));
    // listen for slider change events
    connect(m_channelBpm, SIGNAL(valueChanged(double)), this, 
        SLOT(slotChannel1BpmChanged(double)));

    m_pLoadedTrack = PlayerInfo::Instance().getTrackInfo(m_pChannel);
    setRate();
    updateFilterText();
}


void SelectorLibraryTableModel::slotChannel1BpmChanged(double value) {
    qDebug() << "setRate() slotChannel1BpmChanged = " << value;
    setRate();
    updateFilterText();
}

void SelectorLibraryTableModel::slotFiltersChanged() {
    //qDebug() << "slotFiltersChanged()";
    slotSearch(QString());
}

void SelectorLibraryTableModel::slotSearch(const QString& searchText) {
    qDebug() << "slotSearch()" << searchText << "," << m_filtersText;
    BaseSqlTableModel::search(searchText, m_filtersText);
}

// PRIVATE METHODS

void SelectorLibraryTableModel::updateFilterText() {
    QStringList filters;

    if (m_pLoadedTrack) {

        // Genre
        if (m_bFilterGenre) {
            QString TrackGenre = m_pLoadedTrack->getGenre();
            if (TrackGenre != "")
                filters << QString("Genre == '%1'").arg(TrackGenre);
        }

        // Year
        if (m_bFilterYear) {
            QString TrackYear = m_pLoadedTrack->getYear();
            if (TrackYear!="")
                filters << QString("Year == '%1'").arg(TrackYear);
        }

        // Rating
        if (m_bFilterRating) {
            int TrackRating = m_pLoadedTrack->getRating();
            if (TrackRating > 0)
                filters << QString("Rating >= %1").arg(TrackRating);
        }

        // calculate the current BPM
        float trackBpm = m_pLoadedTrack->getBpm();
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

        // calculate the new pitch
        // const int semitonesPerOctave = 12;
        float frequencyRatio = currentBpm / trackBpm;
        float semitoneOffset = 12 * log(frequencyRatio) / log(2);

        QString trackKey = adjustPitchBy(m_pLoadedTrack->getKey(), semitoneOffset);


        QStringList keyfilters;
        // Key
        if (m_bFilterKey) {
            if (trackKey!="")
                keyfilters << QString("Key == '%1'").arg(trackKey);
        }

        // Harmonic Key
        if (m_bFilterHarmonicKey) {
            QString hKeys;

            // determine major or minor key "m" 
            if (trackKey.contains("m", Qt::CaseInsensitive)) {
                hKeys = getHarmonicKeys(m_minors,m_majors,trackKey);
            } else {
                hKeys = getHarmonicKeys(m_majors,m_minors,trackKey);
            }
            
            if (hKeys!="")
                keyfilters << QString("Key in (%1)").arg(hKeys);

        }

        if (keyfilters.count()>0)
            filters << QString("(%1)").arg(keyfilters.join(" OR "));

        QString text = filters.join(" AND ");
     

        if (m_filtersText != text) {
            qDebug() << "updateFilterText() filters changed: " << text;
            m_filtersText = text;
            emit(filtersChanged());
        }


    }

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
        //qDebug() << "setRate() rateSlider = " << rateSlider->get();
        //qDebug() << "setRate() rateRange = " << rateRange->get();
        //qDebug() << "setRate() rateDirection = " << rateDirection->get();
        m_rate = (1 + rateSlider->get() * rateRange->get() * rateDirection->get());        
    }
}

void SelectorLibraryTableModel::search(const QString& searchText) {
    // qDebug() << "SelectorLibraryTableModel::search()" << searchText
    //          << QThread::currentThread();
    emit(doSearch(searchText));
}

QString SelectorLibraryTableModel::getHarmonicKeys(QStringList keys1, QStringList keys2, QString key) const {
    int index = keys1.indexOf(key);
    int len = keys1.count();
    if (index < 0) return QString("");
    int lower = index-1;
    if (lower < 0) lower += len; 
    int upper = index+1;
    if (upper >= len) upper -= len; 
    //qDebug() << "getHarmonicKeys index = " << index;
    //qDebug() << "getHarmonicKeys lower = " << lower;
    //qDebug() << "getHarmonicKeys upper = " << upper;
    //qDebug() << "getHarmonicKeys len = " << len;
    return QString("'%1','%2','%3'").arg(keys1[lower],keys2[index],keys1[upper]);
}

QString SelectorLibraryTableModel::adjustPitchBy(QString pitch, int change) {
    if (pitch == "") return pitch;

    //qDebug() << "adjustPitchBy pitch = " << pitch;
    int position = m_semitoneList.indexOf(pitch);
    if (position<0){
      //  qDebug() << "Pitch " << pitch << " not found in config.";
        return pitch;
    }

    int newpos = position + (change * 2);
    if (newpos >= 24) {
        newpos = newpos - 24;
    }
    QString newpitch = m_semitoneList.at(newpos);
    return newpitch;
}

void SelectorLibraryTableModel::initializeHarmonicsData() {
    /*
    m_enharmonic_preference = "b" // # or b
    "A# = Bb"
    "C# = Db"
    "D# = Eb"
    "F# = Gb"
    "G# = Ab"

    m_enharmonic_preference = "#" // # or b
    "Ab = G#"
    "Bb = A#"
    "Db = C#"
    "Eb = D#"
    "Gb = F#"

    m_majors = QString("C,G,D,A,E,B,F#,Db,Ab,Eb,Bb,F").split(",");
    m_minors = QString("Am,Em,Bm,F#m,Dbm,Abm,Ebm,Bbm,Fm,Cm,Gm,Dm").split(",");
    m_semitoneList = QString("C,Cm,C#,C#m,D,Dm,Eb,Ebm,E,Em,F,Fm,F#,F#m,G,Gm,G#,G#m,A,Am,Bb,Bbm,B,Bm").split(",");
    */
    //  if (m_enharmonic_preference == "#") {
            // prefer #'s
            m_majors = QString("C,G,D,A,E,B,F#,C#,G#,D#,A#,F").split(",");
            m_minors = QString("Am,Em,Bm,F#m,C#m,G#m,D#m,A#m,Fm,Cm,Gm,Dm").split(",");
            m_semitoneList = QString("C,Cm,C#,C#m,D,Dm,D#,D#m,E,Em,F,Fm,F#,F#m,G,Gm,G#,G#m,A,Am,A#,A#m,B,Bm").split(",");
    /*  } else {
            // prefer b's
            m_majors = QString("C,G,D,A,E,B,Gb,Db,Ab,Eb,Bb,F").split(",");
            m_minors = QString("Am,Em,Bm,Gbm,Dbm,Abm,Ebm,Bbm,Fm,Cm,Gm,Dm").split(",");
            m_semitoneList = QString("C,Cm,Db,Dbm,D,Dm,Eb,Ebm,E,Em,F,Fm,Gb,Gbm,G,Gm,Ab,Abm,A,Am,Bb,Bbm,B,Bm").split(",");
        }
    */
/*
    // OK notation
    m_majors = QString("1,2,3,4,5,6,7,8,9,10,11,12").split("d,");
    m_minors = QString("1,2,3,4,5,6,7,8,9,10,11,12").split("m,");
    m_semitoneList = QString("1d,10m,8d,5m,3d,12m,10d,7m,5d,2m,12d,9m,7d,4m,2d,11m,9d,6m,4d,1m,11d,8m,6d,Bm").split(",");
*/

/*
QHash<QString, QStringList> hash;

hash["1d"] = "C";
hash["2d"] = "G";
hash["3d"] = "D";
hash["4d"] = "A";
hash["5d"] = "E";
hash["6d"] = "B";
hash["7d"] = "F#,Gb";
hash["8d"] = "C#,Db";
hash["9d"] = "G#,Ab";
hash["10d"] = "D#,Eb";
hash["11d"] = "A#,Bb";
hash["12d"] = "F";

hash["1m"] = "Am";
hash["2m"] = "Em";
hash["3m"] = "Bm";
hash["4m"] = "F#m,Gbm";
hash["5m"] = "C#m,Dbm";
hash["6m"] = "G#m,Abm";
hash["7m"] = "D#m,Ebm";
hash["8m"] = "A#m,Bbm";
hash["9m"] = "Fm";
hash["10m"] = "Cm";
hash["11m"] = "Gm";
hash["12m"] = "Dm";

//                majors    minors
hash["1"] =     { "C",      "Am"        };
hash["2"] =     { "G",      "Em"        };
hash["3"] =     { "D",      "Bm"        };
hash["4"] =     { "A",      "F#m,Gbm"   };
hash["5"] =     { "E",      "C#m,Dbm"   };
hash["6"] =     { "B",      "G#m,Abm"   };
hash["7"] =     { "F#,Gb",  "D#m,Ebm"   };
hash["8"] =     { "C#,Db",  "A#m,Bbm"   };
hash["9"] =     { "G#,Ab",  "Fm"        };
hash["10"] =    { "D#,Eb",  "Cm"        };
hash["11"] =    { "A#,Bb",  "Gm"        };
hash["12"] =    { "F",      "Dm"        };

*/

}