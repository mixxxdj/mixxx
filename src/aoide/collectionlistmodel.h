#pragma once

#include <QAbstractListModel>

#include "aoide/web/listcollectionstask.h"
#include "util/qt.h"

namespace aoide {

class Subsystem;

struct CollectionListFilter {
    QString kind;
};

inline bool operator==(const CollectionListFilter& lhs, const CollectionListFilter& rhs) {
    return lhs.kind == rhs.kind;
}

inline bool operator!=(const CollectionListFilter& lhs, const CollectionListFilter& rhs) {
    return !(lhs == rhs);
}

class CollectionListModel : public QAbstractListModel {
    Q_OBJECT

  public:
    explicit CollectionListModel(
            Subsystem* subsystem,
            QObject* parent = nullptr);
    ~CollectionListModel() override;

    Q_PROPERTY(bool pending READ isPending STORED false NOTIFY pendingChanged)
    Q_PROPERTY(CollectionListFilter filter READ filter WRITE setFilter NOTIFY filterChanged)

    const CollectionListFilter& filter() const {
        return m_filter;
    }

    bool isPending() const {
        return m_pendingTask;
    }

    ///////////////////////////////////////////////////////
    // Inherited from QAbstractItemModel
    ///////////////////////////////////////////////////////

    int rowCount(
            const QModelIndex& parent = QModelIndex()) const final;

    QVariant data(
            const QModelIndex& index,
            int role = Qt::DisplayRole) const override;

  signals:
    void pendingChanged(bool pending);
    void filterChanged(const CollectionListFilter& filter);

  public slots:
    void setFilter(
            const CollectionListFilter& filter);

    void reload();

    void abortPendingTask();

  private slots:
    void onPendingTaskSucceeded(
            const QVector<json::CollectionEntity>& rows);
    void onPendingTaskDestroyed(
            QObject* obj);

  private:
    void loadRows(const CollectionListFilter& filter);

    const mixxx::SafeQPointer<Subsystem> m_subsystem;

    CollectionListFilter m_filter;

    QVector<json::CollectionEntity> m_rows;

    CollectionListFilter m_pendingFilter;

    mixxx::SafeQPointer<ListCollectionsTask> m_pendingTask;
};

} // namespace aoide

Q_DECLARE_METATYPE(aoide::CollectionListFilter);
