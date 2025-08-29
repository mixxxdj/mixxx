#include <widget/wbpmeditor.h>

#include <QHBoxLayout>
#include <QMouseEvent>
#include <QStackedLayout>

#include "control/controlobject.h"
#include "moc_wbpmeditor.cpp"
#include "skin/legacy/skincontext.h"

namespace {
// Intervals for the hide timer
// Use same timeout as the WTrackProperty select timer for consistency
const int kHoverLeaveTimeout = 2000;
const int kInactiveTimeout = 5000;

// Property name for the BPM/rate spinbox
const char* kOrigVal("originalValue");
const double kBpmStepSize = 1.0;
const double kRateStepSize = 0.01; // +- 1 %

} // anonymous namespace

void TapPushButton::mousePressEvent(QMouseEvent* e) {
    if (e->type() == QEvent::MouseButtonPress && e->button() == Qt::RightButton) {
        emit rightClicked();
        return;
    }
    QPushButton::mousePressEvent(e);
}

void BpmSpinBox::stepBy(int steps) {
    QDoubleSpinBox::stepBy(steps);
    emit stepped(steps);
}

WBpmEditor::WBpmEditor(const QString& group, QWidget* pParent)
        : WWidget(pParent),
          m_tempoTapCO(group, QStringLiteral("tempo_tap")),
          m_bpmTapCO(group, QStringLiteral("bpm_tap")),
          // FIXME make it a ControlProxy to quit editor when track is unloaded so we
          // don't accidentally set the file BPM while 'somehow' the track is swapped?
          m_trackLoadedCO(group, QStringLiteral("track_loaded")),
          m_bpmCO(group, QStringLiteral("bpm")),
          m_fileBpmCO(group, QStringLiteral("file_bpm")),
          m_rateRatioCO(group, QStringLiteral("rate_ratio")),
          m_pClickOverlay(nullptr),
          m_pSelectWidget(nullptr),
          m_pTapSelectButton(nullptr),
          m_pEditSelectButton(nullptr),
          m_pTapButton(nullptr),
          m_pEditBox(nullptr),
          m_spinboxDecimals(2) {
    setContentsMargins(0, 0, 0, 0);
    setFocusPolicy(Qt::NoFocus);
    // Setup the layout with the mode selectors (Tap|Edit), tap button and
    // tempo editor. Only one of these widgets can be visible.
    m_pClickOverlay = make_parented<QPushButton>(this);
    m_pClickOverlay->setObjectName(QStringLiteral("BpmClickOverlay"));
    m_pClickOverlay->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    m_pClickOverlay->setFocusPolicy(Qt::NoFocus);
    m_pClickOverlay->installEventFilter(this);
    connect(m_pClickOverlay.get(),
            &QPushButton::clicked,
            this,
            [this]() { switchMode(Mode::Select); });

    m_pTapSelectButton = make_parented<QPushButton>(this);
    m_pTapSelectButton->setObjectName(QStringLiteral("BpmTapSelectButton"));
    m_pTapSelectButton->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    m_pTapSelectButton->setFocusPolicy(Qt::NoFocus);
    m_pTapSelectButton->installEventFilter(this);
    connect(m_pTapSelectButton.get(),
            &QPushButton::clicked,
            this,
            [this]() { switchMode(Mode::Tap); });

    m_pEditSelectButton = make_parented<QPushButton>(this);
    m_pEditSelectButton->setObjectName(QStringLiteral("BpmEditSelectButton"));
    m_pEditSelectButton->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    m_pEditSelectButton->setFocusPolicy(Qt::NoFocus);
    m_pEditSelectButton->installEventFilter(this);
    connect(m_pEditSelectButton.get(),
            &QPushButton::clicked,
            this,
            [this]() { switchMode(Mode::Edit); });

    m_pSelectWidget = make_parented<QWidget>(this);
    m_pSelectWidget->installEventFilter(this);
    m_pSelectWidget->setFocusPolicy(Qt::NoFocus);
    auto pSelectLayout = std::make_unique<QHBoxLayout>();
    pSelectLayout->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
    pSelectLayout->setContentsMargins(0, 0, 0, 0);
    pSelectLayout->setSpacing(0);
    pSelectLayout->setSizeConstraint(QLayout::SetNoConstraint);
    pSelectLayout->addWidget(m_pTapSelectButton.get());
    pSelectLayout->addWidget(m_pEditSelectButton.get());
    pSelectLayout->update();
    pSelectLayout->activate();
    m_pSelectWidget->setLayout(pSelectLayout.release());

    m_pTapButton = make_parented<TapPushButton>(this);
    m_pTapButton->setObjectName(QStringLiteral("BpmTapButton"));
    m_pTapButton->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    connect(m_pTapButton.get(),
            &QPushButton::clicked,
            this,
            [this]() {
                // Tap engine BPM
                m_hideTimer.start(kInactiveTimeout);
                m_tempoTapCO.set(1.0);
                m_tempoTapCO.set(0.0);
            });
    connect(m_pTapButton.get(),
            &TapPushButton::rightClicked,
            this,
            [this]() {
                // Tap file BPM
                m_hideTimer.start(kInactiveTimeout);
                m_bpmTapCO.set(1.0);
                m_bpmTapCO.set(0.0);
            });

    m_pEditBox = make_parented<BpmSpinBox>(this);
    m_pEditBox->setObjectName(QStringLiteral("EditBox"));
    m_pEditBox->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    m_pEditBox->setLineeditAlignment(Qt::AlignHCenter | Qt::AlignTop);
    // Step size, min and max are set when the spinbox is set up in switchMode().
    // Stepping via Up/Down or scroll wheel is applied immediately.
    connect(m_pEditBox.get(),
            &BpmSpinBox::stepped,
            this,
            &WBpmEditor::applySpinboxSteps);
    // Use the event filter to catch Enter/Return presses to apply manually
    // entered values and close the editor, as well as Esc to quit.
    m_pEditBox->installEventFilter(this);
    // Note: don't connect to &QDoubleSpinBox::editingFinished as that would call
    // the apply slot twice: first when Enter is pressed and again when the spinbox
    // is hidden (FocusOut/hide event when the current widget is switched).

    m_pModeLayout = make_parented<QStackedLayout>(this);
    m_pModeLayout->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
    m_pModeLayout->setContentsMargins(0, 0, 0, 0);
    m_pModeLayout->setSpacing(0);
    m_pModeLayout->setSizeConstraint(QLayout::SetNoConstraint);
    m_pModeLayout->addWidget(m_pClickOverlay.get());
    m_pModeLayout->addWidget(m_pSelectWidget.get());
    m_pModeLayout->addWidget(m_pTapButton.get());
    m_pModeLayout->addWidget(m_pEditBox.get());
    m_pModeLayout->setCurrentWidget(m_pClickOverlay.get());
    setLayout(m_pModeLayout.get());
    layout()->update();
    layout()->activate();

    // The hide timer is started when switching to Select mode, in Tap mode when
    // the cursor leaves the Tap button or after N seconds of inactivity.
    m_hideTimer.setSingleShot(true);
    m_hideTimer.callOnTimeout(this, [this]() { switchMode(Mode::Listen); });
    installEventFilter(this);
}

void WBpmEditor::setup(const QDomNode& node, const SkinContext& context) {
    Q_UNUSED(context);

    // Number of digits after the decimal.
    context.hasNodeSelectInt(node, "NumberOfDigits", &m_spinboxDecimals);
    m_pEditBox->setDecimals(m_spinboxDecimals);

    // TODO Check if we need to set the scale factor for the spinbox
    // double scaleFactor = context.getScaleFactor();
    // setScaleFactor(scaleFactor);
    // m_pEditBox->setsc >setScaleFactor(scaleFactor);
}

void WBpmEditor::setTapButtonTooltip(const QString& tooltip) {
    m_pTapSelectButton.get()->setToolTip(tooltip);
    m_pTapButton.get()->setToolTip(tooltip);
}

void WBpmEditor::setEditButtonTooltip(const QString& tooltip) {
    m_pEditSelectButton.get()->setToolTip(tooltip);
    m_pEditBox.get()->setToolTip(tooltip);
}

bool WBpmEditor::eventFilter(QObject* pObj, QEvent* pEvent) {
    if (pObj == m_pEditBox.get() && pEvent->type() == QEvent::KeyPress) {
        // Esc will close & reset.
        // Any other keypress is forwarded.
        QKeyEvent* keyEvent = static_cast<QKeyEvent*>(pEvent);
        if (keyEvent->key() == Qt::Key_Escape) {
            switchMode(Mode::Listen);
            return true;
        } else if (keyEvent->key() == Qt::Key_Enter ||
                keyEvent->key() == Qt::Key_Return) {
            applySpinboxValueAndQuit();
        }
    } else if (pEvent->type() == QEvent::HoverEnter) {
        m_hideTimer.stop();
    } else if (pEvent->type() == QEvent::HoverLeave &&
            pObj != m_pEditBox.get()) {
        // Don't auto-quit the spinbox when while it focus
        m_hideTimer.start(kHoverLeaveTimeout);
    } else if (pEvent->type() == QEvent::FocusOut) {
        // Only the spinbox can have focus - quit and discard pending changes
        switchMode(Mode::Listen);
    }
    return QWidget::eventFilter(pObj, pEvent);
}

void WBpmEditor::startEditMode() {
    double fileBpm = m_fileBpmCO.get();
    // The `rateRange` ControlPotmeter allows 1-400 %.
    // For rate % these are the limits, for BPM the lower limit is 1 (to keep
    // the track playing)
    if (fileBpm == 0.0) { // Rate mode
        // Track has currently no BPM so we'll control the rate.
        // Note that this might change while we edit the rate, so we set
        // the '%' suffix (and we'll use it to detect BPM/rate mode when
        // committing the new value).
        // Also, clamp like rateRange.
        double currRate = m_rateRatioCO.get();
        m_pEditBox->setSuffix(" %");
        m_pEditBox->setMinimum(1);
        m_pEditBox->setMaximum(400);
        m_pEditBox->setSingleStep(kRateStepSize);
        m_pEditBox->setValue(currRate * 100);
        // The value may be cropped/rounded if the decimals don't fit in the box.
        // Store the rounded value for comparison when committing
        m_pEditBox->setProperty(kOrigVal, m_pEditBox->value());
    } else { // BPM mode
        double currBpm = m_bpmCO.get();
        m_pEditBox->setSuffix("");
        m_pEditBox->setMinimum(1);
        m_pEditBox->setMaximum(fileBpm * 4);
        m_pEditBox->setSingleStep(kBpmStepSize);
        m_pEditBox->setValue(currBpm);
        // The value may be cropped/rounded if the decimals don't fit in the box.
        // Store the rounded value for comparison when attempting to apply typed values.
        m_pEditBox->setProperty(kOrigVal, m_pEditBox->value());
    }

    m_pModeLayout->setCurrentWidget(m_pEditBox.get());
    m_pEditBox->setFocus();
    m_pEditBox->selectAll();
}

void WBpmEditor::applySpinboxSteps(int steps) {
    // This ignores the spinbox value after Up/Down steps and applies +- steps
    // sto the COs directly.
    // The purpose is to allow us to return to the initial BPM/rate, eg. center,
    // with Up/Down steps (without intermediate manual changes).
    // With default QDoubleSpinBox behavior this does not work due to rounding.

    if (m_pEditBox->suffix().isEmpty()) {
        // No suffix means we're in BPM mode
        double newValue = m_bpmCO.get() + (steps * kBpmStepSize);
        m_bpmCO.set(newValue);
        m_pEditBox->setValue(newValue);
    } else {
        // Rate mode
        double newValue = m_rateRatioCO.get() + (steps * kRateStepSize);
        m_rateRatioCO.set(newValue);
        m_pEditBox->setValue(newValue * 100);
    }
    m_pEditBox->setProperty(kOrigVal, m_pEditBox->value());
}

void WBpmEditor::applySpinboxValueAndQuit() {
    // Filter no-op: avoid shifting rate/BPM due to rounding when the
    // original value has more decimals than the spinbox can display
    const QVariant origValueVar = m_pEditBox->property(kOrigVal);
    VERIFY_OR_DEBUG_ASSERT(origValueVar.isValid() && origValueVar.canConvert<double>()) {
        // Something went wrong during spinbox setup, quit.
        // This property is cleared when switching to Listen mode.
        switchMode(Mode::Listen);
        return;
    }

    double origValue = origValueVar.toDouble();
    double newValue = m_pEditBox->value();

    // Note: this comparison should suffice, no need for epsilon check to
    // account for floating point imprecision, eg. like this
    // fabs(newValue - origValue) < (1 / pow(10, m_spinboxDecimals))
    if (newValue == origValue) {
        switchMode(Mode::Listen);
        return;
    }

    if (m_pEditBox->suffix().isEmpty()) {
        // No suffix means we're in BPM mode
        m_bpmCO.set(newValue);
    } else {
        // Rate mode
        m_rateRatioCO.set(newValue / 100);
    }

    switchMode(Mode::Listen);
}

void WBpmEditor::switchMode(Mode mode) {
    m_hideTimer.stop();
    if (!m_trackLoadedCO.toBool()) {
        mode = Mode::Listen;
    }
    switch (mode) {
    case Mode::Select:
        m_pModeLayout->setCurrentWidget(m_pSelectWidget.get());
        return;
    case Mode::Tap:
        m_pModeLayout->setCurrentWidget(m_pTapButton.get());
        return;
    case Mode::Edit: {
        startEditMode();
        return;
    }
    case Mode::Listen:
    default:
        m_pEditBox->setProperty(kOrigVal, QVariant());
        m_pModeLayout->setCurrentWidget(m_pClickOverlay.get());
        ControlObject::set(ConfigKey(QStringLiteral("[Library]"),
                                   QStringLiteral("refocus_prev_widget")),
                1);
        return;
    }
}
