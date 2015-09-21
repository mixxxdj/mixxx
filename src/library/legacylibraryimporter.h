#ifndef LEGACY_LIBRARY_IMPORTER_H
#define LEGACY_LIBRARY_IMPORTER_H

#include <QDomNode>

#include "library/dao/trackdao.h"
#include "library/dao/playlistdao.h"
#include "trackinfoobject.h"

// Upgrades your library from 1.7 to the current DB format.
class LegacyLibraryImporter : public QObject {
    Q_OBJECT
    public: 
        LegacyLibraryImporter(TrackDAO& trackDao, PlaylistDAO& playlistDao);
        void import();
    signals:
        void progress(QString);
    private:
        static void importTrack(TrackInfoObject* pTrack, const QDomNode &nodeHeader);

        TrackDAO& m_trackDao;
        PlaylistDAO& m_playlistDao;
};

#endif //LEGACY_LIBRARY_IMPORTER_H
