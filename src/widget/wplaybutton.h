#pragma once

#include "widget/wpushbutton.h"

struct HotcueDragInfo;

class WPlayButton : public WPushButton {
    Q_OBJECT
  public:
    WPlayButton(QWidget* pParent, const QString& group);

  private:
    void dragEnterEvent(QDragEnterEvent* pEvent) override;
    void dropEvent(QDropEvent* pEvent) override;

    QString m_group;
};
