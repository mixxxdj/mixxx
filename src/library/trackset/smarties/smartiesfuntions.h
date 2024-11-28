#ifndef SMARTIESFUNCTIONS_H
#define SMARTIESFUNCTIONS_H

#include <QString>

const bool sDebug = false;

inline bool checkDateValid(std::string date) {
    std::istringstream in{std::move(date)};
    std::chrono::year_month_day ymd;
    in >> std::chrono::parse("%Y-%m-%d", ymd);
    return !in.fail();
}

inline QString buildCondition(QString field, QString op, QString value) {
    // Check if field, operator, and value are not null
    // if (!field.isEmpty() && !op.isEmpty() && !value.isEmpty()) {
    //
    QStringList stringFieldOptions = {"artist",
            "title",
            "album",
            "album_artist",
            "genre",
            "comment",
            "composer",
            "filetype",
            "key"};
    QStringList numberFieldOptions = {"duration", "bpm", "played", "timesplayed", "rating"};

    QStringList playlistCrateFieldOptions = {"playlist", "crate", "history"};
    QStringList playlistCrateOperatorOptions = {"is", "is not"};

    QString condition = "";

    //        condition = "";
    if (!field.isEmpty() && !op.isEmpty()) {
        //          hasConditions = true;

        // Prepare the condition

        // fields like strings -> operator translations
        if (stringFieldOptions.contains(field)) {
            if (op == "contains") {
                condition = QString("library.%1 LIKE '%%2%'").arg(field, value);
            } else if (op == "does not contain") {
                condition = QString("library.%1 NOT LIKE '%%2%'").arg(field, value);
            } else if (op == "starts with") {
                condition = QString("library.%1 LIKE '%2%'").arg(field, value);
            } else if (op == "ends with") {
                condition = QString("library.%1 LIKE '%%2'").arg(field, value);
            } else if (op == "is not empty") {
                condition = QString("library.%1 IS NOT NULL").arg(field);
            } else if (op == "is empty") {
                condition = QString("library.%1 IS NULL").arg(field);
            } else if (op == "equal to") {
                condition = QString("library.%1 = '%2'").arg(field, value);
            } else if (op == "not equal to") {
                condition = QString("library.%1 != '%2'").arg(field, value);
            } else {
                // continue; // Skip unrecognized operators
            }
        }
        // year field -> operator translations
        if (field == "year") {
            if (op == "before") {
                condition = QString(
                        "CAST(substr(library.%1,1,4) as INTEGER) < %2")
                                    .arg(field, value);
            } else if (op == "after") {
                condition = QString(
                        "CAST(substr(library.%1,1,4) as INTEGER) > %2")
                                    .arg(field, value);
            } else if (op == "equal to") {
                condition = QString(
                        "CAST(substr(library.%1,1,4) as INTEGER) = %2")
                                    .arg(field, value);
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
                        condition = QString(
                                "CAST(substr(library.%1,1,4) as INTEGER) BETWEEN %2 AND %3")
                                            .arg(field, yearFrom, yearTo);
                    } else if (yearFrom.toInt() > yearTo.toInt()) {
                        // qDebug() << "between years - yearFrom" << yearFrom <<
                        // " > yearTo" << yearTo;
                        condition = QString(
                                "CAST(substr(library.%1,1,4) as INTEGER) BETWEEN %2 AND %3")
                                            .arg(field, yearTo, yearFrom);
                    } else if (yearFrom.toInt() == yearTo.toInt()) {
                        // qDebug() << "between years - yearFrom" << yearFrom <<
                        // " == yearTo" << yearTo;
                        condition = QString(
                                "CAST(substr(library.%1,1,4) as INTEGER) = %2")
                                            .arg(field, yearFrom);
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
                            QString("strftime('%Y-%m-%d',library.%1) < "
                                    "strftime('%Y-%m-%d','%2')")
                                    .arg(field, value);
                } else if (op == "after") {
                    condition =
                            QString("strftime('%Y-%m-%d',library.%1) > "
                                    "strftime('%Y-%m-%d','%2')")
                                    .arg(field, value);
                } else if (op == "equal to") {
                    condition =
                            QString("strftime('%Y-%m-%d',library.%1) = "
                                    "strftime('%Y-%m-%d','%2')")
                                    .arg(field, value);
                } else if (op == "between") {
                    if ((value.indexOf("|", 0) == 10) && (value.length() == 21)) {
                        QString dateFrom = value.mid(0, 10);
                        QString dateTo = value.mid(11, 10);
                        if (dateFrom < dateTo) {
                            // qDebug() << "between dates - dateFrom" <<
                            // dateFrom << " < dateTo" << dateTo;
                            condition = QString(
                                    "strftime('%Y-%m-%d',library.%1) BETWEEN "
                                    "strftime('%Y-%m-%d','%2') AND "
                                    "strftime('%Y-%m-%d','%3')")
                                                .arg(field, dateFrom, dateTo);
                        } else if (dateFrom > dateTo) {
                            // qDebug() << "between dates - dateFrom" <<
                            // dateFrom << " > dateTo" << dateTo;
                            condition = QString(
                                    "strftime('%Y-%m-%d',library.%1) BETWEEN "
                                    "strftime('%Y-%m-%d','%2') AND "
                                    "strftime('%Y-%m-%d','%3')")
                                                .arg(field, dateTo, dateFrom);

                        } else if (dateFrom == dateTo) {
                            // qDebug() << "between dates - dateFrom" <<
                            // dateFrom << " == dateTo" << dateTo;
                            condition = QString(
                                    "strftime('%Y-%m-%d',library.%1) = "
                                    "strftime('%Y-%m-%d','%2')")
                                                .arg(field, dateFrom);
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
                            QString("strftime('%Y-%m-%d',library.%1) < "
                                    "strftime('%Y-%m-%d','%2')")
                                    .arg(field,
                                            nowDateStamp
                                                    .addDays(value.toInt() *
                                                            -1)
                                                    .toString(
                                                            "yyyy-MM-dd"));
                } else if (op == "after") {
                    condition =
                            QString("strftime('%Y-%m-%d',library.%1) > "
                                    "strftime('%Y-%m-%d','%2')")
                                    .arg(field,
                                            nowDateStamp
                                                    .addDays(value.toInt() *
                                                            -1)
                                                    .toString(
                                                            "yyyy-MM-dd"));
                } else if (op == "last") {
                    condition =
                            QString("strftime('%Y-%m-%d',library.%1) > "
                                    "strftime('%Y-%m-%d','%2')")
                                    .arg(field,
                                            nowDateStamp
                                                    .addDays(((value.toInt()) + 1) *
                                                            -1)
                                                    .toString(
                                                            "yyyy-MM-dd"));
                } else if (op == "equal to") {
                    condition =
                            QString("strftime('%Y-%m-%d',library.%1) = "
                                    "strftime('%Y-%m-%d','%2')")
                                    .arg(field,
                                            nowDateStamp
                                                    .addDays(value.toInt() *
                                                            -1)
                                                    .toString(
                                                            "yyyy-MM-dd"));
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
                                    QString("strftime('%Y-%m-%d',library.%1) "
                                            "BETWEEN "
                                            "strftime('%Y-%m-%d','%2') AND "
                                            "strftime('%Y-%m-%d','%3')")
                                            .arg(field,
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
                                    QString("strftime('%Y-%m-%d',library.%1) "
                                            "BETWEEN "
                                            "strftime('%Y-%m-%d','%2') AND "
                                            "strftime('%Y-%m-%d','%3')")
                                            .arg(field,
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
                                    QString("strftime('%Y-%m-%d',library.%1) = "
                                            "strftime('%Y-%m-%d','%2')")
                                            .arg(field,
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
                condition = QString("library.%1 < %2").arg(field, value);
            } else if (op == "greater than") {
                condition = QString("library.%1 > %2").arg(field, value);
            } else if (op == "equal to") {
                condition = QString("library.%1 = %2").arg(field, value);
            } else if (op == "not equal to") {
                condition = QString("library.%1 <> %2").arg(field, value);
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
                        condition = QString("library.%1 BETWEEN %2 AND %3")
                                            .arg(field, nrFrom, nrTo);
                    } else if (nrFrom.toInt() > nrTo.toInt()) {
                        if (sDebug) {
                            qDebug() << "between nrs - nrFrom" << nrFrom << " > nrTo" << nrTo;
                        }
                        condition = QString("library.%1 BETWEEN %2 AND %3")
                                            .arg(field, nrTo, nrFrom);
                    } else if (nrFrom.toInt() == nrTo.toInt()) {
                        if (sDebug) {
                            qDebug() << "between nrs - nrFrom" << nrFrom << " == nrTo" << nrTo;
                        }
                        condition = QString("library.%1 = %2").arg(field, nrFrom);
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
                    condition = QString(
                            "library.id IN (SELECT PlaylistTracks.track_id "
                            "from PlaylistTracks WHERE "
                            "PlaylistTracks.playlist_id=%1)")
                                        .arg(playlistId);
                } else if (op == "is not") {
                    condition = QString(
                            "library.id NOT IN (SELECT PlaylistTracks.track_id "
                            "from PlaylistTracks WHERE "
                            "PlaylistTracks.playlist_id=%1)")
                                        .arg(playlistId);
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
                    condition = QString(
                            "library.id IN (SELECT PlaylistTracks.track_id "
                            "from PlaylistTracks WHERE "
                            "PlaylistTracks.playlist_id=%1)")
                                        .arg(historyId);
                } else if (op == "is not") {
                    condition = QString(
                            "library.id NOT IN (SELECT PlaylistTracks.track_id "
                            "from PlaylistTracks WHERE "
                            "PlaylistTracks.playlist_id=%1)")
                                        .arg(historyId);
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
                    condition = QString(
                            "library.id IN (SELECT crate_tracks.track_id from "
                            "crate_tracks WHERE crate_tracks.crate_id=%1)")
                                        .arg(crateId);
                } else if (op == "is not") {
                    condition = QString(
                            "library.id NOT IN (SELECT crate_tracks.track_id "
                            "from crate_tracks WHERE crate_tracks.crate_id=%1)")
                                        .arg(crateId);
                } else {
                    // continue; // Skip unrecognized operators
                }
            }
        }
    }
    return condition;
}

#endif /* SMARTIESFUNCTIONS_H */
