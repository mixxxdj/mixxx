#ifndef SEARCHQUERYPARSER_H
#define SEARCHQUERYPARSER_H

#include <QRegExp>
#include <QString>
#include <QtSql>

#include "util.h"
#include "library/trackcollection.h"
#include "trackinfoobject.h"
#include "library/searchquery.h"

class SearchQueryParser {
  public:
    SearchQueryParser(TrackCollection* pTrackCollection);
    virtual ~SearchQueryParser();

    QueryNode* parseQuery(const QString& query,
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
    QStringList m_allFilters;
    QHash<QString, QStringList> m_fieldToSqlColumns;

    QRegExp m_fuzzyMatcher;
    QRegExp m_textFilterMatcher;
    QRegExp m_numericFilterMatcher;
    QRegExp m_specialFilterMatcher;

    DISALLOW_COPY_AND_ASSIGN(SearchQueryParser);
};

#endif /* SEARCHQUERYPARSER_H */
