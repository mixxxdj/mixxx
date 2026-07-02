#pragma once

#include <QWidget>

#include "library/browse/browsetablemodel.h"
#include "library/libraryview.h"
#include "library/proxytrackmodel.h"
#include "preferences/usersettings.h"
#include "track/track_decl.h"
#ifdef __STEM__
#include "engine/engine.h"
#endif

class WLibrary;
class WTrackTableView;
class Library;
class KeyboardEventFilter;
class RecordingManager;

class DlgSamples : public QWidget, public virtual LibraryView {
    Q_OBJECT
  public:
    DlgSamples(WLibrary* parent,
            UserSettingsPointer pConfig,
            Library* pLibrary,
            KeyboardEventFilter* pKeyboard);
    ~DlgSamples() override;

    void onSearch(const QString& text) override;
    void onShow() override {
    }
    bool hasFocus() const override;
    void setFocus() override;
    inline const QString currentSearch() {
        return m_proxyModel.currentSearch();
    }
    void saveCurrentViewState() override;
    bool restoreCurrentViewState() override;

  public slots:
    void refreshBrowseModel();
    void slotRestoreSearch();

  signals:
    void loadTrack(TrackPointer tio);
#ifdef __STEM__
    void loadTrackToPlayer(TrackPointer tio,
            const QString& group,
            mixxx::StemChannelSelection stemMask,
            bool);
#else
    void loadTrackToPlayer(TrackPointer tio,
            const QString& group,
            bool);
#endif
    void restoreSearch(const QString& search);
    void restoreModelState();

  private:
    UserSettingsPointer m_pConfig;
    WTrackTableView* m_pTrackTableView;
    BrowseTableModel m_browseModel;
    ProxyTrackModel m_proxyModel;
};
