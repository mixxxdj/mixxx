#ifndef QUERYUTIL_H
#define QUERYUTIL_H

#include <QtDebug>
#include <QtSql>


#define LOG_FAILED_QUERY(query) qWarning() << __FILE__ << __LINE__ << "FAILED QUERY [" \
    << (query).executedQuery() << "]" << (query).lastError()

class ScopedTransaction {
  public:
    explicit ScopedTransaction(const QSqlDatabase& database) :
            m_database(database),
            m_active(false) {
        if (!transaction()) {
            qDebug() << "ERROR: Could not start transaction on"
                     << m_database.connectionName();
        }
    }
    virtual ~ScopedTransaction() {
        if (m_active) {
            rollback();
        }
    }
    bool active() const {
        return m_active;
    }
    bool transaction() {
        if (m_active) {
            qDebug() << "WARNING: Transaction already active and received transaction() request on"
                     << m_database.connectionName();
            return false;
        }
        m_active = m_database.transaction();
        return m_active;
    }
    bool commit() {
        if (!m_active) {
            qDebug() << "WARNING: commit() called on inactive transaction for"
                     << m_database.connectionName();
            return false;
        }
        bool result = m_database.commit();
        qDebug() << "Committing transaction on"
                 << m_database.connectionName()
                 << "result:" << result;
        m_active = false;
        return result;
    }
    bool rollback() {
        if (!m_active) {
            qDebug() << "WARNING: rollback() called on inactive transaction for"
                     << m_database.connectionName();
            return false;
        }
        bool result = m_database.rollback();
        qDebug() << "Rolling back transaction on"
                 << m_database.connectionName()
                 << "result:" << result;
        m_active = false;
        return result;
    }
  private:
    QSqlDatabase m_database;
    bool m_active;
};

class FieldEscaper final {
  public:
    FieldEscaper(const QSqlDatabase& database)
            : m_database(database),
              m_stringField("string", QVariant::String) {
    }

    // Escapes a string for use in a SQL query by wrapping with quotes and
    // escaping embedded quote characters.
    QString escapeString(const QString& escapeString) const {
        m_stringField.setValue(escapeString);
        return m_database.driver()->formatValue(m_stringField);
    }

    QStringList escapeStrings(const QStringList& escapeStrings) const {
        QStringList result = escapeStrings;
        escapeStringsInPlace(&result);
        return result;
    }

  private:
    void escapeStringsInPlace(QStringList* pEscapeStrings) const {
        QMutableStringListIterator it(*pEscapeStrings);
        while (it.hasNext()) {
            it.setValue(escapeString(it.next()));
        }
    }

    QSqlDatabase m_database;
    mutable QSqlField m_stringField;
};

#endif /* QUERYUTIL_H */
