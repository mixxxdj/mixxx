#pragma once

#include <QLabel>
#include <QLineEdit>
#include <QPushButton>

#include "preferences/colorpalettesettings.h"
#include "track/cue.h"
#include "track/track_decl.h"
#include "util/widgethelper.h"
#include "widget/wcolorpicker.h"

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

    void setTrackAndCue(TrackPointer pTrack, CuePointer pCue);

    void setColorPalette(const ColorPalette& palette) {
        if (m_pColorPicker != nullptr) {
            m_pColorPicker->setColorPalette(palette);
        }
    }

    void popup(const QPoint& p) {
        auto parentWidget = static_cast<QWidget*>(parent());
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
    void slotChangeCueColor(mixxx::RgbColor::optional_t color);

  private:
    ColorPaletteSettings m_colorPaletteSettings;
    CuePointer m_pCue;
    TrackPointer m_pTrack;

    QLabel* m_pCueNumber;
    QLabel* m_pCuePosition;
    QLineEdit* m_pEditLabel;
    WColorPicker* m_pColorPicker;
    QPushButton* m_pDeleteCue;

  protected:
    void closeEvent(QCloseEvent* event) override;
};
