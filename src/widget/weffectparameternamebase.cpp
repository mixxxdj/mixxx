#include "widget/weffectparameternamebase.h"

#include <QDrag>
#include <QMimeData>
#include <QtDebug>

#include "effects/effectslot.h"
#include "effects/effectsmanager.h"

namespace {
const QString kMimeTextDelimiter = QStringLiteral("\n");
} // anonymous namespace

WEffectParameterNameBase::WEffectParameterNameBase(
        QWidget* pParent, EffectsManager* pEffectsManager)
        : WLabel(pParent), m_pEffectsManager(pEffectsManager) {
    setAcceptDrops(true);
    parameterUpdated();
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
            setText(m_pParameterSlot->shortName());
        } else {
            setText(m_pParameterSlot->name());
        }
        setBaseTooltip(QString("%1\n%2").arg(
                m_pParameterSlot->name(),
                m_pParameterSlot->description()));
    } else {
        setText(kNoEffectString);
        setBaseTooltip(tr("No effect loaded."));
    }
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
