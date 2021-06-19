//
// C++ Interface: parser
//
// Description: Interface header for the parser Parser
//
//
// Author: Ingo Kossyk <kossyki@cs.tu-berlin.de>, (C) 2004
// Author: Tobias Rafreider trafreider@mixxx.org, (C) 2011
//
// Copyright: See COPYING file that comes with this distribution
//
//

#pragma once

/**Developer Information:
This is the rootclass for all parser classes for the Importer class.
It can be used to write a new type-specific parser by deriving a new class
from it and overwrite the parse function and add class specific functions to
it afterwards for proper functioning
**/

#include <QList>
#include <QObject>
#include <QString>

#include "util/fileinfo.h"

class Parser : public QObject {
  public:
    static bool isPlaylistFilenameSupported(const QString& fileName) {
        return fileName.endsWith(".m3u", Qt::CaseInsensitive) ||
                fileName.endsWith(".m3u8", Qt::CaseInsensitive) ||
                fileName.endsWith(".pls", Qt::CaseInsensitive) ||
                fileName.endsWith(".csv", Qt::CaseInsensitive);
    }

    Parser();
    ~Parser() override;
    /**Can be called to parse a pls file
    Note for developers:
    This function should return an empty PtrList
     or 0 in order for the trackimporter to function**/
    virtual QList<QString> parse(const QString&) = 0;

  protected:
    // Pointer to the parsed Filelocations
    QList<QString> m_sLocations;
    // Returns the number of parsed locations
    long countParsed();
    // Clears m_psLocations
    void clearLocations();
    // check for Utf8 encoding
    static bool isUtf8(const char* string);
    // Resolve an absolute or relative file path
    mixxx::FileInfo playlistEntryToFileInfo(
            const QString& playlistEntry,
            const QString& basePath = QString());
};
