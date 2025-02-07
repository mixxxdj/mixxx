#pragma once

#include <QString>

#include "track/trackid.h"
#include "util/parented_ptr.h"
#include "widget/wcuemenupopup.h"
#include "widget/wpushbutton.h"

class WHotcueButton : public WPushButton {
    Q_OBJECT

    struct HotcueDragInfo {
        HotcueDragInfo(TrackId id, int cue)
                : trackId(id),
                  hotcue(cue) {};

        static HotcueDragInfo fromByteArray(const QByteArray& bytes) {
            QDataStream stream(bytes);
            TrackId trackId;
            int hotcue;
            stream >> trackId >> hotcue;
            return HotcueDragInfo(trackId, hotcue);
        };

        QByteArray toByteArray() {
            QByteArray bytes;
            QDataStream dataStream(&bytes, QIODevice::WriteOnly);
            dataStream << trackId << hotcue;
            return bytes;
        };

        bool isValid() {
            return trackId.isValid() && hotcue != Cue::kNoHotCue;
        }

        TrackId trackId = TrackId();
        int hotcue = Cue::kNoHotCue;
    };

  public:
    WHotcueButton(const QString& group, QWidget* pParent);

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
