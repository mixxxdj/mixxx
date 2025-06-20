#include "widget/weffectparameternamebase.h"

#include <QDrag>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QMimeData>

#include "effects/effectknobparameterslot.h"
#include "effects/effectparameterslotbase.h"
#include "effects/effectslot.h"
#include "moc_weffectparameternamebase.cpp"
#include "util/dnd.h"
#include "util/math.h"

namespace {
const QString kMimeTextDelimiter = QStringLiteral("\n");
} // anonymous namespace

WEffectParameterNameBase::WEffectParameterNameBase(
        QWidget* pParent, EffectsManager* pEffectsManager)
        : WLabel(pParent),
          m_pEffectsManager(pEffectsManager),
          m_widthHint(0) {
    setAcceptDrops(true);
    setCursor(Qt::OpenHandCursor);
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
        if (qobject_cast<EffectKnobParameterSlot*>(m_pParameterSlot.data())) {
            // Make connection to show parameter value instead of name briefly
            // after value has changed.
            connect(m_pParameterSlot.data(),
                    &EffectParameterSlotBase::valueChanged,
                    this,
                    &WEffectParameterNameBase::showNewValue);
        }
    }
    parameterUpdated();
}

void WEffectParameterNameBase::parameterUpdated() {
    int valueWidth = 0;
    QFontMetrics metrics(font());
    if (m_pParameterSlot) {
        if (!m_pParameterSlot->shortName().isEmpty()) {
            m_text = m_pParameterSlot->shortName();
        } else {
            m_text = m_pParameterSlot->name();
        }
        setBaseTooltip(QStringLiteral("%1\n%2").arg(
                m_pParameterSlot->name(),
                m_pParameterSlot->description()));
        EffectManifestParameterPointer pManifest = m_pParameterSlot->getManifest();
        if (!pManifest.isNull()) {
            m_unitString = m_pParameterSlot->getManifest()->unitString();
            if (!m_unitString.isEmpty()) {
                m_unitString.prepend(QChar(' '));
            }
            double maxValue = m_pParameterSlot->getManifest()->getMaximum();
            double minValue = m_pParameterSlot->getManifest()->getMaximum();
            QString maxValueString = QString::number(maxValue - 0.01) + m_unitString;
            QString minValueString = QString::number(minValue + 0.01) + m_unitString;
            valueWidth = math_max(
                    metrics.size(0, maxValueString).width(),
                    metrics.size(0, minValueString).width());
        } else {
            m_unitString = QString();
        }
    } else {
        m_unitString = QString();
        m_text = kNoEffectString;
        setBaseTooltip(tr("No effect loaded."));
    }
    // frameWidth() is the maximum of the sum of margin, border and padding
    // width of the left and the right side.
    m_widthHint = math_max(
                          valueWidth,
                          metrics.size(0, m_text).width()) +
            2 * frameWidth();
    setText(m_text);
    m_parameterUpdated = true;
}

void WEffectParameterNameBase::showNewValue(double newValue) {
    // Don't show the value for a newly loaded parameter. 'valueChanged' is emitted
    // if this parameter is linked to the Meta knob
    if (m_parameterUpdated) {
        m_parameterUpdated = false;
        return;
    }
    int absVal = abs(static_cast<int>(round(newValue)));
    int tenPowDecimals = 1; // omit decimals
    if (absVal < 100) {     // 0-99: round to 2 decimals
        tenPowDecimals = 100;
    } else if (absVal < 1000) { // 100-999: round to 1 decimal
        tenPowDecimals = 10;
    }
    double dispVal = round(newValue * tenPowDecimals) / tenPowDecimals;

    setText(QString::number(dispVal) + m_unitString);
    m_displayNameResetTimer.start();
}

void WEffectParameterNameBase::mousePressEvent(QMouseEvent* pEvent) {
    VERIFY_OR_DEBUG_ASSERT(m_pParameterSlot && m_pParameterSlot->isLoaded()) {
        return;
    }
    VERIFY_OR_DEBUG_ASSERT(m_pEffectSlot && m_pEffectSlot->isLoaded()) {
        return;
    }
    DragAndDropHelper::mousePressed(pEvent);
}

void WEffectParameterNameBase::mouseMoveEvent(QMouseEvent* pEvent) {
    if (DragAndDropHelper::mouseMoveInitiatesDrag(pEvent)) {
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

void WEffectParameterNameBase::dragEnterEvent(QDragEnterEvent* pEvent) {
    VERIFY_OR_DEBUG_ASSERT(m_pParameterSlot && m_pParameterSlot->isLoaded()) {
        return;
    }
    VERIFY_OR_DEBUG_ASSERT(m_pEffectSlot && m_pEffectSlot->isLoaded()) {
        return;
    }
    const QString& mimeText = pEvent->mimeData()->text();
    QStringList mimeTextLines = mimeText.split(kMimeTextDelimiter);
    if (mimeTextLines.at(0) == mimeTextIdentifier() &&
            mimeTextLines.at(1) == m_pEffectSlot->getManifest()->uniqueId()) {
        pEvent->acceptProposedAction();
    }
}

void WEffectParameterNameBase::dropEvent(QDropEvent* pEvent) {
    VERIFY_OR_DEBUG_ASSERT(m_pParameterSlot && m_pParameterSlot->isLoaded()) {
        return;
    }
    VERIFY_OR_DEBUG_ASSERT(m_pEffectSlot && m_pEffectSlot->isLoaded()) {
        return;
    }
    const QString& mimeText = pEvent->mimeData()->text();
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

QSize WEffectParameterNameBase::sizeHint() const {
    // make sure the sizeHint is not changing because of the label or value string
    QSize size = WLabel::sizeHint();
    size.setWidth(m_widthHint);
    return size;
}
