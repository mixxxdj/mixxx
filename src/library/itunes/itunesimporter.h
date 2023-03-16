#pragma once

#include <memory>

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
