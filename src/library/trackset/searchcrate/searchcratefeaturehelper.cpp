#include "library/trackset/searchcrate/searchcratefeaturehelper.h"

#include <QInputDialog>
#include <QLineEdit>
#include <QRegularExpression>
#include <QString>
#include <QStringList>

#include "library/trackcollection.h"
#include "library/trackset/basetracksetfeature.h"
#include "library/trackset/searchcrate/searchcrate.h"
#include "library/trackset/searchcrate/searchcratesummary.h"
#include "moc_searchcratefeaturehelper.cpp"

const bool sDebug = false;

SearchCrateFeatureHelper::SearchCrateFeatureHelper(
        TrackCollection* pTrackCollection,
        UserSettingsPointer pConfig)
        : m_pTrackCollection(pTrackCollection),
          m_pConfig(pConfig) {
}

QString SearchCrateFeatureHelper::proposeNameForNewSearchCrate(
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
    } while (m_pTrackCollection->searchCrates().readSearchCrateByName(proposedName));
    // Found an unused searchCrate name
    if (sDebug) {
        qDebug() << "[SEARCHCRATES] [HELPER] [PROPOSE NEW NAME] -> proposedName" << proposedName;
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
        qDebug() << "[SEARCHCRATES] [HELPER] [PROPOSE NEW NAME] -> cleaned "
                    "proposedName"
                 << proposedName;
    }
    return proposedName;
}

SearchCrateId SearchCrateFeatureHelper::createEmptySearchCrateFromSearch(const QString& text) {
    if (sDebug) {
        qDebug() << "[SEARCHCRATES] [HELPER] [NEW SEARCHCRATES FROM SEARCH]";
    }
    SearchCrate newSearchCrate;
    const QString proposedSearchCrateName =
            proposeNameForNewSearchCrate(tr("Search for ") + text);
    for (;;) {
        bool ok = false;
        auto newName =
                QInputDialog::getText(
                        nullptr,
                        tr("New SearchCrate From Search"),
                        tr("Enter name for new searchCrate:"),
                        QLineEdit::Normal,
                        proposedSearchCrateName,
                        &ok)
                        .trimmed();
        if (!ok) {
            return SearchCrateId();
        }
        if (newName.isEmpty()) {
            QMessageBox::warning(
                    nullptr,
                    tr("Creating SearchCrate Failed"),
                    tr("A searchCrate cannot have a blank name."));
            continue;
        }
        if (m_pTrackCollection->searchCrates().readSearchCrateByName(newName)) {
            QMessageBox::warning(
                    nullptr,
                    tr("Creating SearchCrate Failed"),
                    tr("A searchCrate by that name already exists."));
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
                    newSearchCrate.setCondition1Field(term);
                    newSearchCrate.setCondition1Operator(operatorType);
                    newSearchCrate.setCondition1Value(value);
                    newSearchCrate.setCondition1Combiner(") END");
                    break;
                case 2:
                    newSearchCrate.setCondition2Field(term);
                    newSearchCrate.setCondition2Operator(operatorType);
                    newSearchCrate.setCondition2Value(value);
                    newSearchCrate.setCondition2Combiner(") END");
                    newSearchCrate.setCondition1Combiner(orCombiner ? ") OR (" : ") AND (");
                    break;
                case 3:
                    newSearchCrate.setCondition3Field(term);
                    newSearchCrate.setCondition3Operator(operatorType);
                    newSearchCrate.setCondition3Value(value);
                    newSearchCrate.setCondition3Combiner(") END");
                    newSearchCrate.setCondition2Combiner(orCombiner ? ") OR (" : ") AND (");
                    break;
                case 4:
                    newSearchCrate.setCondition4Field(term);
                    newSearchCrate.setCondition4Operator(operatorType);
                    newSearchCrate.setCondition4Value(value);
                    newSearchCrate.setCondition4Combiner(") END");
                    newSearchCrate.setCondition3Combiner(orCombiner ? ") OR (" : ") AND (");
                    break;
                case 5:
                    newSearchCrate.setCondition5Field(term);
                    newSearchCrate.setCondition5Operator(operatorType);
                    newSearchCrate.setCondition5Value(value);
                    newSearchCrate.setCondition5Combiner(") END");
                    newSearchCrate.setCondition4Combiner(orCombiner ? ") OR (" : ") AND (");
                    break;
                case 6:
                    newSearchCrate.setCondition6Field(term);
                    newSearchCrate.setCondition6Operator(operatorType);
                    newSearchCrate.setCondition6Value(value);
                    newSearchCrate.setCondition6Combiner(") END");
                    newSearchCrate.setCondition5Combiner(orCombiner ? ") OR (" : ") AND (");
                    break;
                case 7:
                    newSearchCrate.setCondition7Field(term);
                    newSearchCrate.setCondition7Operator(operatorType);
                    newSearchCrate.setCondition7Value(value);
                    newSearchCrate.setCondition7Combiner(") END");
                    newSearchCrate.setCondition6Combiner(orCombiner ? ") OR (" : ") AND (");
                    break;
                case 8:
                    newSearchCrate.setCondition8Field(term);
                    newSearchCrate.setCondition8Operator(operatorType);
                    newSearchCrate.setCondition8Value(value);
                    newSearchCrate.setCondition8Combiner(") END");
                    newSearchCrate.setCondition7Combiner(orCombiner ? ") OR (" : ") AND (");
                    break;
                case 9:
                    newSearchCrate.setCondition9Field(term);
                    newSearchCrate.setCondition9Operator(operatorType);
                    newSearchCrate.setCondition9Value(value);
                    newSearchCrate.setCondition9Combiner(") END");
                    newSearchCrate.setCondition8Combiner(orCombiner ? ") OR (" : ") AND (");
                    break;
                case 10:
                    newSearchCrate.setCondition10Field(term);
                    newSearchCrate.setCondition10Operator(operatorType);
                    newSearchCrate.setCondition10Value(value);
                    newSearchCrate.setCondition10Combiner(") END");
                    newSearchCrate.setCondition9Combiner(orCombiner ? ") OR (" : ") AND (");
                    break;
                case 11:
                    newSearchCrate.setCondition11Field(term);
                    newSearchCrate.setCondition11Operator(operatorType);
                    newSearchCrate.setCondition11Value(value);
                    newSearchCrate.setCondition11Combiner(") END");
                    newSearchCrate.setCondition10Combiner(orCombiner ? ") OR (" : ") AND (");
                    break;
                case 12:
                    newSearchCrate.setCondition12Field(term);
                    newSearchCrate.setCondition12Operator(operatorType);
                    newSearchCrate.setCondition12Value(value);
                    newSearchCrate.setCondition12Combiner(") END");
                    newSearchCrate.setCondition11Combiner(orCombiner ? ") OR (" : ") AND (");
                    break;
                }

                if (sDebug) {
                    qDebug() << "[SEARCHCRATES] [HELPER] [NEW SEARCHCRATES FROM SEARCH] -> "
                                "Detected Term:"
                             << term << ", Value:" << value;
                }
                ++conditionIndex;
            }
        }

        // what if no terms (field) were in the input ....
        if (!foundTerms) {
            if (sDebug) {
                qDebug() << "[SEARCHCRATES] [HELPER] [NEW SEARCHCRATES FROM SEARCH] -> No term "
                            "detected, setting noTermValue:"
                         << text.trimmed();
            }
            cleanedText.replace("\"", "");
            newSearchCrate.setSearchSql(newName);
            newSearchCrate.setCondition1Field("artist");
            newSearchCrate.setCondition1Operator("contains");
            newSearchCrate.setCondition1Value(cleanedText);
            newSearchCrate.setCondition1Combiner(") OR (");
            newSearchCrate.setCondition2Field("album_artist");
            newSearchCrate.setCondition2Operator("contains");
            newSearchCrate.setCondition2Value(cleanedText);
            newSearchCrate.setCondition2Combiner(") OR (");
            newSearchCrate.setCondition3Field("title");
            newSearchCrate.setCondition3Operator("contains");
            newSearchCrate.setCondition3Value(cleanedText);
            newSearchCrate.setCondition3Combiner(") OR (");
            newSearchCrate.setCondition4Field("album");
            newSearchCrate.setCondition4Operator("contains");
            newSearchCrate.setCondition4Value(cleanedText);
            newSearchCrate.setCondition4Combiner(") OR (");
            newSearchCrate.setCondition5Field("genre");
            newSearchCrate.setCondition5Operator("contains");
            newSearchCrate.setCondition5Value(cleanedText);
            newSearchCrate.setCondition5Combiner(") OR (");
            newSearchCrate.setCondition6Field("composer");
            newSearchCrate.setCondition6Operator("contains");
            newSearchCrate.setCondition6Value(cleanedText);
            newSearchCrate.setCondition6Combiner(") OR (");
            newSearchCrate.setCondition7Field("comment");
            newSearchCrate.setCondition7Operator("contains");
            newSearchCrate.setCondition7Value(cleanedText);
            newSearchCrate.setCondition7Combiner(") END");
        }

        newSearchCrate.setName(std::move(newName));
        DEBUG_ASSERT(newSearchCrate.hasName());
        newSearchCrate.setSearchInput(text);
        newSearchCrate.setSearchSql(text);
        break;
    }

    SearchCrateId newSearchCrateId;

    if (m_pTrackCollection->insertSearchCrate(newSearchCrate, &newSearchCrateId)) {
        DEBUG_ASSERT(newSearchCrateId.isValid());
        newSearchCrate.setId(newSearchCrateId);
        if (sDebug) {
            qDebug() << "[SEARCHCRATES] [HELPER] [NEW SEARCHCRATES FROM SEARCH] -> Created new "
                        "searchCrate"
                     << newSearchCrate;
            qDebug() << "[SEARCHCRATES] [HELPER] [NEW SEARCHCRATES FROM SEARCH] -> Created new "
                        "searchCrate ID: "
                     << newSearchCrateId;
        }
    } else {
        DEBUG_ASSERT(!newSearchCrateId.isValid());
        qWarning() << "[SEARCHCRATES] [HELPER] [NEW SEARCHCRATES FROM SEARCH] "
                      "-> Failed to create new searchCrate"
                   << "->" << newSearchCrate.getName();
        QMessageBox::warning(nullptr,
                tr("Creating SearchCrate Failed"),
                tr("An unknown error occurred while creating searchCrate: ") +
                        newSearchCrate.getName());
    }
    if (sDebug) {
        qDebug() << "[SEARCHCRATES] [HELPER] [NEW SEARCHCRATES FROM SEARCH] -> "
                    "newSearchCrateId "
                 << newSearchCrateId;
    }
    return newSearchCrateId;
}

SearchCrateId SearchCrateFeatureHelper::createEmptySearchCrateFromUI() {
    if (sDebug) {
        qDebug() << "[SEARCHCRATES] [HELPER] [NEW SEARCHCRATES FROM UI] ";
    }
    SearchCrate newSearchCrate;
    const QString proposedSearchCrateName =
            proposeNameForNewSearchCrate("New SearchCrate From Edit");
    if (sDebug) {
        qDebug() << "[SEARCHCRATES] [HELPER] [PROPOSE NEW NAME] -> proposedName"
                 << proposedSearchCrateName;
    }
    for (;;) {
        auto newName = proposedSearchCrateName;
        SearchCrateId();
        newSearchCrate.setName(std::move(newName));
        DEBUG_ASSERT(newSearchCrate.hasName());
        break;
    }
    SearchCrateId newSearchCrateId;
    if (m_pTrackCollection->insertSearchCrate(newSearchCrate, &newSearchCrateId)) {
        DEBUG_ASSERT(newSearchCrateId.isValid());
        newSearchCrate.setId(newSearchCrateId);
        if (sDebug) {
            qDebug() << "[SEARCHCRATES] [HELPER] [NEW SEARCHCRATES FROM UI] "
                        "Created new searchCrate"
                     << newSearchCrate;
        }
    } else {
        DEBUG_ASSERT(!newSearchCrateId.isValid());
        qWarning() << "Failed to create new searchCrate"
                   << "->" << newSearchCrate.getName();
    }
    if (sDebug) {
        qDebug() << "[SEARCHCRATES] [HELPER] [NEW SEARCHCRATES FROM UI] -> "
                    "newSearchCrateId "
                 << newSearchCrateId;
    }
    return newSearchCrateId;
}

SearchCrateId SearchCrateFeatureHelper::createEmptySearchCrate() {
    if (sDebug) {
        qDebug() << "[SEARCHCRATES] [HELPER] [NEW EMPTY SEARCHCRATES] ";
    }
    const QString proposedSearchCrateName =
            proposeNameForNewSearchCrate(tr("New SearchCrate"));
    SearchCrate newSearchCrate;
    for (;;) {
        bool ok = false;
        auto newName =
                QInputDialog::getText(
                        nullptr,
                        tr("Create New SearchCrate"),
                        tr("Enter name for new searchCrate:"),
                        QLineEdit::Normal,
                        proposedSearchCrateName,
                        &ok)
                        .trimmed();
        if (!ok) {
            return SearchCrateId();
        }
        if (newName.isEmpty()) {
            QMessageBox::warning(
                    nullptr,
                    tr("Creating SearchCrate Failed"),
                    tr("A searchCrate cannot have a blank name."));
            continue;
        }
        if (m_pTrackCollection->searchCrates().readSearchCrateByName(newName)) {
            QMessageBox::warning(
                    nullptr,
                    tr("Creating SearchCrate Failed"),
                    tr("A searchCrate by that name already exists."));
            continue;
        }
        newSearchCrate.setName(std::move(newName));
        DEBUG_ASSERT(newSearchCrate.hasName());
        break;
    }

    SearchCrateId newSearchCrateId;
    if (m_pTrackCollection->insertSearchCrate(newSearchCrate, &newSearchCrateId)) {
        DEBUG_ASSERT(newSearchCrateId.isValid());
        newSearchCrate.setId(newSearchCrateId);
        if (sDebug) {
            qDebug() << "Created new searchCrate" << newSearchCrate;
        }
    } else {
        DEBUG_ASSERT(!newSearchCrateId.isValid());
        qWarning() << "Failed to create new searchCrate"
                   << "->" << newSearchCrate.getName();
        QMessageBox::warning(nullptr,
                tr("Creating SearchCrate Failed"),
                tr("An unknown error occurred while creating searchCrate: ") +
                        newSearchCrate.getName());
    }
    if (sDebug) {
        qDebug() << "[SEARCHCRATES] [HELPER] [NEW EMPTY SEARCHCRATES] -> "
                    "newSearchCrateId "
                 << newSearchCrateId;
    }
    return newSearchCrateId;
}

SearchCrateId SearchCrateFeatureHelper::duplicateSearchCrate(const SearchCrate& oldSearchCrate) {
    if (sDebug) {
        qDebug() << "[SEARCHCRATES] [HELPER] [DUPLICATE SEARCHCRATES] -> START";
    }
    const QString& proposedSearchCrateName =
            proposeNameForNewSearchCrate(
                    QStringLiteral("%1 %2")
                            .arg(oldSearchCrate.getName(), tr("copy", "//:")));

    const QString& newSearchInput = oldSearchCrate.getSearchInput();
    if (sDebug) {
        qDebug() << "[SEARCHCRATES] [HELPER] [DUPLICATE] -> old searchInput" << newSearchInput;
    }
    const QString& newSearchSql = oldSearchCrate.getSearchSql();
    if (sDebug) {
        qDebug() << "[SEARCHCRATES] [HELPER] [DUPLICATE] -> old searchSql" << newSearchSql;
    }
    const QString& newCondition1Field = oldSearchCrate.getCondition1Field();
    if (sDebug) {
        qDebug() << "[SEARCHCRATES] [HELPER] [DUPLICATE] -> old Condition1Field"
                 << newCondition1Field;
    }
    const QString& newCondition2Field = oldSearchCrate.getCondition2Field();
    if (sDebug) {
        qDebug() << "[SEARCHCRATES] [HELPER] [DUPLICATE] -> old Condition2Field"
                 << newCondition2Field;
    }
    const QString& newCondition3Field = oldSearchCrate.getCondition3Field();
    if (sDebug) {
        qDebug() << "[SEARCHCRATES] [HELPER] [DUPLICATE] -> old Condition3Field"
                 << newCondition3Field;
    }
    const QString& newCondition4Field = oldSearchCrate.getCondition4Field();
    const QString& newCondition5Field = oldSearchCrate.getCondition5Field();
    const QString& newCondition6Field = oldSearchCrate.getCondition6Field();
    const QString& newCondition7Field = oldSearchCrate.getCondition7Field();
    const QString& newCondition8Field = oldSearchCrate.getCondition8Field();
    const QString& newCondition9Field = oldSearchCrate.getCondition9Field();
    const QString& newCondition10Field = oldSearchCrate.getCondition10Field();
    const QString& newCondition11Field = oldSearchCrate.getCondition11Field();
    const QString& newCondition12Field = oldSearchCrate.getCondition12Field();

    const QString& newCondition1Operator = oldSearchCrate.getCondition1Operator();
    if (sDebug) {
        qDebug() << "[SEARCHCRATES] [HELPER] [DUPLICATE] -> old "
                    "Condition1Operator"
                 << newCondition1Operator;
    }
    const QString& newCondition2Operator = oldSearchCrate.getCondition2Operator();
    if (sDebug) {
        qDebug() << "[SEARCHCRATES] [HELPER] [DUPLICATE] -> old "
                    "Condition2Operator"
                 << newCondition2Operator;
    }
    const QString& newCondition3Operator = oldSearchCrate.getCondition3Operator();
    if (sDebug) {
        qDebug() << "[SEARCHCRATES] [HELPER] [DUPLICATE] -> old "
                    "Condition3Operator"
                 << newCondition3Operator;
    }
    const QString& newCondition4Operator = oldSearchCrate.getCondition4Operator();
    const QString& newCondition5Operator = oldSearchCrate.getCondition5Operator();
    const QString& newCondition6Operator = oldSearchCrate.getCondition6Operator();
    const QString& newCondition7Operator = oldSearchCrate.getCondition7Operator();
    const QString& newCondition8Operator = oldSearchCrate.getCondition8Operator();
    const QString& newCondition9Operator = oldSearchCrate.getCondition9Operator();
    const QString& newCondition10Operator = oldSearchCrate.getCondition10Operator();
    const QString& newCondition11Operator = oldSearchCrate.getCondition11Operator();
    const QString& newCondition12Operator = oldSearchCrate.getCondition12Operator();

    const QString& newCondition1Value = oldSearchCrate.getCondition1Value();
    if (sDebug) {
        qDebug() << "[SEARCHCRATES] [HELPER] [DUPLICATE] -> old Condition1Value"
                 << newCondition1Value;
    }
    const QString& newCondition2Value = oldSearchCrate.getCondition2Value();
    if (sDebug) {
        qDebug() << "[SEARCHCRATES] [HELPER] [DUPLICATE] -> old Condition2Value"
                 << newCondition2Value;
    }
    const QString& newCondition3Value = oldSearchCrate.getCondition3Value();
    if (sDebug) {
        qDebug() << "[SEARCHCRATES] [HELPER] [DUPLICATE] -> old Condition3Value"
                 << newCondition2Value;
    }
    const QString& newCondition4Value = oldSearchCrate.getCondition4Value();
    const QString& newCondition5Value = oldSearchCrate.getCondition5Value();
    const QString& newCondition6Value = oldSearchCrate.getCondition6Value();
    const QString& newCondition7Value = oldSearchCrate.getCondition7Value();
    const QString& newCondition8Value = oldSearchCrate.getCondition8Value();
    const QString& newCondition9Value = oldSearchCrate.getCondition9Value();
    const QString& newCondition10Value = oldSearchCrate.getCondition10Value();
    const QString& newCondition11Value = oldSearchCrate.getCondition11Value();
    const QString& newCondition12Value = oldSearchCrate.getCondition12Value();

    const QString& newCondition1Combiner = oldSearchCrate.getCondition1Combiner();
    if (sDebug) {
        qDebug() << "[SEARCHCRATES] [HELPER] [DUPLICATE] -> old "
                    "Condition1Combiner"
                 << newCondition1Combiner;
    }
    const QString& newCondition2Combiner = oldSearchCrate.getCondition2Combiner();
    if (sDebug) {
        qDebug() << "[SEARCHCRATES] [HELPER] [DUPLICATE] -> old "
                    "Condition2Combiner"
                 << newCondition2Combiner;
    }
    const QString& newCondition3Combiner = oldSearchCrate.getCondition3Combiner();
    if (sDebug) {
        qDebug() << "[SEARCHCRATES] [HELPER] [DUPLICATE] -> old "
                    "Condition3Combiner"
                 << newCondition3Combiner;
    }
    const QString& newCondition4Combiner = oldSearchCrate.getCondition4Combiner();
    const QString& newCondition5Combiner = oldSearchCrate.getCondition5Combiner();
    const QString& newCondition6Combiner = oldSearchCrate.getCondition6Combiner();
    const QString& newCondition7Combiner = oldSearchCrate.getCondition7Combiner();
    const QString& newCondition8Combiner = oldSearchCrate.getCondition8Combiner();
    const QString& newCondition9Combiner = oldSearchCrate.getCondition9Combiner();
    const QString& newCondition10Combiner = oldSearchCrate.getCondition10Combiner();
    const QString& newCondition11Combiner = oldSearchCrate.getCondition11Combiner();
    const QString& newCondition12Combiner = oldSearchCrate.getCondition12Combiner();

    SearchCrate newSearchCrate;
    for (;;) {
        bool ok = false;
        auto newName =
                QInputDialog::getText(
                        nullptr,
                        tr("Duplicate SearchCrate"),
                        tr("Enter name for new SearchCrate:"),
                        QLineEdit::Normal,
                        proposedSearchCrateName,
                        &ok)
                        .trimmed();
        if (!ok) {
            return SearchCrateId();
        }
        if (newName.isEmpty()) {
            QMessageBox::warning(
                    nullptr,
                    tr("Duplicating SearchCrate Failed"),
                    tr("A SearchCrate cannot have a blank name."));
            continue;
        }
        if (m_pTrackCollection->searchCrates().readSearchCrateByName(newName)) {
            QMessageBox::warning(
                    nullptr,
                    tr("Duplicating SearchCrate Failed"),
                    tr("A SearchCrate by that name already exists."));
            continue;
        }
        newSearchCrate.setName(std::move(newName));
        newSearchCrate.setSearchInput(newSearchInput);
        newSearchCrate.setSearchSql(newSearchSql);
        newSearchCrate.setCondition1Field(newCondition1Field);
        newSearchCrate.setCondition2Field(newCondition2Field);
        newSearchCrate.setCondition3Field(newCondition3Field);
        newSearchCrate.setCondition4Field(newCondition4Field);
        newSearchCrate.setCondition5Field(newCondition5Field);
        newSearchCrate.setCondition6Field(newCondition6Field);
        newSearchCrate.setCondition7Field(newCondition7Field);
        newSearchCrate.setCondition8Field(newCondition8Field);
        newSearchCrate.setCondition9Field(newCondition9Field);
        newSearchCrate.setCondition10Field(newCondition10Field);
        newSearchCrate.setCondition11Field(newCondition11Field);
        newSearchCrate.setCondition12Field(newCondition12Field);
        newSearchCrate.setCondition1Operator(newCondition1Operator);
        newSearchCrate.setCondition2Operator(newCondition2Operator);
        newSearchCrate.setCondition3Operator(newCondition3Operator);
        newSearchCrate.setCondition4Operator(newCondition4Operator);
        newSearchCrate.setCondition5Operator(newCondition5Operator);
        newSearchCrate.setCondition6Operator(newCondition6Operator);
        newSearchCrate.setCondition7Operator(newCondition7Operator);
        newSearchCrate.setCondition8Operator(newCondition8Operator);
        newSearchCrate.setCondition9Operator(newCondition9Operator);
        newSearchCrate.setCondition10Operator(newCondition10Operator);
        newSearchCrate.setCondition11Operator(newCondition11Operator);
        newSearchCrate.setCondition12Operator(newCondition12Operator);
        newSearchCrate.setCondition1Value(newCondition1Value);
        newSearchCrate.setCondition2Value(newCondition2Value);
        newSearchCrate.setCondition3Value(newCondition3Value);
        newSearchCrate.setCondition4Value(newCondition4Value);
        newSearchCrate.setCondition5Value(newCondition5Value);
        newSearchCrate.setCondition6Value(newCondition6Value);
        newSearchCrate.setCondition7Value(newCondition7Value);
        newSearchCrate.setCondition8Value(newCondition8Value);
        newSearchCrate.setCondition9Value(newCondition9Value);
        newSearchCrate.setCondition10Value(newCondition10Value);
        newSearchCrate.setCondition11Value(newCondition11Value);
        newSearchCrate.setCondition12Value(newCondition12Value);
        newSearchCrate.setCondition1Combiner(newCondition1Combiner);
        newSearchCrate.setCondition2Combiner(newCondition2Combiner);
        newSearchCrate.setCondition3Combiner(newCondition3Combiner);
        newSearchCrate.setCondition4Combiner(newCondition4Combiner);
        newSearchCrate.setCondition5Combiner(newCondition5Combiner);
        newSearchCrate.setCondition6Combiner(newCondition6Combiner);
        newSearchCrate.setCondition7Combiner(newCondition7Combiner);
        newSearchCrate.setCondition8Combiner(newCondition8Combiner);
        newSearchCrate.setCondition9Combiner(newCondition9Combiner);
        newSearchCrate.setCondition10Combiner(newCondition10Combiner);
        newSearchCrate.setCondition11Combiner(newCondition11Combiner);
        newSearchCrate.setCondition12Combiner(newCondition12Combiner);

        DEBUG_ASSERT(newSearchCrate.hasName());
        break;
    }

    SearchCrateId newSearchCrateId;
    if (m_pTrackCollection->insertSearchCrate(newSearchCrate, &newSearchCrateId)) {
        DEBUG_ASSERT(newSearchCrateId.isValid());
        newSearchCrate.setId(newSearchCrateId);

        if (sDebug) {
            qDebug() << "[SEARCHCRATES] [HELPER] [DUPLICATE SEARCHCRATES] -> "
                        "Created new searchCrate"
                     << newSearchCrate;
        }
        QList<TrackId> trackIds;
        trackIds.reserve(
                m_pTrackCollection->searchCrates().countSearchCrateTracks(oldSearchCrate.getId()));
        {
            SearchCrateTrackSelectResult searchCrateTracks(
                    m_pTrackCollection->searchCrates()
                            .selectSearchCrateTracksSorted(
                                    oldSearchCrate.getId()));
            while (searchCrateTracks.next()) {
                trackIds.append(searchCrateTracks.trackId());
            }
        }
        if (m_pTrackCollection->addSearchCrateTracks(newSearchCrateId, trackIds)) {
            if (sDebug) {
                qDebug() << "[SEARCHCRATES] [HELPER] [DUPLICATE SEARCHCRATES] "
                            "Duplicated searchCrate -> "
                         << oldSearchCrate << "->" << newSearchCrate;
            }
        } else {
            qWarning() << "Failed to copy tracks from "
                       << oldSearchCrate << "into" << newSearchCrate;
        }
    } else {
        qWarning() << "Failed to duplicate searchCrate "
                   << oldSearchCrate << "->" << newSearchCrate.getName();
        QMessageBox::warning(nullptr,
                tr("Duplicating SearchCrate Failed"),
                tr("An unknown error occurred while creating searchCrate: ") +
                        newSearchCrate.getName());
    }
    if (sDebug) {
        qDebug() << "[SEARCHCRATES] [HELPER] [DUPLICATE SEARCHCRATES] -> END "
                    "-> newSearchCrateId "
                 << newSearchCrateId;
    }
    return newSearchCrateId;
}
