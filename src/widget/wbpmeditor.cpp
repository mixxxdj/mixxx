#include <widget/wbpmeditor.h>

#include <QDoubleSpinBox>
#include <QHBoxLayout>
#include <QMouseEvent>
#include <QStackedLayout>

#include "control/controlobject.h"
#include "moc_wbpmeditor.cpp"
#include "skin/legacy/skincontext.h"

namespace {
// property name for the BPM/rate spinbox
const char* kOrigVal("originalValue");
} // anonymous namespace

void TapPushButton::mousePressEvent(QMouseEvent* e) {
    if (e->type() == QEvent::MouseButtonPress && e->button() == Qt::RightButton) {
        emit rightClicked();
        return;
    }
    QPushButton::mousePressEvent(e);
}

WBpmEditor::WBpmEditor(const QString& group, QWidget* pParent)
        : WWidget(pParent),
          m_group(group),
          m_tempoTapCO(m_group, QStringLiteral("tempo_tap")),
          m_bpmTapCO(m_group, QStringLiteral("bpm_tap")),
          // TODO ControlProxy, quit editor when track is unloaded?
          m_trackLoadedCO(m_group, QStringLiteral("track_loaded")),
          m_bpmCO(m_group, QStringLiteral("bpm")),
          m_fileBpmCO(m_group, QStringLiteral("file_bpm")),
          m_rateRatioCO(m_group, QStringLiteral("rate_ratio")),
          m_pClickOverlay(nullptr),
          m_pSelectWidget(nullptr),
          m_pTapSelectButton(nullptr),
          m_pEditSelectButton(nullptr),
          m_pTapButton(nullptr),
          m_pEditBox(nullptr) {
    setContentsMargins(0, 0, 0, 0);
    // Setup the layout with the mode selectors (Tap|Edit), tap button and
    // tempo editor. Only one of these widgets can be visible.
    m_pClickOverlay = make_parented<QPushButton>(this);
    m_pClickOverlay->setObjectName(QStringLiteral("BpmClickButton"));
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
                m_hideTimer.start(5000);
                m_tempoTapCO.set(1.0);
                m_tempoTapCO.set(0.0);
            });
    connect(m_pTapButton.get(),
            &TapPushButton::rightClicked,
            this,
            [this]() {
                m_hideTimer.start(5000);
                m_bpmTapCO.set(1.0);
                m_bpmTapCO.set(0.0);
            });

    m_pEditBox = make_parented<QDoubleSpinBox>(this);
    m_pEditBox->setObjectName(QStringLiteral("EditBox"));
    m_pEditBox->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    m_pEditBox->setDecimals(5);
    // min and max are set when the spinbox is set up in switchMode()
    m_pEditBox->installEventFilter(this);
    connect(m_pEditBox.get(),
            &QDoubleSpinBox::editingFinished,
            this,
            &WBpmEditor::slotEditigFinished);

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

    // The hide timer is started when starting select mode and
    // in tap mode when the cursor leaves the Tap button.
    // Use same timeout as WTrackProperty select timer for consistency
    m_hideTimer.setSingleShot(true);
    m_hideTimer.callOnTimeout(this, [this]() { switchMode(Mode::Listen); });
    installEventFilter(this);
}

void WBpmEditor::setup(const QDomNode& node, const SkinContext& context) {
    Q_UNUSED(node);
    Q_UNUSED(context);
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
        }
    } else if (pEvent->type() == QEvent::HoverEnter) {
        m_hideTimer.stop();
    } else if (pEvent->type() == QEvent::HoverLeave) {
        m_hideTimer.start(2000);
    }
    return QWidget::eventFilter(pObj, pEvent);
}

void WBpmEditor::slotEditigFinished() {
    switchMode(Mode::Listen);

    // Filter no-op: avoid shifting rate/BPM due to rounding when the
    // original value has more decimals than the spinbox can display
    QVariant origValueVar = m_pEditBox->property(kOrigVal);
    VERIFY_OR_DEBUG_ASSERT(origValueVar.isValid() && origValueVar.canConvert<double>()) {
        // Something went wrong during spinbox setuo, quit.
        return;
    }
    double origValue = origValueVar.toDouble();
    double mag = pow(10, m_pEditBox->decimals());
    double roundedOrigValue = std::round(origValue * mag) / mag;
    // Validate, don't accept the QValidator's fixup(), ie. don't reset
    // invalid values to current min/max.
    double newValue = m_pEditBox->value();
    double fileBpm = m_fileBpmCO.get();

    if (newValue == roundedOrigValue) {
        return;
    }

    if (fileBpm != 0.0) {
        // treat value as BPM
        m_bpmCO.set(newValue);
    } else {
        // treat value as rate with optional '%' suffix
        m_rateRatioCO.set(newValue / 100);
    }
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
        double fileBpm = m_fileBpmCO.get();
        // The `rateRange` ControlPotmeter allows 1-400 %.
        // For rate % these are the limits, for BPM the lower limit is 1.
        if (fileBpm != 0.0) {
            double currBpm = m_bpmCO.get();
            m_pEditBox->setSuffix("");
            m_pEditBox->setMinimum(1);
            m_pEditBox->setMaximum(fileBpm * 4);
            m_pEditBox->setProperty(kOrigVal, currBpm);
            m_pEditBox->setValue(currBpm);
        } else {
            // Track has currently no BPM.
            // Add '%' suffix and clamp like rateRange.
            double currRate = m_rateRatioCO.get();
            m_pEditBox->setSuffix(" %");
            m_pEditBox->setMinimum(1);
            m_pEditBox->setMaximum(400);
            m_pEditBox->setValue(currRate * 100);
            m_pEditBox->setProperty(kOrigVal, currRate * 100);
        }

        m_pModeLayout->setCurrentWidget(m_pEditBox.get());
        m_pEditBox->setFocus();
        m_pEditBox->selectAll();
        return;
    }
    case Mode::Listen:
    default:
        m_pModeLayout->setCurrentWidget(m_pClickOverlay.get());
        ControlObject::set(ConfigKey(QStringLiteral("[Library]"),
                                   QStringLiteral("refocus_prev_widget")),
                1);
        return;
    }
}
