#pragma once

#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QMouseEvent>
#include <QPushButton>
#include <QVBoxLayout>
#include <array>
#include <vector>

#include "control/pollingcontrolproxy.h"
#include "preferences/colorpalettesettings.h"
#include "track/cue.h"
#include "track/track_decl.h"
#include "util/widgethelper.h"
#include "widget/wcolorpicker.h"

class ControlProxy;
class PlayerManager;

struct SamplerLayout {
    QList<int> samplerNumbers; // sampler nrs in visual order
    QList<int> columnsPerRow;  // columns in each row
};

// Custom PushButton which emit a custom signal when right-clicked
class CueMenuPushButton : public QPushButton {
    Q_OBJECT
  public:
    explicit CueMenuPushButton(QWidget* parent = 0)
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
    void slotStandardCue();
    void slotSavedJumpCueManual();
    void slotSavedJumpCueAuto();
    /// This slot is called when the saved loop button is being left pressed,
    /// which effectively toggle the cue loop between standard cue and saved
    /// loop. If the cue was never a saved loop, it will use the current
    /// beatloop size to define the saved loop size. If it was previously a
    /// saved loop, it will use the previously known loop size.
    void slotSavedLoopCueAuto();
    /// This slot is called when the saved loop button is being left pressed,
    /// which effectively makes the cue a saved loop and use the current play
    /// position as loop end
    void slotSavedLoopCueManual();
    void slotChangeCueColor(mixxx::RgbColor::optional_t color);

    void slotExportCue();
    void slotExportToSampler(int samplerIndex);

  private:
    void updateTypeAndColorIfDefault(mixxx::CueType newType);
    mixxx::audio::FramePos getCurrentPlayPositionWithQuantize() const;

    UserSettingsPointer m_pConfig;
    ColorPaletteSettings m_colorPaletteSettings;
    PollingControlProxy m_pBeatLoopSize;
    PollingControlProxy m_pPlayPos;
    PollingControlProxy m_pTrackSample;
    PollingControlProxy m_pQuantizeEnabled;
    CuePointer m_pCue;
    TrackPointer m_pTrack;

    std::unique_ptr<QLabel> m_pCueNumber;
    std::unique_ptr<QLabel> m_pCuePosition;
    std::unique_ptr<QLineEdit> m_pEditLabel;
    std::unique_ptr<WColorPicker> m_pColorPicker;
    std::unique_ptr<CueMenuPushButton> m_pDeleteCue;
    std::unique_ptr<CueMenuPushButton> m_pStandardCue;
    std::unique_ptr<CueMenuPushButton> m_pSavedLoopCue;
    std::unique_ptr<CueMenuPushButton> m_pSavedJumpCue;

    QString buildExportPath(const QString& trackId,
            const QString& artist,
            const QString& title,
            const QString& tag,
            const QString& ext) const;

    bool trackHasStems() const;

    std::unique_ptr<CueMenuPushButton> m_pExportCue;
    QString m_group;

    void updateExportToSamplerButtons();
    QString exportCueToFile(bool blocking);
    bool exportLoopByStreamCopy(const QString& src,
            const mixxx::audio::FramePos& start,
            const mixxx::audio::FramePos& end,
            const QString& dst,
            const QString& title,
            bool isStemFile,
            bool blocking);

    bool exportLoopByRendering(const mixxx::audio::FramePos& start,
            const mixxx::audio::FramePos& end,
            const std::array<double, 4>& stemGains,
            bool isStemFile,
            const QString& dst,
            const QString& title,
            bool blocking);

    void getStemState(std::array<double, 4>& stemGains) const;

    bool isFileLoadedInAnySampler(const QString& path) const;

    PlayerManager* m_pPlayerManager = nullptr;

    SamplerLayout currentSamplerLayout() const;
    void rebuildExportToSamplerButtons();
    void clearExportToSamplerButtons();

    std::vector<std::unique_ptr<CueMenuPushButton>> m_pExportToSamplerButtons;
    std::vector<QHBoxLayout*> m_pSamplerButtonRows;
    QVBoxLayout* m_pLeftLayout = nullptr;

    bool samplerIsPlaying(const QString& group) const;
    bool samplerHasLoadedTrack(PlayerManager* pPlayerManager,
            const QString& group) const;
    bool samplerIsLooping(const QString& group) const;

  protected:
    void closeEvent(QCloseEvent* event) override;
};
