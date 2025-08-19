#ifndef SEARCHCRATESFUNCTIONS_H
#define SEARCHCRATESFUNCTIONS_H

#include <QString>

const bool sDebug = false;

inline QString buildCondition(const QString& field, const QString& op, const QString& value) {
    // Check if field, operator, and value are not null
    // if (!field.isEmpty() && !op.isEmpty() && !value.isEmpty()) {
    //
    // QStringList stringFieldOptions = {"artist",
    //        "title",
    //        "album",
    //        "album_artist",
    //        "genre",
    //        "comment",
    //        "composer",
    //        "filetype",
    //        "key"};
    // QStringList numberFieldOptions = {"duration", "bpm", "played", "timesplayed", "rating"};
    QStringList stringFieldOptions = {
            LIBRARYTABLE_ARTIST,
            LIBRARYTABLE_TITLE,
            LIBRARYTABLE_ALBUM,
            LIBRARYTABLE_ALBUMARTIST,
            LIBRARYTABLE_GENRE,
            LIBRARYTABLE_COMMENT,
            LIBRARYTABLE_COMPOSER,
            LIBRARYTABLE_FILETYPE,
            LIBRARYTABLE_KEY};
    QStringList numberFieldOptions = {
            LIBRARYTABLE_DURATION,
            LIBRARYTABLE_BPM,
            LIBRARYTABLE_PLAYED,
            LIBRARYTABLE_TIMESPLAYED,
            LIBRARYTABLE_RATING};
    // QStringList trackFieldOptions = {"track"};
    // QStringList playlistCrateFieldOptions = {"playlist", "crate", "history"};
    // QStringList playlistCrateOperatorOptions = {"is", "is not"};

    QString condition = "";

    //        condition = "";
    if (!field.isEmpty() && !op.isEmpty()) {
        //          hasConditions = true;

        // Prepare the condition

        // fields like strings -> operator translations
        if (stringFieldOptions.contains(field)) {
            if (op == "contains") {
                condition = QStringLiteral("%1.%2 LIKE '%%3%'").arg(LIBRARY_TABLE, field, value);
            } else if (op == "does not contain") {
                condition = QStringLiteral("%1.%2 NOT LIKE '%%3%'")
                                    .arg(LIBRARY_TABLE, field, value);
            } else if (op == "starts with") {
                condition = QStringLiteral("%1.%2 LIKE '%3%'").arg(LIBRARY_TABLE, field, value);
            } else if (op == "ends with") {
                condition = QStringLiteral("%1.%2 LIKE '%%3'").arg(LIBRARY_TABLE, field, value);
            } else if (op == "is not empty") {
                condition = QStringLiteral("%1.%2 IS NOT NULL").arg(LIBRARY_TABLE, field);
            } else if (op == "is empty") {
                condition = QStringLiteral("%1.%2 IS NULL").arg(LIBRARY_TABLE, field);
            } else if (op == "equal to") {
                condition = QStringLiteral("%1.%2 = '%3'").arg(LIBRARY_TABLE, field, value);
            } else if (op == "not equal to") {
                condition = QStringLiteral("%1.%2 != '%3'").arg(LIBRARY_TABLE, field, value);
            } else {
                // continue; // Skip unrecognized operators
            }
        }
        // year field -> operator translations
        if (field == "year") {
            if (op == "before") {
                condition = QStringLiteral(
                        "CAST(substr(%1.%2,1,4) as INTEGER) < %3")
                                    .arg(LIBRARY_TABLE, field, value);
            } else if (op == "after") {
                condition = QStringLiteral(
                        "CAST(substr(%1.%2,1,4) as INTEGER) > %3")
                                    .arg(LIBRARY_TABLE, field, value);
            } else if (op == "equal to") {
                condition = QStringLiteral(
                        "CAST(substr(%1.%2,1,4) as INTEGER) = %3")
                                    .arg(LIBRARY_TABLE, field, value);
            } else if (op == "between") {
                if (value.indexOf("|", 0) > 0) {
                    int posBar = value.indexOf("|", 0);
                    QString yearFrom = value.mid(0, posBar).replace("-", "");
                    QString yearTo =
                            value.mid(posBar + 1, value.length() - posBar + 1)
                                    .replace("-", "");
                    if (yearFrom.toInt() < yearTo.toInt()) {
                        // qDebug() << "between years - yearFrom" << yearFrom <<
                        // " < yearTo" << yearTo;
                        condition = QStringLiteral(
                                "CAST(substr(%1.%2,1,4) as INTEGER) BETWEEN %3 AND %4")
                                            .arg(LIBRARY_TABLE, field, yearFrom, yearTo);
                    } else if (yearFrom.toInt() > yearTo.toInt()) {
                        // qDebug() << "between years - yearFrom" << yearFrom <<
                        // " > yearTo" << yearTo;
                        condition = QStringLiteral(
                                "CAST(substr(%1.%2,1,4) as INTEGER) BETWEEN %3 AND %4")
                                            .arg(LIBRARY_TABLE, field, yearTo, yearFrom);
                    } else if (yearFrom.toInt() == yearTo.toInt()) {
                        // qDebug() << "between years - yearFrom" << yearFrom <<
                        // " == yearTo" << yearTo;
                        condition = QStringLiteral(
                                "CAST(substr(%1.%2,1,4) as INTEGER) = %3")
                                            .arg(LIBRARY_TABLE, field, yearFrom);
                    }
                } else {
                    // continue; // Skip unrecognized operators
                }
            } else {
                // continue; // Skip unrecognized operators
            }
        }
        // fields like dates -> operator translations
        if (field == "datetime_added" || field == "last_played_at") {
            if (value.indexOf("-", 0) > 1) {
                if (op == "before") {
                    condition =
                            QStringLiteral(
                                    "strftime('%Y-%m-%d',%1.%2) < "
                                    "strftime('%Y-%m-%d','%3')")
                                    .arg(LIBRARY_TABLE, field, value);
                } else if (op == "after") {
                    condition =
                            QStringLiteral(
                                    "strftime('%Y-%m-%d',%1.%2) > "
                                    "strftime('%Y-%m-%d','%3')")
                                    .arg(LIBRARY_TABLE, field, value);
                } else if (op == "equal to") {
                    condition =
                            QStringLiteral(
                                    "strftime('%Y-%m-%d',%1.%2) = "
                                    "strftime('%Y-%m-%d','%3')")
                                    .arg(LIBRARY_TABLE, field, value);
                } else if (op == "between") {
                    if ((value.indexOf("|", 0) == 10) && (value.length() == 21)) {
                        const QString& dateFrom = value.mid(0, 10);
                        const QString& dateTo = value.mid(11, 10);
                        if (dateFrom < dateTo) {
                            // qDebug() << "between dates - dateFrom" <<
                            // dateFrom << " < dateTo" << dateTo;
                            condition = QStringLiteral(
                                    "strftime('%Y-%m-%d',%1.%2) BETWEEN "
                                    "strftime('%Y-%m-%d','%3') AND "
                                    "strftime('%Y-%m-%d','%4')")
                                                .arg(LIBRARY_TABLE, field, dateFrom, dateTo);
                        } else if (dateFrom > dateTo) {
                            // qDebug() << "between dates - dateFrom" <<
                            // dateFrom << " > dateTo" << dateTo;
                            condition = QStringLiteral(
                                    "strftime('%Y-%m-%d',%1.%2) BETWEEN "
                                    "strftime('%Y-%m-%d','%3') AND "
                                    "strftime('%Y-%m-%d','%4')")
                                                .arg(LIBRARY_TABLE, field, dateTo, dateFrom);

                        } else if (dateFrom == dateTo) {
                            // qDebug() << "between dates - dateFrom" <<
                            // dateFrom << " == dateTo" << dateTo;
                            condition = QStringLiteral(
                                    "strftime('%Y-%m-%d',%1.%2) = "
                                    "strftime('%Y-%m-%d','%3')")
                                                .arg(LIBRARY_TABLE, field, dateFrom);
                        }
                    } else {
                        //  continue;
                    }
                } else {
                    // continue; // Skip unrecognized operators
                }
            } else {
                QDate nowDateStamp = QDate::currentDate();
                if (op == "before") {
                    condition =
                            QStringLiteral(
                                    "strftime('%Y-%m-%d',%1.%2) < "
                                    "strftime('%Y-%m-%d','%3')")
                                    .arg(LIBRARY_TABLE,
                                            field,
                                            nowDateStamp
                                                    .addDays(value.toInt() * -1)
                                                    .toString("yyyy-MM-dd"));
                } else if (op == "after") {
                    condition =
                            QStringLiteral(
                                    "strftime('%Y-%m-%d',%1.%2) > "
                                    "strftime('%Y-%m-%d','%3')")
                                    .arg(LIBRARY_TABLE,
                                            field,
                                            nowDateStamp
                                                    .addDays(value.toInt() * -1)
                                                    .toString("yyyy-MM-dd"));
                } else if (op == "last") {
                    condition =
                            QStringLiteral(
                                    "strftime('%Y-%m-%d',%1.%2) > "
                                    "strftime('%Y-%m-%d','%3')")
                                    .arg(LIBRARY_TABLE,
                                            field,
                                            nowDateStamp
                                                    .addDays(((value.toInt()) +
                                                                     1) *
                                                            -1)
                                                    .toString("yyyy-MM-dd"));
                } else if (op == "equal to") {
                    condition =
                            QStringLiteral(
                                    "strftime('%Y-%m-%d',%1.%2) = "
                                    "strftime('%Y-%m-%d','%3')")
                                    .arg(LIBRARY_TABLE,
                                            field,
                                            nowDateStamp
                                                    .addDays(value.toInt() * -1)
                                                    .toString("yyyy-MM-dd"));
                } else if (op == "between") {
                    if (value.indexOf("|", 0) > 0) {
                        int posBar = value.indexOf("|", 0);
                        QString nrFrom = value.mid(0, posBar).replace("-", "");
                        QString nrTo = value.mid(posBar + 1,
                                                    value.length() - posBar + 1)
                                               .replace("-", "");
                        // qDebug() << "between numbers - nrfrom" << nrFrom << " nrTo" << nrTo;
                        if (nrFrom.toInt() < nrTo.toInt()) {
                            // qDebug() << "between nrs - nrFrom" << nrFrom << " < nrTo" << nrTo;
                            condition =
                                    QStringLiteral(
                                            "strftime('%Y-%m-%d',%1.%2) "
                                            "BETWEEN "
                                            "strftime('%Y-%m-%d','%3') AND "
                                            "strftime('%Y-%m-%d','%4')")
                                            .arg(LIBRARY_TABLE,
                                                    field,
                                                    nowDateStamp
                                                            .addDays(
                                                                    nrTo.toInt() *
                                                                    -1)
                                                            .toString("yyyy-MM-"
                                                                      "dd"),
                                                    nowDateStamp
                                                            .addDays(
                                                                    nrFrom.toInt() *
                                                                    -1)
                                                            .toString("yyyy-MM-"
                                                                      "dd"));
                        } else if (nrFrom.toInt() > nrTo.toInt()) {
                            // qDebug() << "between nrs - nrFrom" << nrFrom << " > nrTo" << nrTo;
                            condition =
                                    QStringLiteral(
                                            "strftime('%Y-%m-%d',%1.%2) "
                                            "BETWEEN "
                                            "strftime('%Y-%m-%d','%3') AND "
                                            "strftime('%Y-%m-%d','%4')")
                                            .arg(LIBRARY_TABLE,
                                                    field,
                                                    nowDateStamp
                                                            .addDays(
                                                                    nrFrom.toInt() *
                                                                    -1)
                                                            .toString("yyyy-MM-"
                                                                      "dd"),
                                                    nowDateStamp
                                                            .addDays(
                                                                    nrTo.toInt() *
                                                                    -1)
                                                            .toString("yyyy-MM-"
                                                                      "dd"));

                        } else if (nrFrom.toInt() == nrTo.toInt()) {
                            // qDebug() << "between nrs - nrFrom" << nrFrom << " == nrTo" << nrTo;
                            condition =
                                    QStringLiteral(
                                            "strftime('%Y-%m-%d',%1.%2) = "
                                            "strftime('%Y-%m-%d','%3')")
                                            .arg(LIBRARY_TABLE,
                                                    field,
                                                    nowDateStamp
                                                            .addDays(
                                                                    nrFrom.toInt() *
                                                                    -1)
                                                            .toString("yyyy-MM-"
                                                                      "dd"));
                        }
                    } else {
                        //  continue;
                    }
                } else {
                    // continue; // Skip unrecognized operators
                }
            }
        }
        // fields like numbers -> operator translations
        if (numberFieldOptions.contains(field)) {
            if (op == "less than") {
                condition = QStringLiteral("%1.%2 < %3").arg(LIBRARY_TABLE, field, value);
            } else if (op == "greater than") {
                condition = QStringLiteral("%1.%2 > %3").arg(LIBRARY_TABLE, field, value);
            } else if (op == "equal to") {
                condition = QStringLiteral("%1.%2 = %3").arg(LIBRARY_TABLE, field, value);
            } else if (op == "not equal to") {
                condition = QStringLiteral("%1.%2 <> %3").arg(LIBRARY_TABLE, field, value);
            } else if (op == "between") {
                if (value.indexOf("|", 0) > 0) {
                    int posBar = value.indexOf("|", 0);
                    QString nrFrom = value.mid(0, posBar).replace("-", "");
                    QString nrTo =
                            value.mid(posBar + 1, value.length() - posBar + 1)
                                    .replace("-", "");
                    if (sDebug) {
                        qDebug() << "between numbers - nrfrom" << nrFrom << " nrTo" << nrTo;
                    }
                    if (nrFrom.toInt() < nrTo.toInt()) {
                        if (sDebug) {
                            qDebug() << "between nrs - nrFrom" << nrFrom << " < nrTo" << nrTo;
                        }
                        condition = QStringLiteral("%1.%2 BETWEEN %3 AND %4")
                                            .arg(LIBRARY_TABLE, field, nrFrom, nrTo);
                    } else if (nrFrom.toInt() > nrTo.toInt()) {
                        if (sDebug) {
                            qDebug() << "between nrs - nrFrom" << nrFrom << " > nrTo" << nrTo;
                        }
                        condition = QStringLiteral("%1.%2 BETWEEN %3 AND %4")
                                            .arg(LIBRARY_TABLE, field, nrTo, nrFrom);
                    } else if (nrFrom.toInt() == nrTo.toInt()) {
                        if (sDebug) {
                            qDebug() << "between nrs - nrFrom" << nrFrom << " == nrTo" << nrTo;
                        }
                        condition = QStringLiteral("%1.%2 = %3").arg(LIBRARY_TABLE, field, nrFrom);
                    }
                }
            } else {
                // continue; // Skip unrecognized operators
            }
        }
        // if (playlistCrateFieldOptions.contains(field)) {
        if (field == "playlist") {
            //                value = "459";
            if (value.indexOf("|||", 0) > 0) {
                int posBar = value.indexOf("|||", 0);
                QString playlistId = value.mid(0, posBar);
                // QString playlistName = value.mid(posBar + 3, value.length() - posBar + 3);
                if (sDebug) {
                    qDebug() << "PLAYLIST -> playlistId: " << playlistId;
                }
                if (op == "is") {
                    condition = QStringLiteral(
                            "%1.%2 IN (SELECT %3.%4 from %3 WHERE %3.%5=%6)")
                                        .arg(LIBRARY_TABLE,
                                                LIBRARYTABLE_ID,
                                                PLAYLIST_TRACKS_TABLE,
                                                PLAYLISTTRACKSTABLE_TRACKID,
                                                PLAYLISTTRACKSTABLE_PLAYLISTID,
                                                playlistId);
                } else if (op == "is not") {
                    condition = QStringLiteral(
                            "%1.%2 NOT IN (SELECT %3.%4 from %3 WHERE %3.%5_id=%6)")
                                        .arg(LIBRARY_TABLE,
                                                LIBRARYTABLE_ID,
                                                PLAYLIST_TRACKS_TABLE,
                                                PLAYLISTTRACKSTABLE_TRACKID,
                                                PLAYLISTTRACKSTABLE_PLAYLISTID,
                                                playlistId);
                } else {
                    // continue; // Skip unrecognized operators
                }
            }
        }
        if (field == "history") {
            //                value = "5";
            if (value.indexOf("|||", 0) > 0) {
                int posBar = value.indexOf("|||", 0);
                QString historyId = value.mid(0, posBar);
                // QString historyName = value.mid(posBar + 3, value.length() - posBar + 3);
                if (sDebug) {
                    qDebug() << "HISTORY -> historyId: " << historyId;
                }
                if (op == "is") {
                    condition = QStringLiteral(
                            "%1.%2 IN (SELECT %3.%4 from %3 WHERE %3.%5=%6)")
                                        .arg(LIBRARY_TABLE,
                                                LIBRARYTABLE_ID,
                                                PLAYLIST_TRACKS_TABLE,
                                                PLAYLISTTRACKSTABLE_TRACKID,
                                                PLAYLISTTRACKSTABLE_PLAYLISTID,
                                                historyId);
                } else if (op == "is not") {
                    condition = QStringLiteral(
                            "%1.%2 NOT IN (SELECT %3.%4 from %3 WHERE %3.%5=%6)")
                                        .arg(LIBRARY_TABLE,
                                                LIBRARYTABLE_ID,
                                                PLAYLIST_TRACKS_TABLE,
                                                PLAYLISTTRACKSTABLE_TRACKID,
                                                PLAYLISTTRACKSTABLE_PLAYLISTID,
                                                historyId);
                } else {
                    // continue; // Skip unrecognized operators
                }
            }
        }
        if (field == "crate") {
            //                value = "5";
            if (value.indexOf("|||", 0) > 0) {
                int posBar = value.indexOf("|||", 0);
                QString crateId = value.mid(0, posBar);
                // QString crateName = value.mid(posBar + 3, value.length() - posBar + 3);
                if (sDebug) {
                    qDebug() << "CRATE -> crateId: " << crateId;
                }
                if (op == "is") {
                    condition = QStringLiteral(
                            "library.id IN (SELECT crate_tracks.track_id from "
                            "crate_tracks WHERE crate_tracks.crate_id=%1)")
                                        .arg(crateId);
                    //            condition = QStringLiteral(
                    //                    "%1.%2 IN (SELECT %3.%4 from %3 WHERE %3.%5=%6)")
                    //                                .arg(LIBRARY_TABLE,
                    //                                        LIBRARYTABLE_ID,
                    //                                        CRATE_TRACKS_TABLE,
                    //                                        CRATETRACKSTABLE_TRACKID,
                    //                                        CRATETRACKSTABLE_CRATEID,
                    //                                        crateId);
                } else if (op == "is not") {
                    //            condition = QStringLiteral(
                    //                    "%1.%2 NOT IN (SELECT %3.%4 from %3 WHERE %3.%5=%6)")
                    //                                .arg(LIBRARY_TABLE,
                    //                                        LIBRARYTABLE_ID,
                    //                                        CRATE_TRACKS_TABLE,
                    //                                        CRATETRACKSTABLE_TRACKID,
                    //                                        CRATETRACKSTABLE_CRATEID,
                    //                                        crateId);
                    condition = QStringLiteral(
                            "library.id NOT IN (SELECT crate_tracks.track_id "
                            "from crate_tracks WHERE crate_tracks.crate_id=%1)")
                                        .arg(crateId);
                } else {
                    // continue; // Skip unrecognized operators
                }
            }
        }

        if (field == "track") {
            //                value = "5";
            if (value == "all crates") {
                if (op == "is a member of") {
                    if (sDebug) {
                        qDebug() << "Track -> is a member of: " << value;
                    }
                    condition = QStringLiteral(
                            "library.id IN (SELECT crate_tracks.track_id from "
                            "crate_tracks)");
                    // condition = QStringLiteral(
                    //         "%1.%2 IN (SELECT %3.%4 from %3)")
                    //                     .arg(LIBRARY_TABLE,
                    //                             LIBRARYTABLE_ID,
                    //                             CRATE_TRACKS_TABLE,
                    //                             CRATETRACKSTABLE_TRACKID);
                }
                if (op == "is not a member of") {
                    if (sDebug) {
                        qDebug() << "Track -> is NOT a member of: " << value;
                    }
                    condition = QStringLiteral(
                            "library.id NOT IN (SELECT crate_tracks.track_id from "
                            "crate_tracks)");
                    // condition = QStringLiteral(
                    //         "%1.%2 NOT IN (SELECT %3.%4 from %3)")
                    //                     .arg(LIBRARY_TABLE,
                    //                             LIBRARYTABLE_ID,
                    //                             CRATE_TRACKS_TABLE,
                    //                             CRATETRACKSTABLE_TRACKID);
                }
            }
            if (value == "all playlists") {
                if (op == "is a member of") {
                    if (sDebug) {
                        qDebug() << "Track -> is a member of: " << value;
                    }
                    condition = QStringLiteral(
                            "%1.%2 IN (SELECT %3.%4 FROM %3 "
                            "JOIN %6 "
                            "  ON %3.%5 = %6.%7 "
                            "WHERE %6.%8 = 0)")
                                        .arg(LIBRARY_TABLE,
                                                LIBRARYTABLE_ID,
                                                PLAYLIST_TRACKS_TABLE,
                                                PLAYLISTTRACKSTABLE_TRACKID,
                                                PLAYLISTTRACKSTABLE_PLAYLISTID,
                                                PLAYLIST_TABLE,
                                                PLAYLISTTABLE_ID,
                                                PLAYLISTTABLE_HIDDEN);
                }
                if (op == "is not a member of") {
                    if (sDebug) {
                        qDebug() << "Track -> is NOT a member of: " << value;
                    }
                    condition = QStringLiteral(
                            "%1.%2 NOT IN (SELECT %3.%4 FROM %3 "
                            "JOIN %6 "
                            "  ON %3.%5 = %6.%7 "
                            "WHERE %6.%8 = 0)")
                                        .arg(LIBRARY_TABLE,
                                                LIBRARYTABLE_ID,
                                                PLAYLIST_TRACKS_TABLE,
                                                PLAYLISTTRACKSTABLE_TRACKID,
                                                PLAYLISTTRACKSTABLE_PLAYLISTID,
                                                PLAYLIST_TABLE,
                                                PLAYLISTTABLE_ID,
                                                PLAYLISTTABLE_HIDDEN);
                }
            }
            if (value == "all historylists") {
                if (op == "is a member of") {
                    if (sDebug) {
                        qDebug() << "Track -> is a member of: " << value;
                    }
                    condition = QStringLiteral(
                            "%1.%2 IN (SELECT %3.%4 FROM %3 "
                            "JOIN %6 "
                            "  ON %3.%5 = %6.%7 "
                            "WHERE %6.%8 = 2)")
                                        .arg(LIBRARY_TABLE,
                                                LIBRARYTABLE_ID,
                                                PLAYLIST_TRACKS_TABLE,
                                                PLAYLISTTRACKSTABLE_TRACKID,
                                                PLAYLISTTRACKSTABLE_PLAYLISTID,
                                                PLAYLIST_TABLE,
                                                PLAYLISTTABLE_ID,
                                                PLAYLISTTABLE_HIDDEN);
                }
                if (op == "is not a member of") {
                    if (sDebug) {
                        qDebug() << "Track -> is NOT a member of: " << value;
                    }

                    condition = QStringLiteral(
                            "%1.%2 NOT IN (SELECT %3.%4 FROM %3 "
                            "JOIN %6 "
                            "  ON %3.%5 = %6.%7 "
                            "WHERE %6.%8 = 2)")
                                        .arg(LIBRARY_TABLE,
                                                LIBRARYTABLE_ID,
                                                PLAYLIST_TRACKS_TABLE,
                                                PLAYLISTTRACKSTABLE_TRACKID,
                                                PLAYLISTTRACKSTABLE_PLAYLISTID,
                                                PLAYLIST_TABLE,
                                                PLAYLISTTABLE_ID,
                                                PLAYLISTTABLE_HIDDEN);
                }
            }
        }
    }
    return condition;
}

#endif /* SEARCHCRATESFUNCTIONS_H */
