#ifndef LEGACY_LIBRARY_IMPORTER_H
#define LEGACY_LIBRARY_IMPORTER_H

#include "library/dao/trackdao.h"

/** Upgrades your library from 1.7 to the current DB format. */
class LegacyLibraryImporter : public QObject
{
    Q_OBJECT
    public: 
        LegacyLibraryImporter(TrackDAO& trackDao);
        ~LegacyLibraryImporter();
        void import();
    signals:
        void progress(QString);
    private:
        TrackDAO& m_trackDao;
};

#endif //LEGACY_LIBRARY_IMPORTER_H
