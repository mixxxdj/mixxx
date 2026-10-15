#pragma once

#include <QString>
#include <QWidget>

#include "library/autodj/autodjprocessor.h"
#include "library/autodj/ui_dlgautodj.h"
#include "library/libraryview.h"
#include "preferences/usersettings.h"
#include "track/track_decl.h"

class PlaylistTableModel;
class WLibrary;
class WTrackTableView;
class Library;
class KeyboardEventFilter;
class QBoxLayout;
class QLabel;
class QLineEdit;

class DlgAutoDJ : public QWidget, public Ui::DlgAutoDJ, public LibraryView {
    Q_OBJECT
  public:
    DlgAutoDJ(WLibrary* parent,
            UserSettingsPointer pConfig,
            Library* pLibrary,
            AutoDJProcessor* pProcessor,
            KeyboardEventFilter* pKeyboard);
    ~DlgAutoDJ() override;

    /// Enables or disables the AI set planner row. It stays disabled while the
    /// AI assistant is switched off in the preferences.
    void setAiMixAvailable(bool available);
    /// Shows a status message next to the AI instruction field.
    void setAiStatusMessage(const QString& message, bool isError);
    /// Disables the AI controls while a request is in flight.
    void setAiMixBusy(bool busy);

    void onShow() override;
    bool hasFocus() const override;
    void setFocus() override;
    void pasteFromSidebar() override;
    void onSearch(const QString& text) override;
    void saveCurrentViewState() override;
    bool restoreCurrentViewState() override;

  public slots:
    void shufflePlaylistButton(bool buttonChecked);
    void skipNextButton(bool buttonChecked);
    void fadeNowButton(bool buttonChecked);
    void toggleAutoDJButton(bool enable);
    void autoDJError(AutoDJProcessor::AutoDJError error);
    void transitionTimeChanged(int time);
    void transitionSliderChanged(int value);
    void autoDJStateChanged(AutoDJProcessor::AutoDJState state);
    void updateSelectionInfo();
    void slotTransitionModeChanged(int comboboxIndex);
    void slotRepeatPlaylistChanged(bool checked);
    void slotAiMixButtonClicked();

  signals:
    void addRandomTrackButton(bool buttonChecked);
    /// Emitted when the user asks the AI assistant to build a set list.
    void aiMixRequested(const QString& instruction);
    void loadTrack(TrackPointer tio);
#ifdef __STEM__
    void loadTrackToPlayer(TrackPointer tio,
            const QString& group,
            mixxx::StemChannelSelection stemMask,
            bool);
#else
    void loadTrackToPlayer(TrackPointer tio, const QString& group, bool);
#endif
    void trackSelected(TrackPointer pTrack);

  private:
    void setupActionButton(QPushButton* pButton,
            void (DlgAutoDJ::*pSlot)(bool),
            const QString& fallbackText);
    /// Creates the "describe the set" row and inserts it above the queue.
    void setupAiMixBar(QBoxLayout* pLayout);
    void keyPressEvent(QKeyEvent* pEvent) override;

    const UserSettingsPointer m_pConfig;

    AutoDJProcessor* const m_pAutoDJProcessor;
    WTrackTableView* const m_pTrackTableView;
    const bool m_bShowButtonText;

    PlaylistTableModel* m_pAutoDJTableModel;

    QString m_enableBtnTooltip;
    QString m_disableBtnTooltip;

    QLineEdit* m_pAiInstructionLineEdit = nullptr;
    QPushButton* m_pAiMixButton = nullptr;
    QLabel* m_pAiStatusLabel = nullptr;
};
