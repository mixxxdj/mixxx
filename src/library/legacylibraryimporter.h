#ifndef LEGACY_LIBRARY_IMPORTER_H
#define LEGACY_LIBRARY_IMPORTER_H

#include "library/dao/trackdao.h"
#include "library/dao/playlistdao.h"

class TrackCollection;

// Upgrades your library from 1.7 to the current DB format.
class LegacyLibraryImporter : public QObject {
    Q_OBJECT
    public: 
        LegacyLibraryImporter(TrackCollection* pTrackCollection, TrackDAO& trackDao, PlaylistDAO& playlistDao);
        ~LegacyLibraryImporter();
        void import();
    signals:
        void progress(QString);
    private:
        TrackDAO& m_trackDao;
        PlaylistDAO& m_playlistDao;
        TrackCollection* m_pTrackCollection;
};

#endif //LEGACY_LIBRARY_IMPORTER_H
