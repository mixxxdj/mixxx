#ifndef DLGAUTODJ_H
#define DLGAUTODJ_H

#include <QWidget>
#include <QString>

#include "library/autodj/ui_dlgautodj.h"
#include "preferences/usersettings.h"
#include "track/track.h"
#include "library/libraryview.h"
#include "library/library.h"
#include "library/trackcollection.h"
#include "library/autodj/autodjprocessor.h"
#include "controllers/keyboard/keyboardeventfilter.h"

class PlaylistTableModel;
class WTrackTableView;

class DlgAutoDJ : public QWidget, public Ui::DlgAutoDJ, public LibraryView {
    Q_OBJECT
  public:
    DlgAutoDJ(QWidget* parent, UserSettingsPointer pConfig,
              Library* pLibrary,
              AutoDJProcessor* pProcessor, TrackCollection* pTrackCollection,
              KeyboardEventFilter* pKeyboard);
    virtual ~DlgAutoDJ();

    void onShow();
    void onSearch(const QString& text);
    void loadSelectedTrack();
    void loadSelectedTrackToGroup(QString group, bool play);
    void moveSelection(int delta);

  public slots:
    void shufflePlaylistButton(bool buttonChecked);
    void skipNextButton(bool buttonChecked);
    void fadeNowButton(bool buttonChecked);
    void toggleAutoDJButton(bool enable);
    void transitionTimeChanged(int time);
    void transitionSliderChanged(int value);
    void autoDJStateChanged(AutoDJProcessor::AutoDJState state);
    void setTrackTableFont(const QFont& font);
    void setTrackTableRowHeight(int rowHeight);
    void updateSelectionInfo();

  signals:
    void addRandomButton(bool buttonChecked);
    void loadTrack(TrackPointer tio);
    void loadTrackToPlayer(TrackPointer tio, QString group, bool);
    void trackSelected(TrackPointer pTrack);

  private:
    AutoDJProcessor* m_pAutoDJProcessor;
    WTrackTableView* m_pTrackTableView;
    PlaylistTableModel* m_pAutoDJTableModel;
};

#endif //DLGAUTODJ_H
