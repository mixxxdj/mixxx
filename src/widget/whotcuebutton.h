#pragma once

#include <QString>

#include "util/parented_ptr.h"
#include "widget/wcuemenupopup.h"
#include "widget/wpushbutton.h"

/// Pushbutton with hotcue controls and cue menu popup on right-click.
/// This button can be dropped onto other WHotcueButtons to swap their hotcues.
/// Can also be dropped onto WPlayButton in order to easily switch from hotcue
/// previewing to regular play.
class WHotcueButton : public WPushButton {
    Q_OBJECT
  public:
    WHotcueButton(QWidget* pParent, const QString& group);

    void setup(const QDomNode& node, const SkinContext& context) override;

    ConfigKey getLeftClickConfigKey() {
        return createConfigKey(QStringLiteral("activate"));
    }
    ConfigKey getClearConfigKey() {
        return createConfigKey(QStringLiteral("clear"));
    }

    Q_PROPERTY(bool light MEMBER m_bCueColorIsLight);
    Q_PROPERTY(bool dark MEMBER m_bCueColorIsDark);
    Q_PROPERTY(QString type MEMBER m_type);

  protected:
    void mousePressEvent(QMouseEvent* pEvent) override;
    void mouseReleaseEvent(QMouseEvent* pEvent) override;
    void mouseMoveEvent(QMouseEvent* pEvent) override;
    void dragEnterEvent(QDragEnterEvent* pEvent) override;
    void dropEvent(QDropEvent* pEvent) override;
    void restyleAndRepaint() override;

  private slots:
    void slotColorChanged(double color);
    void slotTypeChanged(double type);

  private:
    ConfigKey createConfigKey(const QString& name);
    void updateStyleSheet();

    const QString m_group;
    int m_hotcue;
    bool m_hoverCueColor;
    parented_ptr<ControlProxy> m_pCoColor;
    parented_ptr<ControlProxy> m_pCoType;
    parented_ptr<WCueMenuPopup> m_pCueMenuPopup;
    int m_cueColorDimThreshold;
    bool m_bCueColorDimmed;
    bool m_bCueColorIsLight;
    bool m_bCueColorIsDark;
    QString m_type;
    QMargins m_dndRectMargins;
};
