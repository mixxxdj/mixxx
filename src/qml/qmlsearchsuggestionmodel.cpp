#include "qml/qmlsearchsuggestionmodel.h"

#include <QHash>
#include <QSqlDatabase>
#include <QString>
#include <QVector>

#include "library/dao/trackschema.h"
#include "library/trackcollection.h"
#include "moc_qmlsearchsuggestionmodel.cpp"
#include "track/keyutils.h"
#include "util/assert.h"
#include "util/db/fwdsqlquery.h"
#include "util/db/sqllikewildcards.h"

namespace mixxx {
namespace qml {
namespace {

constexpr int kMaxSuggestions = 50;

const QHash<int, QByteArray> kRoleNames = {
        {QmlSearchSuggestionModel::ValueRole, "value"},
        {QmlSearchSuggestionModel::LabelRole, "label"},
};

// Returns the SQL expression that yields the human-readable value of the
// library column associated with a search field, or an empty string when the
// field does not map to a library column.
QString fieldExpression(QmlSearchSuggestionModel::SearchField field) {
    using SearchField = QmlSearchSuggestionModel::SearchField;
    switch (field) {
    case SearchField::Artist:
        return LIBRARYTABLE_ARTIST;
    case SearchField::Album:
        return LIBRARYTABLE_ALBUM;
    case SearchField::Title:
        return LIBRARYTABLE_TITLE;
    case SearchField::Genre:
        return LIBRARYTABLE_GENRE;
    case SearchField::Composer:
        return LIBRARYTABLE_COMPOSER;
    case SearchField::Comment:
        return LIBRARYTABLE_COMMENT;
    case SearchField::Year:
        return QStringLiteral("CAST(%1 AS TEXT)").arg(LIBRARYTABLE_YEAR);
    case SearchField::BPM:
        return QStringLiteral("CAST(ROUND(%1) AS TEXT)").arg(LIBRARYTABLE_BPM);
    case SearchField::Key:
    case SearchField::Track:
    case SearchField::Invalid:
        return QString();
    }
    return QString();
}

// Maps the canonical (lowercase) search-field name used in the query syntax to
// the corresponding enum value.
QmlSearchSuggestionModel::SearchField searchFieldFromName(const QString& field) {
    using SearchField = QmlSearchSuggestionModel::SearchField;
    static const QHash<QString, SearchField> map = {
            {QStringLiteral("artist"), SearchField::Artist},
            {QStringLiteral("album"), SearchField::Album},
            {QStringLiteral("title"), SearchField::Title},
            {QStringLiteral("genre"), SearchField::Genre},
            {QStringLiteral("composer"), SearchField::Composer},
            {QStringLiteral("comment"), SearchField::Comment},
            {QStringLiteral("year"), SearchField::Year},
            {QStringLiteral("bpm"), SearchField::BPM},
            {QStringLiteral("key"), SearchField::Key},
            {QStringLiteral("track"), SearchField::Track},
    };
    return map.value(field.toLower(), SearchField::Invalid);
}

QString escapeLikePattern(QString pattern) {
    pattern.replace(QStringLiteral("\\"), QStringLiteral("\\\\"));
    pattern.replace(QStringLiteral("%"), QStringLiteral("\\%"));
    pattern.replace(QStringLiteral("_"), QStringLiteral("\\_"));
    return pattern;
}

QString keyNotationLabel(KeyUtils::KeyNotation notation) {
    switch (notation) {
    case KeyUtils::KeyNotation::OpenKey:
        return QStringLiteral("OpenKey");
    case KeyUtils::KeyNotation::Lancelot:
        return QStringLiteral("Lancelot");
    case KeyUtils::KeyNotation::Traditional:
        return QStringLiteral("Traditional");
    default:
        return QString();
    }
}

} // namespace

QmlSearchSuggestionModel::QmlSearchSuggestionModel(
        TrackCollection* pTrackCollection, QObject* parent)
        : QAbstractListModel(parent),
          m_pTrackCollection(pTrackCollection) {
}

void QmlSearchSuggestionModel::setQuery(const QString& field, const QString& prefix) {
    const SearchField searchField = searchFieldFromName(field);
    switch (searchField) {
    case SearchField::Artist:
    case SearchField::Album:
    case SearchField::Title:
    case SearchField::Genre:
    case SearchField::Composer:
    case SearchField::Comment:
    case SearchField::Year:
    case SearchField::BPM:
        setValueSuggestions(searchField, prefix);
        return;
    case SearchField::Key:
        setKeySuggestions(prefix);
        return;
    case SearchField::Track:
        setTrackSuggestions(prefix);
        return;
    case SearchField::Invalid:
        beginResetModel();
        m_suggestions.clear();
        endResetModel();
        return;
    }
}

void QmlSearchSuggestionModel::setValueSuggestions(
        SearchField field, const QString& prefix) {
    const QString expression = fieldExpression(field);

    VERIFY_OR_DEBUG_ASSERT(!expression.isEmpty()) {
        beginResetModel();
        m_suggestions.clear();
        endResetModel();
        return;
    }

    const QString sql =
            QStringLiteral("SELECT DISTINCT %1 AS value FROM " LIBRARY_TABLE
                           " WHERE %1 != '' AND %1 LIKE :prefix ESCAPE '\\'"
                           " ORDER BY value COLLATE NOCASE LIMIT %2")
                    .arg(expression, QString::number(kMaxSuggestions));

    beginResetModel();
    m_suggestions = runSuggestionsQuery(sql, prefix);
    endResetModel();
}

void QmlSearchSuggestionModel::setKeySuggestions(const QString& prefix) {
    const QString needle = prefix.trimmed();
    QVector<Suggestion> suggestions;

    for (int keyValue = mixxx::track::io::key::INVALID + 1;
            keyValue <= mixxx::track::io::key::ChromaticKey_MAX;
            ++keyValue) {
        const auto key = static_cast<mixxx::track::io::key::ChromaticKey>(keyValue);
        for (auto notation : {KeyUtils::KeyNotation::Traditional,
                     KeyUtils::KeyNotation::OpenKey,
                     KeyUtils::KeyNotation::Lancelot}) {
            const QString value = KeyUtils::keyToString(key, notation);
            if (value.isEmpty()) {
                continue;
            }
            if (!needle.isEmpty() && !value.contains(needle, Qt::CaseInsensitive)) {
                continue;
            }
            suggestions.push_back({value, keyNotationLabel(notation)});
        }
    }

    beginResetModel();
    m_suggestions = std::move(suggestions);
    endResetModel();
}

void QmlSearchSuggestionModel::setTrackSuggestions(const QString& prefix) {
    const QString sql =
            QStringLiteral(
                    "SELECT DISTINCT %1 || ' - ' || %2"
                    " || CASE WHEN COALESCE(%3, '') != '' THEN ' (' || "
                    "%3 || ')' ELSE '' END AS value"
                    " FROM " LIBRARY_TABLE
                    " WHERE %1 LIKE :prefix ESCAPE '\\' OR %2 LIKE "
                    ":prefix ESCAPE '\\' OR %3 LIKE :prefix ESCAPE '\\'"
                    " ORDER BY value COLLATE NOCASE LIMIT %4")
                    .arg(LIBRARYTABLE_ARTIST,
                            LIBRARYTABLE_TITLE,
                            LIBRARYTABLE_ALBUM,
                            QString::number(kMaxSuggestions));

    beginResetModel();
    m_suggestions = runSuggestionsQuery(sql, prefix);
    endResetModel();
}

QVector<QmlSearchSuggestionModel::Suggestion> QmlSearchSuggestionModel::runSuggestionsQuery(
        const QString& sql, const QString& prefix) {
    VERIFY_OR_DEBUG_ASSERT(m_pTrackCollection) {
        return {};
    }

    const auto database = m_pTrackCollection->database();
    VERIFY_OR_DEBUG_ASSERT(database.isOpen()) {
        return {};
    }

    FwdSqlQuery query(database, sql);
    query.bindValue(QStringLiteral(":prefix"),
            QString(kSqlLikeMatchAll) + escapeLikePattern(prefix) + QString(kSqlLikeMatchAll));

    QVector<Suggestion> suggestions;
    if (query.execPrepared()) {
        const auto valueIndex = query.fieldIndex(QStringLiteral("value"));
        while (query.next()) {
            const QString value = query.fieldValue(valueIndex).toString();
            if (value.isEmpty()) {
                continue;
            }
            suggestions.push_back({value, QString()});
        }
    }
    return suggestions;
}

QVariant QmlSearchSuggestionModel::data(const QModelIndex& index, int role) const {
    if (!index.isValid() || index.row() < 0 || index.row() >= m_suggestions.size()) {
        return {};
    }
    const auto& suggestion = m_suggestions.at(index.row());
    switch (role) {
    case ValueRole:
    case Qt::DisplayRole:
        return suggestion.value;
    case LabelRole:
        return suggestion.label;
    default:
        return {};
    }
}

int QmlSearchSuggestionModel::rowCount(const QModelIndex& parent) const {
    if (parent.isValid()) {
        return 0;
    }
    return m_suggestions.size();
}

QHash<int, QByteArray> QmlSearchSuggestionModel::roleNames() const {
    return kRoleNames;
}

QVariant QmlSearchSuggestionModel::get(int row) const {
    QVariantMap dataMap;
    const QModelIndex idx = index(row, 0);
    if (!idx.isValid()) {
        return dataMap;
    }
    for (auto it = kRoleNames.constBegin(); it != kRoleNames.constEnd(); ++it) {
        dataMap.insert(QString::fromUtf8(it.value()), data(idx, it.key()));
    }
    return dataMap;
}

} // namespace qml
} // namespace mixxx
