#ifndef SEARCHQUERY_H
#define SEARCHQUERY_H

#include <QList>
#include <QRegExp>
#include <QSqlDatabase>
#include <QString>
#include <QStringList>
#include <utility>
#include <vector>

#include "library/crate/cratestorage.h"
#include "proto/keys.pb.h"
#include "track/track_decl.h"
#include "util/assert.h"
#include "util/memory.h"

const QString kMissingFieldSearchTerm = "\"\""; // "" searches for an empty string

QVariant getTrackValueForColumn(const TrackPointer& pTrack, const QString& column);

class QueryNode {
  public:
    QueryNode(const QueryNode&) = delete; // prevent copying
    virtual ~QueryNode() {}

    virtual bool match(const TrackPointer& pTrack) const = 0;
    virtual QString toSql() const = 0;

  protected:
    QueryNode() {}

    static QString concatSqlClauses(const QStringList& sqlClauses, const QString& sqlConcatOp);
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
    bool match(const TrackPointer& pTrack) const override;
    QString toSql() const override;
};

class AndNode : public GroupNode {
  public:
    bool match(const TrackPointer& pTrack) const override;
    QString toSql() const override;
};

class NotNode : public QueryNode {
  public:
    explicit NotNode(std::unique_ptr<QueryNode> pNode)
        : m_pNode(std::move(pNode)) {
        DEBUG_ASSERT(m_pNode);
    }

    bool match(const TrackPointer& pTrack) const override;
    QString toSql() const override;

  private:
    std::unique_ptr<QueryNode> m_pNode;
};

class TextFilterNode : public QueryNode {
  public:
    TextFilterNode(const QSqlDatabase& database,
                   const QStringList& sqlColumns,
                   const QString& argument);

    bool match(const TrackPointer& pTrack) const override;
    QString toSql() const override;

  private:
    QSqlDatabase m_database;
    QStringList m_sqlColumns;
    QString m_argument;
};

class NullOrEmptyTextFilterNode : public QueryNode {
  public:
    NullOrEmptyTextFilterNode(const QSqlDatabase& database,
                   const QStringList& sqlColumns)
            : m_database(database),
              m_sqlColumns(sqlColumns) {
    }

    bool match(const TrackPointer& pTrack) const override;
    QString toSql() const override;

  private:
    QSqlDatabase m_database;
    QStringList m_sqlColumns;
};


class CrateFilterNode : public QueryNode {
  public:
    CrateFilterNode(const CrateStorage* pCrateStorage,
                    const QString& crateNameLike);

    bool match(const TrackPointer& pTrack) const override;
    QString toSql() const override;

  private:
    const CrateStorage* m_pCrateStorage;
    QString m_crateNameLike;
    mutable bool m_matchInitialized;
    mutable std::vector<TrackId> m_matchingTrackIds;
};

class NoCrateFilterNode : public QueryNode {
  public:
    explicit NoCrateFilterNode(const CrateStorage* pCrateStorage);

    bool match(const TrackPointer& pTrack) const override;
    QString toSql() const override;

  private:
    const CrateStorage* m_pCrateStorage;
    QString m_crateNameLike;
    mutable bool m_matchInitialized;
    mutable std::vector<TrackId> m_matchingTrackIds;
};

class NumericFilterNode : public QueryNode {
  public:
    NumericFilterNode(const QStringList& sqlColumns, const QString& argument);

    bool match(const TrackPointer& pTrack) const override;
    QString toSql() const override;

  protected:
    // Single argument constructor for that does not call init()
    explicit NumericFilterNode(const QStringList& sqlColumns);

    // init() must always be called in the constructor of the
    // most derived class directly, because internally it calls
    // the virtual function parse() that will be overridden by
    // derived classes.
    void init(QString argument);

  private:
    virtual double parse(const QString& arg, bool *ok);

    QStringList m_sqlColumns;
    bool m_bOperatorQuery;
    bool m_bNullQuery;
    QString m_operator;
    double m_dOperatorArgument;
    bool m_bRangeQuery;
    double m_dRangeLow;
    double m_dRangeHigh;
};

class NullNumericFilterNode : public QueryNode {
  public:
    explicit NullNumericFilterNode(const QStringList& sqlColumns);

    bool match(const TrackPointer& pTrack) const override;
    QString toSql() const override;

    QStringList m_sqlColumns;
};

class DurationFilterNode : public NumericFilterNode {
  public:
    DurationFilterNode(const QStringList& sqlColumns, const QString& argument);

  private:
    double parse(const QString& arg, bool* ok) override;
};

class KeyFilterNode : public QueryNode {
  public:
    KeyFilterNode(mixxx::track::io::key::ChromaticKey key, bool fuzzy);

    bool match(const TrackPointer& pTrack) const override;
    QString toSql() const override;

  private:
    QList<mixxx::track::io::key::ChromaticKey> m_matchKeys;
};

class SqlNode : public QueryNode {
  public:
    explicit SqlNode(const QString& sqlExpression)
            // No need to wrap into parentheses here! This will be done
            // later in toSql() if this node is a component of another
            // composite node.
            : m_sql(sqlExpression) {
    }

    bool match(const TrackPointer& pTrack) const override {
        // We are usually embedded in an AND node so if we don't match
        // everything then we block everything.
        Q_UNUSED(pTrack);
        return true;
    }

    QString toSql() const override {
        return m_sql;
    }

  private:
    QString m_sql;
};


#endif /* SEARCHQUERY_H */
