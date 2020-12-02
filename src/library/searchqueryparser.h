#ifndef SEARCHQUERYPARSER_H
#define SEARCHQUERYPARSER_H

#include <QHashNode>
#include <QRegExp>
#include <QString>
#include <QStringList>
#include <QtSql>
#include <memory>

#include "library/searchquery.h"
#include "library/trackcollection.h"
#include "util/class.h"

class AndNode;
class QueryNode;
class TrackCollection;

class SearchQueryParser {
  public:
    explicit SearchQueryParser(TrackCollection* pTrackCollection);

    virtual ~SearchQueryParser();

    std::unique_ptr<QueryNode> parseQuery(
            const QString& query,
            const QStringList& searchColumns,
            const QString& extraFilter) const;


  private:
    void parseTokens(QStringList tokens,
                     QStringList searchColumns,
                     AndNode* pQuery) const;

    QString getTextArgument(QString argument,
                            QStringList* tokens) const;

    TrackCollection* m_pTrackCollection;
    QStringList m_textFilters;
    QStringList m_numericFilters;
    QStringList m_specialFilters;
    QStringList m_ignoredColumns;
    QStringList m_allFilters;
    QHash<QString, QStringList> m_fieldToSqlColumns;

    QRegExp m_fuzzyMatcher;
    QRegExp m_textFilterMatcher;
    QRegExp m_crateFilterMatcher;
    QRegExp m_numericFilterMatcher;
    QRegExp m_specialFilterMatcher;

    DISALLOW_COPY_AND_ASSIGN(SearchQueryParser);
};

#endif /* SEARCHQUERYPARSER_H */
