#include "wtempomenu.h"

#include <QtGui/QDoubleValidator>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QPushButton>

namespace {
constexpr double kSlightChangeValue = 0.05;
constexpr double kBigChangeValue = 0.5;
// TODO(hacksdump): Read BPM range from preferences or some other central source
constexpr double kMinBpm = 60;
constexpr double kMaxBpm = 180;
constexpr int kBpmDigitsAfterDecimal = 3;
} // namespace

WTempoMenu::WTempoMenu(QWidget* parent)
        : QWidget(parent),
          m_pBpmEditBox(make_parented<QLineEdit>(this)),
          m_pBpmSlightDecreaseButton(make_parented<QPushButton>(this)),
          m_pBpmSlightIncreaseButton(make_parented<QPushButton>(this)),
          m_pBpmBigDecreaseButton(make_parented<QPushButton>(this)),
          m_pBpmBigIncreaseButton(make_parented<QPushButton>(this)) {
    hide();
    setWindowFlags(Qt::Popup);
    setAttribute(Qt::WA_StyledBackground);
    setObjectName("WTempoMenu");

    // TODO(hacksdump): Probably need to write a custom/regex validator since it won't allow typing decimal point.
    m_pBpmEditBox->setValidator(new QDoubleValidator(
            kMinBpm, kMaxBpm, kBpmDigitsAfterDecimal, this));

    parented_ptr<QVBoxLayout> pMainLayout = make_parented<QVBoxLayout>(this);
    parented_ptr<QHBoxLayout> pSlightAdjustmentButtonsLayout =
            make_parented<QHBoxLayout>(pMainLayout->widget());
    parented_ptr<QHBoxLayout> pBigAdjustmentButtonsLayout =
            make_parented<QHBoxLayout>(pMainLayout->widget());
    pMainLayout->addWidget(m_pBpmEditBox, Qt::AlignHCenter);
    m_pBpmSlightIncreaseButton->setText("+");
    m_pBpmSlightDecreaseButton->setText("-");
    pSlightAdjustmentButtonsLayout->addWidget(m_pBpmSlightDecreaseButton);
    pSlightAdjustmentButtonsLayout->addWidget(m_pBpmSlightIncreaseButton);
    m_pBpmBigIncreaseButton->setText("++");
    m_pBpmBigDecreaseButton->setText("--");
    pBigAdjustmentButtonsLayout->addWidget(m_pBpmBigDecreaseButton);
    pBigAdjustmentButtonsLayout->addWidget(m_pBpmBigIncreaseButton);
    pMainLayout->addLayout(pSlightAdjustmentButtonsLayout);
    pMainLayout->addLayout(pBigAdjustmentButtonsLayout);
    setLayout(pMainLayout);
    connect(m_pBpmEditBox,
            &QLineEdit::textChanged,
            this,
            &WTempoMenu::slotTextInput);
    connect(m_pBpmSlightDecreaseButton,
            &QPushButton::clicked,
            this,
            &WTempoMenu::slotSlightDecrease);
    connect(m_pBpmSlightIncreaseButton,
            &QPushButton::clicked,
            this,
            &WTempoMenu::slotSlightIncrease);
    connect(m_pBpmBigDecreaseButton,
            &QPushButton::clicked,
            this,
            &WTempoMenu::slotBigDecrease);
    connect(m_pBpmBigIncreaseButton,
            &QPushButton::clicked,
            this,
            &WTempoMenu::slotBigIncrease);
}

WTempoMenu::~WTempoMenu() {
}

void WTempoMenu::setBeat(std::optional<mixxx::Beat> beat) {
    m_beat = beat;
    if (m_beat) {
        m_pBpmEditBox->setText(QString::number(beat->bpm().getValue()));
    }
}

void WTempoMenu::popup(const QPoint& p) {
    auto parentWidget = dynamic_cast<QWidget*>(parent());
    QPoint topLeft = mixxx::widgethelper::mapPopupToScreen(*parentWidget, p, size());
    move(topLeft);
    show();
}

void WTempoMenu::setBpm(mixxx::Bpm bpm) {
    m_pBeats->setBpm(bpm, m_beat->beatIndex());
}

void WTempoMenu::slotTextInput(const QString& bpmString) {
    auto bpm = mixxx::Bpm(bpmString.toFloat());
    setBpm(bpm);
}

void WTempoMenu::slotSlightDecrease() {
    setBpm(mixxx::Bpm(m_beat->bpm().getValue() - kSlightChangeValue));
}

void WTempoMenu::slotSlightIncrease() {
    setBpm(mixxx::Bpm(m_beat->bpm().getValue() + kSlightChangeValue));
}

void WTempoMenu::slotBigDecrease() {
    setBpm(mixxx::Bpm(m_beat->bpm().getValue() - kBigChangeValue));
}

void WTempoMenu::slotBigIncrease() {
    setBpm(mixxx::Bpm(m_beat->bpm().getValue() + kBigChangeValue));
}
