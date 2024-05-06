#pragma once

#include <QLabel>
#include <QLineEdit>
#include <QMouseEvent>
#include <QPushButton>

#include "control/pollingcontrolproxy.h"
#include "preferences/colorpalettesettings.h"
#include "track/cue.h"
#include "track/track_decl.h"
#include "util/parented_ptr.h"
#include "util/widgethelper.h"
#include "widget/wcolorpicker.h"

class ControlProxy;

// Custom PushButton which emit a custom signal when right-clicked
class CueTypePushButton : public QPushButton {
    Q_OBJECT
  public:
    explicit CueTypePushButton(QWidget* parent = 0)
            : QPushButton(parent) {
    }

  protected:
    void mousePressEvent(QMouseEvent* e) override;

  signals:
    void rightClicked();
};

class WCueMenuPopup : public QWidget {
    Q_OBJECT
  public:
    WCueMenuPopup(UserSettingsPointer pConfig, QWidget* parent = nullptr);

    ~WCueMenuPopup() {
        delete m_pCueNumber;
        delete m_pCuePosition;
        delete m_pEditLabel;
        delete m_pColorPicker;
        delete m_pDeleteCue;
    }

    void setTrackCueGroup(TrackPointer pTrack, const CuePointer& pCue, const QString& group);

    void setColorPalette(const ColorPalette& palette) {
        if (m_pColorPicker != nullptr) {
            m_pColorPicker->setColorPalette(palette);
        }
    }

    void popup(const QPoint& p) {
        auto parentWidget = qobject_cast<QWidget*>(parent());
        QPoint topLeft = mixxx::widgethelper::mapPopupToScreen(*parentWidget, p, size());
        move(topLeft);
        show();
    }

    void show() {
        setColorPalette(m_colorPaletteSettings.getHotcueColorPalette());
        m_pEditLabel->setFocus();
        emit aboutToShow();
        QWidget::show();
    }

  signals:
    void aboutToHide();
    void aboutToShow();

  private slots:
    void slotEditLabel();
    void slotDeleteCue();
    void slotUpdate();
    void slotSavedLoopCue();
    void slotAdjustSavedLoopCue();
    void slotChangeCueColor(mixxx::RgbColor::optional_t color);

  private:
    ColorPaletteSettings m_colorPaletteSettings;
    PollingControlProxy m_pBeatLoopSize;
    PollingControlProxy m_pPlayPos;
    PollingControlProxy m_pTrackSample;
    PollingControlProxy m_pQuantizeEnabled;
    CuePointer m_pCue;
    TrackPointer m_pTrack;

    QLabel* m_pCueNumber;
    QLabel* m_pCuePosition;
    QLineEdit* m_pEditLabel;
    WColorPicker* m_pColorPicker;
    QPushButton* m_pDeleteCue;
    CueTypePushButton* m_pSavedLoopCue;

  protected:
    void closeEvent(QCloseEvent* event) override;
};
