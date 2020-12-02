#ifndef BASEEXTERNALTRACKMODEL_H
#define BASEEXTERNALTRACKMODEL_H

#include <QModelIndex>
#include <QObject>
#include <QSharedPointer>
#include <QString>
#include <QtCore>

#include "library/basesqltablemodel.h"
#include "library/trackmodel.h"
#include "track/track_decl.h"
#include "track/trackid.h"

class TrackCollection;
class BaseTrackCache;
class TrackCollectionManager;
template<class T>
class QSharedPointer;

class BaseExternalTrackModel : public BaseSqlTableModel {
    Q_OBJECT
  public:
    BaseExternalTrackModel(QObject* parent,
                           TrackCollectionManager* pTrackCollectionManager,
                           const char* settingsNamespace,
                           const QString& trackTable,
                           QSharedPointer<BaseTrackCache> trackSource);
    ~BaseExternalTrackModel() override;

    CapabilitiesFlags getCapabilities() const override;
    TrackId getTrackId(const QModelIndex& index) const override;
    TrackPointer getTrack(const QModelIndex& index) const override;
    bool isColumnInternal(int column) override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;

  private:
    TrackId doGetTrackId(const TrackPointer& pTrack) const override;
};

#endif /* BASEEXTERNALTRACKMODEL_H */
