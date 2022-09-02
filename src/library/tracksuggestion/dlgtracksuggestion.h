#pragma once

#include "controllers/keyboard/keyboardeventfilter.h"
#include "library/baseexternalplaylistmodel.h"
#include "library/library.h"
#include "library/libraryview.h"
#include "library/proxytrackmodel.h"
#include "library/recording/ui_dlgtracksuggestion.h"
#include "library/trackcollection.h"
#include "library/tracksuggestion/suggestionfetcher.h"
#include "preferences/usersettings.h"
#include "track/track_decl.h"

class WLibrary;
class WTrackTableView;
class PlaylistTableModel;

//This class is just a placeholder for now to show fetching process.
//Basically taken from recording feature.
//In the future it can have its own view.
class DlgTrackSuggestion : public QWidget,
                           public Ui::DlgTrackSuggestion,
                           public virtual LibraryView {
    Q_OBJECT
  public:
    DlgTrackSuggestion(WLibrary* parent,
            UserSettingsPointer pConfig,
            Library* pLibrary,
            KeyboardEventFilter* pKeyboard,
            SuggestionFetcher* pSuggestionFetcher,
            BaseExternalPlaylistModel* pSuggestionTableModel);
    ~DlgTrackSuggestion() override;

    void onSearch(const QString& text) override{};
    void onShow() override{};
    bool hasFocus() const override;
    void setFocus() override{};
    void activateSelectedTrack() override{};
    void loadSelectedTrackToGroup(const QString& group, bool play) override;
    void moveSelection(int delta) override{};

  private slots:
    void slotStartFetching();
    void slotChangeButtonLabel(const QString& buttonLabel);
    void slotShowFetchingProgress(const QString& message);
    void slotNetworkError(int httpStatus, const QString& app, const QString& message, int code);

  signals:
    void buttonPressed();
    void suggestionFileWrittenSuccessfully(const QString& filePath);

  private:
    UserSettingsPointer m_pConfig;
    WTrackTableView* m_pTrackTableView;
    void refreshLabels();
};
