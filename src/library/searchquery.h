#ifndef SEARCHQUERY_H
#define SEARCHQUERY_H

#include <QList>
#include <QSqlDatabase>
#include <QRegExp>
#include <QString>
#include <QStringList>

#include "trackinfoobject.h"
#include "proto/keys.pb.h"

QVariant getTrackValueForColumn(const TrackPointer& pTrack, const QString& column);

class QueryNode {
  public:
    QueryNode() {}
    virtual ~QueryNode() {}

    virtual bool match(const TrackPointer& pTrack) const = 0;
    virtual QString toSql() const = 0;
};

class GroupNode : public QueryNode {
  public:
    GroupNode() {}
    virtual ~GroupNode() {
        while (!m_nodes.empty()) {
            delete m_nodes.takeLast();
        }
    }

    void addNode(QueryNode* pNode) {
        m_nodes.append(pNode);
    }

  protected:
    QList<QueryNode*> m_nodes;
};

class OrNode : public GroupNode {
  public:
    OrNode() {}

    bool match(const TrackPointer& pTrack) const;
    QString toSql() const;
};

class AndNode : public GroupNode {
  public:
    AndNode() {}

    bool match(const TrackPointer& pTrack) const;
    QString toSql() const;
};

class NotNode : public QueryNode {
  public:
    explicit NotNode(QueryNode* pNode);

    bool match(const TrackPointer& pTrack) const;
    QString toSql() const;

  private:
    QueryNode* m_pNode;
};

class TextFilterNode : public QueryNode {
  public:
    TextFilterNode(const QSqlDatabase& database,
                   const QStringList& sqlColumns,
                   const QString& argument)
            : m_database(database),
              m_sqlColumns(sqlColumns),
              m_argument(argument) {
    }

    bool match(const TrackPointer& pTrack) const;
    QString toSql() const;

  private:
    QSqlDatabase m_database;
    QStringList m_sqlColumns;
    QString m_argument;
};

class NumericFilterNode : public QueryNode {
  public:
    NumericFilterNode(const QStringList& sqlColumns, QString argument);
    NumericFilterNode(const QStringList& sqlColumns);
    bool match(const TrackPointer& pTrack) const;
    QString toSql() const;

  protected:
    virtual void init(QString argument);
    virtual double parse(const QString& arg, bool *ok);
    QStringList m_sqlColumns;
    bool m_bOperatorQuery;
    QString m_operator;
    double m_dOperatorArgument;
    bool m_bRangeQuery;
    double m_dRangeLow;
    double m_dRangeHigh;
};

class DurationFilterNode : public NumericFilterNode {
  public:
    DurationFilterNode(const QStringList& sqlColumns, QString argument);

  private:
    virtual double parse(const QString& arg, bool* ok);
};

class KeyFilterNode : public QueryNode {
  public:
    KeyFilterNode(mixxx::track::io::key::ChromaticKey key, bool fuzzy);

    bool match(const TrackPointer& pTrack) const;
    QString toSql() const;

  private:
    QList<mixxx::track::io::key::ChromaticKey> m_matchKeys;
};

class SqlNode : public QueryNode {
  public:
    explicit SqlNode(const QString& sqlExpression)
            // Need to wrap it since we don't know if the caller wrapped it.
            : m_sql(QString("(%1)").arg(sqlExpression)) {
    }
    virtual ~SqlNode() {}

    bool match(const TrackPointer& pTrack) const {
        // We are usually embedded in an AND node so if we don't match
        // everything then we block everything.
        Q_UNUSED(pTrack);
        return true;
    }

    QString toSql() const {
        return m_sql;
    }

  private:
    QString m_sql;
};


#endif /* SEARCHQUERY_H */
