#pragma once

#include <QAbstractListModel>
#include <QColor>
#include <QQmlEngine>
#include <QString>
#include <QVector>

#include "track/keys.h"
#include "util/db/dbconnectionpool.h"

namespace mixxx {
namespace qml {

class QmlSearchSuggestionModel : public QAbstractListModel {
    Q_OBJECT
    QML_NAMED_ELEMENT(SearchSuggestionModel)
    QML_UNCREATABLE("Only accessible via Mixxx.Library.searchSuggestions")

  public:
    enum class SearchField {
        Invalid,
        Artist,
        Album,
        Title,
        Genre,
        Composer,
        Comment,
        Year,
        BPM,
        Key,
        Track,
    };
    Q_ENUM(SearchField)

    enum Roles {
        ValueRole = Qt::UserRole + 1,
        LabelRole,
        KeyIdRole,
        KeyColorRole,
    };
    Q_ENUM(Roles);

    explicit QmlSearchSuggestionModel(
            mixxx::DbConnectionPoolPtr pDbConnectionPool,
            QObject* parent = nullptr);
    ~QmlSearchSuggestionModel() override = default;

    Q_INVOKABLE void setQuery(const QString& field, const QString& prefix);
    Q_INVOKABLE QVariant get(int row) const;

    QVariant data(const QModelIndex& index, int role) const override;
    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    QHash<int, QByteArray> roleNames() const override;

  private:
    struct Suggestion {
        QString value;
        QString label;
        int keyId = mixxx::track::io::key::INVALID;
        QColor keyColor;
    };

    void resetSuggestions();
    void setKeySuggestions(const QString& prefix);
    void scheduleSuggestionsQuery(SearchField field, const QString& prefix);
    void applySuggestions(QVector<Suggestion> suggestions);

    static QVector<Suggestion> querySuggestions(
            const mixxx::DbConnectionPoolPtr& pDbConnectionPool,
            SearchField field,
            const QString& prefix);
    static QVector<Suggestion> queryValueSuggestions(
            const mixxx::DbConnectionPoolPtr& pDbConnectionPool,
            SearchField field,
            const QString& prefix);
    static QVector<Suggestion> queryTrackSuggestions(
            const mixxx::DbConnectionPoolPtr& pDbConnectionPool,
            const QString& prefix);
    static QVector<Suggestion> runSuggestionsQuery(
            const mixxx::DbConnectionPoolPtr& pDbConnectionPool,
            const QString& sql,
            const QString& prefix);

    mixxx::DbConnectionPoolPtr m_pDbConnectionPool;
    QVector<Suggestion> m_suggestions;
    quint64 m_requestId = 0;
};

} // namespace qml
} // namespace mixxx
