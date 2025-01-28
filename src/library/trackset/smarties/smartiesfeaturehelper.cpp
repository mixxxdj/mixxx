#include "library/trackset/smarties/smartiesfeaturehelper.h"

#include <QInputDialog>
#include <QLineEdit>
#include <QRegularExpression>
#include <QString>
#include <QStringList>

#include "library/trackcollection.h"
#include "library/trackset/smarties/smarties.h"
#include "library/trackset/smarties/smartiessummary.h"
#include "moc_smartiesfeaturehelper.cpp"

const bool sDebug = false;

SmartiesFeatureHelper::SmartiesFeatureHelper(
        TrackCollection* pTrackCollection,
        UserSettingsPointer pConfig)
        : m_pTrackCollection(pTrackCollection),
          m_pConfig(pConfig) {
}

QString SmartiesFeatureHelper::proposeNameForNewSmarties(
        const QString& initialName) const {
    DEBUG_ASSERT(!initialName.isEmpty());
    QString proposedName;
    int suffixCounter = 0;
    do {
        if (suffixCounter++ > 0) {
            // Append suffix " 2", " 3", ...
            proposedName = QStringLiteral("%1 %2")
                                   .arg(initialName, QString::number(suffixCounter));
        } else {
            proposedName = initialName;
        }
    } while (m_pTrackCollection->smarties().readSmartiesByName(proposedName));
    // Found an unused smarties name
    if (sDebug) {
        qDebug() << "[SMARTIES] [PROPOSE NEW NAME] -> proposedName" << proposedName;
    }
    // would be better with regex but ... :-)
    proposedName.replace("artist:", "")
            .replace("title:", "")
            .replace("album_artist:", "")
            .replace("album:", "")
            .replace("genre:", "")
            .replace("composer:", "")
            .replace("grouping:", "")
            .replace("comment:", "")
            .replace("type:", "")
            .replace("played:", "")
            .replace("rating:", "")
            .replace("year:", "")
            .replace("key:", "")
            .replace("bpm:", "")
            .replace("duration:", "")
            .replace("datetime_added:", "")
            .replace("\"", "");
    if (sDebug) {
        qDebug() << "[SMARTIES] [PROPOSE NEW NAME] -> cleaned proposedName" << proposedName;
    }
    return proposedName;
}

SmartiesId SmartiesFeatureHelper::createEmptySmartiesFromSearch(const QString& text) {
    if (sDebug) {
        qDebug() << "[SMARTIES] [NEW SMARTIES FROM SEARCH]";
    }
    Smarties newSmarties;
    const QString proposedSmartiesName =
            proposeNameForNewSmarties(tr("Search for ") + text);
    for (;;) {
        bool ok = false;
        auto newName =
                QInputDialog::getText(
                        nullptr,
                        tr("New Smarties From Search"),
                        tr("Enter name for new smarties:"),
                        QLineEdit::Normal,
                        proposedSmartiesName,
                        &ok)
                        .trimmed();
        if (!ok) {
            return SmartiesId();
        }
        if (newName.isEmpty()) {
            QMessageBox::warning(
                    nullptr,
                    tr("Creating Smarties Failed"),
                    tr("A smarties cannot have a blank name."));
            continue;
        }
        if (m_pTrackCollection->smarties().readSmartiesByName(newName)) {
            QMessageBox::warning(
                    nullptr,
                    tr("Creating Smarties Failed"),
                    tr("A smarties by that name already exists."));
            continue;
        }

        // here a given text will be converted to parts: terms (fields) and
        // values QStringList terms = {"artist:", "album_artist:", "album:",
        // "title:", "genre:", "composer:", "grouping:", "comment:",
        // "location:", "type:", "played:", "rating:", "year:", "key:", "bpm:",
        // "duration:", "datetime_added:"}; clean the name
        // would be better with regex but ... :-)
        newName.replace("artist:", "")
                .replace("title:", "")
                .replace("album_artist:", "")
                .replace("album:", "")
                .replace("genre:", "")
                .replace("composer:", "")
                .replace("grouping:", "")
                .replace("comment:", "")
                .replace("type:", "")
                .replace("played:", "")
                .replace("rating:", "")
                .replace("year:", "")
                .replace("key:", "")
                .replace("bpm:", "")
                .replace("duration:", "")
                .replace("datetime_added:", "")
                .replace("\"", "")
                .replace("   ", " ")
                .replace("  ", " ");

        // clean the input text
        QStringList stringTerms = {"artist",
                "title",
                "album",
                "album_artist",
                "genre",
                "comment",
                "composer",
                "filetype",
                "key"};
        QStringList dateTerms = {"year", "datetime_added", "last_played_at"};
        QStringList numberTerms = {"duration", "bpm", "played", "timesplayed", "rating"};

        QString cleanedText = text;
        // possible typing errors: replace ; / 2x  & 3x colons with a single colon
        // replace , ?
        cleanedText.replace("?", "");
        cleanedText.replace(",", "");
        cleanedText.replace(";", ":");
        cleanedText.replace("::", ":");
        cleanedText.replace(":::", ":");
        cleanedText.replace("~", "");
        cleanedText.replace("=", "");

        // if the or-symbol | is in the text the combiners will be OR, else AND (default)
        bool orCombiner = cleanedText.indexOf("|", 0) > 1;
        cleanedText.replace("|", "");

        if (sDebug) {
            qDebug() << "Cleaned Text:" << cleanedText;
        }

        //        QString pattern =
        //        "\\b(?:artist:|album_artist:|album:|title:|genre:|composer:|grouping:|comment:|location:|type:|played:|rating:|year:|key:|bpm:|duration:|datetime_added:)\\s*(?:\"([^\"]*)\"|(\\S+))";
        //        QRegularExpression termRegex(pattern,
        //        QRegularExpression::CaseInsensitiveOption);

        const QString& pattern =
                R"(\b(artist:|album_artist:|album:|title:|genre:|composer:|grouping:|comment:|location:|type:|played:|rating:|year:|key:|bpm:|duration:|datetime_added:)\s*([^:]+?)(?=\s*\b(?:artist:|album_artist:|album:|title:|genre:|composer:|grouping:|comment:|location:|type:|played:|rating:|year:|key:|bpm:|duration:|datetime_added:|$)))";
        static QRegularExpression termRegex(pattern, QRegularExpression::CaseInsensitiveOption);

        QRegularExpressionMatchIterator X = termRegex.globalMatch(cleanedText);
        int conditionIndex = 1;
        bool foundTerms = false;

        while (X.hasNext() && conditionIndex <= 12) {
            QRegularExpressionMatch match = X.next();
            QString term = match.captured(1).split(":")[0].trimmed();
            QString value = match.captured(2).trimmed();

            if (!term.isEmpty() && !value.isEmpty()) {
                foundTerms = true;

                // Determine operator type based on term type
                QString operatorType;
                if (stringTerms.contains(term)) {
                    operatorType = "contains";
                } else if (numberTerms.contains(term)) {
                    if (value.contains("-")) {
                        operatorType = "between";
                        value.replace("-", "|");
                    } else if (value.startsWith("<")) {
                        operatorType = "less than";
                        value.replace("<", "");
                    } else if (value.startsWith(">")) {
                        operatorType = "greater than";
                        value.replace(">", "");
                    } else {
                        operatorType = "equal to";
                    }
                } else if (dateTerms.contains(term)) {
                    if (value.contains("-")) {
                        operatorType = "between";
                        value.replace("-", "|");
                    } else if (value.startsWith("<")) {
                        operatorType = "before";
                        value.replace("<", "");
                    } else if (value.startsWith(">")) {
                        operatorType = "after";
                        value.replace(">", "");
                    } else {
                        operatorType = "equal to";
                    }
                }

                // Assign to the appropriate condition field
                switch (conditionIndex) {
                case 1:
                    newSmarties.setCondition1Field(term);
                    newSmarties.setCondition1Operator(operatorType);
                    newSmarties.setCondition1Value(value);
                    newSmarties.setCondition1Combiner(") END");
                    break;
                case 2:
                    newSmarties.setCondition2Field(term);
                    newSmarties.setCondition2Operator(operatorType);
                    newSmarties.setCondition2Value(value);
                    newSmarties.setCondition2Combiner(") END");
                    newSmarties.setCondition1Combiner(orCombiner ? ") OR (" : ") AND (");
                    break;
                case 3:
                    newSmarties.setCondition3Field(term);
                    newSmarties.setCondition3Operator(operatorType);
                    newSmarties.setCondition3Value(value);
                    newSmarties.setCondition3Combiner(") END");
                    newSmarties.setCondition2Combiner(orCombiner ? ") OR (" : ") AND (");
                    break;
                case 4:
                    newSmarties.setCondition4Field(term);
                    newSmarties.setCondition4Operator(operatorType);
                    newSmarties.setCondition4Value(value);
                    newSmarties.setCondition4Combiner(") END");
                    newSmarties.setCondition3Combiner(orCombiner ? ") OR (" : ") AND (");
                    break;
                case 5:
                    newSmarties.setCondition5Field(term);
                    newSmarties.setCondition5Operator(operatorType);
                    newSmarties.setCondition5Value(value);
                    newSmarties.setCondition5Combiner(") END");
                    newSmarties.setCondition4Combiner(orCombiner ? ") OR (" : ") AND (");
                    break;
                case 6:
                    newSmarties.setCondition6Field(term);
                    newSmarties.setCondition6Operator(operatorType);
                    newSmarties.setCondition6Value(value);
                    newSmarties.setCondition6Combiner(") END");
                    newSmarties.setCondition5Combiner(orCombiner ? ") OR (" : ") AND (");
                    break;
                case 7:
                    newSmarties.setCondition7Field(term);
                    newSmarties.setCondition7Operator(operatorType);
                    newSmarties.setCondition7Value(value);
                    newSmarties.setCondition7Combiner(") END");
                    newSmarties.setCondition6Combiner(orCombiner ? ") OR (" : ") AND (");
                    break;
                case 8:
                    newSmarties.setCondition8Field(term);
                    newSmarties.setCondition8Operator(operatorType);
                    newSmarties.setCondition8Value(value);
                    newSmarties.setCondition8Combiner(") END");
                    newSmarties.setCondition7Combiner(orCombiner ? ") OR (" : ") AND (");
                    break;
                case 9:
                    newSmarties.setCondition9Field(term);
                    newSmarties.setCondition9Operator(operatorType);
                    newSmarties.setCondition9Value(value);
                    newSmarties.setCondition9Combiner(") END");
                    newSmarties.setCondition8Combiner(orCombiner ? ") OR (" : ") AND (");
                    break;
                case 10:
                    newSmarties.setCondition10Field(term);
                    newSmarties.setCondition10Operator(operatorType);
                    newSmarties.setCondition10Value(value);
                    newSmarties.setCondition10Combiner(") END");
                    newSmarties.setCondition9Combiner(orCombiner ? ") OR (" : ") AND (");
                    break;
                case 11:
                    newSmarties.setCondition11Field(term);
                    newSmarties.setCondition11Operator(operatorType);
                    newSmarties.setCondition11Value(value);
                    newSmarties.setCondition11Combiner(") END");
                    newSmarties.setCondition10Combiner(orCombiner ? ") OR (" : ") AND (");
                    break;
                case 12:
                    newSmarties.setCondition12Field(term);
                    newSmarties.setCondition12Operator(operatorType);
                    newSmarties.setCondition12Value(value);
                    newSmarties.setCondition12Combiner(") END");
                    newSmarties.setCondition11Combiner(orCombiner ? ") OR (" : ") AND (");
                    break;
                }

                if (sDebug) {
                    qDebug() << "[SMARTIES] [NEW SMARTIES FROM SEARCH] -> "
                                "Detected Term:"
                             << term << ", Value:" << value;
                }
                ++conditionIndex;
            }
        }

        // what if no terms (field) were in the input ....
        if (!foundTerms) {
            if (sDebug) {
                qDebug() << "[SMARTIES] [NEW SMARTIES FROM SEARCH] -> No term "
                            "detected, setting noTermValue:"
                         << text.trimmed();
            }
            cleanedText.replace("\"", "");
            newSmarties.setSearchSql(newName);
            newSmarties.setCondition1Field("artist");
            newSmarties.setCondition1Operator("contains");
            newSmarties.setCondition1Value(cleanedText);
            newSmarties.setCondition1Combiner(") OR (");
            newSmarties.setCondition2Field("album_artist");
            newSmarties.setCondition2Operator("contains");
            newSmarties.setCondition2Value(cleanedText);
            newSmarties.setCondition2Combiner(") OR (");
            newSmarties.setCondition3Field("title");
            newSmarties.setCondition3Operator("contains");
            newSmarties.setCondition3Value(cleanedText);
            newSmarties.setCondition3Combiner(") OR (");
            newSmarties.setCondition4Field("album");
            newSmarties.setCondition4Operator("contains");
            newSmarties.setCondition4Value(cleanedText);
            newSmarties.setCondition4Combiner(") OR (");
            newSmarties.setCondition5Field("genre");
            newSmarties.setCondition5Operator("contains");
            newSmarties.setCondition5Value(cleanedText);
            newSmarties.setCondition5Combiner(") OR (");
            newSmarties.setCondition6Field("composer");
            newSmarties.setCondition6Operator("contains");
            newSmarties.setCondition6Value(cleanedText);
            newSmarties.setCondition6Combiner(") OR (");
            newSmarties.setCondition7Field("comment");
            newSmarties.setCondition7Operator("contains");
            newSmarties.setCondition7Value(cleanedText);
            newSmarties.setCondition7Combiner(") END");
        }

        newSmarties.setName(std::move(newName));
        DEBUG_ASSERT(newSmarties.hasName());
        newSmarties.setSearchInput(text);
        newSmarties.setSearchSql(text);
        break;
    }

    SmartiesId newSmartiesId;

    if (m_pTrackCollection->insertSmarties(newSmarties, &newSmartiesId)) {
        DEBUG_ASSERT(newSmartiesId.isValid());
        newSmarties.setId(newSmartiesId);
        if (sDebug) {
            qDebug() << "[SMARTIES] [NEW SMARTIES FROM SEARCH] -> Created new "
                        "smarties"
                     << newSmarties;
            qDebug() << "[SMARTIES] [NEW SMARTIES FROM SEARCH] -> Created new "
                        "smarties ID: "
                     << newSmartiesId;
        }
    } else {
        DEBUG_ASSERT(!newSmartiesId.isValid());
        qWarning() << "[SMARTIES] [NEW SMARTIES FROM SEARCH] -> Failed to create new smarties"
                   << "->" << newSmarties.getName();
        QMessageBox::warning(
                nullptr,
                tr("Creating Smarties Failed"),
                tr("An unknown error occurred while creating smarties: ") + newSmarties.getName());
    }
    if (sDebug) {
        qDebug() << "[SMARTIES] [NEW SMARTIES FROM SEARCH] -> newSmartiesId " << newSmartiesId;
    }
    return newSmartiesId;
}

SmartiesId SmartiesFeatureHelper::createEmptySmartiesFromUI() {
    if (sDebug) {
        qDebug() << "[SMARTIES] [NEW SMARTIES FROM UI] ";
    }
    Smarties newSmarties;
    const QString proposedSmartiesName =
            proposeNameForNewSmarties("New Smarties From Edit");
    if (sDebug) {
        qDebug() << "[SMARTIES] [PROPOSE NEW NAME] -> proposedName" << proposedSmartiesName;
    }
    for (;;) {
        auto newName = proposedSmartiesName;
        SmartiesId();
        newSmarties.setName(std::move(newName));
        DEBUG_ASSERT(newSmarties.hasName());
        break;
    }
    SmartiesId newSmartiesId;
    if (m_pTrackCollection->insertSmarties(newSmarties, &newSmartiesId)) {
        DEBUG_ASSERT(newSmartiesId.isValid());
        newSmarties.setId(newSmartiesId);
        if (sDebug) {
            qDebug() << "[SMARTIES] [NEW SMARTIES FROM UI] Created new smarties" << newSmarties;
        }
    } else {
        DEBUG_ASSERT(!newSmartiesId.isValid());
        qWarning() << "Failed to create new smarties"
                   << "->" << newSmarties.getName();
    }
    if (sDebug) {
        qDebug() << "[SMARTIES] [NEW SMARTIES FROM UI] -> newSmartiesId " << newSmartiesId;
    }
    return newSmartiesId;
}

SmartiesId SmartiesFeatureHelper::createEmptySmarties() {
    if (sDebug) {
        qDebug() << "[SMARTIES] [NEW EMPTY SMARTIES] ";
    }
    const QString proposedSmartiesName =
            proposeNameForNewSmarties(tr("New Smarties"));
    Smarties newSmarties;
    for (;;) {
        bool ok = false;
        auto newName =
                QInputDialog::getText(
                        nullptr,
                        tr("Create New Smarties"),
                        tr("Enter name for new smarties:"),
                        QLineEdit::Normal,
                        proposedSmartiesName,
                        &ok)
                        .trimmed();
        if (!ok) {
            return SmartiesId();
        }
        if (newName.isEmpty()) {
            QMessageBox::warning(
                    nullptr,
                    tr("Creating Smarties Failed"),
                    tr("A smarties cannot have a blank name."));
            continue;
        }
        if (m_pTrackCollection->smarties().readSmartiesByName(newName)) {
            QMessageBox::warning(
                    nullptr,
                    tr("Creating Smarties Failed"),
                    tr("A smarties by that name already exists."));
            continue;
        }
        newSmarties.setName(std::move(newName));
        DEBUG_ASSERT(newSmarties.hasName());
        break;
    }

    SmartiesId newSmartiesId;
    if (m_pTrackCollection->insertSmarties(newSmarties, &newSmartiesId)) {
        DEBUG_ASSERT(newSmartiesId.isValid());
        newSmarties.setId(newSmartiesId);
        if (sDebug) {
            qDebug() << "Created new smarties" << newSmarties;
        }
    } else {
        DEBUG_ASSERT(!newSmartiesId.isValid());
        qWarning() << "Failed to create new smarties"
                   << "->" << newSmarties.getName();
        QMessageBox::warning(
                nullptr,
                tr("Creating Smarties Failed"),
                tr("An unknown error occurred while creating smarties: ") + newSmarties.getName());
    }
    if (sDebug) {
        qDebug() << "[SMARTIES] [NEW EMPTY SMARTIES] -> newSmartiesId " << newSmartiesId;
    }
    return newSmartiesId;
}

SmartiesId SmartiesFeatureHelper::duplicateSmarties(const Smarties& oldSmarties) {
    if (sDebug) {
        qDebug() << "[SMARTIES] [DUPLICATE SMARTIES] -> START";
    }
    const QString& proposedSmartiesName =
            proposeNameForNewSmarties(
                    QStringLiteral("%1 %2")
                            .arg(oldSmarties.getName(), tr("copy", "//:")));

    const QString& newSearchInput = oldSmarties.getSearchInput();
    if (sDebug) {
        qDebug() << "[SMARTIES] [DUPLICATE] -> old searchInput" << newSearchInput;
    }
    const QString& newSearchSql = oldSmarties.getSearchSql();
    if (sDebug) {
        qDebug() << "[SMARTIES] [DUPLICATE] -> old searchSql" << newSearchSql;
    }
    const QString& newCondition1Field = oldSmarties.getCondition1Field();
    if (sDebug) {
        qDebug() << "[SMARTIES] [DUPLICATE] -> old Condition1Field" << newCondition1Field;
    }
    const QString& newCondition2Field = oldSmarties.getCondition2Field();
    if (sDebug) {
        qDebug() << "[SMARTIES] [DUPLICATE] -> old Condition2Field" << newCondition2Field;
    }
    const QString& newCondition3Field = oldSmarties.getCondition3Field();
    if (sDebug) {
        qDebug() << "[SMARTIES] [DUPLICATE] -> old Condition3Field" << newCondition3Field;
    }
    const QString& newCondition4Field = oldSmarties.getCondition4Field();
    const QString& newCondition5Field = oldSmarties.getCondition5Field();
    const QString& newCondition6Field = oldSmarties.getCondition6Field();
    const QString& newCondition7Field = oldSmarties.getCondition7Field();
    const QString& newCondition8Field = oldSmarties.getCondition8Field();
    const QString& newCondition9Field = oldSmarties.getCondition9Field();
    const QString& newCondition10Field = oldSmarties.getCondition10Field();
    const QString& newCondition11Field = oldSmarties.getCondition11Field();
    const QString& newCondition12Field = oldSmarties.getCondition12Field();

    const QString& newCondition1Operator = oldSmarties.getCondition1Operator();
    if (sDebug) {
        qDebug() << "[SMARTIES] [DUPLICATE] -> old Condition1Operator" << newCondition1Operator;
    }
    const QString& newCondition2Operator = oldSmarties.getCondition2Operator();
    if (sDebug) {
        qDebug() << "[SMARTIES] [DUPLICATE] -> old Condition2Operator" << newCondition2Operator;
    }
    const QString& newCondition3Operator = oldSmarties.getCondition3Operator();
    if (sDebug) {
        qDebug() << "[SMARTIES] [DUPLICATE] -> old Condition3Operator" << newCondition3Operator;
    }
    const QString& newCondition4Operator = oldSmarties.getCondition4Operator();
    const QString& newCondition5Operator = oldSmarties.getCondition5Operator();
    const QString& newCondition6Operator = oldSmarties.getCondition6Operator();
    const QString& newCondition7Operator = oldSmarties.getCondition7Operator();
    const QString& newCondition8Operator = oldSmarties.getCondition8Operator();
    const QString& newCondition9Operator = oldSmarties.getCondition9Operator();
    const QString& newCondition10Operator = oldSmarties.getCondition10Operator();
    const QString& newCondition11Operator = oldSmarties.getCondition11Operator();
    const QString& newCondition12Operator = oldSmarties.getCondition12Operator();

    const QString& newCondition1Value = oldSmarties.getCondition1Value();
    if (sDebug) {
        qDebug() << "[SMARTIES] [DUPLICATE] -> old Condition1Value" << newCondition1Value;
    }
    const QString& newCondition2Value = oldSmarties.getCondition2Value();
    if (sDebug) {
        qDebug() << "[SMARTIES] [DUPLICATE] -> old Condition2Value" << newCondition2Value;
    }
    const QString& newCondition3Value = oldSmarties.getCondition3Value();
    if (sDebug) {
        qDebug() << "[SMARTIES] [DUPLICATE] -> old Condition3Value" << newCondition2Value;
    }
    const QString& newCondition4Value = oldSmarties.getCondition4Value();
    const QString& newCondition5Value = oldSmarties.getCondition5Value();
    const QString& newCondition6Value = oldSmarties.getCondition6Value();
    const QString& newCondition7Value = oldSmarties.getCondition7Value();
    const QString& newCondition8Value = oldSmarties.getCondition8Value();
    const QString& newCondition9Value = oldSmarties.getCondition9Value();
    const QString& newCondition10Value = oldSmarties.getCondition10Value();
    const QString& newCondition11Value = oldSmarties.getCondition11Value();
    const QString& newCondition12Value = oldSmarties.getCondition12Value();

    const QString& newCondition1Combiner = oldSmarties.getCondition1Combiner();
    if (sDebug) {
        qDebug() << "[SMARTIES] [DUPLICATE] -> old Condition1Combiner" << newCondition1Combiner;
    }
    const QString& newCondition2Combiner = oldSmarties.getCondition2Combiner();
    if (sDebug) {
        qDebug() << "[SMARTIES] [DUPLICATE] -> old Condition2Combiner" << newCondition2Combiner;
    }
    const QString& newCondition3Combiner = oldSmarties.getCondition3Combiner();
    if (sDebug) {
        qDebug() << "[SMARTIES] [DUPLICATE] -> old Condition3Combiner" << newCondition3Combiner;
    }
    const QString& newCondition4Combiner = oldSmarties.getCondition4Combiner();
    const QString& newCondition5Combiner = oldSmarties.getCondition5Combiner();
    const QString& newCondition6Combiner = oldSmarties.getCondition6Combiner();
    const QString& newCondition7Combiner = oldSmarties.getCondition7Combiner();
    const QString& newCondition8Combiner = oldSmarties.getCondition8Combiner();
    const QString& newCondition9Combiner = oldSmarties.getCondition9Combiner();
    const QString& newCondition10Combiner = oldSmarties.getCondition10Combiner();
    const QString& newCondition11Combiner = oldSmarties.getCondition11Combiner();
    const QString& newCondition12Combiner = oldSmarties.getCondition12Combiner();

    Smarties newSmarties;
    for (;;) {
        bool ok = false;
        auto newName =
                QInputDialog::getText(
                        nullptr,
                        tr("Duplicate Smarties"),
                        tr("Enter name for new smarties:"),
                        QLineEdit::Normal,
                        proposedSmartiesName,
                        &ok)
                        .trimmed();
        if (!ok) {
            return SmartiesId();
        }
        if (newName.isEmpty()) {
            QMessageBox::warning(
                    nullptr,
                    tr("Duplicating Smarties Failed"),
                    tr("A smarties cannot have a blank name."));
            continue;
        }
        if (m_pTrackCollection->smarties().readSmartiesByName(newName)) {
            QMessageBox::warning(
                    nullptr,
                    tr("Duplicating Smarties Failed"),
                    tr("A smarties by that name already exists."));
            continue;
        }
        newSmarties.setName(std::move(newName));
        newSmarties.setSearchInput(newSearchInput);
        newSmarties.setSearchSql(newSearchSql);
        newSmarties.setCondition1Field(newCondition1Field);
        newSmarties.setCondition2Field(newCondition2Field);
        newSmarties.setCondition3Field(newCondition3Field);
        newSmarties.setCondition4Field(newCondition4Field);
        newSmarties.setCondition5Field(newCondition5Field);
        newSmarties.setCondition6Field(newCondition6Field);
        newSmarties.setCondition7Field(newCondition7Field);
        newSmarties.setCondition8Field(newCondition8Field);
        newSmarties.setCondition9Field(newCondition9Field);
        newSmarties.setCondition10Field(newCondition10Field);
        newSmarties.setCondition11Field(newCondition11Field);
        newSmarties.setCondition12Field(newCondition12Field);
        newSmarties.setCondition1Operator(newCondition1Operator);
        newSmarties.setCondition2Operator(newCondition2Operator);
        newSmarties.setCondition3Operator(newCondition3Operator);
        newSmarties.setCondition4Operator(newCondition4Operator);
        newSmarties.setCondition5Operator(newCondition5Operator);
        newSmarties.setCondition6Operator(newCondition6Operator);
        newSmarties.setCondition7Operator(newCondition7Operator);
        newSmarties.setCondition8Operator(newCondition8Operator);
        newSmarties.setCondition9Operator(newCondition9Operator);
        newSmarties.setCondition10Operator(newCondition10Operator);
        newSmarties.setCondition11Operator(newCondition11Operator);
        newSmarties.setCondition12Operator(newCondition12Operator);
        newSmarties.setCondition1Value(newCondition1Value);
        newSmarties.setCondition2Value(newCondition2Value);
        newSmarties.setCondition3Value(newCondition3Value);
        newSmarties.setCondition4Value(newCondition4Value);
        newSmarties.setCondition5Value(newCondition5Value);
        newSmarties.setCondition6Value(newCondition6Value);
        newSmarties.setCondition7Value(newCondition7Value);
        newSmarties.setCondition8Value(newCondition8Value);
        newSmarties.setCondition9Value(newCondition9Value);
        newSmarties.setCondition10Value(newCondition10Value);
        newSmarties.setCondition11Value(newCondition11Value);
        newSmarties.setCondition12Value(newCondition12Value);
        newSmarties.setCondition1Combiner(newCondition1Combiner);
        newSmarties.setCondition2Combiner(newCondition2Combiner);
        newSmarties.setCondition3Combiner(newCondition3Combiner);
        newSmarties.setCondition4Combiner(newCondition4Combiner);
        newSmarties.setCondition5Combiner(newCondition5Combiner);
        newSmarties.setCondition6Combiner(newCondition6Combiner);
        newSmarties.setCondition7Combiner(newCondition7Combiner);
        newSmarties.setCondition8Combiner(newCondition8Combiner);
        newSmarties.setCondition9Combiner(newCondition9Combiner);
        newSmarties.setCondition10Combiner(newCondition10Combiner);
        newSmarties.setCondition11Combiner(newCondition11Combiner);
        newSmarties.setCondition12Combiner(newCondition12Combiner);

        DEBUG_ASSERT(newSmarties.hasName());
        break;
    }

    SmartiesId newSmartiesId;
    if (m_pTrackCollection->insertSmarties(newSmarties, &newSmartiesId)) {
        DEBUG_ASSERT(newSmartiesId.isValid());
        newSmarties.setId(newSmartiesId);

        if (sDebug) {
            qDebug() << "[SMARTIES] [DUPLICATE SMARTIES] -> Created new smarties" << newSmarties;
        }
        QList<TrackId> trackIds;
        trackIds.reserve(
                m_pTrackCollection->smarties().countSmartiesTracks(oldSmarties.getId()));
        {
            SmartiesTrackSelectResult smartiesTracks(
                    m_pTrackCollection->smarties().selectSmartiesTracksSorted(oldSmarties.getId()));
            while (smartiesTracks.next()) {
                trackIds.append(smartiesTracks.trackId());
            }
        }
        if (m_pTrackCollection->addSmartiesTracks(newSmartiesId, trackIds)) {
            if (sDebug) {
                qDebug() << "[SMARTIES] [DUPLICATE SMARTIES] Duplicated smarties -> "
                         << oldSmarties << "->" << newSmarties;
            }
        } else {
            qWarning() << "Failed to copy tracks from "
                       << oldSmarties << "into" << newSmarties;
        }
    } else {
        qWarning() << "Failed to duplicate smarties "
                   << oldSmarties << "->" << newSmarties.getName();
        QMessageBox::warning(
                nullptr,
                tr("Duplicating Smarties Failed"),
                tr("An unknown error occurred while creating smarties: ") + newSmarties.getName());
    }
    if (sDebug) {
        qDebug() << "[SMARTIES] [DUPLICATE SMARTIES] -> END -> newSmartiesId " << newSmartiesId;
    }
    return newSmartiesId;
}
