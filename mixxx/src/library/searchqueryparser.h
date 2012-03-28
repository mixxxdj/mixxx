#ifndef SEARCHQUERYPARSER_H
#define SEARCHQUERYPARSER_H

#include <QRegExp>
#include <QString>
#include <QtSql>

#include "util.h"

class SearchQueryParser {
  public:
    SearchQueryParser(QSqlDatabase& database);
    virtual ~SearchQueryParser();

    QString parseQuery(const QString& query,
                       const QStringList& searchColumns,
                       const QString& extraFilter) const;

  private:
    bool searchFieldsForPhrase(const QString& phrase,
                               const QStringList& fields,
                               QStringList* output) const;

    void parseTokens(QStringList tokens,
                     QStringList searchColumns,
                     QStringList* output) const;

    QString getTextArgument(QString argument,
                            QStringList* tokens) const;

    bool parseFuzzyMatch(QString field, QStringList* output) const;
    bool parseTextFilter(QString field, QString argument,
                         QStringList* tokens, QStringList* output) const;
    bool parseNumericFilter(QString field, QString argument,
                            QStringList* tokens, QStringList* output) const;
    bool parseSpecialFilter(QString field, QString argument,
                            QStringList* tokens, QStringList* output) const;

    QSqlDatabase m_database;

    QStringList m_textFilters;
    QStringList m_numericFilters;
    QStringList m_specialFilters;
    QStringList m_allFilters;
    QHash<QString, QString> m_fieldToSqlColumn;

    QRegExp m_operatorMatcher;
    QRegExp m_fuzzyMatcher;
    QRegExp m_textFilterMatcher;
    QRegExp m_numericFilterMatcher;
    QRegExp m_specialFilterMatcher;

    DISALLOW_COPY_AND_ASSIGN(SearchQueryParser);
};

#endif /* SEARCHQUERYPARSER_H */
