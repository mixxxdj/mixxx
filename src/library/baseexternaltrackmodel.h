#ifndef BASEEXTERNALTRACKMODEL_H
#define BASEEXTERNALTRACKMODEL_H

#include <QModelIndex>
#include <QObject>
#include <QString>

#include "library/trackmodel.h"
#include "library/basesqltablemodel.h"
#include "trackinfoobject.h"

class TrackCollection;

class BaseExternalTrackModel : public BaseSqlTableModel {
    Q_OBJECT
  public:
    BaseExternalTrackModel(QObject* parent,
                           TrackCollection* pTrackCollection,
                           const char* settingsNamespace,
                           const QString& trackTable,
                           QSharedPointer<BaseTrackCache> trackSource);
    virtual ~BaseExternalTrackModel();

    virtual TrackModel::CapabilitiesFlags getCapabilities() const;
    TrackPointer getTrack(const QModelIndex& index) const;
    virtual bool isColumnInternal(int column);
    Qt::ItemFlags flags(const QModelIndex &index) const;

  private:
    QString m_trackTable;
    QString m_trackSource;
};

#endif /* BASEEXTERNALTRACKMODEL_H */
