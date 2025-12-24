#pragma once

#include <memory>

class TreeItem;
class ITunesFeature;

struct ITunesImport {
    std::unique_ptr<TreeItem> playlistRoot;
};

class ITunesImporter {
  public:
    ITunesImporter(ITunesFeature* pParentFeature);

    virtual ~ITunesImporter() = default;

    virtual ITunesImport importLibrary() = 0;

    bool canceled() const;

  protected:
    // This is a borrowed pointer. The ITunesFeature owns the ITunesImporter.
    ITunesFeature* m_pParentFeature;
};
