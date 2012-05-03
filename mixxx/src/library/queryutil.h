#ifndef QUERYUTIL_H
#define QUERYUTIL_H

#include <QtDebug>
#include <QtSql>

#define LOG_FAILED_QUERY(query) qDebug() << __FILE__ << __LINE__ << "FAILED QUERY [" \
    << query.executedQuery() << "]" << query.lastError()

class FieldEscaper {
  public:
    FieldEscaper(const QSqlDatabase& database)
            : m_database(database),
              m_stringField("string", QVariant::String) {
    }
    virtual ~FieldEscaper() {
    }

    QString escapeString(const QString& escapeString) const {
        m_stringField.setValue(escapeString);
        return m_database.driver()->formatValue(m_stringField);
    }

  private:
    const QSqlDatabase& m_database;
    mutable QSqlField m_stringField;
};
#endif /* QUERYUTIL_H */
