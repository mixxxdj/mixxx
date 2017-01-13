#ifndef BASEEXTERNALTRACKMODEL_H
#define BASEEXTERNALTRACKMODEL_H

#include <QModelIndex>
#include <QObject>
#include <QString>

#include "library/trackmodel.h"
#include "library/basesqltablemodel.h"
#include "track/track.h"

class TrackCollection;

class BaseExternalTrackModel : public BaseSqlTableModel {
    Q_OBJECT
  public:
    BaseExternalTrackModel(QObject* parent,
                           TrackCollection* pTrackCollection,
                           const char* settingsNamespace,
                           const QString& trackTable,
                           QSharedPointer<BaseTrackCache> trackSource);
    ~BaseExternalTrackModel() override;

    CapabilitiesFlags getCapabilities() const override;
    TrackPointer getTrack(const QModelIndex& index) const override;
    void trackLoaded(QString group, TrackPointer pTrack) override;
    bool isColumnInternal(int column) override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;
};

#endif /* BASEEXTERNALTRACKMODEL_H */
