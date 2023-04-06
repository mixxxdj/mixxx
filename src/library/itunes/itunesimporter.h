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

    // TODO: Add thread-safe `cancelImport` method and move `m_cancelImport`
    // from ITunesFeature to the individual importers (replacing the current
    // mechanism of passing (the private member) `m_cancelImport` by reference
    // to each importer). We should then call `cancelImport` from the
    // `ITunesFeature` destructor on the importer (which we'd need to store e.g.
    // in a member of `ITunesFeature`).
};
