#include "widget/wcuemenupopup.h"

#include <QHBoxLayout>
#include <QInputDialog>
#include <QVBoxLayout>

#include "engine/engine.h"
#include "track/track.h"
#include "util/color/color.h"

WCueMenuPopup::WCueMenuPopup(UserSettingsPointer pConfig, QWidget* parent)
        : QWidget(parent),
          m_colorPaletteSettings(ColorPaletteSettings(pConfig)) {
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

void WCueMenuPopup::setTrackAndCue(TrackPointer pTrack, CuePointer pCue) {
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
        double startPosition = m_pCue->getPosition();
        double endPosition = m_pCue->getEndPosition();
        if (startPosition != Cue::kNoPosition) {
            double startPositionSeconds = startPosition / m_pTrack->getSampleRate() / mixxx::kEngineChannelCount;
            positionText = mixxx::Duration::formatTime(startPositionSeconds, mixxx::Duration::Precision::CENTISECONDS);
            if (endPosition != Cue::kNoPosition) {
                double endPositionSeconds = endPosition / m_pTrack->getSampleRate() / mixxx::kEngineChannelCount;
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

void WCueMenuPopup::closeEvent(QCloseEvent* event) {
    emit aboutToHide();
    QWidget::closeEvent(event);
}
