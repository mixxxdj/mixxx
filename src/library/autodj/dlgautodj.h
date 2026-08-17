#pragma once

#include <QString>
#include <QTimer>
#include <QWidget>

#include "library/autodj/autodjprocessor.h"
#include "library/autodj/ui_dlgautodj.h"
#include "library/libraryview.h"
#include "preferences/usersettings.h"
#include "track/track_decl.h"
#include "util/parented_ptr.h"

class PlaylistTableModel;
class WLibrary;
class WTrackTableView;
class Library;
class KeyboardEventFilter;

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

    void slotTransitionModeChanged(int comboboxIndex);
    void slotRepeatPlaylistChanged(bool checked);
    void slotAddEndMarker(bool);

  signals:
    void addRandomTrackButton(bool buttonChecked);
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

  private slots:
    void slotRecalcQueueDuration();
    void slotUpdateQueueDuration();

  private:
    void setupActionButton(QPushButton* pButton,
            void (DlgAutoDJ::*pSlot)(bool),
            const QString& fallbackText);
    void refocusPrevWidget();
    bool eventFilter(QObject* pObj, QEvent* pEvent) override;
    void keyPressEvent(QKeyEvent* pEvent) override;
    void hideEvent(QHideEvent* event) override;

    const UserSettingsPointer m_pConfig;

    AutoDJProcessor* const m_pAutoDJProcessor;
    WTrackTableView* const m_pTrackTableView;
    const bool m_bShowButtonText;

    PlaylistTableModel* m_pAutoDJTableModel;
    parented_ptr<QTimer> m_pQueueDurationTimer;
    double m_queueSeconds;
    bool m_showEndTime;
    bool m_autoDJWasActive;

    QString m_enableBtnTooltip;
    QString m_disableBtnTooltip;
};
