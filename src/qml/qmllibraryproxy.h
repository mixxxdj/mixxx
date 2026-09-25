#pragma once
#include <QObject>
#include <QQmlEngine>
#include <QString>
#include <memory>

#include "qml/qmllibrarytracklistmodel.h"
#include "track/track_decl.h"
#include "util/parented_ptr.h"

class Library;

namespace mixxx {
namespace qml {

class QmlLibraryTrackListModel;

class QmlLibraryProxy : public QObject {
    Q_OBJECT
    Q_PROPERTY(mixxx::qml::QmlLibraryTrackListModel* model READ model CONSTANT)
    Q_PROPERTY(QString selectedTitle READ selectedTitle NOTIFY selectedTrackChanged)
    Q_PROPERTY(QString selectedArtist READ selectedArtist NOTIFY selectedTrackChanged)
    Q_PROPERTY(QString selectedAlbum READ selectedAlbum NOTIFY selectedTrackChanged)
    QML_NAMED_ELEMENT(Library)
    QML_SINGLETON

  public:
    explicit QmlLibraryProxy(std::shared_ptr<Library> pLibrary, QObject* parent = nullptr);

    QmlLibraryTrackListModel* model();
    const QString& selectedTitle() const {
        return m_selectedTitle;
    }
    const QString& selectedArtist() const {
        return m_selectedArtist;
    }
    const QString& selectedAlbum() const {
        return m_selectedAlbum;
    }

    static QmlLibraryProxy* create(QQmlEngine* pQmlEngine, QJSEngine* pJsEngine);
    static void registerLibrary(std::shared_ptr<Library> pLibrary) {
        s_pLibrary = std::move(pLibrary);
    }

  signals:
    void selectedTrackChanged();

  private slots:
    void slotTrackSelected(TrackPointer pTrack);

  private:
    static inline std::shared_ptr<Library> s_pLibrary;

    std::shared_ptr<Library> m_pLibrary;

    QmlLibraryTrackListModel* m_pModelProperty{};
    QString m_selectedTitle;
    QString m_selectedArtist;
    QString m_selectedAlbum;
};

} // namespace qml
} // namespace mixxx
