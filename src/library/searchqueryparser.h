#pragma once

#include <QRegExp>
#include <QString>
#include <QtSql>

#include "library/searchquery.h"
#include "library/trackcollection.h"
#include "util/class.h"

class SearchQueryParser {
  public:
    explicit SearchQueryParser(TrackCollection* pTrackCollection, QStringList searchColumns);

    virtual ~SearchQueryParser();

    void setSearchColumns(QStringList searchColumns);

    std::unique_ptr<QueryNode> parseQuery(
            const QString& query,
            const QString& extraFilter) const;


  private:
    void parseTokens(QStringList tokens,
                     AndNode* pQuery) const;

    QString getTextArgument(QString argument,
                            QStringList* tokens) const;

    TrackCollection* m_pTrackCollection;
    QStringList m_queryColumns;
    bool m_searchCrates;
    QStringList m_textFilters;
    QStringList m_numericFilters;
    QStringList m_specialFilters;
    QStringList m_allFilters;
    QHash<QString, QStringList> m_fieldToSqlColumns;

    QRegExp m_fuzzyMatcher;
    QRegExp m_textFilterMatcher;
    QRegExp m_crateFilterMatcher;
    QRegExp m_numericFilterMatcher;
    QRegExp m_specialFilterMatcher;

    DISALLOW_COPY_AND_ASSIGN(SearchQueryParser);
};
