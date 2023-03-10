#pragma once

#include <memory>
#include <optional>

#include "library/itunes/itunesutils.h"
#include "library/treeitem.h"

struct ITunesImport {
    std::unique_ptr<TreeItem> playlistRoot;
    bool isMusicFolderLocatedAfterTracks;
};

class ITunesImporter {
  public:
    virtual ~ITunesImporter() = default;

    virtual ITunesImport importLibrary() = 0;
};
