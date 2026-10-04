#include "dialog/dlgautorecommend.h"

#include <QCheckBox>
#include <QCoreApplication>
#include <QDoubleSpinBox>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QSpinBox>
#include <QStringList>
#include <QVBoxLayout>

#include "moc_dlgautorecommend.cpp"
#include "util/assert.h"

namespace {

QString weightText(const QString& name, double weight) {
    if (weight <= 0.0) {
        return QCoreApplication::translate("DlgAutoRecommend", "%1: off")
                .arg(name);
    }
    return QCoreApplication::translate("DlgAutoRecommend", "%1: %2%")
            .arg(name, QString::number(qRound(weight * 100.0)));
}

QString statusText(
        const AutoRecommendBot* pBot,
        int recommendedCount,
        const QString& lastPreloadedDeckGroup,
        const QString& lastPreloadedTrackTitle) {
    const mixxx::AutoRecommendationWeights weights = pBot->weights();
    const QString state = pBot->isEnabled()
            ? QCoreApplication::translate(
                      "DlgAutoRecommend", "The bot is enabled.")
            : QCoreApplication::translate(
                      "DlgAutoRecommend", "The bot is disabled.");
    QStringList lines;
    lines << state
          << weightText(QCoreApplication::translate("DlgAutoRecommend", "Tempo"),
                  weights.tempo)
          << weightText(QCoreApplication::translate("DlgAutoRecommend", "Energy"),
                  weights.energy)
          << weightText(QCoreApplication::translate("DlgAutoRecommend", "Key"),
                  weights.key)
          << weightText(QCoreApplication::translate("DlgAutoRecommend", "Fame"),
                  weights.fame);
    if (recommendedCount > 0) {
        lines << QCoreApplication::translate(
                "DlgAutoRecommend",
                "Last run: added %1 recommendation(s) to the Auto DJ queue.")
                         .arg(recommendedCount);
    }
    if (!lastPreloadedTrackTitle.isEmpty()) {
        lines << QCoreApplication::translate(
                "DlgAutoRecommend",
                "Last preload: \"%1\" loaded into %2.")
                          .arg(lastPreloadedTrackTitle, lastPreloadedDeckGroup);
    }
    return lines.join(QLatin1Char('\n'));
}

} // anonymous namespace

DlgAutoRecommend::DlgAutoRecommend(
        AutoRecommendBot* pBot,
        QWidget* pParent)
        : QDialog(pParent),
          m_pBot(pBot),
          m_pEnabledCheckBox(nullptr),
          m_pDeckPreloadingCheckBox(nullptr),
          m_pTransitionAwareCheckBox(nullptr),
          m_pQueueSizeSpinBox(nullptr),
          m_pTempoWeightSpinBox(nullptr),
          m_pEnergyWeightSpinBox(nullptr),
          m_pKeyWeightSpinBox(nullptr),
          m_pFameWeightSpinBox(nullptr),
          m_pStatusLabel(nullptr) {
    DEBUG_ASSERT(m_pBot);
    setWindowTitle(tr("Auto-Recommendation Bot"));
    setMinimumSize(420, 340);

    auto* pLayout = new QVBoxLayout(this);

    auto* pIntro = new QLabel(
            tr("While enabled, this bot watches the currently playing track "
               "and automatically fills the Auto DJ queue with the best "
               "matching tracks from your library. Matches are scored by "
               "tempo (including half/double time), loudness, key "
               "compatibility, and fame (play count). Optionally, it "
               "preloads the top match into an idle deck whenever a deck "
               "stops playing, and sequences the queue for smooth fades "
               "using each track's duration and BPM."),
            this);
    pIntro->setWordWrap(true);
    pLayout->addWidget(pIntro);

    m_pEnabledCheckBox = new QCheckBox(tr("Enable bot"), this);
    m_pEnabledCheckBox->setChecked(m_pBot->isEnabled());
    connect(m_pEnabledCheckBox,
            &QCheckBox::toggled,
            this,
            &DlgAutoRecommend::slotEnabledToggled);
    pLayout->addWidget(m_pEnabledCheckBox);

    m_pDeckPreloadingCheckBox =
            new QCheckBox(tr("Preload next track into idle deck"), this);
    m_pDeckPreloadingCheckBox->setChecked(m_pBot->isDeckPreloadingEnabled());
    m_pDeckPreloadingCheckBox->setToolTip(
            tr("When a deck stops playing, load the best matching track "
               "into the first empty deck (or into the deck that just "
               "stopped). Skipped while Auto DJ is running."));
    connect(m_pDeckPreloadingCheckBox,
            &QCheckBox::toggled,
            this,
            &DlgAutoRecommend::slotDeckPreloadingToggled);
    pLayout->addWidget(m_pDeckPreloadingCheckBox);

    m_pTransitionAwareCheckBox =
            new QCheckBox(tr("Fade/transition aware queue order"), this);
    m_pTransitionAwareCheckBox->setChecked(m_pBot->isTransitionAware());
    m_pTransitionAwareCheckBox->setToolTip(
            tr("Sequences the queue by blending each track's match "
               "score with its transition quality: BPM compatibility "
               "with the previous entry (including half/double time) "
               "and the track's duration, so consecutive tracks blend "
               "smoothly and no track is too short for a proper fade."));
    connect(m_pTransitionAwareCheckBox,
            &QCheckBox::toggled,
            this,
            &DlgAutoRecommend::slotTransitionAwareToggled);
    pLayout->addWidget(m_pTransitionAwareCheckBox);

    auto* pCriteria = new QGridLayout;
    pCriteria->addWidget(new QLabel(tr("Keep queue at:"), this), 0, 0);
    m_pQueueSizeSpinBox = new QSpinBox(this);
    m_pQueueSizeSpinBox->setRange(1, AutoRecommendBot::kMaxQueueSize);
    m_pQueueSizeSpinBox->setValue(m_pBot->queueSize());
    m_pQueueSizeSpinBox->setToolTip(
            tr("The bot adds tracks until the Auto DJ queue holds this "
               "many tracks."));
    connect(m_pQueueSizeSpinBox,
            &QSpinBox::valueChanged,
            this,
            &DlgAutoRecommend::slotQueueSizeChanged);
    pCriteria->addWidget(m_pQueueSizeSpinBox, 0, 1);

    auto addWeightRow = [pCriteria, this](int row, const QString& label) {
        pCriteria->addWidget(new QLabel(label, this), row, 0);
        auto* pSpinBox = new QDoubleSpinBox(this);
        pSpinBox->setRange(0.0, 3.0);
        pSpinBox->setSingleStep(0.1);
        pSpinBox->setDecimals(1);
        pSpinBox->setToolTip(
                tr("Relative weight of this criterion, 0 disables it."));
        connect(pSpinBox,
                &QDoubleSpinBox::valueChanged,
                this,
                &DlgAutoRecommend::slotWeightsChanged);
        pCriteria->addWidget(pSpinBox, row, 1);
        return pSpinBox;
    };
    m_pTempoWeightSpinBox =
            addWeightRow(1, tr("Tempo weight:"));
    m_pEnergyWeightSpinBox =
            addWeightRow(2, tr("Energy (loudness) weight:"));
    m_pKeyWeightSpinBox =
            addWeightRow(3, tr("Key weight:"));
    m_pFameWeightSpinBox =
            addWeightRow(4, tr("Fame (play count) weight:"));
    const mixxx::AutoRecommendationWeights weights = m_pBot->weights();
    m_pTempoWeightSpinBox->setValue(weights.tempo);
    m_pEnergyWeightSpinBox->setValue(weights.energy);
    m_pKeyWeightSpinBox->setValue(weights.key);
    m_pFameWeightSpinBox->setValue(weights.fame);
    pLayout->addLayout(pCriteria);

    m_pStatusLabel = new QLabel(this);
    m_pStatusLabel->setWordWrap(true);
    pLayout->addWidget(m_pStatusLabel);

    pLayout->addStretch();

    auto* pButtonRow = new QHBoxLayout;
    auto* pCloseButton = new QPushButton(tr("Close"), this);
    connect(pCloseButton, &QPushButton::clicked, this, &QDialog::close);
    pButtonRow->addStretch();
    pButtonRow->addWidget(pCloseButton);
    pLayout->addLayout(pButtonRow);

    connect(m_pBot,
            &AutoRecommendBot::recommended,
            this,
            &DlgAutoRecommend::slotRecommended);
    connect(m_pBot,
            &AutoRecommendBot::trackPreloaded,
            this,
            &DlgAutoRecommend::slotTrackPreloaded);
    connect(m_pBot,
            &AutoRecommendBot::deckPreloadingEnabledChanged,
            m_pDeckPreloadingCheckBox,
            [this](bool enabled) {
                const QSignalBlocker blocker(m_pDeckPreloadingCheckBox);
                m_pDeckPreloadingCheckBox->setChecked(enabled);
            });
    connect(m_pBot,
            &AutoRecommendBot::transitionAwareChanged,
            m_pTransitionAwareCheckBox,
            [this](bool transitionAware) {
                const QSignalBlocker blocker(m_pTransitionAwareCheckBox);
                m_pTransitionAwareCheckBox->setChecked(transitionAware);
            });
    connect(m_pBot,
            &AutoRecommendBot::enabledChanged,
            this,
            [this](bool) { slotRecommended(0); });
    connect(m_pBot,
            &AutoRecommendBot::weightsChanged,
            this,
            [this](const mixxx::AutoRecommendationWeights&) {
                slotRecommended(0);
            });

    slotRecommended(0);
}

void DlgAutoRecommend::slotEnabledToggled(bool checked) {
    m_pBot->setEnabled(checked);
}

void DlgAutoRecommend::slotDeckPreloadingToggled(bool checked) {
    m_pBot->setDeckPreloadingEnabled(checked);
}

void DlgAutoRecommend::slotTransitionAwareToggled(bool checked) {
    m_pBot->setTransitionAware(checked);
}

void DlgAutoRecommend::slotQueueSizeChanged(int value) {
    m_pBot->setQueueSize(value);
}

void DlgAutoRecommend::slotWeightsChanged() {
    mixxx::AutoRecommendationWeights weights;
    weights.tempo = m_pTempoWeightSpinBox->value();
    weights.energy = m_pEnergyWeightSpinBox->value();
    weights.key = m_pKeyWeightSpinBox->value();
    weights.fame = m_pFameWeightSpinBox->value();
    m_pBot->setWeights(weights);
}

void DlgAutoRecommend::slotRecommended(int count) {
    if (count > 0) {
        m_lastRecommendedCount = count;
    }
    m_pStatusLabel->setText(statusText(
            m_pBot,
            m_lastRecommendedCount,
            m_lastPreloadedDeckGroup,
            m_lastPreloadedTrackTitle));
}

void DlgAutoRecommend::slotTrackPreloaded(
        const QString& deckGroup,
        const QString& trackTitle) {
    m_lastPreloadedDeckGroup = deckGroup;
    m_lastPreloadedTrackTitle = trackTitle;
    slotRecommended(0);
}
