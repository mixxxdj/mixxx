#include "dialog/dlgsongsuggester.h"

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

#include "dialog/songsuggesterutils.h"
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

using mixxx::SongSuggesterEnergyLevel;
using mixxx::SongSuggesterFameLevel;

QString energyLevelText(SongSuggesterEnergyLevel level) {
    switch (level) {
    case SongSuggesterEnergyLevel::Low:
        return QCoreApplication::translate("DlgSongSuggester", "low energy");
    case SongSuggesterEnergyLevel::Medium:
        return QCoreApplication::translate("DlgSongSuggester", "medium energy");
    case SongSuggesterEnergyLevel::High:
        return QCoreApplication::translate("DlgSongSuggester", "high energy");
    case SongSuggesterEnergyLevel::Any:
        break;
    }
    return QString();
}

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
    m_pEnergyComboBox->addItem(tr("Any"), static_cast<int>(SongSuggesterEnergyLevel::Any));
    m_pEnergyComboBox->addItem(
            tr("Low (quiet master)"), static_cast<int>(SongSuggesterEnergyLevel::Low));
    m_pEnergyComboBox->addItem(
            tr("Medium"), static_cast<int>(SongSuggesterEnergyLevel::Medium));
    m_pEnergyComboBox->addItem(
            tr("High (loud master)"), static_cast<int>(SongSuggesterEnergyLevel::High));
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
    m_pFameComboBox->addItem(tr("Any"), static_cast<int>(SongSuggesterFameLevel::Any));
    m_pFameComboBox->addItem(
            tr("Underground (< 5 plays)"),
            static_cast<int>(SongSuggesterFameLevel::Underground));
    m_pFameComboBox->addItem(
            tr("Known (5-49 plays)"), static_cast<int>(SongSuggesterFameLevel::Known));
    m_pFameComboBox->addItem(
            tr("Popular (50+ plays)"),
            static_cast<int>(SongSuggesterFameLevel::Popular));
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

    mixxx::SongSuggesterCriteria criteria;
    criteria.targetBpm = m_pTargetBpmSpinBox->value();
    criteria.bpmTolerance = m_pBpmToleranceSpinBox->value();
    criteria.energy = static_cast<SongSuggesterEnergyLevel>(
            m_pEnergyComboBox->currentData().toInt());
    criteria.language = m_pLanguageEdit->text().trimmed();
    criteria.fame = static_cast<SongSuggesterFameLevel>(
            m_pFameComboBox->currentData().toInt());

    const QSet<QString> locations =
            m_pTrackCollectionManager->internalCollection()
                    ->getTrackDAO()
                    .getAllTrackLocations();

    m_matches.clear();
    m_pResultsList->clear();

    const ScopedWaitCursor waitCursor;

    QList<TrackPointer> candidates;
    candidates.reserve(locations.size());
    for (const QString& location : locations) {
        const TrackPointer pTrack = m_pTrackCollectionManager->getTrackByRef(
                TrackRef::fromFilePath(location));
        if (pTrack) {
            candidates.append(pTrack);
        }
    }

    m_matches = mixxx::filterAndRankSongSuggesterMatches(
            candidates, m_pReferenceTrack, criteria);

    for (const TrackPointer& pTrack : m_matches) {
        QStringList details;
        details << tr("%1 BPM").arg(QString::number(pTrack->getBpm(), 'f', 1));
        const auto replayGain = pTrack->getReplayGain();
        if (replayGain.hasRatio()) {
            details << energyLevelText(
                    mixxx::songSuggesterEnergyLevelForRatio(replayGain.getRatio()));
        }
        const QString trackLanguage = mixxx::songSuggesterTrackLanguage(pTrack);
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
                static_cast<int>(mixxx::songSuggesterEnergyLevelForRatio(
                        replayGain.getRatio()))));
    } else {
        m_pEnergyComboBox->setCurrentIndex(m_pEnergyComboBox->findData(
                static_cast<int>(SongSuggesterEnergyLevel::Any)));
    }
    m_pLanguageEdit->setText(mixxx::songSuggesterTrackLanguage(pTrack));
    m_pFameComboBox->setCurrentIndex(m_pFameComboBox->findData(
            static_cast<int>(mixxx::songSuggesterFameLevelForPlays(
                    pTrack->getTimesPlayed()))));
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