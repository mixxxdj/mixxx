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
          m_colorPaletteSettings(ColorPaletteSettings(pConfig)) {
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

    // Eve
    m_pEditStem1vol = std::make_unique<QLineEdit>(this);
    m_pEditStem1vol->setToolTip(tr("Edit cue Volume for Stem 1 (- for mute; 0-100 volume)"));
    m_pEditStem1vol->setObjectName("CueStem1volEdit");
    m_pEditStem1vol->setPlaceholderText(tr("Volume for Stem 1..."));
    m_pEditStem1vol->setMaxLength(10);
    m_pEditStem1vol->setMaximumSize(50, 20);
    connect(m_pEditStem1vol.get(), &QLineEdit::textEdited, this, &WCueMenuPopup::slotEditStem1vol);
    connect(m_pEditStem1vol.get(), &QLineEdit::returnPressed, this, &WCueMenuPopup::hide);

    m_pEditStem2vol = std::make_unique<QLineEdit>(this);
    m_pEditStem2vol->setToolTip(tr("Edit cue Volume for Stem 2 (- for mute; 0-100 volume)"));
    m_pEditStem2vol->setObjectName("CueStem2volEdit");
    m_pEditStem2vol->setPlaceholderText(tr("Volume for Stem 2..."));
    m_pEditStem2vol->setMaxLength(10);
    m_pEditStem2vol->setMaximumSize(50, 20);
    connect(m_pEditStem2vol.get(), &QLineEdit::textEdited, this, &WCueMenuPopup::slotEditStem2vol);
    connect(m_pEditStem2vol.get(), &QLineEdit::returnPressed, this, &WCueMenuPopup::hide);

    m_pEditStem3vol = std::make_unique<QLineEdit>(this);
    m_pEditStem3vol->setToolTip(tr("Edit cue Volume for Stem 3 (- for mute; 0-100 volume)"));
    m_pEditStem3vol->setObjectName("CueStem3volEdit");
    m_pEditStem3vol->setPlaceholderText(tr("Volume for Stem 3..."));
    m_pEditStem3vol->setMaxLength(10);
    m_pEditStem3vol->setMaximumSize(50, 20);
    connect(m_pEditStem3vol.get(), &QLineEdit::textEdited, this, &WCueMenuPopup::slotEditStem3vol);
    connect(m_pEditStem3vol.get(), &QLineEdit::returnPressed, this, &WCueMenuPopup::hide);

    m_pEditStem4vol = std::make_unique<QLineEdit>(this);
    m_pEditStem4vol->setToolTip(tr("Edit cue Volume for Stem 4 (- for mute; 0-100 volume)"));
    m_pEditStem4vol->setObjectName("CueStem4volEdit");
    m_pEditStem4vol->setPlaceholderText(tr("Volume for Stem 4..."));
    m_pEditStem4vol->setMaxLength(10);
    m_pEditStem4vol->setMaximumSize(50, 20);
    connect(m_pEditStem4vol.get(), &QLineEdit::textEdited, this, &WCueMenuPopup::slotEditStem4vol);
    connect(m_pEditStem4vol.get(), &QLineEdit::returnPressed, this, &WCueMenuPopup::hide);

    // Eve

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

    QHBoxLayout* pLabelLayout = new QHBoxLayout();
    pLabelLayout->addWidget(m_pCueNumber.get());
    pLabelLayout->addStretch(1);
    pLabelLayout->addWidget(m_pCuePosition.get());

    // EVE
    QHBoxLayout* pStemvolLayout = new QHBoxLayout();
    pStemvolLayout->addWidget(m_pEditStem1vol.get(), 1);
    pStemvolLayout->addSpacing(5);
    pStemvolLayout->addWidget(m_pEditStem2vol.get(), 1);
    pStemvolLayout->addSpacing(5);
    pStemvolLayout->addWidget(m_pEditStem3vol.get(), 1);
    pStemvolLayout->addSpacing(5);
    pStemvolLayout->addWidget(m_pEditStem4vol.get(), 1);
    //    pStemvolLayout->maximumSize();
    // EVE

    QVBoxLayout* pLeftLayout = new QVBoxLayout();
    pLeftLayout->addLayout(pLabelLayout);
    pLeftLayout->addWidget(m_pEditLabel.get());
    pLeftLayout->addLayout(pStemvolLayout);
    pLeftLayout->addWidget(m_pColorPicker.get());

    QVBoxLayout* pRightLayout = new QVBoxLayout();
    pRightLayout->addWidget(m_pDeleteCue.get());
    pRightLayout->addStretch(1);
    pRightLayout->addWidget(m_pSavedLoopCue.get());

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
                positionText = QString("%1 - %2").arg(positionText,
                        mixxx::Duration::formatTime(endPositionSeconds,
                                mixxx::Duration::Precision::CENTISECONDS));
            }
        }
        m_pCuePosition->setText(positionText);

        m_pEditLabel->setText(m_pCue->getLabel());
        // Eve
        m_pEditStem1vol->setText(QString("%1").arg(round(m_pCue->getStem1vol() * 100)));
        m_pEditStem2vol->setText(QString("%1").arg(round(m_pCue->getStem2vol() * 100)));
        m_pEditStem3vol->setText(QString("%1").arg(round(m_pCue->getStem3vol() * 100)));
        m_pEditStem4vol->setText(QString("%1").arg(round(m_pCue->getStem4vol() * 100)));
        // Eve
        m_pColorPicker->setSelectedColor(m_pCue->getColor());
        m_pSavedLoopCue->setChecked(m_pCue->getType() == mixxx::CueType::Loop);
    } else {
        m_pTrack.reset();
        m_pCue.reset();
        m_pCueNumber->setText(QString(""));
        m_pCuePosition->setText(QString(""));
        m_pEditLabel->setText(QString(""));
        // Eve
        m_pEditStem1vol->setText(QString("100"));
        m_pEditStem2vol->setText(QString("100"));
        m_pEditStem3vol->setText(QString("100"));
        m_pEditStem4vol->setText(QString("100"));
        // Eve
        m_pColorPicker->setSelectedColor(std::nullopt);
    }
}

void WCueMenuPopup::slotEditLabel() {
    VERIFY_OR_DEBUG_ASSERT(m_pCue != nullptr) {
        return;
    }
    m_pCue->setLabel(m_pEditLabel->text());
}

// Eve
void WCueMenuPopup::slotEditStem1vol() {
    VERIFY_OR_DEBUG_ASSERT(m_pCue != nullptr) {
        return;
    }
    m_pCue->setStem1vol((m_pEditStem1vol->text()).toDouble() / 100);
}

void WCueMenuPopup::slotEditStem2vol() {
    VERIFY_OR_DEBUG_ASSERT(m_pCue != nullptr) {
        return;
    }
    m_pCue->setStem2vol((m_pEditStem2vol->text()).toDouble() / 100);
}

void WCueMenuPopup::slotEditStem3vol() {
    VERIFY_OR_DEBUG_ASSERT(m_pCue != nullptr) {
        return;
    }
    m_pCue->setStem3vol((m_pEditStem3vol->text()).toDouble() / 100);
}

void WCueMenuPopup::slotEditStem4vol() {
    VERIFY_OR_DEBUG_ASSERT(m_pCue != nullptr) {
        return;
    }
    m_pCue->setStem4vol((m_pEditStem4vol->text()).toDouble() / 100);
}
// Eve

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
    if (m_pCue->getType() == mixxx::CueType::Loop) {
        m_pCue->setType(mixxx::CueType::HotCue);
    } else {
        auto cueStartEnd = m_pCue->getStartAndEndPosition();
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
        m_pCue->setType(mixxx::CueType::Loop);
    }
    slotUpdate();
}

void WCueMenuPopup::slotSavedLoopCueManual() {
    VERIFY_OR_DEBUG_ASSERT(m_pCue != nullptr) {
        return;
    }
    VERIFY_OR_DEBUG_ASSERT(m_pTrack != nullptr) {
        return;
    }
    VERIFY_OR_DEBUG_ASSERT(m_pBeatLoopSize.valid()) {
        return;
    }
    const mixxx::BeatsPointer pBeats = m_pTrack->getBeats();
    auto position = mixxx::audio::FramePos::fromEngineSamplePos(
            m_pPlayPos.get() * m_pTrackSample.get());
    if (m_pQuantizeEnabled.toBool() && pBeats) {
        mixxx::audio::FramePos nextBeatPosition, prevBeatPosition;
        pBeats->findPrevNextBeats(position, &prevBeatPosition, &nextBeatPosition, false);
        position = (nextBeatPosition - position > position - prevBeatPosition)
                ? prevBeatPosition
                : nextBeatPosition;
    }
    if (position <= m_pCue->getPosition()) {
        return;
    }
    m_pCue->setEndPosition(position);
    m_pCue->setType(mixxx::CueType::Loop);
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
