#pragma once
#include <QAbstractListModel>
#include <memory>

#include "track/steminfo.h"

namespace mixxx {
namespace qml {

class QmlStemsModel : public QAbstractListModel {
    Q_OBJECT
  public:
    enum Roles {
        LabelRole,
        ColorRole,
    };
    Q_ENUM(Roles)
    explicit QmlStemsModel(QObject* pParent = nullptr);

    void setStems(QList<StemInfo> stems);

    QVariant data(const QModelIndex& index, int role) const override;
    int rowCount(const QModelIndex& parent) const override;
    QHash<int, QByteArray> roleNames() const override;
    Q_INVOKABLE QVariant get(int row) const;

  private:
    QList<StemInfo> m_stems;
};

} // namespace qml
} // namespace mixxx
