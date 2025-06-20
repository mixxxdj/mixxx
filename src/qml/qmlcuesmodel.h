#pragma once
#include <QAbstractListModel>
#include <memory>

class CuePointer;

namespace mixxx {
namespace qml {

class QmlCuesModel : public QAbstractListModel {
    Q_OBJECT
  public:
    enum Roles {
        StartPositionRole = Qt::UserRole + 1,
        EndPositionRole,
        LabelRole,
        IsLoopRole,
        HotcueNumberRole,
    };
    Q_ENUM(Roles)

    explicit QmlCuesModel(QObject* pParent = nullptr);

    void setCues(QList<CuePointer> cues);

    QVariant data(const QModelIndex& index, int role) const override;
    int rowCount(const QModelIndex& parent) const override;
    QHash<int, QByteArray> roleNames() const override;
    Q_INVOKABLE QVariant get(int row) const;

  private:
    QList<CuePointer> m_cues;
};

} // namespace qml
} // namespace mixxx
