#pragma once

#include <QAbstractListModel>
#include <QQmlEngine>
#include <QString>
#include <QVariantList>
#include <QVector>

#include "preferences/usersettings.h"

namespace mixxx {
namespace qml {

/// Model of the user's recent library searches, persisted in the user
/// settings ([SearchQueries]). Owns the whole load/parse/serialize cycle so
/// that QML only reads roles and calls persist().
class QmlRecentSearchModel : public QAbstractListModel {
    Q_OBJECT
    QML_NAMED_ELEMENT(RecentSearchModel)
    QML_UNCREATABLE("Only accessible via Mixxx.Library.recentSearches")

  public:
    enum Roles {
        TokensRole = Qt::UserRole + 1,
        FreeTextRole,
        QueryStringRole,
    };
    Q_ENUM(Roles);

    explicit QmlRecentSearchModel(
            UserSettingsPointer pConfig, QObject* parent = nullptr);
    ~QmlRecentSearchModel() override = default;

    /// Stores the given criteria as a recent search, replacing the entry at
    /// activeRow if that is a valid row and inserting at the front otherwise.
    /// Returns the index of the stored entry or -1 if there was nothing to
    /// store.
    Q_INVOKABLE int persist(const QVariantList& tokens,
            const QString& freeText,
            int activeRow);
    Q_INVOKABLE QVariantMap get(int row) const;

    QVariant data(const QModelIndex& index, int role) const override;
    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    QHash<int, QByteArray> roleNames() const override;

  private:
    struct RecentSearch {
        QVariantList tokens;
        QString freeText;
        QString queryString;
    };

    void saveQueriesToConfig() const;

    UserSettingsPointer m_pConfig;
    QVector<RecentSearch> m_searches;
};

} // namespace qml
} // namespace mixxx
