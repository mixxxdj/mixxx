#include "wtimesignaturemenu.h"

#include <QtWidgets/QPushButton>
#include <QtGui/QIntValidator>
#include <QtWidgets/QHBoxLayout>

namespace {
constexpr int kMinBeatsPerBar = 1;
constexpr int kMaxBeatsPerBar = 32;
constexpr int kMaxBeatLengthFractionDenominator = 32;
} // namespace

WTimeSignatureMenu::WTimeSignatureMenu(QWidget* parent)
        : QWidget(parent),
          m_pBeatCountBox(make_parented<QSpinBox>(this)),
          m_pBeatLengthBox(make_parented<QComboBox>(this)),
          m_pHalfButton(make_parented<QPushButton>(this)),
          m_pDoubleButton(make_parented<QPushButton>(this)),
          m_beat(mixxx::kInvalidFramePos) {
    hide();
    setWindowFlags(Qt::Popup);
    setAttribute(Qt::WA_StyledBackground);
    setObjectName("WTimeSignatureMenu");

    connect(m_pBeatCountBox,
            QOverload<int>::of(&QSpinBox::valueChanged),
            this,
            &WTimeSignatureMenu::slotBeatCountChanged);

    // 2^comboBoxIndex corresponds to beat size
    int beatLengthFractionDenominator = 1;
    while (beatLengthFractionDenominator <= kMaxBeatLengthFractionDenominator) {
        m_pBeatLengthBox->addItem(QString::number(beatLengthFractionDenominator));
        beatLengthFractionDenominator *= 2;
    }

    connect(m_pBeatLengthBox,
            QOverload<int>::of(&QComboBox::activated),
            this,
            &WTimeSignatureMenu::slotBeatSizeChanged);

    m_pBeatCountBox->setMinimum(kMinBeatsPerBar);
    m_pBeatCountBox->setMaximum(kMaxBeatsPerBar);

    m_pHalfButton->setText("/2");
    m_pHalfButton->setToolTip(tr("Halve beats per bar and double the note size."));
    m_pDoubleButton->setText("x2");
    m_pDoubleButton->setToolTip(tr("Double beats per bar and halve the note size."));

    connect(m_pHalfButton, &QPushButton::clicked, this, &WTimeSignatureMenu::slotTimeSignatureHalved);
    connect(m_pDoubleButton, &QPushButton::clicked, this, &WTimeSignatureMenu::slotTimeSignatureDoubled);

    parented_ptr<QVBoxLayout> pMainLayout = make_parented<QVBoxLayout>(this);
    parented_ptr<QHBoxLayout> pBasicControlsContainer = make_parented<QHBoxLayout>(pMainLayout->widget());
    parented_ptr<QHBoxLayout> pHalfDoubleButtonsContainer = make_parented<QHBoxLayout>(pMainLayout->widget());
    pBasicControlsContainer->addWidget(m_pBeatCountBox);
    pBasicControlsContainer->addWidget(m_pBeatLengthBox);
    pHalfDoubleButtonsContainer->addWidget(m_pHalfButton);
    pHalfDoubleButtonsContainer->addWidget(m_pDoubleButton);
    pMainLayout->addLayout(pBasicControlsContainer);
    pMainLayout->addLayout(pHalfDoubleButtonsContainer);
    setLayout(pMainLayout);
}

WTimeSignatureMenu::~WTimeSignatureMenu() {
}

void WTimeSignatureMenu::slotBeatCountChanged(int value) {
    mixxx::TimeSignature newTimeSignature(value, m_beat->timeSignature().getNoteValue());
    setTimeSignature(newTimeSignature);
}

void WTimeSignatureMenu::slotBeatSizeChanged(int index) {
    int beatSize = pow(2, index);
    mixxx::TimeSignature newTimeSignature(m_beat->timeSignature().getBeatsPerBar(), beatSize);
    setTimeSignature(newTimeSignature);
}

void WTimeSignatureMenu::setTimeSignature(mixxx::TimeSignature timeSignature) {
    if (m_pBeats && m_beat->framePosition() != mixxx::kInvalidFramePos &&
            kMinBeatsPerBar <= timeSignature.getBeatsPerBar() &&
            timeSignature.getBeatsPerBar() <= kMaxBeatsPerBar) {
        m_pBeats->setSignature(timeSignature, m_beat->barIndex());
    }
    updateHalfDoubleButtonsActiveStatus();
}

void WTimeSignatureMenu::setBeat(std::optional<mixxx::Beat> beat) {
    m_beat = beat;
    m_pBeatCountBox->setValue(beat->timeSignature().getBeatsPerBar());
    m_pBeatLengthBox->setCurrentIndex(
            static_cast<int>(log2(beat->timeSignature().getNoteValue())));
    updateHalfDoubleButtonsActiveStatus();
}

void WTimeSignatureMenu::popup(const QPoint& p) {
    m_pBeatCountBox->setValue(m_beat->timeSignature().getBeatsPerBar());
    auto parentWidget = dynamic_cast<QWidget*>(parent());
    QPoint topLeft = mixxx::widgethelper::mapPopupToScreen(*parentWidget, p, size());
    move(topLeft);
    show();
}

void WTimeSignatureMenu::slotTimeSignatureHalved() {
    auto currentTimeSignature = m_beat->timeSignature();
    int currentBeatsPerBar = currentTimeSignature.getBeatsPerBar();
    int currentNoteValue = currentTimeSignature.getNoteValue();
    if (canHalveBothValues()) {
        auto newTimeSignature = mixxx::TimeSignature(currentBeatsPerBar / 2, currentNoteValue / 2);
        setTimeSignature(newTimeSignature);
    } else {
        qWarning() << "Attempt to halve both time signature numbers is invalid.";
    }
}

void WTimeSignatureMenu::slotTimeSignatureDoubled() {
    auto currentTimeSignature = m_beat->timeSignature();
    int newBeatsPerBar = currentTimeSignature.getBeatsPerBar() * 2;
    int newNoteValue = currentTimeSignature.getNoteValue() * 2;
    if (canDoubleBothValues()) {
        auto newTimeSignature = mixxx::TimeSignature(newBeatsPerBar, newNoteValue);
        setTimeSignature(newTimeSignature);
    } else {
        qWarning() << "Attempt to double both time signature numbers is invalid.";
    }
}

bool WTimeSignatureMenu::canHalveBothValues() const {
    auto currentTimeSignature = m_beat->timeSignature();
    int currentBeatsPerBar = currentTimeSignature.getBeatsPerBar();
    int currentNoteValue = currentTimeSignature.getNoteValue();
    return currentBeatsPerBar >= 2 && currentBeatsPerBar % 2 == 0 && currentNoteValue >= 2 && currentNoteValue % 2 == 0;
}

bool WTimeSignatureMenu::canDoubleBothValues() const {
    auto currentTimeSignature = m_beat->timeSignature();
    int newBeatsPerBar = currentTimeSignature.getBeatsPerBar() * 2;
    int newNoteValue = currentTimeSignature.getNoteValue() * 2;
    return newBeatsPerBar <= kMaxBeatsPerBar && newNoteValue <= kMaxBeatLengthFractionDenominator;
}

void WTimeSignatureMenu::updateHalfDoubleButtonsActiveStatus() {
    m_pHalfButton->setEnabled(canHalveBothValues());
    m_pDoubleButton->setEnabled(canDoubleBothValues());
}
