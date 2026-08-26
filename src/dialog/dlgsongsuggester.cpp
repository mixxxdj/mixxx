#include "dialog/dlgsongsuggester.h"

#include <algorithm>
#include <cmath>

#include <QComboBox>
#include <QCoreApplication>
#include <QDoubleSpinBox>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QPushButton>
#include <QSet>
#include <QStringList>
#include <QVBoxLayout>

#include "library/trackcollection.h"
#include "library/trackcollectionmanager.h"
#include "mixer/playerinfo.h"
#include "mixer/playermanager.h"
#include "moc_dlgsongsuggester.cpp"
#include "track/track.h"
#include "track/trackref.h"
#include "util/assert.h"
#include "util/scopedoverridecursor.h"

namespace {

// Energy is estimated from ReplayGain loudness, which is stored as a
// linear ratio where 1.0 == 0 dB. A louder master requires negative gain
// (ratio < 1.0) for normalization and is treated as high energy.
enum class EnergyLevel { Any, Low, Medium, High };

constexpr double kHighEnergyRatioMax = 0.8; // about -2 dB
constexpr double kLowEnergyRatioMin = 1.25; // about +2 dB

EnergyLevel energyLevelForRatio(double ratio) {
    if (ratio < kHighEnergyRatioMax) {
        return EnergyLevel::High;
    }
    if (ratio > kLowEnergyRatioMin) {
        return EnergyLevel::Low;
    }
    return EnergyLevel::Medium;
}

QString energyLevelText(EnergyLevel level) {
    switch (level) {
    case EnergyLevel::Low:
        return QCoreApplication::translate("DlgSongSuggester", "low energy");
    case EnergyLevel::Medium:
        return QCoreApplication::translate("DlgSongSuggester", "medium energy");
    case EnergyLevel::High:
        return QCoreApplication::translate("DlgSongSuggester", "high energy");
    case EnergyLevel::Any:
        break;
    }
    return QString();
}

// Fame is approximated by the number of times a track has been played.
enum class FameLevel { Any, Underground, Known, Popular };

constexpr int kKnownPlays = 5;
constexpr int kPopularPlays = 50;

FameLevel fameLevelForPlays(int plays) {
    if (plays >= kPopularPlays) {
        return FameLevel::Popular;
    }
    if (plays >= kKnownPlays) {
        return FameLevel::Known;
    }
    return FameLevel::Underground;
}

constexpr int kMaxResults = 100;

} // namespace

DlgSongSuggester::DlgSongSuggester(
        TrackCollectionManager* pTrackCollectionManager,
        PlayerManager* pPlayerManager,
        QWidget* pParent)
        : QDialog(pParent),
          m_pTrackCollectionManager(pTrackCollectionManager),
          m_pPlayerManager(pPlayerManager),
          m_pReferenceTrack(),
          m_matches(),
          m_pReferenceLabel(nullptr),
          m_pResultCountLabel(nullptr),
          m_pTargetBpmSpinBox(nullptr),
          m_pBpmToleranceSpinBox(nullptr),
          m_pEnergyComboBox(nullptr),
          m_pLanguageEdit(nullptr),
          m_pFameComboBox(nullptr),
          m_pResultsList(nullptr) {
    setWindowTitle(tr("Song Suggester"));
    setMinimumSize(560, 540);

    auto* pLayout = new QVBoxLayout(this);

    auto* pIntro = new QLabel(
            tr("Finds tracks in your library that match the tempo, energy "
               "(loudness), language, and fame (play count) of a reference "
               "track. Tracks must be analyzed to have BPM and ReplayGain "
               "values."),
            this);
    pIntro->setWordWrap(true);
    pLayout->addWidget(pIntro);

    m_pReferenceLabel = new QLabel(this);
    m_pReferenceLabel->setWordWrap(true);
    auto* pUseCurrentTrackButton = new QPushButton(tr("Use currently playing track"), this);
    connect(pUseCurrentTrackButton,
            &QPushButton::clicked,
            this,
            &DlgSongSuggester::slotUseCurrentTrack);
    auto* pReferenceRow = new QHBoxLayout;
    pReferenceRow->addWidget(m_pReferenceLabel, 1);
    pReferenceRow->addWidget(pUseCurrentTrackButton);
    pLayout->addLayout(pReferenceRow);

    auto* pCriteria = new QGridLayout;
    pCriteria->addWidget(new QLabel(tr("Tempo (BPM):"), this), 0, 0);
    m_pTargetBpmSpinBox = new QDoubleSpinBox(this);
    m_pTargetBpmSpinBox->setRange(20.0, 300.0);
    m_pTargetBpmSpinBox->setDecimals(1);
    m_pTargetBpmSpinBox->setValue(120.0);
    pCriteria->addWidget(m_pTargetBpmSpinBox, 0, 1);
    pCriteria->addWidget(new QLabel(tr("±"), this), 0, 2);
    m_pBpmToleranceSpinBox = new QDoubleSpinBox(this);
    m_pBpmToleranceSpinBox->setRange(0.0, 30.0);
    m_pBpmToleranceSpinBox->setDecimals(1);
    m_pBpmToleranceSpinBox->setValue(5.0);
    m_pBpmToleranceSpinBox->setToolTip(
            tr("Only tracks within this range of the target tempo match."));
    pCriteria->addWidget(m_pBpmToleranceSpinBox, 0, 3);

    pCriteria->addWidget(new QLabel(tr("Energy:"), this), 1, 0);
    m_pEnergyComboBox = new QComboBox(this);
    m_pEnergyComboBox->addItem(tr("Any"), static_cast<int>(EnergyLevel::Any));
    m_pEnergyComboBox->addItem(tr("Low (quiet master)"), static_cast<int>(EnergyLevel::Low));
    m_pEnergyComboBox->addItem(tr("Medium"), static_cast<int>(EnergyLevel::Medium));
    m_pEnergyComboBox->addItem(tr("High (loud master)"), static_cast<int>(EnergyLevel::High));
    m_pEnergyComboBox->setToolTip(
            tr("Energy is estimated from ReplayGain loudness."));
    pCriteria->addWidget(m_pEnergyComboBox, 1, 1, 1, 3);

    pCriteria->addWidget(new QLabel(tr("Language:"), this), 2, 0);
    m_pLanguageEdit = new QLineEdit(this);
    m_pLanguageEdit->setPlaceholderText(tr("Any"));
    m_pLanguageEdit->setToolTip(
            tr("Only tracks whose language tag contains this text match. "
               "Empty matches every language."));
    pCriteria->addWidget(m_pLanguageEdit, 2, 1, 1, 3);

    pCriteria->addWidget(new QLabel(tr("Fame:"), this), 3, 0);
    m_pFameComboBox = new QComboBox(this);
    m_pFameComboBox->addItem(tr("Any"), static_cast<int>(FameLevel::Any));
    m_pFameComboBox->addItem(
            tr("Underground (< 5 plays)"), static_cast<int>(FameLevel::Underground));
    m_pFameComboBox->addItem(tr("Known (5-49 plays)"), static_cast<int>(FameLevel::Known));
    m_pFameComboBox->addItem(tr("Popular (50+ plays)"), static_cast<int>(FameLevel::Popular));
    m_pFameComboBox->setToolTip(
            tr("Fame is estimated from how often a track has been played."));
    pCriteria->addWidget(m_pFameComboBox, 3, 1, 1, 3);
    pLayout->addLayout(pCriteria);

    auto* pSuggestRow = new QHBoxLayout;
    auto* pSuggestButton = new QPushButton(tr("Suggest"), this);
    pSuggestButton->setDefault(true);
    connect(pSuggestButton, &QPushButton::clicked, this, &DlgSongSuggester::slotSuggest);
    pSuggestRow->addWidget(pSuggestButton);
    m_pResultCountLabel = new QLabel(this);
    m_pResultCountLabel->setWordWrap(true);
    pSuggestRow->addWidget(m_pResultCountLabel, 1);
    pLayout->addLayout(pSuggestRow);

    m_pResultsList = new QListWidget(this);
    m_pResultsList->setWordWrap(true);
    m_pResultsList->setMinimumHeight(220);
    connect(m_pResultsList,
            &QListWidget::itemDoubleClicked,
            this,
            [this](QListWidgetItem*) { loadSelectedTrackToDeck(1); });
    pLayout->addWidget(m_pResultsList, 1);

    auto* pDeckRow = new QHBoxLayout;
    auto* pLoadDeck1Button = new QPushButton(tr("Load to Deck 1"), this);
    auto* pLoadDeck2Button = new QPushButton(tr("Load to Deck 2"), this);
    connect(pLoadDeck1Button,
            &QPushButton::clicked,
            this,
            [this] { loadSelectedTrackToDeck(1); });
    connect(pLoadDeck2Button,
            &QPushButton::clicked,
            this,
            [this] { loadSelectedTrackToDeck(2); });
    pDeckRow->addWidget(pLoadDeck1Button);
    pDeckRow->addWidget(pLoadDeck2Button);
    pDeckRow->addStretch();
    auto* pCloseButton = new QPushButton(tr("Close"), this);
    connect(pCloseButton, &QPushButton::clicked, this, &QDialog::close);
    pDeckRow->addWidget(pCloseButton);
    pLayout->addLayout(pDeckRow);

    setReferenceTrack(PlayerInfo::instance().getCurrentPlayingTrack());
    if (!m_pReferenceTrack) {
        const auto loadedTracks = PlayerInfo::instance().getLoadedTracks();
        if (!loadedTracks.isEmpty()) {
            setReferenceTrack(loadedTracks.cbegin().value());
        }
    }
}

void DlgSongSuggester::slotSuggest() {
    DEBUG_ASSERT(m_pTrackCollectionManager);
    if (!m_pTrackCollectionManager) {
        return;
    }

    const double targetBpm = m_pTargetBpmSpinBox->value();
    const double tolerance = m_pBpmToleranceSpinBox->value();
    const EnergyLevel energy =
            static_cast<EnergyLevel>(m_pEnergyComboBox->currentData().toInt());
    const QString language = m_pLanguageEdit->text().trimmed();
    const FameLevel fame = static_cast<FameLevel>(m_pFameComboBox->currentData().toInt());

    const QSet<QString> locations =
            m_pTrackCollectionManager->internalCollection()
                    ->getTrackDAO()
                    .getAllTrackLocations();

    m_matches.clear();
    m_pResultsList->clear();

    const ScopedWaitCursor waitCursor;

    for (const QString& location : locations) {
        const TrackPointer pTrack = m_pTrackCollectionManager->getTrackByRef(
                TrackRef::fromFilePath(location));
        if (!pTrack) {
            continue;
        }
        if (m_pReferenceTrack && m_pReferenceTrack->getId().isValid() &&
                pTrack->getId() == m_pReferenceTrack->getId()) {
            continue;
        }
        if (std::abs(pTrack->getBpm() - targetBpm) > tolerance) {
            continue;
        }
        if (energy != EnergyLevel::Any) {
            const auto replayGain = pTrack->getReplayGain();
            if (!replayGain.hasRatio() ||
                    energyLevelForRatio(replayGain.getRatio()) != energy) {
                continue;
            }
        }
        if (!language.isEmpty() &&
                !pTrack->getMetadata().getTrackInfo().getLanguage().contains(
                        language, Qt::CaseInsensitive)) {
            continue;
        }
        if (fame != FameLevel::Any &&
                fameLevelForPlays(pTrack->getTimesPlayed()) != fame) {
            continue;
        }
        m_matches.append(pTrack);
    }

    std::stable_sort(
            m_matches.begin(),
            m_matches.end(),
            [targetBpm](const TrackPointer& a, const TrackPointer& b) {
                const double distanceA = std::abs(a->getBpm() - targetBpm);
                const double distanceB = std::abs(b->getBpm() - targetBpm);
                if (distanceA != distanceB) {
                    return distanceA < distanceB;
                }
                return a->getTimesPlayed() > b->getTimesPlayed();
            });

    if (m_matches.size() > kMaxResults) {
        m_matches.resize(kMaxResults);
    }

    for (const TrackPointer& pTrack : m_matches) {
        QStringList details;
        details << tr("%1 BPM").arg(QString::number(pTrack->getBpm(), 'f', 1));
        const auto replayGain = pTrack->getReplayGain();
        if (replayGain.hasRatio()) {
            details << energyLevelText(energyLevelForRatio(replayGain.getRatio()));
        }
        const QString trackLanguage = pTrack->getMetadata().getTrackInfo().getLanguage();
        if (!trackLanguage.isEmpty()) {
            details << trackLanguage;
        }
        details << tr("played %1 times").arg(pTrack->getTimesPlayed());
        new QListWidgetItem(
                QStringLiteral("%1\n%2")
                        .arg(pTrack->getTitleInfo(), details.join(QStringLiteral(" · "))),
                m_pResultsList);
    }

    m_pResultCountLabel->setText(tr("Found %1 matching tracks.").arg(m_matches.size()));
    if (!m_matches.isEmpty()) {
        m_pResultsList->setCurrentRow(0);
    }
}

void DlgSongSuggester::slotUseCurrentTrack() {
    TrackPointer pTrack = PlayerInfo::instance().getCurrentPlayingTrack();
    if (!pTrack) {
        const auto loadedTracks = PlayerInfo::instance().getLoadedTracks();
        if (!loadedTracks.isEmpty()) {
            pTrack = loadedTracks.cbegin().value();
        }
    }
    setReferenceTrack(pTrack);
}

void DlgSongSuggester::setReferenceTrack(const TrackPointer& pTrack) {
    m_pReferenceTrack = pTrack;
    if (pTrack) {
        m_pReferenceLabel->setText(
                tr("Matches are based on: %1").arg(pTrack->getTitleInfo()));
        prefillCriteria(pTrack);
    } else {
        m_pReferenceLabel->setText(
                tr("No track is loaded. Load a track into a deck or adjust the "
                   "criteria manually."));
    }
}

void DlgSongSuggester::prefillCriteria(const TrackPointer& pTrack) {
    const double bpm = pTrack->getBpm();
    if (bpm > 0.0) {
        m_pTargetBpmSpinBox->setValue(bpm);
    }
    const auto replayGain = pTrack->getReplayGain();
    if (replayGain.hasRatio()) {
        m_pEnergyComboBox->setCurrentIndex(m_pEnergyComboBox->findData(
                static_cast<int>(energyLevelForRatio(replayGain.getRatio()))));
    } else {
        m_pEnergyComboBox->setCurrentIndex(m_pEnergyComboBox->findData(
                static_cast<int>(EnergyLevel::Any)));
    }
    m_pLanguageEdit->setText(pTrack->getMetadata().getTrackInfo().getLanguage());
    m_pFameComboBox->setCurrentIndex(m_pFameComboBox->findData(
            static_cast<int>(fameLevelForPlays(pTrack->getTimesPlayed()))));
}

void DlgSongSuggester::loadSelectedTrackToDeck(int deck) {
    const auto* pItem = m_pResultsList->currentItem();
    if (!pItem) {
        return;
    }
    const int row = m_pResultsList->row(pItem);
    if (row < 0 || row >= m_matches.size()) {
        return;
    }
    DEBUG_ASSERT(m_pPlayerManager);
    if (m_pPlayerManager) {
        m_pPlayerManager->slotLoadToDeck(m_matches.at(row)->getLocation(), deck);
    }
}