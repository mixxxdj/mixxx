#pragma once
#include <QObject>
#include <QQmlEngine>

#include "library/dao/playlistdao.h"
#include "library/trackset/crate/crate.h"
#include "library/trackset/crate/cratestorage.h"
#include "library/trackset/crate/cratesummary.h"
#include "qmllibrarytracklistmodel.h"
#include "qmltrackproxy.h"

class Library;
class TrackCollection;

namespace mixxx {
namespace qml {

class QmlCrateProxy : public QObject {
    Q_OBJECT
    Q_PROPERTY(QString name READ name WRITE setName NOTIFY nameChanged)
    Q_PROPERTY(QmlLibraryTrackListModel* model READ model CONSTANT)
    Q_PROPERTY(bool locked READ isLocked WRITE setLocked NOTIFY lockChanged)
    QML_NAMED_ELEMENT(Crate)
    QML_UNCREATABLE("")

  public:
    explicit QmlCrateProxy(QObject* parent,
            TrackCollection* trackCollection,
            const CrateSummary& crate);

    Q_INVOKABLE void addTrack(mixxx::qml::QmlTrackProxy* track);
    Q_INVOKABLE void removeTrack(mixxx::qml::QmlTrackProxy* track);

    QString name() const;
    void setName(const QString& value);

    bool isLocked() const;
    void setLocked(bool lock);

    Q_INVOKABLE uint trackCount() const;

    QmlLibraryTrackListModel* model() const;
  signals:
    void nameChanged();
    void lockChanged();

  private:
    TrackCollection* m_trackCollection;
    CrateSummary m_internal;
};

} // namespace qml
} // namespace mixxx
