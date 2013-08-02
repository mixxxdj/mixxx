// Created 3/17/2012 by Keith Salisbury (keithsalisbury@gmail.com)

#include <QObject>
#include <QStringBuilder>

#include "library/queryutil.h"
#include "library/trackcollection.h"
#include "basetrackplayer.h"
#include "playerinfo.h"

#include "controlobjectthreadmain.h"
#include "controlobject.h"
#include "trackinfoobject.h"

#include "track/keyutils.h"
#include "track/timbreutils.h"
#include "track/tagutils.h"

#include "util/timer.h"

#include "library/selector/selector_preferences.h"
#include "library/selector/selectorlibrarytablemodel.h"

using mixxx::track::io::key::ChromaticKey;
using mixxx::track::io::key::ChromaticKey_IsValid;

const bool sDebug = true;
const QString tableName = "selector_table";
const double maxBpmDiff = 10.0;

SelectorLibraryTableModel::SelectorLibraryTableModel(QObject* parent,
                                                     ConfigObject<ConfigValue>* pConfig,
                                                     TrackCollection* pTrackCollection)
        : LibraryTableModel(parent, pTrackCollection,
                            "mixxx.db.model.selector"),
          m_pConfig(pConfig) {
    setTableModel();

    // Detect when deck has changed
    connect(&PlayerInfo::Instance(), SIGNAL(currentPlayingDeckChanged(int)),
           this, SLOT(slotPlayingDeckChanged(int)));

    connect(this, SIGNAL(filtersChanged()),
            this, SLOT(slotFiltersChanged()));

    connect(this, SIGNAL(resetFilters()),
            this, SLOT(slotResetFilters()));

    connect(this, SIGNAL(filtersChanged()),
            this, SLOT(slotFiltersChanged()));

    m_channelBpm = NULL;
    m_channelKey = NULL;
    m_bActive = false;

    slotResetFilters();
    clearSeedTrackInfo();

}

SelectorLibraryTableModel::~SelectorLibraryTableModel() {
}


void SelectorLibraryTableModel::setTableModel(int id){
    Q_UNUSED(id);

    QStringList columns;
    columns << "library."+LIBRARYTABLE_ID << "'' as preview" << " 0.0 as score";

    QSqlQuery query(m_pTrackCollection->getDatabase());
    QString queryString = "CREATE TEMPORARY TABLE IF NOT EXISTS "+tableName+" AS "
            "SELECT " + columns.join(", ") +
            " FROM library INNER JOIN track_locations "
            "ON library.location = track_locations.id "
            "WHERE (" + LibraryTableModel::DEFAULT_LIBRARYFILTER + ")";
    query.prepare(queryString);
    if (!query.exec()) {
        LOG_FAILED_QUERY(query);
    }

    QStringList tableColumns;
    tableColumns << LIBRARYTABLE_ID;
    tableColumns << "preview";
    tableColumns << "score";
    setTable(tableName, LIBRARYTABLE_ID, tableColumns,
             m_pTrackCollection->getTrackSource("default"));

    initHeaderData();

    setSearch("");
    setDefaultSort(fieldIndex("score"), Qt::DescendingOrder);
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

void SelectorLibraryTableModel::setSeedTrack(TrackPointer pSeedTrack) {
    m_pSeedTrack = pSeedTrack;
    if (!m_pSeedTrack.isNull()) {
        m_sSeedTrackInfo = m_pSeedTrack->getInfo();
        m_sSeedTrackGenre = m_pSeedTrack->getGenre();
        m_fSeedTrackBpm = m_pSeedTrack->getBpm();
        m_seedTrackKey = m_pSeedTrack->getKey();
        m_pSeedTrackTimbre = m_pSeedTrack->getTimbre();
        m_seedTrackTags = m_pSeedTrack->getTags();
    } else {
        clearSeedTrackInfo();
        emit(resetFilters());
    }
    loadStoredSimilarityContributions();
    emit(seedTrackInfoChanged());
}

QString SelectorLibraryTableModel::getSeedTrackInfo() {
    return m_sSeedTrackInfo;
}

bool SelectorLibraryTableModel::seedTrackGenreExists() {
    return m_sSeedTrackGenre != QString();
}

bool SelectorLibraryTableModel::seedTrackBpmExists() {
    return m_fSeedTrackBpm > 0;
}

bool SelectorLibraryTableModel::seedTrackKeyExists() {
    return ChromaticKey_IsValid(m_seedTrackKey) &&
               m_seedTrackKey != mixxx::track::io::key::INVALID;
}

void SelectorLibraryTableModel::setSimilarityContributions(
        const QHash<QString, double>& contributions) {
    m_similarityContributions = contributions;
    normalizeContributions();
}

void SelectorLibraryTableModel::loadStoredSimilarityContributions() {
    int iTimbreCoefficient = m_pConfig->getValueString(
        ConfigKey(SELECTOR_CONFIG_KEY, TIMBRE_COEFFICIENT)).toInt();
    int iRhythmCoefficient = m_pConfig->getValueString(
        ConfigKey(SELECTOR_CONFIG_KEY, RHYTHM_COEFFICIENT)).toInt();
    int iLastFmCoefficient = m_pConfig->getValueString(
        ConfigKey(SELECTOR_CONFIG_KEY, LASTFM_COEFFICIENT)).toInt();

    QHash<QString, double> contributions;
    contributions.insert("timbre", iTimbreCoefficient/100.0);
    contributions.insert("rhythm", iRhythmCoefficient/100.0);
    contributions.insert("lastfm", iLastFmCoefficient/100.0);
    setSimilarityContributions(contributions);
}

void SelectorLibraryTableModel::normalizeContributions() {
    if (m_seedTrackTags.isEmpty()) {
        m_similarityContributions["lastfm"] = 0.0;
    }
    if (m_pSeedTrackTimbre.isNull()) {
        m_similarityContributions["timbre"] = 0.0;
        m_similarityContributions["beat"] = 0.0;
    }

    // ensure non-zero items sum to 1
    double total = 0.0;

    foreach (double value, m_similarityContributions.values()) {
        total += value;
    }

    if (total > 0.0) {
        foreach (QString key, m_similarityContributions.keys()) {
            m_similarityContributions[key] /= total;
        }
    }
}
void SelectorLibraryTableModel::calculateSimilarity() {
    ScopedTimer t("SelectorLibraryTableModel::calculateSimilarity()");
//    qDebug() << "SelectorLibraryTableModel::calculateSimilarity()";
    loadStoredSimilarityContributions();

    if (!m_pSeedTrack.isNull()) {
        normalizeContributions();
        QSqlQuery query(m_pTrackCollection->getDatabase());
        query.prepare("UPDATE " + tableName + " SET score=:score "
                      "WHERE " + LIBRARYTABLE_ID + "=:id;");
        QVariantList scores;
        QVariantList trackIds;

        for (int i = 0, n = rowCount(); i < n; i++) {
            QModelIndex index = createIndex(i, fieldIndex(LIBRARYTABLE_ID));
            int trackId = getTrackId(index);
            QVariant score = scoreTrack(index);

            // if the score could not be calculated (i.e. a field is missing),
            // skip it
            if (!score.isValid())
                continue;

            trackIds << QVariant(trackId);
            scores << score;
        }

        query.bindValue(":score", scores);
        query.bindValue(":id", trackIds);
        if (!query.execBatch()) {
            qDebug() << query.lastError();
        } else {
            select(); // update the view
        }
    }
}

void SelectorLibraryTableModel::calculateAllSimilarities(
        const QString& filename) {
    QFile file(filename);
    file.open(QIODevice::WriteOnly | QIODevice::Text);
    QTextStream out(&file);

    for (int i = 0, n = rowCount(); i < n; i++) {
        QModelIndex index1 = createIndex(i, fieldIndex(LIBRARYTABLE_ID));
        TrackPointer pTrack1 = getTrack(index1);
        TimbrePointer pTimbre1 = pTrack1->getTimbre();
        QString sTrack1 = pTrack1->getFilename();
        for (int j = i + 1; j < n; j++) {
            QModelIndex index2 = createIndex(j, fieldIndex(LIBRARYTABLE_ID));
            TrackPointer pTrack2 = getTrack(index2);
            TimbrePointer pTimbre2 = pTrack2->getTimbre();
            QString sTrack2 = pTrack2->getFilename();
            double timbreScore =
                    TimbreUtils::symmetricKlDivergence(pTimbre1,
                                                       pTimbre2);
            double rhythmScore =
                    TimbreUtils::modelDistanceBeats(pTimbre1,
                                                        pTimbre2);
            double score = 0.5 * (timbreScore + rhythmScore);
            out << sTrack1 % "," % sTrack2 % "," %
                   QString::number(timbreScore) % "," %
                   QString::number(rhythmScore) % "," %
                   QString::number(score) % "\n";
            if (i % 100 == 0 && j % 100 == 0) {
                qDebug() << QString::number(i*j) << "comparisons processed";
            }
        }
    }

    file.close();
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
        if (m_channelBpm) {
            disconnect(m_channelBpm, 0, this, 0);
        }

        if (m_channelKey) {
            disconnect(m_channelKey, 0, this, 0);
        }

        // get the new pitch slider object
        m_channelBpm = new ControlObjectThreadMain(
                    ControlObject::getControl(ConfigKey(m_pChannel, "bpm")));

        m_channelKey = new ControlObjectThreadMain(
                    ControlObject::getControl(ConfigKey(m_pChannel, "key")));

        // listen for slider change events
        connect(m_channelBpm, SIGNAL(valueChanged(double)), this,
            SLOT(slotChannelBpmChanged(double)));
        connect(m_channelKey, SIGNAL(valueChanged(double)), this,
            SLOT(slotChannelKeyChanged(double)));

        m_pLoadedTrack = PlayerInfo::Instance().getTrackInfo(m_pChannel);
    } else {
        m_pLoadedTrack = TrackPointer();
    }

    setSeedTrack(m_pLoadedTrack);
    emit(seedTrackInfoChanged());
    updateFilterText();
}

void SelectorLibraryTableModel::slotChannelBpmChanged(double value) {
    qDebug() << "BPM changed to " << value;
    if (m_pLoadedTrack == m_pSeedTrack) {
        m_fSeedTrackBpm = value;
    }
    updateFilterText();
}

void SelectorLibraryTableModel::slotChannelKeyChanged(double value) {
    qDebug() << "Key changed to " << value;
    if (m_pLoadedTrack == m_pSeedTrack) {
        m_seedTrackKey = KeyUtils::keyFromNumericValue(value);
    }
    updateFilterText();
}

void SelectorLibraryTableModel::slotFiltersChanged() {
    search("");
}

void SelectorLibraryTableModel::slotResetFilters() {
    m_bFilterGenre = false;
    m_bFilterBpm = false;
    m_iFilterBpmRange = 0;
    m_bFilterKey = false;
    m_bFilterKey4th = false;
    m_bFilterKey5th = false;
    m_bFilterKeyRelative = false;
    m_filterString = QString();
}

void SelectorLibraryTableModel::search(const QString& text) {
    setSearch(text, m_filterString);
    select();
}

// PRIVATE METHODS

void SelectorLibraryTableModel::initHeaderData() {
    // call the base class method first
    BaseSqlTableModel::initHeaderData();
    setHeaderData(fieldIndex("score"),
                  Qt::Horizontal, tr("Score"));
}

void SelectorLibraryTableModel::clearSeedTrackInfo() {
    m_sSeedTrackInfo = QString();
    m_sSeedTrackGenre = QString();
    m_fSeedTrackBpm = 0;
    m_seedTrackKey = mixxx::track::io::key::INVALID;
    m_pSeedTrackTimbre = TimbrePointer();
    m_seedTrackTags = TagCounts();
}

QVariant SelectorLibraryTableModel::scoreTrack(const QModelIndex& index) {
    // return a QVariant::Double from 0 (no match) to 100 (perfect match)
    // or QVariant::Invalid if the field is missing
    // assume that seed track info is valid

    QVariant score;

    // m_similarityContributions's values add up to 1

    double bpmContribution = m_similarityContributions.value("bpm");
    if (bpmContribution > 0.0) {
        QVariant bpm = data(index.sibling(index.row(), fieldIndex("bpm")));
        if (bpm.isValid()) {
            double bpmDiff = abs(bpm.toDouble() - m_fSeedTrackBpm);
            double bpmScore = ((maxBpmDiff - bpmDiff) / maxBpmDiff);

            if (bpmScore > 1.0) bpmScore = 1.0;
            else if (bpmScore < 0.0) bpmScore = 0.0;
            bpmScore *= bpmContribution * 100.0;

            score.setValue(bpmScore + score.toDouble());
        }
    }

    double timbreContribution = m_similarityContributions.value("timbre");
    double rhythmContribution = m_similarityContributions.value("rhythm");

    if (timbreContribution > 0.0 || rhythmContribution > 0.0) {
        TrackPointer otherTrack = getTrack(index);
        TimbrePointer pTimbre = otherTrack->getTimbre();
        if (!m_pSeedTrackTimbre.isNull() && !pTimbre.isNull()) {
            double timbreScore =
                    1 - TimbreUtils::hellingerDistance(m_pSeedTrackTimbre,
                                                       pTimbre);
            double rhythmScore =
                    1 - TimbreUtils::modelDistanceBeats(m_pSeedTrackTimbre,
                                                        pTimbre);

//            qDebug() << m_sSeedTrackInfo << "x"
//                     << otherTrack->getInfo() << timbreScore;
            timbreScore *= timbreContribution;
            rhythmScore *= rhythmContribution;
            score.setValue(timbreScore + rhythmScore + score.toDouble());
        }
    }

    double lastFmContribution = m_similarityContributions.value("lastfm");
    if (lastFmContribution > 0.0) {
        TrackPointer otherTrack = getTrack(index);
        TagCounts otherTags = otherTrack->getTags();
        if (!m_seedTrackTags.isEmpty() && !otherTags.isEmpty()) {
            double tagsScore =
                TagUtils::overlapSimilarity(m_seedTrackTags, otherTags);
            tagsScore *= lastFmContribution;
            score.setValue(tagsScore + score.toDouble());
        }
    }

    return score;
}

void SelectorLibraryTableModel::updateFilterText() {
	if (!m_bActive) return;
    if (m_pSeedTrack) {
		QStringList filters;

        // Genre
        if (m_bFilterGenre) {
            QString TrackGenre = m_sSeedTrackGenre;
            if (TrackGenre != "")
                filters << QString("Genre == '%1'").arg(TrackGenre);
        }

        // BPM
        if (m_bFilterBpm) {
            if (m_fSeedTrackBpm > 0)
                filters << QString("(Bpm > %1 AND Bpm < %2)").arg(
                    floor(m_fSeedTrackBpm - m_iFilterBpmRange)).arg(
                    ceil(m_fSeedTrackBpm + m_iFilterBpmRange));
        } 

        // Keys

        QList<ChromaticKey> hKeys = getHarmonicKeys(m_seedTrackKey);

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

    } else { // no seed track
        emit(filtersChanged());
    }
}

QList<ChromaticKey> SelectorLibraryTableModel::getHarmonicKeys(ChromaticKey key) {
    QList<ChromaticKey> keys;

    if (seedTrackKeyExists()) {
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
