#include "widget/weffectparameternamebase.h"

#include <QDrag>
#include <QMimeData>
#include <QtDebug>

#include "effects/effectslot.h"
#include "effects/effectsmanager.h"

namespace {
const QString kMimeTextDelimiter = QStringLiteral("\n");
// for rounding the value display to 2 decimals
constexpr int kValDecimals = 100;
} // anonymous namespace

WEffectParameterNameBase::WEffectParameterNameBase(
        QWidget* pParent, EffectsManager* pEffectsManager)
        : WLabel(pParent),
          m_pEffectsManager(pEffectsManager),
          m_text("") {
    setAcceptDrops(true);
    parameterUpdated();
    // When the parameter value changed it is display briefly.
    // Set up the timer that restores the parameter name.
    m_displayNameResetTimer.setSingleShot(true);
    m_displayNameResetTimer.setInterval(800);
    m_displayNameResetTimer.callOnTimeout(this, [this]() { setText(m_text); });
}

void WEffectParameterNameBase::setEffectParameterSlot(
        EffectParameterSlotBasePointer pEffectKnobParameterSlot) {
    m_pParameterSlot = pEffectKnobParameterSlot;
    if (m_pParameterSlot) {
        connect(m_pParameterSlot.data(),
                &EffectParameterSlotBase::updated,
                this,
                &WEffectParameterNameBase::parameterUpdated);
    }
    parameterUpdated();
}

void WEffectParameterNameBase::parameterUpdated() {
    if (m_pParameterSlot) {
        if (!m_pParameterSlot->shortName().isEmpty()) {
            m_text = m_pParameterSlot->shortName();
        } else {
            m_text = m_pParameterSlot->name();
        }
        setBaseTooltip(QString("%1\n%2").arg(
                m_pParameterSlot->name(),
                m_pParameterSlot->description()));
        // Make connection to show parameter value instead of name briefly
        // after value has changed.
        if (m_pParameterSlot->parameterType() == EffectParameterType::Knob) {
            connect(m_pParameterSlot.data(),
                    &EffectParameterSlotBase::valueChanged,
                    this,
                    &WEffectParameterNameBase::showNewValue);
        }
    } else {
        m_text = kNoEffectString;
        setBaseTooltip(tr("No effect loaded."));
    }
    setText(m_text);
}

void WEffectParameterNameBase::showNewValue(double newValue) {
    double newValRounded =
            std::ceil(newValue * kValDecimals) / kValDecimals;
    setText(QString::number(newValRounded));
    m_displayNameResetTimer.start();
}

void WEffectParameterNameBase::mousePressEvent(QMouseEvent* event) {
    VERIFY_OR_DEBUG_ASSERT(m_pParameterSlot && m_pParameterSlot->isLoaded()) {
        return;
    }
    VERIFY_OR_DEBUG_ASSERT(m_pEffectSlot && m_pEffectSlot->isLoaded()) {
        return;
    }
    if (event->button() == Qt::LeftButton) {
        QDrag* drag = new QDrag(this);
        QMimeData* mimeData = new QMimeData;

        mimeData->setText(
                mimeTextIdentifier() + kMimeTextDelimiter +
                m_pEffectSlot->getManifest()->uniqueId() + kMimeTextDelimiter +
                QString::number(m_pParameterSlot->slotNumber()));
        drag->setMimeData(mimeData);
        drag->exec();
    }
}

void WEffectParameterNameBase::dragEnterEvent(QDragEnterEvent* event) {
    VERIFY_OR_DEBUG_ASSERT(m_pParameterSlot && m_pParameterSlot->isLoaded()) {
        return;
    }
    VERIFY_OR_DEBUG_ASSERT(m_pEffectSlot && m_pEffectSlot->isLoaded()) {
        return;
    }
    const QString& mimeText = event->mimeData()->text();
    QStringList mimeTextLines = mimeText.split(kMimeTextDelimiter);
    if (mimeTextLines.at(0) == mimeTextIdentifier() &&
            mimeTextLines.at(1) == m_pEffectSlot->getManifest()->uniqueId()) {
        event->acceptProposedAction();
    }
}

void WEffectParameterNameBase::dropEvent(QDropEvent* event) {
    VERIFY_OR_DEBUG_ASSERT(m_pParameterSlot && m_pParameterSlot->isLoaded()) {
        return;
    }
    VERIFY_OR_DEBUG_ASSERT(m_pEffectSlot && m_pEffectSlot->isLoaded()) {
        return;
    }
    const QString& mimeText = event->mimeData()->text();
    QStringList mimeTextLines = mimeText.split(kMimeTextDelimiter);
    m_pEffectSlot->swapParameters(m_pParameterSlot->parameterType(),
            m_pParameterSlot->slotNumber(),
            mimeTextLines.at(2).toInt());
}

const QString WEffectParameterNameBase::mimeTextIdentifier() const {
    return QStringLiteral("Mixxx effect parameter ") +
            QString::number(
                    static_cast<int>(m_pParameterSlot->parameterType()));
}
