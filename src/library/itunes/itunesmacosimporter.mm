#include "library/itunes/itunesmacosimporter.h"

#include <QString>
#include "library/itunes/itunesimporter.h"

ITunesMacOSImporter::ITunesMacOSImporter(QString iTunesFile)
    : m_iTunesFile(iTunesFile) {}

ITunesImport ITunesMacOSImporter::importLibrary() {
    ITunesImport iTunesImport;
    iTunesImport.isMusicFolderLocatedAfterTracks = false;

    // TODO

    return iTunesImport;
}
