#pragma once

#include <QMenu>

#include "track/track.h"
#include "track/cue.h"
#include "widget/colormenu.h"

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

    void useColorSet(const HotcueColorPalette& colorPalette) {
        if (m_pColorMenu != nullptr) {
            m_pColorMenu->useColorPalette(colorPalette);
        }
    }

  private slots:
    void slotEditLabel();
    void slotRemoveCue();
    void slotChangeCueColor(QColor color);

  private:
    CuePointer m_pCue;
    TrackPointer m_pTrack;

    QAction* m_pEditLabel;
    ColorMenu* m_pColorMenu;
    QAction* m_pRemoveCue;
};
