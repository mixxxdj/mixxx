#include "wtimesignaturemenu.h"

#include <QtGui/QIntValidator>
#include <QtWidgets/QHBoxLayout>

namespace {
constexpr int kMinBeatsPerBar = 2;
constexpr int kMaxBeatsPerBar = 32;
} // namespace

WTimeSignatureMenu::WTimeSignatureMenu(QWidget* parent)
        : QWidget(parent),
          m_pBeatCountBox(make_parented<QSpinBox>(this)),
          m_beat(mixxx::kInvalidFramePos) {
    hide();
    setWindowFlags(Qt::Popup);
    setAttribute(Qt::WA_StyledBackground);
    setObjectName("WTimeSignatureMenu");

    QHBoxLayout* pMainLayout = new QHBoxLayout();
    pMainLayout->addWidget(m_pBeatCountBox);
    setLayout(pMainLayout);
    connect(m_pBeatCountBox,
            QOverload<int>::of(&QSpinBox::valueChanged),
            this,
            &WTimeSignatureMenu::slotBeatCountChanged);

    m_pBeatCountBox->setMinimum(kMinBeatsPerBar);
    m_pBeatCountBox->setMaximum(kMaxBeatsPerBar);
}

WTimeSignatureMenu::~WTimeSignatureMenu() {
}

void WTimeSignatureMenu::slotBeatCountChanged(int value) {
    if (m_beat.getFramePosition() != mixxx::kInvalidFramePos &&
            kMinBeatsPerBar <= value && value <= kMaxBeatsPerBar) {
        mixxx::TimeSignature newTimeSignature(value, m_beat.getTimeSignature().getNoteValue());
        m_pBeats->setSignature(newTimeSignature, m_beat.getBarIndex());
    }
}

void WTimeSignatureMenu::setBeat(mixxx::Beat beat) {
    m_beat = beat;
    m_pBeatCountBox->setValue(beat.getTimeSignature().getBeatsPerBar());
}

void WTimeSignatureMenu::popup(const QPoint& p) {
    m_pBeatCountBox->setValue(m_beat.getTimeSignature().getBeatsPerBar());
    auto parentWidget = static_cast<QWidget*>(parent());
    QPoint topLeft = mixxx::widgethelper::mapPopupToScreen(*parentWidget, p, size());
    move(topLeft);
    show();
}
