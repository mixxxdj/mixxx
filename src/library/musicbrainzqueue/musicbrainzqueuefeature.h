#pragma once

#include <QObject>
#include <QVariant>

#include "library/libraryfeature.h"
#include "library/treeitemmodel.h"
#include "preferences/usersettings.h"
#include "util/parented_ptr.h"

class DlgMusicBrainzQueue;
class KeyboardEventFilter;
class Library;
class WLibrary;

class MusicBrainzQueueFeature : public LibraryFeature {
    Q_OBJECT

  public:
    MusicBrainzQueueFeature(
            Library* pLibrary,
            UserSettingsPointer pConfig);
    ~MusicBrainzQueueFeature() override = default;

    QVariant title() override;

    void bindLibraryWidget(
            WLibrary* libraryWidget,
            KeyboardEventFilter* keyboard) override;

    TreeItemModel* sidebarModel() const override;

  public slots:
    void activate() override;

  private:
    static const QString kViewName;

    parented_ptr<TreeItemModel> m_pSidebarModel;

    // Raw pointer: owned by the WLibrary widget (its parent).
    DlgMusicBrainzQueue* m_pView;
};
