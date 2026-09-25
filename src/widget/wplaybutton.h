#pragma once

#include "widget/wpushbutton.h"

struct HotcueDragInfo;

/// This is a pushbutton that accepts dropping of WCueButton and WHotcueButton
/// and then (indirectly) sets `play_latched` to 1 on drop.
/// This allows to easily switch from cue/hotcue previewing to regular play.
/// Note: this only handles the dragEnter and drop events.
/// It's up to the skin designer to pick appropriate controls for the left-,
/// right-click and display connection.
class WPlayButton : public WPushButton {
    Q_OBJECT
  public:
    WPlayButton(QWidget* pParent, const QString& group);

  private:
    void dragEnterEvent(QDragEnterEvent* pEvent) override;
    void dropEvent(QDropEvent* pEvent) override;

    QString m_group;
};
