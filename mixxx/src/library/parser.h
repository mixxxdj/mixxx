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
#include <QString>

#include "util/fileinfo.h"

class Parser {
  public:
    static bool isPlaylistFilenameSupported(const QString& playlistFile);
    static QList<QString> parseAllLocations(const QString& playlistFile);
    static QList<QString> parse(const QString& playlistFile);

  protected:
    // check for Utf8 encoding
    static bool isUtf8(const char* string);
    // Resolve an absolute or relative file path
    static mixxx::FileInfo playlistEntryToFileInfo(
            const QString& playlistEntry,
            const QString& basePath = QString());
};
