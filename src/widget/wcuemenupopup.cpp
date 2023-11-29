#include "widget/wcuemenupopup.h"

#include <QHBoxLayout>
#include <QVBoxLayout>

#include "moc_wcuemenupopup.cpp"
#include "track/track.h"

WCueMenuPopup::WCueMenuPopup(UserSettingsPointer pConfig, const QString& group, QWidget* parent)
        : QWidget(parent),
          m_colorPaletteSettings(ColorPaletteSettings(pConfig)),
          m_pQuantize(group, QStringLiteral("quantize")) {
    QWidget::hide();
    setWindowFlags(Qt::Popup);
    setAttribute(Qt::WA_StyledBackground);
    setObjectName("WCueMenuPopup");

    // TODO Improve / polish Tab key navigation:
    // Order of widget creation apearantly determines the Tab key navigation order.
    // We may create the Delete and Shift buttons before the color picker so they're
    // first after the edit label. That way, if we want to delete or shift a cue
    // via keyboard we don't need to skip through the entire color button grid first.
    // Proper styles need to be added (per skin), e.g. remove 'outline: none' or
    // add a custom border to WCueMenuPopup QPushButton:focus.

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

    m_pShiftCueEarlier = new QPushButton("", this);
    m_pShiftCueEarlier->setToolTip(tr(
            "Shift this cue backwards by %1 milliseconds.\n"
            "If quantize is enabled shift this cue to the previous beat.")
                                           .arg(Cue::kShiftCuesOffsetMillis));
    m_pShiftCueEarlier->setObjectName("CueShiftEarlier");
    connect(m_pShiftCueEarlier,
            &QPushButton::clicked,
            this,
            [this]() {
                slotShiftCue(-1);
            });
    m_pShiftCueLater = new QPushButton("", this);
    m_pShiftCueLater->setToolTip(tr(
            "Shift this cue forwards by %1 milliseconds.\n"
            "If quantize is enabled shift this cue to the next beat.")
                                         .arg(Cue::kShiftCuesOffsetMillis));
    m_pShiftCueLater->setObjectName("CueShiftLater");
    connect(m_pShiftCueLater,
            &QPushButton::clicked,
            this,
            [this]() {
                slotShiftCue(1);
            });

    QHBoxLayout* pLabelLayout = new QHBoxLayout();
    pLabelLayout->addWidget(m_pCueNumber);
    pLabelLayout->addStretch(1);
    pLabelLayout->addWidget(m_pCuePosition);

    QVBoxLayout* pLeftLayout = new QVBoxLayout();
    pLeftLayout->addLayout(pLabelLayout);
    pLeftLayout->addWidget(m_pEditLabel);
    pLeftLayout->addWidget(m_pColorPicker);

    QHBoxLayout* pShiftLayout = new QHBoxLayout();
    pShiftLayout->setObjectName("CueMenuShiftLayout");
    pShiftLayout->addWidget(m_pShiftCueEarlier);
    pShiftLayout->addWidget(m_pShiftCueLater);
    // no margin, make it look like the beatjump button grid
    pShiftLayout->setSpacing(0);

    QVBoxLayout* pRightLayout = new QVBoxLayout();
    pRightLayout->addWidget(m_pDeleteCue);
    pRightLayout->addLayout(pShiftLayout);
    pRightLayout->addStretch(1);

    QHBoxLayout* pMainLayout = new QHBoxLayout();
    pMainLayout->addLayout(pLeftLayout);
    pMainLayout->addSpacing(5);
    pMainLayout->addLayout(pRightLayout);
    setLayout(pMainLayout);
    // we need to update the layout here since the size is used to
    // calculate the positioning later
    layout()->update();
    layout()->activate();
}

void WCueMenuPopup::setTrackAndCue(TrackPointer pTrack, const CuePointer& pCue) {
    if (pTrack && pCue) {
        m_pTrack = pTrack;
        m_pCue = pCue;

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

void WCueMenuPopup::slotShiftCue(int direction) {
    VERIFY_OR_DEBUG_ASSERT(m_pCue != nullptr) {
        return;
    }
    VERIFY_OR_DEBUG_ASSERT(m_pTrack != nullptr) {
        return;
    }
    if (direction == 0) {
        return;
    }

    if (m_pQuantize.toBool()) {
        m_pTrack->shiftCuePositionBeats(m_pCue, direction);
    } else {
        m_pTrack->shiftCuePositionMillis(m_pCue, Cue::kShiftCuesOffsetMillis * direction);
    }
}

void WCueMenuPopup::closeEvent(QCloseEvent* event) {
    emit aboutToHide();
    QWidget::closeEvent(event);
}
