#include "library/autodj/smart/energystore.h"

#include <QSet>
#include <QStringList>
#include <QSqlError>
#include <QSqlQuery>
#include <QVariant>
#include <cmath>

#include "library/queryutil.h"

// static
bool EnergyStore::ensureTable(const QSqlDatabase& db) {
    QSqlQuery query(db);
    if (!query.exec(QStringLiteral(
                "CREATE TABLE IF NOT EXISTS autodj_energy ("
                "track_id INTEGER PRIMARY KEY, "
                "energy REAL NOT NULL, "
                "loudness_db REAL, "
                "bright_ratio REAL, "
                "onsets_per_sec REAL, "
                "bass_ratio REAL, "
                "version INTEGER NOT NULL)"))) {
        LOG_FAILED_QUERY(query);
        return false;
    }
    // Columns added in analysis v2. Older tables get them added once.
    QSet<QString> columns;
    if (query.exec(QStringLiteral("PRAGMA table_info(autodj_energy)"))) {
        while (query.next()) {
            columns.insert(query.value(1).toString());
        }
    }
    for (const QString& column : {QStringLiteral("body_start_sec"), QStringLiteral("body_end_sec")}) {
        if (!columns.contains(column) &&
                !query.exec(QStringLiteral("ALTER TABLE autodj_energy ADD COLUMN %1 REAL")
                                    .arg(column))) {
            LOG_FAILED_QUERY(query);
            return false;
        }
    }
    if (!query.exec(QStringLiteral(
                "CREATE TABLE IF NOT EXISTS autodj_auto_markers ("
                "track_id INTEGER PRIMARY KEY, "
                "intro_end_sec REAL, "
                "outro_start_sec REAL)"))) {
        LOG_FAILED_QUERY(query);
        return false;
    }
    if (!query.exec(QStringLiteral(
                "CREATE TABLE IF NOT EXISTS autodj_grid_check ("
                "track_id INTEGER PRIMARY KEY, "
                "bpm REAL, "
                "first_beat_sec REAL, "
                "drift_beats REAL)"))) {
        LOG_FAILED_QUERY(query);
        return false;
    }
    // Added with grid check v2. Older rows get NULL = version 0.
    bool hasVersion = false;
    if (query.exec(QStringLiteral("PRAGMA table_info(autodj_grid_check)"))) {
        while (query.next()) {
            hasVersion = hasVersion || query.value(1).toString() == QStringLiteral("version");
        }
    }
    if (!hasVersion &&
            !query.exec(QStringLiteral(
                    "ALTER TABLE autodj_grid_check ADD COLUMN version INTEGER"))) {
        LOG_FAILED_QUERY(query);
        return false;
    }
    if (!query.exec(QStringLiteral(
                "CREATE TABLE IF NOT EXISTS autodj_energy_manual ("
                "track_id INTEGER PRIMARY KEY, "
                "rating INTEGER NOT NULL)"))) {
        LOG_FAILED_QUERY(query);
        return false;
    }
    return true;
}

// static
bool EnergyStore::setManualRating(
        const QSqlDatabase& db, const QList<TrackId>& trackIds, int rating) {
    if (trackIds.isEmpty() || !ensureTable(db)) {
        return false;
    }
    const bool clear = rating == 0;
    if (!clear && (rating < kMinRating || rating > kMaxRating)) {
        return false;
    }
    ScopedTransaction transaction(db);
    QSqlQuery query(db);
    query.prepare(clear
                    ? QStringLiteral("DELETE FROM autodj_energy_manual WHERE track_id=:id")
                    : QStringLiteral(
                              "INSERT OR REPLACE INTO autodj_energy_manual "
                              "(track_id, rating) VALUES (:id, :rating)"));
    for (const TrackId& id : trackIds) {
        query.bindValue(QStringLiteral(":id"), id.toVariant());
        if (!clear) {
            query.bindValue(QStringLiteral(":rating"), rating);
        }
        if (!query.exec()) {
            LOG_FAILED_QUERY(query);
            return false;
        }
    }
    transaction.commit();
    return true;
}

// static
std::optional<int> EnergyStore::storedVersion(const QSqlDatabase& db, TrackId trackId) {
    QSqlQuery query(db);
    query.prepare(QStringLiteral(
            "SELECT version FROM autodj_energy WHERE track_id=:id"));
    query.bindValue(QStringLiteral(":id"), trackId.toVariant());
    if (!query.exec()) {
        LOG_FAILED_QUERY(query);
        return std::nullopt;
    }
    if (query.next()) {
        return query.value(0).toInt();
    }
    return std::nullopt;
}

// static
bool EnergyStore::save(const QSqlDatabase& db,
        TrackId trackId,
        const EnergyCalculator::Result& result,
        int version) {
    QSqlQuery query(db);
    query.prepare(QStringLiteral(
            "INSERT OR REPLACE INTO autodj_energy "
            "(track_id, energy, loudness_db, bright_ratio, onsets_per_sec, bass_ratio, version, "
            "body_start_sec, body_end_sec) "
            "VALUES (:id, :energy, :loudness, :bright, :onsets, :bass, :version, "
            ":body_start, :body_end)"));
    query.bindValue(QStringLiteral(":id"), trackId.toVariant());
    query.bindValue(QStringLiteral(":energy"), result.energy);
    query.bindValue(QStringLiteral(":loudness"), result.loudnessDb);
    query.bindValue(QStringLiteral(":bright"), result.brightRatio);
    query.bindValue(QStringLiteral(":onsets"), result.onsetsPerSec);
    query.bindValue(QStringLiteral(":bass"), result.bassRatio);
    query.bindValue(QStringLiteral(":version"), version);
    query.bindValue(QStringLiteral(":body_start"), result.bodyStartSec);
    query.bindValue(QStringLiteral(":body_end"), result.bodyEndSec);
    if (!query.exec()) {
        LOG_FAILED_QUERY(query);
        return false;
    }
    return true;
}

// static
std::optional<EnergyStore::Body> EnergyStore::loadBody(const QSqlDatabase& db, TrackId trackId) {
    if (!ensureTable(db)) {
        return std::nullopt;
    }
    QSqlQuery query(db);
    query.prepare(QStringLiteral(
            "SELECT body_start_sec, body_end_sec FROM autodj_energy "
            "WHERE track_id=:id AND body_start_sec IS NOT NULL "
            "AND body_end_sec IS NOT NULL"));
    query.bindValue(QStringLiteral(":id"), trackId.toVariant());
    if (!query.exec()) {
        LOG_FAILED_QUERY(query);
        return std::nullopt;
    }
    if (!query.next()) {
        return std::nullopt;
    }
    Body body;
    body.startSec = query.value(0).toDouble();
    body.endSec = query.value(1).toDouble();
    if (body.endSec <= body.startSec) {
        return std::nullopt;
    }
    return body;
}

// static
EnergyStore::AutoMarkers EnergyStore::loadAutoMarkers(const QSqlDatabase& db, TrackId trackId) {
    AutoMarkers markers;
    if (!ensureTable(db)) {
        return markers;
    }
    QSqlQuery query(db);
    query.prepare(QStringLiteral(
            "SELECT intro_end_sec, outro_start_sec FROM autodj_auto_markers "
            "WHERE track_id=:id"));
    query.bindValue(QStringLiteral(":id"), trackId.toVariant());
    if (!query.exec()) {
        LOG_FAILED_QUERY(query);
        return markers;
    }
    if (query.next()) {
        if (!query.value(0).isNull()) {
            markers.introEndSec = query.value(0).toDouble();
        }
        if (!query.value(1).isNull()) {
            markers.outroStartSec = query.value(1).toDouble();
        }
    }
    return markers;
}

// static
bool EnergyStore::saveAutoMarkers(
        const QSqlDatabase& db, TrackId trackId, const AutoMarkers& markers) {
    if (!ensureTable(db)) {
        return false;
    }
    QSqlQuery query(db);
    query.prepare(QStringLiteral(
            "INSERT OR REPLACE INTO autodj_auto_markers "
            "(track_id, intro_end_sec, outro_start_sec) VALUES (:id, :intro, :outro)"));
    query.bindValue(QStringLiteral(":id"), trackId.toVariant());
    query.bindValue(QStringLiteral(":intro"),
            markers.introEndSec >= 0.0 ? QVariant(markers.introEndSec) : QVariant());
    query.bindValue(QStringLiteral(":outro"),
            markers.outroStartSec >= 0.0 ? QVariant(markers.outroStartSec) : QVariant());
    if (!query.exec()) {
        LOG_FAILED_QUERY(query);
        return false;
    }
    return true;
}

bool EnergyStore::GridCheck::isCurrent(bool beatMap) const {
    return version ==
            (beatMap ? EnergyCalculator::kGridCheckMapVersion
                     : EnergyCalculator::kGridCheckVersion);
}

bool EnergyStore::GridCheck::isFor(
        double gridBpm, double gridFirstBeatSec, bool beatMap) const {
    const int oldVersion = beatMap ? EnergyCalculator::kGridCheckOldMapVersion
                                   : EnergyCalculator::kGridCheckOldVersion;
    // Tiny differences are rounding in the database, not a new grid.
    return (isCurrent(beatMap) || version == oldVersion) &&
            std::fabs(bpm - gridBpm) < 0.001 &&
            std::fabs(firstBeatSec - gridFirstBeatSec) < 0.002;
}

// static
std::optional<EnergyStore::GridCheck> EnergyStore::loadGridCheck(
        const QSqlDatabase& db, TrackId trackId) {
    if (!ensureTable(db)) {
        return std::nullopt;
    }
    QSqlQuery query(db);
    query.prepare(QStringLiteral(
            "SELECT bpm, first_beat_sec, drift_beats, version FROM autodj_grid_check "
            "WHERE track_id=:id"));
    query.bindValue(QStringLiteral(":id"), trackId.toVariant());
    if (!query.exec()) {
        LOG_FAILED_QUERY(query);
        return std::nullopt;
    }
    if (!query.next()) {
        return std::nullopt;
    }
    GridCheck check;
    check.bpm = query.value(0).toDouble();
    check.firstBeatSec = query.value(1).toDouble();
    check.driftBeats = query.value(2).isNull() ? -1.0 : query.value(2).toDouble();
    check.version = query.value(3).toInt(); // NULL -> 0
    return check;
}

// static
bool EnergyStore::saveGridCheck(
        const QSqlDatabase& db, TrackId trackId, const GridCheck& check) {
    if (!ensureTable(db)) {
        return false;
    }
    QSqlQuery query(db);
    query.prepare(QStringLiteral(
            "INSERT OR REPLACE INTO autodj_grid_check "
            "(track_id, bpm, first_beat_sec, drift_beats, version) "
            "VALUES (:id, :bpm, :first, :drift, :version)"));
    query.bindValue(QStringLiteral(":id"), trackId.toVariant());
    query.bindValue(QStringLiteral(":bpm"), check.bpm);
    query.bindValue(QStringLiteral(":first"), check.firstBeatSec);
    query.bindValue(QStringLiteral(":drift"), check.driftBeats);
    query.bindValue(QStringLiteral(":version"), check.version);
    if (!query.exec()) {
        LOG_FAILED_QUERY(query);
        return false;
    }
    return true;
}

// static
QHash<TrackId, EnergyStore::Value> EnergyStore::loadEnergies(
        const QSqlDatabase& db, const QList<TrackId>& trackIds) {
    QHash<TrackId, Value> energies;
    if (trackIds.isEmpty() || !ensureTable(db)) {
        return energies;
    }
    const QSet<TrackId> wanted(trackIds.begin(), trackIds.end());
    // Small requests (e.g. one selected track) ask for just those ids;
    // large ones read the whole table, which is quicker than a huge IN list.
    constexpr int kMaxIdsInQuery = 500;
    const bool filter = wanted.size() <= kMaxIdsInQuery;
    QString idList;
    if (filter) {
        QStringList parts;
        parts.reserve(wanted.size());
        for (const TrackId& id : wanted) {
            parts << id.toString(); // integers from our own ids, safe to inline
        }
        idList = QStringLiteral(" WHERE track_id IN (") + parts.join(QChar(',')) + QChar(')');
    }
    const auto readInto = [&](const QString& sql, bool manual) {
        QSqlQuery query(db);
        if (!query.exec(sql + idList)) {
            LOG_FAILED_QUERY(query);
            return;
        }
        while (query.next()) {
            const TrackId id(query.value(0));
            if (wanted.contains(id)) {
                energies.insert(id, Value{query.value(1).toDouble(), manual});
            }
        }
    };
    readInto(QStringLiteral("SELECT track_id, energy FROM autodj_energy"), false);
    // Ratings last, so they replace measured values.
    readInto(QStringLiteral("SELECT track_id, rating FROM autodj_energy_manual"), true);
    return energies;
}
