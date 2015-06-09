#ifndef SEARCHQUERY_H
#define SEARCHQUERY_H

#include <memory>
#include <vector>

#include <QList>
#include <QSqlDatabase>
#include <QRegExp>
#include <QString>
#include <QStringList>

#include "trackinfoobject.h"
#include "proto/keys.pb.h"
#include "util/assert.h"

QVariant getTrackValueForColumn(const TrackPointer& pTrack, const QString& column);

class QueryNode {
  public:
    QueryNode(const QueryNode&) = delete; // prevent copying
    virtual ~QueryNode() {}

    virtual bool match(const TrackPointer& pTrack) const = 0;
    virtual QString toSql() const = 0;

  protected:
    QueryNode() {}
};

class GroupNode : public QueryNode {
  public:
    void addNode(std::unique_ptr<QueryNode> pNode) {
        DEBUG_ASSERT(pNode);
        m_nodes.push_back(std::move(pNode));
    }

  protected:
    // NOTE(uklotzde): std::vector is more suitable (efficiency)
    // than a QList for a private member. And QList from Qt 4
    // does not support std::unique_ptr yet.
    std::vector<std::unique_ptr<QueryNode>> m_nodes;
};

class OrNode : public GroupNode {
  public:
    bool match(const TrackPointer& pTrack) const;
    QString toSql() const;
};

class AndNode : public GroupNode {
  public:
    bool match(const TrackPointer& pTrack) const;
    QString toSql() const;
};

class NotNode : public QueryNode {
  public:
    explicit NotNode(std::unique_ptr<QueryNode> pNode)
        : m_pNode(std::move(pNode)) {
        DEBUG_ASSERT(m_pNode);
    }

    bool match(const TrackPointer& pTrack) const;
    QString toSql() const;

  private:
    std::unique_ptr<QueryNode> m_pNode;
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
