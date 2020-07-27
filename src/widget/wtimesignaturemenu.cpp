#include "wtimesignaturemenu.h"

#include <QtGui/QIntValidator>
#include <QtWidgets/QHBoxLayout>

namespace {
constexpr int kMinBeatsPerBar = 1;
constexpr int kMaxBeatsPerBar = 32;
} // namespace

WTimeSignatureMenu::WTimeSignatureMenu(QWidget* parent)
        : QWidget(parent),
          m_pBeatCountBox(make_parented<QSpinBox>(this)),
          m_pBeatLengthBox(make_parented<QComboBox>(this)),
          m_beat(mixxx::kInvalidFramePos) {
    hide();
    setWindowFlags(Qt::Popup);
    setAttribute(Qt::WA_StyledBackground);
    setObjectName("WTimeSignatureMenu");

    QHBoxLayout* pMainLayout = new QHBoxLayout();
    pMainLayout->addWidget(m_pBeatCountBox);
    pMainLayout->addWidget(m_pBeatLengthBox);
    setLayout(pMainLayout);
    connect(m_pBeatCountBox,
            QOverload<int>::of(&QSpinBox::valueChanged),
            this,
            &WTimeSignatureMenu::slotBeatCountChanged);

    // 2^comboBoxIndex corresponds to beat size
    m_pBeatLengthBox->addItem("1");
    m_pBeatLengthBox->addItem("2");
    m_pBeatLengthBox->addItem("4");
    m_pBeatLengthBox->addItem("8");
    m_pBeatLengthBox->addItem("16");
    m_pBeatLengthBox->addItem("32");

    connect(m_pBeatLengthBox,
            QOverload<int>::of(&QComboBox::activated),
            this,
            &WTimeSignatureMenu::slotBeatSizeChanged);

    m_pBeatCountBox->setMinimum(kMinBeatsPerBar);
    m_pBeatCountBox->setMaximum(kMaxBeatsPerBar);
}

WTimeSignatureMenu::~WTimeSignatureMenu() {
}

void WTimeSignatureMenu::slotBeatCountChanged(int value) {
    mixxx::TimeSignature newTimeSignature(value, m_beat.getTimeSignature().getNoteValue());
    setTimeSignature(newTimeSignature);
}

void WTimeSignatureMenu::slotBeatSizeChanged(int index) {
    int beatSize = pow(2, index);
    mixxx::TimeSignature newTimeSignature(m_beat.getTimeSignature().getBeatsPerBar(), beatSize);
    setTimeSignature(newTimeSignature);
}

void WTimeSignatureMenu::setTimeSignature(mixxx::TimeSignature timeSignature) {
    if (m_beat.getFramePosition() != mixxx::kInvalidFramePos &&
            kMinBeatsPerBar <= timeSignature.getBeatsPerBar() &&
            timeSignature.getBeatsPerBar() <= kMaxBeatsPerBar) {
        m_pBeats->setSignature(timeSignature, m_beat.getBarIndex());
    }
}

void WTimeSignatureMenu::setBeat(mixxx::Beat beat) {
    m_beat = beat;
    m_pBeatCountBox->setValue(beat.getTimeSignature().getBeatsPerBar());
    m_pBeatLengthBox->setCurrentIndex(
            static_cast<int>(log2(beat.getTimeSignature().getNoteValue())));
}

void WTimeSignatureMenu::popup(const QPoint& p) {
    m_pBeatCountBox->setValue(m_beat.getTimeSignature().getBeatsPerBar());
    auto parentWidget = static_cast<QWidget*>(parent());
    QPoint topLeft = mixxx::widgethelper::mapPopupToScreen(*parentWidget, p, size());
    move(topLeft);
    show();
}