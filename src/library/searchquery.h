#ifndef SEARCHQUERY_H
#define SEARCHQUERY_H

#include <QList>
#include <QSqlDatabase>
#include <QString>
#include <QStringList>
#include <memory>
#include <utility>
#include <vector>

#include "proto/keys.pb.h"
#include "track/track_decl.h"
#include "util/assert.h"

class CrateStorage;
class GenreStorage;
class TrackId;

const QString kMissingFieldSearchTerm = "\"\""; // "" searches for an empty string

enum class StringMatch {
    Contains = 0,
    Equals,
};

class QueryNode {
  public:
    QueryNode(const QueryNode&) = delete; // prevent copying
    virtual ~QueryNode() = default;

    virtual bool match(const TrackPointer& pTrack) const = 0;
    virtual QString toSql() const = 0;

  protected:
    QueryNode() = default;
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
            const QString& argument,
            const StringMatch matchMode = StringMatch::Contains);

    bool match(const TrackPointer& pTrack) const override;
    QString toSql() const override;

  private:
    QSqlDatabase m_database;
    QStringList m_sqlColumns;
    QString m_argument;
    StringMatch m_matchMode;
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

class GenreFilterNode : public QueryNode {
  public:
    GenreFilterNode(const GenreStorage* pGenreStorage,
            const QString& genreNameLike);

    bool match(const TrackPointer& pTrack) const override;
    QString toSql() const override;

  private:
    const GenreStorage* m_pGenreStorage;
    QString m_genreNameLike;
    mutable bool m_genreMatchInitialized;
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

class NoGenreFilterNode : public QueryNode {
  public:
    explicit NoGenreFilterNode(const GenreStorage* pGenreStorage);

    bool match(const TrackPointer& pTrack) const override;
    QString toSql() const override;

  private:
    const GenreStorage* m_pGenreStorage;
    QString m_genreNameLike;
    mutable bool m_genreMatchInitialized;
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

    virtual double parse(const QString& arg, bool* ok);

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

// BPM filter that supports fuzzy matching via ~ prefix.
// If no operator is provided (bpm:123) it also finds half & double BPM matches.
// Half/double values aren't integers, int ranges are used. E.g. bpm:123.1 finds
// 61-61, 123.1 and 246-247 BPM
class BpmFilterNode : public QueryNode {
  public:
    static constexpr double kRelativeRangeDefault = 0.06;
    static void setBpmRelativeRange(double range);

    BpmFilterNode(QString& argument, bool fuzzy, bool negate = false);

    enum class MatchMode {
        Invalid,
        Null,              // bpm:- | bpm:000.0 | bpm:0,0 | bpm:""
        Explicit,          // bpm:=120
        ExplicitStrict,    // bpm:=120.0
        Fuzzy,             // ~bpm:120
        Range,             // bpm:120-130
        HalveDouble,       // bpm:120
        HalveDoubleStrict, // bpm:120.0
        Operator,          // bpm:<=120
        Locked,            // bpm:locked
    };

    // Allows WSearchRelatedTracksMenu to construct the QAction title
    std::pair<double, double> getBpmRange() const {
        return std::pair<double, double>(m_rangeLower, m_rangeUpper);
    }

    QString toSql() const override;

  private:
    bool match(const TrackPointer& pTrack) const override;

    MatchMode m_matchMode;

    QString m_operator;

    double m_bpm;
    double m_rangeLower;
    double m_rangeUpper;
    double m_bpmHalfLower;
    double m_bpmHalfUpper;
    double m_bpmDoubleLower;
    double m_bpmDoubleUpper;

    static double s_relativeRange;
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

class YearFilterNode : public NumericFilterNode {
  public:
    YearFilterNode(const QStringList& sqlColumns, const QString& argument);
    QString toSql() const override;
};

#endif /* SEARCHQUERY_H */
