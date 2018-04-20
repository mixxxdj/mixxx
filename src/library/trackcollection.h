#ifndef TRACKCOLLECTION_H
#define TRACKCOLLECTION_H

#include <QList>
#include <QSharedPointer>
#include <QSqlDatabase>

#include "preferences/usersettings.h"
#include "library/basetrackcache.h"
#include "library/features/crates/cratemanager.h"
#include "library/dao/trackdao.h"
#include "library/dao/cuedao.h"
#include "library/dao/playlistdao.h"
#include "library/dao/analysisdao.h"
#include "library/dao/directorydao.h"
#include "library/dao/libraryhashdao.h"
#include "library/dao/savedqueriesdao.h"

// forward declaration(s)
class Track;

// Manages everything around tracks.
class TrackCollection : public QObject,
    public virtual /*implements*/ SqlStorage {
    Q_OBJECT

  public:
    explicit TrackCollection(
            const UserSettingsPointer& pConfig);
    ~TrackCollection() override;

    void repairDatabase(
            QSqlDatabase database) override;

    void connectDatabase(
            QSqlDatabase database) override;
    void disconnectDatabase() override;

    QSqlDatabase database() const {
        return m_database;
    }

    CrateManager* crates() {
        return &m_crates;
    }

    TrackDAO& getTrackDAO() {
        return m_trackDao;
    }
    PlaylistDAO& getPlaylistDAO() {
        return m_playlistDao;
    }
    DirectoryDAO& getDirectoryDAO() {
        return m_directoryDao;
    }
    AnalysisDao& getAnalysisDAO() {
        return m_analysisDao;
    }
    SavedQueriesDAO& getSavedQueriesDAO() {
        return m_savedDao;
    }

    QSharedPointer<BaseTrackCache> getTrackSource() const {
        return m_pTrackSource;
    }
    void setTrackSource(QSharedPointer<BaseTrackCache> pTrackSource);

    void cancelLibraryScan();

    void relocateDirectory(QString oldDir, QString newDir);

    bool hideTracks(const QList<TrackId>& trackIds);
    bool unhideTracks(const QList<TrackId>& trackIds);

    bool purgeTracks(const QList<TrackId>& trackIds);
    bool purgeTracks(const QDir& dir);

    // Might be called from any thread
    void exportTrackMetadata(Track* pTrack) const;

    // Must be called from the main thread
    void saveTrack(Track* pTrack);
  private:
    UserSettingsPointer m_pConfig;

    QSqlDatabase m_database;

    PlaylistDAO m_playlistDao;
    CrateManager m_crates;
    CueDAO m_cueDao;
    DirectoryDAO m_directoryDao;
    AnalysisDao m_analysisDao;
    LibraryHashDAO m_libraryHashDao;
    SavedQueriesDAO m_savedDao;
    TrackDAO m_trackDao;

    QSharedPointer<BaseTrackCache> m_pTrackSource;
};

#endif // TRACKCOLLECTION_H
