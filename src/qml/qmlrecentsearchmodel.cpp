#include "qml/qmlrecentsearchmodel.h"

#include <QHash>
#include <QStringList>
#include <QVariantMap>

#include "library/searchqueries.h"
#include "moc_qmlrecentsearchmodel.cpp"
#include "qml/qmlconfigproxy.h"

namespace mixxx {
namespace qml {
namespace {

const QHash<int, QByteArray> kRoleNames = {
        {QmlRecentSearchModel::TokensRole, "tokens"},
        {QmlRecentSearchModel::FreeTextRole, "freeText"},
        {QmlRecentSearchModel::QueryStringRole, "queryString"},
};

} // anonymous namespace

QmlRecentSearchModel::QmlRecentSearchModel(QObject* parent)
        : QAbstractListModel(parent) {
    const QStringList queries = SearchQueries::loadQueries(QmlConfigProxy::get());
    m_searches.reserve(queries.size());
    for (const QString& query : queries) {
        const QVariantMap parsed = SearchQueries::parseQuery(query);
        m_searches.append({parsed.value(QStringLiteral("tokens")).toList(),
                parsed.value(QStringLiteral("freeText")).toString(),
                query});
    }
}

int QmlRecentSearchModel::persist(const QVariantList& tokens,
        const QString& freeText,
        int activeRow) {
    if (tokens.isEmpty() && freeText.isEmpty()) {
        return -1;
    }
    const QString queryString = SearchQueries::serializeQuery(tokens, freeText);
    if (queryString.isEmpty()) {
        return -1;
    }
    if (activeRow >= 0 && activeRow < m_searches.size()) {
        m_searches[activeRow] = {tokens, freeText, queryString};
        const QModelIndex idx = index(activeRow);
        emit dataChanged(idx, idx);
    } else {
        // Drop an equal entry before prepending, so re-persisting an
        // older query moves it to the front instead of accumulating a
        // duplicate until the next reload.
        for (int row = m_searches.size() - 1; row >= 0; --row) {
            if (m_searches.at(row).queryString == queryString) {
                beginRemoveRows(QModelIndex(), row, row);
                m_searches.removeAt(row);
                endRemoveRows();
            }
        }
        beginInsertRows(QModelIndex(), 0, 0);
        m_searches.prepend({tokens, freeText, queryString});
        endInsertRows();
        if (m_searches.size() > SearchQueries::kMaxQueries) {
            const int row = m_searches.size() - 1;
            beginRemoveRows(QModelIndex(), row, row);
            m_searches.removeLast();
            endRemoveRows();
        }
        activeRow = 0;
    }
    saveQueriesToConfig();
    return activeRow;
}

QVariantMap QmlRecentSearchModel::get(int row) const {
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

QVariant QmlRecentSearchModel::data(const QModelIndex& modelIndex, int role) const {
    if (!modelIndex.isValid() || modelIndex.row() >= m_searches.size()) {
        return {};
    }
    const RecentSearch& search = m_searches.at(modelIndex.row());
    switch (role) {
    case TokensRole:
        return search.tokens;
    case FreeTextRole:
        return search.freeText;
    case QueryStringRole:
        return search.queryString;
    default:
        return {};
    }
}

int QmlRecentSearchModel::rowCount(const QModelIndex& parent) const {
    return parent.isValid() ? 0 : m_searches.size();
}

QHash<int, QByteArray> QmlRecentSearchModel::roleNames() const {
    return kRoleNames;
}

void QmlRecentSearchModel::saveQueriesToConfig() const {
    QStringList queries;
    queries.reserve(m_searches.size());
    for (const RecentSearch& search : m_searches) {
        queries.append(search.queryString);
    }
    SearchQueries::saveQueries(QmlConfigProxy::get(), queries);
}

} // namespace qml
} // namespace mixxx
