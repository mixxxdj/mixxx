#pragma once

#include <QAbstractListModel>
#include <QQmlEngine>
#include <QString>
#include <QVector>

class TrackCollection;

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
    };
    Q_ENUM(Roles);

    explicit QmlSearchSuggestionModel(TrackCollection* pTrackCollection, QObject* parent = nullptr);
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
    };

    void setValueSuggestions(SearchField field, const QString& prefix);
    void setKeySuggestions(const QString& prefix);
    void setTrackSuggestions(const QString& prefix);
    QVector<Suggestion> runSuggestionsQuery(const QString& sql, const QString& prefix);

    TrackCollection* m_pTrackCollection;
    QVector<Suggestion> m_suggestions;
};

} // namespace qml
} // namespace mixxx
