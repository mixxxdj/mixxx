#pragma once

#include "widget/wpushbutton.h"

/// This is a pushbutton that can be dragged and dropped onto a WPlayButton.
/// Note: this only creates and handles the QDrag.
/// Since there are many possible combinations of the various `cue_..` controls
/// for the left-, right-click or display connection, this class doesn't check
/// if those are actually cue controls -- that's up to the skin designer.
class WCueButton : public WPushButton {
    Q_OBJECT
  public:
    WCueButton(QWidget* pParent, const QString& group);

    void setup(const QDomNode& node, const SkinContext& context) override;

  private:
    void mouseMoveEvent(QMouseEvent* pEvent) override;

    QString m_group;
    QMargins m_dndRectMargins;
};
