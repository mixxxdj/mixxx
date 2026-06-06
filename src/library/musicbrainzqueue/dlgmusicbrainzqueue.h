#pragma once

#include "library/libraryview.h"
#include "library/musicbrainzqueue/ui_dlgmusicbrainzqueue.h"
#include "preferences/usersettings.h"

class WLibrary;
class WTrackTableView;
class MusicBrainzQueueTableModel;
class QItemSelection;
class Library;
class KeyboardEventFilter;

class DlgMusicBrainzQueue : public QWidget,
                            public Ui::DlgMusicBrainzQueue,
                            public LibraryView {
    Q_OBJECT

  public:
    DlgMusicBrainzQueue(
            WLibrary* parent,
            UserSettingsPointer pConfig,
            Library* pLibrary,
            KeyboardEventFilter* pKeyboard);
    ~DlgMusicBrainzQueue() override;

    void onShow() override;
    bool hasFocus() const override;
    void setFocus() override;
    void onSearch(const QString& text) override;
    QString currentSearch();
    void saveCurrentViewState() override;
    bool restoreCurrentViewState() override;

  public slots:
    void selectAll();
    void selectionChanged(
            const QItemSelection& selected,
            const QItemSelection& deselected);

  signals:
    void trackSelected(TrackPointer pTrack);

  private slots:
    void slotSubmitSelected();
    void slotSubmitAll();
    void slotRetryFailed();

  private:
    void activateButtons(bool enable);
    void enqueueAndWake(const QList<TrackId>& trackIds);

    // Note: m_pTrackTableView must be deleted before m_pQueueTableModel
    // because the view saves its header state through the model on destruction.
    WTrackTableView* m_pTrackTableView;
    MusicBrainzQueueTableModel* m_pQueueTableModel;
    Library* m_pLibrary;
};
