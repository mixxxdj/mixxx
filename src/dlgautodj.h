#ifndef DLGAUTODJ_H
#define DLGAUTODJ_H

#include <QWidget>
#include <QString>

#include "ui_dlgautodj.h"
#include "configobject.h"
#include "trackinfoobject.h"
#include "library/libraryview.h"
#include "library/trackcollection.h"
#include "library/autodj/autodjprocessor.h"
#include "mixxxkeyboard.h"

class PlaylistTableModel;
class WTrackTableView;

class DlgAutoDJ : public QWidget, public Ui::DlgAutoDJ, public LibraryView {
    Q_OBJECT
  public:
    DlgAutoDJ(QWidget* parent, ConfigObject<ConfigValue>* pConfig,
              AutoDJProcessor* pProcessor, TrackCollection* pTrackCollection,
              MixxxKeyboard* pKeyboard);
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
    void enableRandomButton(bool enabled);
    void autoDJStateChanged(AutoDJProcessor::AutoDJState state);

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
