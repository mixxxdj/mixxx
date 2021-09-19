#pragma once

#include <QRegularExpression>
#include <QString>
#include <QtSql>
#include <memory>
#include <vector>

#include "library/searchquery.h"
#include "library/trackcollection.h"
#include "util/class.h"

class SearchQueryParser {
  public:
    // The id column is needed for custom tag subselects. If it
    // is not empty then searching for tags is disabled.
    explicit SearchQueryParser(
            TrackCollection* pTrackCollection,
            const QString& idColumn = QString());
    virtual ~SearchQueryParser() = default;

    std::unique_ptr<QueryNode> parseQuery(
            const QString& query,
            const QStringList& searchColumns,
            const QString& extraFilter) const;


  private:
    struct QueryNodes {
        std::unique_ptr<AndNode> pMainFilterQueryNode;
        std::unique_ptr<AndNode> pOrTagSubselectFilterNode;
        std::unique_ptr<AndNode> pAndTagSubselectFilterNode;
    };
    QueryNodes parseTokens(
            QStringList tokens,
            QStringList searchColumns) const;

    QString getTextArgument(QString argument,
                            QStringList* tokens) const;

    TrackCollection* const m_pTrackCollection;

    const QString m_idColumn;

    QStringList m_textFilters;
    QStringList m_numericFilters;
    QStringList m_specialFilters;
    QStringList m_tagFilters;
    QStringList m_ignoredColumns;
    QStringList m_allFilters;
    QHash<QString, QStringList> m_fieldToSqlColumns;

    QRegularExpression m_fuzzyMatcher;
    QRegularExpression m_textFilterMatcher;
    QRegularExpression m_crateFilterMatcher;
    QRegularExpression m_numericFilterMatcher;
    QRegularExpression m_specialFilterMatcher;
    QRegularExpression m_tagFilterMatcher;

    DISALLOW_COPY_AND_ASSIGN(SearchQueryParser);
};
