#ifndef DLGAUTODJ_H
#define DLGAUTODJ_H

#include <QWidget>
#include <QString>
#include <QList>

#include "library/features/autodj/autodjprocessor.h"
#include "library/features/autodj/ui_dlgautodj.h"
#include "library/trackcollection.h"
#include "track/track.h"

class PlaylistTableModel;
class WTrackTableView;

class DlgAutoDJ : public QFrame, public Ui::DlgAutoDJ {
    Q_OBJECT
  public:
    DlgAutoDJ(QWidget* parent, AutoDJProcessor* pProcessor);
    virtual ~DlgAutoDJ();
    
    void onShow();
    
    // These seleced rows are always from the focused pane
    void setSelectedRows(const QModelIndexList& selectedRows);

  public slots:
    void shufflePlaylistButton(bool buttonChecked);
    void skipNextButton(bool buttonChecked);
    void fadeNowButton(bool buttonChecked);
    void toggleAutoDJButton(bool enable);
    void transitionTimeChanged(int time);
    void transitionSliderChanged(int value);
    void autoDJStateChanged(AutoDJProcessor::AutoDJState state);
    void updateSelectionInfo();

  signals:
    void addRandomButton(bool buttonChecked);
    
  private:
    AutoDJProcessor* m_pAutoDJProcessor;
    PlaylistTableModel* m_pAutoDJTableModel;
    
    QModelIndexList m_selectedRows;
};

#endif //DLGAUTODJ_H
