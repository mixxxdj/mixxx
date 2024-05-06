#include "widget/wcuemenupopup.h"

#include <QHBoxLayout>
#include <QVBoxLayout>

#include "control/controlobject.h"
#include "moc_wcuemenupopup.cpp"
#include "track/track.h"

void CueTypePushButton::mousePressEvent(QMouseEvent* e) {
    if (e->type() == QEvent::MouseButtonPress && e->button() == Qt::RightButton) {
        emit rightClicked();
        return;
    }
    QPushButton::mousePressEvent(e);
}

WCueMenuPopup::WCueMenuPopup(UserSettingsPointer pConfig, QWidget* parent)
        : QWidget(parent),
          m_colorPaletteSettings(ColorPaletteSettings(pConfig)),
          m_pBeatLoopSize(ControlFlag::AllowMissingOrInvalid),
          m_pPlayPos(ControlFlag::AllowMissingOrInvalid),
          m_pTrackSample(ControlFlag::AllowMissingOrInvalid),
          m_pQuantizeEnabled(ControlFlag::AllowMissingOrInvalid) {
    QWidget::hide();
    setWindowFlags(Qt::Popup);
    setAttribute(Qt::WA_StyledBackground);
    setObjectName("WCueMenuPopup");

    m_pCueNumber = new QLabel(this);
    m_pCueNumber->setToolTip(tr("Cue number"));
    m_pCueNumber->setObjectName("CueNumberLabel");
    m_pCueNumber->setAlignment(Qt::AlignLeft);

    m_pCuePosition = new QLabel(this);
    m_pCuePosition->setToolTip(tr("Cue position"));
    m_pCuePosition->setObjectName("CuePositionLabel");
    m_pCuePosition->setAlignment(Qt::AlignRight);

    m_pEditLabel = new QLineEdit(this);
    m_pEditLabel->setToolTip(tr("Edit cue label"));
    m_pEditLabel->setObjectName("CueLabelEdit");
    m_pEditLabel->setPlaceholderText(tr("Label..."));
    connect(m_pEditLabel, &QLineEdit::textEdited, this, &WCueMenuPopup::slotEditLabel);
    connect(m_pEditLabel, &QLineEdit::returnPressed, this, &WCueMenuPopup::hide);

    m_pColorPicker = new WColorPicker(WColorPicker::Option::NoOptions, m_colorPaletteSettings.getHotcueColorPalette(), this);
    m_pColorPicker->setObjectName("CueColorPicker");
    connect(m_pColorPicker, &WColorPicker::colorPicked, this, &WCueMenuPopup::slotChangeCueColor);

    m_pDeleteCue = new QPushButton("", this);
    m_pDeleteCue->setToolTip(tr("Delete this cue"));
    m_pDeleteCue->setObjectName("CueDeleteButton");
    connect(m_pDeleteCue, &QPushButton::clicked, this, &WCueMenuPopup::slotDeleteCue);

    m_pSavedLoopCue = new CueTypePushButton(this);
    m_pSavedLoopCue->setToolTip(
            tr("Toggle this cue type between normal cue and saved loop, using "
               "the current beatloop size or the current play position") +
            "\n\n" +
            tr("Left-click: Toggle between normal cue and saved loop, using "
               "the current beatloop size as the loop size") +
            "\n" +
            tr("Right-click: Set the current play position as the loop end"));
    m_pSavedLoopCue->setObjectName("CueSavedLoopButton");
    m_pSavedLoopCue->setCheckable(true);
    connect(m_pSavedLoopCue, &CueTypePushButton::clicked, this, &WCueMenuPopup::slotSavedLoopCue);
    connect(m_pSavedLoopCue,
            &CueTypePushButton::rightClicked,
            this,
            &WCueMenuPopup::slotAdjustSavedLoopCue);

    QHBoxLayout* pLabelLayout = new QHBoxLayout();
    pLabelLayout->addWidget(m_pCueNumber);
    pLabelLayout->addStretch(1);
    pLabelLayout->addWidget(m_pCuePosition);

    QVBoxLayout* pLeftLayout = new QVBoxLayout();
    pLeftLayout->addLayout(pLabelLayout);
    pLeftLayout->addWidget(m_pEditLabel);
    pLeftLayout->addWidget(m_pColorPicker);

    QVBoxLayout* pRightLayout = new QVBoxLayout();
    pRightLayout->addWidget(m_pDeleteCue);
    pRightLayout->addStretch(1);
    pRightLayout->addWidget(m_pSavedLoopCue);

    QHBoxLayout* pMainLayout = new QHBoxLayout();
    pMainLayout->addLayout(pLeftLayout);
    pMainLayout->addSpacing(5);
    pMainLayout->addLayout(pRightLayout);
    setLayout(pMainLayout);
    // we need to update the the layout here since the size is used to
    // calculate the positioning later
    layout()->update();
    layout()->activate();
}

void WCueMenuPopup::setTrackCueGroup(
        TrackPointer pTrack, const CuePointer& pCue, const QString& group) {
    m_pTrack = pTrack;
    m_pCue = pCue;

    if (m_pBeatLoopSize.getKey().group != group) {
        m_pBeatLoopSize = PollingControlProxy(group, "beatloop_size");
    }

    if (m_pPlayPos.getKey().group != group) {
        m_pPlayPos = PollingControlProxy(group, "playposition");
    }

    if (m_pTrackSample.getKey().group != group) {
        m_pTrackSample = PollingControlProxy(group, "track_samples");
    }

    if (m_pQuantizeEnabled.getKey().group != group) {
        m_pQuantizeEnabled = PollingControlProxy(group, "quantize");
    }
    slotUpdate();
}

void WCueMenuPopup::slotUpdate() {
    if (m_pTrack && m_pCue) {
        int hotcueNumber = m_pCue->getHotCue();
        QString hotcueNumberText = "";
        if (hotcueNumber != Cue::kNoHotCue) {
            // Programmers count from 0, but DJs count from 1
            hotcueNumberText = QString(tr("Hotcue #%1")).arg(QString::number(hotcueNumber + 1));
        }
        m_pCueNumber->setText(hotcueNumberText);

        QString positionText = "";
        Cue::StartAndEndPositions pos = m_pCue->getStartAndEndPosition();
        if (pos.startPosition.isValid()) {
            double startPositionSeconds = pos.startPosition.value() / m_pTrack->getSampleRate();
            positionText = mixxx::Duration::formatTime(startPositionSeconds, mixxx::Duration::Precision::CENTISECONDS);
            if (pos.endPosition.isValid()) {
                double endPositionSeconds = pos.endPosition.value() / m_pTrack->getSampleRate();
                positionText = QString("%1 - %2").arg(
                    positionText,
                    mixxx::Duration::formatTime(endPositionSeconds, mixxx::Duration::Precision::CENTISECONDS)
                );
            }
        }
        m_pCuePosition->setText(positionText);

        m_pEditLabel->setText(m_pCue->getLabel());
        m_pColorPicker->setSelectedColor(m_pCue->getColor());
        m_pSavedLoopCue->setChecked(m_pCue->getType() == mixxx::CueType::Loop);
    } else {
        m_pTrack.reset();
        m_pCue.reset();
        m_pCueNumber->setText(QString(""));
        m_pCuePosition->setText(QString(""));
        m_pEditLabel->setText(QString(""));
        m_pColorPicker->setSelectedColor(std::nullopt);
    }
}
void WCueMenuPopup::slotEditLabel() {
    VERIFY_OR_DEBUG_ASSERT(m_pCue != nullptr) {
        return;
    }
    m_pCue->setLabel(m_pEditLabel->text());
}

void WCueMenuPopup::slotChangeCueColor(mixxx::RgbColor::optional_t color) {
    VERIFY_OR_DEBUG_ASSERT(m_pCue != nullptr) {
        return;
    }
    VERIFY_OR_DEBUG_ASSERT(color) {
        return;
    }
    m_pCue->setColor(*color);
    m_pColorPicker->setSelectedColor(color);
    hide();
}

void WCueMenuPopup::slotDeleteCue() {
    VERIFY_OR_DEBUG_ASSERT(m_pCue != nullptr) {
        return;
    }
    VERIFY_OR_DEBUG_ASSERT(m_pTrack != nullptr) {
        return;
    }
    m_pTrack->removeCue(m_pCue);
    hide();
}

void WCueMenuPopup::slotSavedLoopCue() {
    VERIFY_OR_DEBUG_ASSERT(m_pCue != nullptr) {
        return;
    }
    VERIFY_OR_DEBUG_ASSERT(m_pTrack != nullptr) {
        return;
    }
    VERIFY_OR_DEBUG_ASSERT(m_pBeatLoopSize.valid()) {
        return;
    }
    if (m_pCue->getType() == mixxx::CueType::Loop) {
        m_pCue->setType(mixxx::CueType::HotCue);
        m_pCue->setEndPosition(mixxx::audio::FramePos());
    } else {
        double beatloopSize = m_pBeatLoopSize.get();
        const mixxx::BeatsPointer pBeats = m_pTrack->getBeats();
        if (beatloopSize <= 0 || !pBeats) {
            m_pCue->setEndPosition(mixxx::audio::FramePos());
            return;
        }
        m_pCue->setEndPosition(pBeats->findNBeatsFromPosition(m_pCue->getPosition(), beatloopSize));
        m_pCue->setType(mixxx::CueType::Loop);
    }
    slotUpdate();
}

void WCueMenuPopup::slotAdjustSavedLoopCue() {
    VERIFY_OR_DEBUG_ASSERT(m_pCue != nullptr) {
        return;
    }
    VERIFY_OR_DEBUG_ASSERT(m_pTrack != nullptr) {
        return;
    }
    VERIFY_OR_DEBUG_ASSERT(m_pBeatLoopSize.valid()) {
        return;
    }
    if (m_pCue->getType() == mixxx::CueType::Loop) {
        const mixxx::BeatsPointer pBeats = m_pTrack->getBeats();
        auto position = mixxx::audio::FramePos::fromEngineSamplePos(
                m_pPlayPos.get() * m_pTrackSample.get());
        if (!m_pQuantizeEnabled.toBool() || !pBeats) {
            m_pCue->setEndPosition(position);
        } else {
            mixxx::audio::FramePos nextBeatPosition, prevBeatPosition;
            pBeats->findPrevNextBeats(position, &prevBeatPosition, &nextBeatPosition, false);
            position = (nextBeatPosition - position > position - prevBeatPosition)
                    ? prevBeatPosition
                    : nextBeatPosition;
            if (position <= m_pCue->getPosition()) {
                return;
            }
            m_pCue->setEndPosition(position);
        }
    }
    slotUpdate();
}

void WCueMenuPopup::closeEvent(QCloseEvent* event) {
    emit aboutToHide();
    QWidget::closeEvent(event);
}
