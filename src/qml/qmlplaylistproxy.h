#pragma once
#include <QObject>
#include <QQmlEngine>

#include "library/dao/playlistdao.h"
#include "qmllibrarytracklistmodel.h"
#include "qmltrackproxy.h"

class Library;

namespace mixxx {
namespace qml {

class QmlPlaylistProxy : public QObject {
    Q_OBJECT
    Q_PROPERTY(QString name MEMBER m_name NOTIFY nameChanged)
    Q_PROPERTY(mixxx::qml::QmlLibraryTrackListModel* model READ model CONSTANT)
    Q_PROPERTY(bool locked READ isLocked WRITE setLocked NOTIFY lockChanged)
    QML_NAMED_ELEMENT(Playlist)
    QML_UNCREATABLE("Only accessible via a LibraryPlaylistSource")

  public:
    explicit QmlPlaylistProxy(QObject* parent, PlaylistDAO& dao, int pid, const QString& name);

    Q_INVOKABLE void addTrack(mixxx::qml::QmlTrackProxy* track);

    bool isLocked() const {
        return m_dao.isPlaylistLocked(m_id);
    }

    void setLocked(bool lock) {
        m_dao.setPlaylistLocked(m_id, lock);
        emit lockChanged();
    }

    QmlLibraryTrackListModel* model() const;
  signals:
    void nameChanged();
    void lockChanged();

  private:
    QString m_name;
    int m_id;
    PlaylistDAO& m_dao;
};

} // namespace qml
} // namespace mixxx
