#ifndef LEGACY_LIBRARY_IMPORTER_H
#define LEGACY_LIBRARY_IMPORTER_H

#include "library/dao/trackdao.h"

/** Upgrades your library from 1.7 to the current DB format. */
class LegacyLibraryImporter
{
    public: 
        LegacyLibraryImporter(TrackDAO& trackDao);
        ~LegacyLibraryImporter();
};

#endif //LEGACY_LIBRARY_IMPORTER_H
