#ifndef SCANNERUTIL_H
#define SCANNERUTIL_H

#include <QDir>
#include <QDesktopServices>
#include <QStringList>

// Library scanner utility methods.
class ScannerUtil {
  public:
    static QStringList getDirectoryBlacklist() {
        QStringList blacklist;
        // The "Album Artwork" folder within iTunes stores album art.  It has
        // hundreds of subfolders but no audio files. We put this folder on a
        // blacklist. On Windows, the iTunes folder is contained within the
        // standard music folder.
        QString iTunesArtFolder = QDir::toNativeSeparators(
            QDesktopServices::storageLocation(QDesktopServices::MusicLocation) +
            "/iTunes/Album Artwork");
        blacklist << iTunesArtFolder;
#ifdef __WINDOWS__
        // Blacklist the _Serato_ directory that pollutes "My Music" on Windows.
        QString seratoDir = QDir::toNativeSeparators(
            QDesktopServices::storageLocation(QDesktopServices::MusicLocation) +
            "/_Serato_");
        blacklist << seratoDir;
#endif
        return blacklist;
    }

  private:
    ScannerUtil() {}
};

#endif /* SCANNERUTIL_H */
