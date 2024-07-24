#pragma once
#include <QAbstractListModel>
#include <memory>

#include "track/beats.h"

namespace mixxx {
namespace qml {

class QmlBeatsModel : public QAbstractListModel {
    Q_OBJECT
  public:
    enum Roles {
        FramePositionRole = Qt::UserRole + 1,
    };
    Q_ENUM(Roles)

    explicit QmlBeatsModel(QObject* parent = nullptr);

    void setBeats(const BeatsPointer pBeats, audio::FramePos trackEndPosition);

    QVariant data(const QModelIndex& index, int role) const override;
    int rowCount(const QModelIndex& parent) const override;
    QHash<int, QByteArray> roleNames() const override;
    Q_INVOKABLE QVariant get(int row) const;

  private:
    BeatsPointer m_pBeats;
    int m_numBeats;
};

} // namespace qml
} // namespace mixxx
