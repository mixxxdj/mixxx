#include "widget/wcuemenupopup.h"

#include <QHBoxLayout>
#include <QInputDialog>
#include <QVBoxLayout>

#include "engine/engine.h"
#include "util/color/color.h"

WCueMenuPopup::WCueMenuPopup(QWidget* parent)
        : QWidget(parent) {
    QWidget::hide();
    setWindowFlags(Qt::Popup);
    setObjectName("WCueMenuPopup");

    m_pCueNumber = new QLabel(this);
    m_pCueNumber->setToolTip(tr("cue number"));
    m_pCueNumber->setObjectName("CueNumberLabel");
    m_pCueNumber->setAlignment(Qt::AlignLeft);

    m_pCuePosition = new QLabel(this);
    m_pCuePosition->setToolTip(tr("Cue position"));
    m_pCuePosition->setObjectName("CuePositionLabel");
    m_pCuePosition->setAlignment(Qt::AlignRight);

    m_pEditLabel = new QLineEdit(this);
    m_pEditLabel->setToolTip(tr("Edit cue label"));
    m_pEditLabel->setObjectName("CueLabelEdit");
    connect(m_pEditLabel, &QLineEdit::textEdited, this, &WCueMenuPopup::slotEditLabel);

    m_pColorPicker = new WColorPicker(this);
    m_pColorPicker->setObjectName("CueColorPicker");
    connect(m_pColorPicker, &WColorPicker::colorPicked, this, &WCueMenuPopup::slotChangeCueColor);

    m_pRemoveCue = new QPushButton("", this);
    m_pRemoveCue->setToolTip(tr("Remove this cue point"));
    m_pRemoveCue->setFixedHeight(m_pEditLabel->sizeHint().height());
    m_pRemoveCue->setObjectName("CueRemoveButton");
    connect(m_pRemoveCue, &QPushButton::clicked, this, &WCueMenuPopup::slotRemoveCue);

    QHBoxLayout* pLabelLayout = new QHBoxLayout();
    pLabelLayout->addWidget(m_pCueNumber);
    pLabelLayout->addStretch(1);
    pLabelLayout->addWidget(m_pCuePosition);

    QVBoxLayout* pLeftLayout = new QVBoxLayout();
    pLeftLayout->addLayout(pLabelLayout);
    pLeftLayout->addWidget(m_pEditLabel);
    pLeftLayout->addWidget(m_pColorPicker);

    QVBoxLayout* pRightLayout = new QVBoxLayout();
    pRightLayout->addWidget(m_pRemoveCue);
    pRightLayout->addStretch(1);

    QHBoxLayout* pMainLayout = new QHBoxLayout();
    pMainLayout->addLayout(pLeftLayout);
    pMainLayout->addSpacing(5);
    pMainLayout->addLayout(pRightLayout);
    setLayout(pMainLayout);
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
            positionText = mixxx::Duration::formatTime(startPosition / m_pTrack->getSampleRate() / mixxx::kEngineChannelCount);
            if (endPosition != Cue::kNoPosition) {
                positionText = QString("%1 - %2").arg(
                    positionText,
                    mixxx::Duration::formatTime(endPosition / m_pTrack->getSampleRate() / mixxx::kEngineChannelCount)
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
        m_pColorPicker->setSelectedColor();
    }
}

void WCueMenuPopup::slotEditLabel() {
    VERIFY_OR_DEBUG_ASSERT(m_pCue != nullptr) {
        return;
    }
    m_pCue->setLabel(m_pEditLabel->text());
}

void WCueMenuPopup::slotChangeCueColor(PredefinedColorPointer pColor) {
    VERIFY_OR_DEBUG_ASSERT(m_pCue != nullptr) {
        return;
    }
    VERIFY_OR_DEBUG_ASSERT(pColor != nullptr) {
        return;
    }
    m_pCue->setColor(pColor);
    m_pColorPicker->setSelectedColor(pColor);
    hide();
}

void WCueMenuPopup::slotRemoveCue() {
    VERIFY_OR_DEBUG_ASSERT(m_pCue != nullptr) {
        return;
    }
    VERIFY_OR_DEBUG_ASSERT(m_pTrack != nullptr) {
        return;
    }
    m_pTrack->removeCue(m_pCue);
    hide();
}
