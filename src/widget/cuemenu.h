#pragma once

#include <QMenu>

#include "track/track.h"
#include "track/cue.h"

class CueMenu : public QMenu {
    Q_OBJECT
  public:
    CueMenu(QWidget *parent = nullptr);
    ~CueMenu() override;

    void setCue(CuePointer pCue) {
        m_pCue = pCue;
    }

    void setTrack(TrackPointer pTrack) {
        m_pTrack = pTrack;
    }

  private slots:
    void slotEditLabel();
    void slotRemoveCue();

  private:
    // This is not a Qt slot because it is connected via a lambda.
    void changeCueColor(PredefinedColorPointer pColor);

    CuePointer m_pCue;
    TrackPointer m_pTrack;

    QAction* m_pEditLabel;
    QMenu* m_pColorMenu;
    QList<QAction*> m_pColorMenuActions;
    QAction* m_pRemoveCue;
};
