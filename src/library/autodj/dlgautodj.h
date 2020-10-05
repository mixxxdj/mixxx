#ifndef DLGAUTODJ_H
#define DLGAUTODJ_H

#include <QString>
#include <QWidget>

#include "controllers/keyboard/keyboardeventfilter.h"
#include "library/autodj/autodjprocessor.h"
#include "library/autodj/ui_dlgautodj.h"
#include "library/library.h"
#include "library/libraryview.h"
#include "library/trackcollection.h"
#include "preferences/usersettings.h"
#include "track/track_decl.h"

class PlaylistTableModel;
class WLibrary;
class WTrackTableView;

class DlgAutoDJ : public QWidget, public Ui::DlgAutoDJ, public LibraryView {
    Q_OBJECT
  public:
    DlgAutoDJ(WLibrary* parent,
            UserSettingsPointer pConfig,
            Library* pLibrary,
            AutoDJProcessor* pProcessor,
            KeyboardEventFilter* pKeyboard);
    ~DlgAutoDJ() override;

    void onShow() override;
    bool hasFocus() const override;
    void onSearch(const QString& text) override;
    void loadSelectedTrack() override;
    void loadSelectedTrackToGroup(QString group, bool play) override;
    void moveSelection(int delta) override;

  public slots:
    void shufflePlaylistButton(bool buttonChecked);
    void skipNextButton(bool buttonChecked);
    void fadeNowButton(bool buttonChecked);
    void toggleAutoDJButton(bool enable);
    void transitionTimeChanged(int time);
    void transitionSliderChanged(int value);
    void autoDJStateChanged(AutoDJProcessor::AutoDJState state);
    void updateSelectionInfo();
    void slotTransitionModeChanged(int comboboxIndex);
    void slotRepeatPlaylistChanged(int checkedState);

  signals:
    void addRandomButton(bool buttonChecked);
    void loadTrack(TrackPointer tio);
    void loadTrackToPlayer(TrackPointer tio, QString group, bool);
    void trackSelected(TrackPointer pTrack);

  private:
    void setupActionButton(QPushButton* pButton,
            void (DlgAutoDJ::*pSlot)(bool),
            QString fallbackText);

    const UserSettingsPointer m_pConfig;

    AutoDJProcessor* const m_pAutoDJProcessor;
    WTrackTableView* const m_pTrackTableView;
    const bool m_bShowButtonText;

    PlaylistTableModel* m_pAutoDJTableModel;

    QString m_enableBtnTooltip;
    QString m_disableBtnTooltip;
};

#endif //DLGAUTODJ_H
