#include "widget/wcuemenupopup.h"

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <optional>

#include "control/controlobject.h"
#include "moc_wcuemenupopup.cpp"
#include "track/track.h"

namespace {
const ConfigKey kHotcueDefaultColorIndexConfigKey("[Controls]", "HotcueDefaultColorIndex");
const ConfigKey kLoopDefaultColorIndexConfigKey("[Controls]", "LoopDefaultColorIndex");
const ConfigKey kJumpDefaultColorIndexConfigKey("[Controls]", "jump_default_color_index");
} // namespace

void CueTypePushButton::mousePressEvent(QMouseEvent* e) {
    if (e->type() == QEvent::MouseButtonPress && e->button() == Qt::RightButton) {
        emit rightClicked();
        return;
    }
    QPushButton::mousePressEvent(e);
}

void WCueMenuPopup::updateTypeAndColorIfDefault(mixxx::CueType newType) {
    auto hotcueColorPalette =
            m_colorPaletteSettings.getHotcueColorPalette();
    int colorIndex;
    switch (m_pCue->getType()) {
    default:
        colorIndex = m_pConfig->getValue(kHotcueDefaultColorIndexConfigKey, -1);
        break;
    case mixxx::CueType::Loop:
        colorIndex = m_pConfig->getValue(kLoopDefaultColorIndexConfigKey, -1);
        break;
    case mixxx::CueType::Jump:
        colorIndex = m_pConfig->getValue(kJumpDefaultColorIndexConfigKey, -1);
        break;
    }
    auto defaultColor =
            (colorIndex < 0 || colorIndex >= hotcueColorPalette.size())
            ? hotcueColorPalette.defaultColor()
            : hotcueColorPalette.at(colorIndex);
    m_pCue->setType(newType);
    if (m_pCue->getColor() != defaultColor) {
        return;
    }
    switch (newType) {
    default:
        colorIndex = m_pConfig->getValue(kHotcueDefaultColorIndexConfigKey, -1);
        break;
    case mixxx::CueType::Loop:
        colorIndex = m_pConfig->getValue(kLoopDefaultColorIndexConfigKey, -1);
        break;
    case mixxx::CueType::Jump:
        colorIndex = m_pConfig->getValue(kJumpDefaultColorIndexConfigKey, -1);
        break;
    }
    if (colorIndex < 0 || colorIndex >= hotcueColorPalette.size()) {
        m_pCue->setColor(hotcueColorPalette.defaultColor());
    } else {
        m_pCue->setColor(hotcueColorPalette.at(colorIndex));
    }
}

WCueMenuPopup::WCueMenuPopup(UserSettingsPointer pConfig, QWidget* parent)
        : QWidget(parent),
          m_pConfig(pConfig),
          m_colorPaletteSettings(ColorPaletteSettings(pConfig)),
          m_pBeatLoopSize(ControlFlag::AllowMissingOrInvalid),
          m_pBeatJumpSize(ControlFlag::AllowMissingOrInvalid),
          m_pPlayPos(ControlFlag::AllowMissingOrInvalid),
          m_pTrackSample(ControlFlag::AllowMissingOrInvalid),
          m_pQuantizeEnabled(ControlFlag::AllowMissingOrInvalid) {
    QWidget::hide();
    setWindowFlags(Qt::Popup);
    setAttribute(Qt::WA_StyledBackground);
    setObjectName("WCueMenuPopup");

    m_pCueNumber = std::make_unique<QLabel>(this);
    m_pCueNumber->setToolTip(tr("Cue number"));
    m_pCueNumber->setObjectName("CueNumberLabel");
    m_pCueNumber->setAlignment(Qt::AlignLeft);

    m_pCuePosition = std::make_unique<QLabel>(this);
    m_pCuePosition->setToolTip(tr("Cue position"));
    m_pCuePosition->setObjectName("CuePositionLabel");
    m_pCuePosition->setAlignment(Qt::AlignRight);

    m_pEditLabel = std::make_unique<QLineEdit>(this);
    m_pEditLabel->setToolTip(tr("Edit cue label"));
    m_pEditLabel->setObjectName("CueLabelEdit");
    m_pEditLabel->setPlaceholderText(tr("Label..."));
    connect(m_pEditLabel.get(), &QLineEdit::textEdited, this, &WCueMenuPopup::slotEditLabel);
    connect(m_pEditLabel.get(), &QLineEdit::returnPressed, this, &WCueMenuPopup::hide);

    m_pColorPicker =
            std::make_unique<WColorPicker>(WColorPicker::Option::NoOptions,
                    m_colorPaletteSettings.getHotcueColorPalette(),
                    this);
    m_pColorPicker->setObjectName("CueColorPicker");
    connect(m_pColorPicker.get(),
            &WColorPicker::colorPicked,
            this,
            &WCueMenuPopup::slotChangeCueColor);

    m_pDeleteCue = std::make_unique<QPushButton>("", this);
    m_pDeleteCue->setToolTip(tr("Delete this cue"));
    m_pDeleteCue->setObjectName("CueDeleteButton");
    connect(m_pDeleteCue.get(), &QPushButton::clicked, this, &WCueMenuPopup::slotDeleteCue);

    m_pStandardCue = std::make_unique<QPushButton>("", this);
    m_pStandardCue->setToolTip(
            tr("Toggle this cue type to normal cue if not."));
    m_pStandardCue->setObjectName("CueStandardButton");
    m_pStandardCue->setCheckable(true);
    connect(m_pStandardCue.get(), &QPushButton::clicked, this, &WCueMenuPopup::slotStandardCue);

    m_pSavedLoopCue = std::make_unique<CueTypePushButton>(this);
    m_pSavedLoopCue->setToolTip(
            tr("Toggle this cue type between normal cue and saved loop") +
            "\n\n" +
            tr("Left-click: Use the old size or the current beatloop size as the loop size") +
            "\n" +
            tr("Right-click: Use the current play position as loop end if it is after the cue"));
    m_pSavedLoopCue->setObjectName("CueSavedLoopButton");
    m_pSavedLoopCue->setCheckable(true);
    connect(m_pSavedLoopCue.get(),
            &CueTypePushButton::clicked,
            this,
            &WCueMenuPopup::slotSavedLoopCueAuto);
    connect(m_pSavedLoopCue.get(),
            &CueTypePushButton::rightClicked,
            this,
            &WCueMenuPopup::slotSavedLoopCueManual);

    m_pSavedJumpCue = std::make_unique<CueTypePushButton>(this);
    m_pSavedJumpCue->setToolTip(
            tr("Toggle this cue type between normal cue and saved jump, to \n"
               "the current play position or the current beatjump size ") +
            "\n\n" +
            tr("Left-click: Toggle this cue type to saved beatjump, using \n"
               "the current play position if not previous jump position was "
               "known. \nIf "
               "the play position is the cue position, uses the current "
               "beatjump size. If a previous jump position exists, it will "
               "swap the jump position and the cue/target position.") +
            "\n" +
            tr("Right-click: Set the current play position as the jump \n"
               "destination and make the cue a saved jump if not"));
    m_pSavedJumpCue->setObjectName("CueSavedJumpButton");
    m_pSavedJumpCue->setCheckable(true);
    connect(m_pSavedJumpCue.get(),
            &CueTypePushButton::clicked,
            this,
            &WCueMenuPopup::slotSavedJumpCueAuto);
    connect(m_pSavedJumpCue.get(),
            &CueTypePushButton::rightClicked,
            this,
            &WCueMenuPopup::slotSavedJumpCueManual);

    QHBoxLayout* pLabelLayout = new QHBoxLayout();
    pLabelLayout->addWidget(m_pCueNumber.get());
    pLabelLayout->addStretch(1);
    pLabelLayout->addWidget(m_pCuePosition.get());

    QVBoxLayout* pLeftLayout = new QVBoxLayout();
    pLeftLayout->addLayout(pLabelLayout);
    pLeftLayout->addWidget(m_pEditLabel.get());
    pLeftLayout->addWidget(m_pColorPicker.get());

    QVBoxLayout* pRightLayout = new QVBoxLayout();
    pRightLayout->addWidget(m_pDeleteCue.get());
    pRightLayout->addWidget(m_pStandardCue.get());
    pRightLayout->addStretch(1);
    pRightLayout->addWidget(m_pSavedLoopCue.get());
    pRightLayout->addStretch(1);
    pRightLayout->addWidget(m_pSavedJumpCue.get());

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
    if (!pTrack || !pCue) {
        return;
    }

    m_pTrack = pTrack;
    m_pCue = pCue;

    if (m_pBeatLoopSize.getKey().group != group) {
        m_pBeatLoopSize = PollingControlProxy(group, "beatloop_size");
    }

    if (m_pBeatJumpSize.getKey().group != group) {
        m_pBeatJumpSize = PollingControlProxy(group, "beatjump_size");
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
            if (pos.endPosition.isValid() && m_pCue->getType() != mixxx::CueType::HotCue) {
                double endPositionSeconds = pos.endPosition.value() / m_pTrack->getSampleRate();
                positionText =
                        QString("%1 %2 %3")
                                .arg(positionText,
                                        m_pCue->getType() ==
                                                        mixxx::CueType::Loop
                                                ? "-"
                                                : "->",
                                        mixxx::Duration::formatTime(
                                                endPositionSeconds,
                                                mixxx::Duration::Precision::
                                                        CENTISECONDS));
            }
        }
        m_pCuePosition->setText(positionText);

        m_pEditLabel->setText(m_pCue->getLabel());
        m_pColorPicker->setSelectedColor(m_pCue->getColor());
        m_pStandardCue->setChecked(m_pCue->getType() == mixxx::CueType::HotCue);
        m_pSavedLoopCue->setChecked(m_pCue->getType() == mixxx::CueType::Loop);
        m_pSavedJumpCue->setChecked(m_pCue->getType() == mixxx::CueType::Jump);
        m_pSavedJumpCue->setProperty("direction",
                m_pCue->getType() != mixxx::CueType::Jump ||
                                m_pCue->getPosition() > m_pCue->getEndPosition()
                        ? "forward"
                        : "backward");
        m_pSavedJumpCue->style()->polish(m_pSavedJumpCue.get());
        m_pSavedJumpCue->repaint();
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

void WCueMenuPopup::slotStandardCue() {
    VERIFY_OR_DEBUG_ASSERT(m_pCue != nullptr) {
        return;
    }
    VERIFY_OR_DEBUG_ASSERT(m_pTrack != nullptr) {
        return;
    }
    if (m_pCue->getType() != mixxx::CueType::HotCue) {
        updateTypeAndColorIfDefault(mixxx::CueType::HotCue);
    }
    slotUpdate();
}

void WCueMenuPopup::slotSavedLoopCueAuto() {
    VERIFY_OR_DEBUG_ASSERT(m_pCue != nullptr) {
        return;
    }
    VERIFY_OR_DEBUG_ASSERT(m_pTrack != nullptr) {
        return;
    }
    VERIFY_OR_DEBUG_ASSERT(m_pBeatLoopSize.valid()) {
        return;
    }
    auto cueStartEnd = m_pCue->getStartAndEndPosition();
    // If we are changing the cue type from a jump, we need to permute the positions
    if (m_pCue->getType() == mixxx::CueType::Jump) {
        auto endPosition = cueStartEnd.endPosition;
        cueStartEnd.endPosition = cueStartEnd.startPosition;
        cueStartEnd.startPosition = endPosition;
        m_pCue->setStartAndEndPosition(cueStartEnd.startPosition, cueStartEnd.endPosition);
    }
    if (!cueStartEnd.endPosition.isValid() ||
            cueStartEnd.endPosition <= cueStartEnd.startPosition) {
        double beatloopSize = m_pBeatLoopSize.get();
        const mixxx::BeatsPointer pBeats = m_pTrack->getBeats();
        if (beatloopSize <= 0 || !pBeats) {
            return;
        }
        auto position = pBeats->findNBeatsFromPosition(
                cueStartEnd.startPosition, beatloopSize);
        if (position <= m_pCue->getPosition()) {
            return;
        }
        m_pCue->setEndPosition(position);
    }
    updateTypeAndColorIfDefault(mixxx::CueType::Loop);
    slotUpdate();
}

std::optional<mixxx::audio::FramePos> WCueMenuPopup::getCurrentPlayPositionWithQuantize() const {
    const mixxx::BeatsPointer pBeats = m_pTrack->getBeats();
    auto position = mixxx::audio::FramePos::fromEngineSamplePos(
            m_pPlayPos.get() * m_pTrackSample.get());
    if (m_pQuantizeEnabled.toBool() && pBeats) {
        mixxx::audio::FramePos nextBeatPosition, prevBeatPosition;
        pBeats->findPrevNextBeats(position, &prevBeatPosition, &nextBeatPosition, false);
        return (nextBeatPosition - position > position - prevBeatPosition)
                ? prevBeatPosition
                : nextBeatPosition;
    }
    return position;
}

void WCueMenuPopup::slotSavedLoopCueManual() {
    VERIFY_OR_DEBUG_ASSERT(m_pCue != nullptr) {
        return;
    }
    VERIFY_OR_DEBUG_ASSERT(m_pTrack != nullptr) {
        return;
    }
    // If we are changing the cue type from a jump, we need to permute the
    // positions if it wasn't going backward
    if (m_pCue->getType() == mixxx::CueType::Jump &&
            m_pCue->getPosition() > m_pCue->getEndPosition()) {
        auto cueStartEnd = m_pCue->getStartAndEndPosition();
        auto endPosition = cueStartEnd.endPosition;
        cueStartEnd.endPosition = cueStartEnd.startPosition;
        cueStartEnd.startPosition = endPosition;
        m_pCue->setStartAndEndPosition(cueStartEnd.startPosition, cueStartEnd.endPosition);
    }
    auto newPosition = getCurrentPlayPositionWithQuantize();
    if (!newPosition.has_value() || newPosition <= m_pCue->getPosition()) {
        return;
    }
    m_pCue->setEndPosition(newPosition.value());
    updateTypeAndColorIfDefault(mixxx::CueType::Loop);
    slotUpdate();
}

void WCueMenuPopup::slotSavedJumpCueAuto() {
    VERIFY_OR_DEBUG_ASSERT(m_pCue != nullptr) {
        return;
    }
    VERIFY_OR_DEBUG_ASSERT(m_pTrack != nullptr) {
        return;
    }
    auto cueStartEnd = m_pCue->getStartAndEndPosition();
    // If we are changing the cue type from a loop, we need to permute the position
    // Also, if the type is already a jump, we swap to the to/from point
    if (m_pCue->getType() == mixxx::CueType::Loop || m_pCue->getType() == mixxx::CueType::Jump) {
        auto endPosition = cueStartEnd.endPosition;
        cueStartEnd.endPosition = cueStartEnd.startPosition;
        cueStartEnd.startPosition = endPosition;
    }
    if (!cueStartEnd.endPosition.isValid()) {
        auto newPosition = getCurrentPlayPositionWithQuantize();
        if (!newPosition.has_value()) {
            return;
        }
        if (newPosition == cueStartEnd.startPosition) {
            double beatjumpSize = m_pBeatJumpSize.get();
            const mixxx::BeatsPointer pBeats = m_pTrack->getBeats();
            if (beatjumpSize <= 0 || !pBeats) {
                return;
            }
            newPosition = pBeats->findNBeatsFromPosition(
                    cueStartEnd.startPosition, -beatjumpSize);
            cueStartEnd.endPosition = newPosition.value();
        }
    }
    m_pCue->setStartAndEndPosition(cueStartEnd.startPosition, cueStartEnd.endPosition);
    updateTypeAndColorIfDefault(mixxx::CueType::Jump);
    slotUpdate();
}

void WCueMenuPopup::slotSavedJumpCueManual() {
    VERIFY_OR_DEBUG_ASSERT(m_pCue != nullptr) {
        return;
    }
    VERIFY_OR_DEBUG_ASSERT(m_pTrack != nullptr) {
        return;
    }
    auto cueStartEnd = m_pCue->getStartAndEndPosition();
    // If we are changing the cue type from a loop, we need to permute the position
    if (m_pCue->getType() == mixxx::CueType::Loop) {
        auto endPosition = cueStartEnd.endPosition;
        cueStartEnd.endPosition = cueStartEnd.startPosition;
        cueStartEnd.startPosition = endPosition;
    }
    auto newPosition = getCurrentPlayPositionWithQuantize();
    if (!newPosition.has_value() || newPosition == cueStartEnd.startPosition) {
        return;
    }
    cueStartEnd.endPosition = newPosition.value();
    m_pCue->setStartAndEndPosition(cueStartEnd.startPosition, cueStartEnd.endPosition);
    updateTypeAndColorIfDefault(mixxx::CueType::Jump);
    slotUpdate();
}

void WCueMenuPopup::closeEvent(QCloseEvent* event) {
    if (m_pTrack && m_pCue) {
        // Check if this is a hotcue, and -if yes- if it has an end position
        // from previous loop cue or temporary state.
        // If yes, remove it.
        if (m_pCue->getType() == mixxx::CueType::HotCue && m_pCue->getEndPosition().isValid()) {
            m_pCue->setEndPosition(mixxx::audio::FramePos());
        }
    }
    emit aboutToHide();
    QWidget::closeEvent(event);
}
