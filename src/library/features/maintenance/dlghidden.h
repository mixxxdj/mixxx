#ifndef DLGHIDDEN_H
#define DLGHIDDEN_H

#include "controllers/keyboard/keyboardeventfilter.h"
#include "library/features/maintenance/ui_dlghidden.h"
#include "library/trackcollection.h"
#include "preferences/usersettings.h"

class WTrackTableView;
class HiddenTableModel;
class QItemSelection;

class DlgHidden : public QFrame, public Ui::DlgHidden {
    Q_OBJECT
  public:
    DlgHidden(QWidget* parent);
    virtual ~DlgHidden();

    // The indexes are always from the focused pane
    void setSelectedIndexes(const QModelIndexList& selectedIndexes);
    void setTableModel(HiddenTableModel* pTableModel);

  public slots:
    void onShow();

  signals:
    void selectAll();  
    void unhide();
    void purge();
    void trackSelected(TrackPointer pTrack);

  private:
    void activateButtons(bool enable);
    HiddenTableModel* m_pHiddenTableModel;
};

#endif //DLGHIDDEN_H
