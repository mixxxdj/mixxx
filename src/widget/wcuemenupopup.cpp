#include "widget/wcuemenupopup.h"

#include <QCryptographicHash>
#include <QDateTime>
#include <QDir>
#include <QFileInfo>
#include <QHBoxLayout>
#include <QProcess>
#include <QRegularExpression>
#include <QStandardPaths>
#include <QVBoxLayout>
#include <algorithm>
#include <array>
#include <iterator>
#include <optional>
#include <vector>

#include "control/controlobject.h"
#include "control/pollingcontrolproxy.h"
#include "mixer/playermanager.h"
#include "mixer/sampler.h"
#include "moc_wcuemenupopup.cpp"
#include "track/track.h"

namespace {
const ConfigKey kHotcueDefaultColorIndexConfigKey("[Controls]", "HotcueDefaultColorIndex");
const ConfigKey kLoopDefaultColorIndexConfigKey("[Controls]", "LoopDefaultColorIndex");
const ConfigKey kJumpDefaultColorIndexConfigKey("[Controls]", "jump_default_color_index");

constexpr mixxx::audio::FrameDiff_t kMinimumAudibleLoopSizeFrames = 150;

struct ShowKey {
    const char* key;
    int count;
    int blockCount;
};

const ShowKey kShowKeys[] = {
        {"show_64samplers", 64, 8},
        {"show_48samplers", 48, 6},
        {"show_32samplers", 32, 4},
        {"show_16samplers", 16, 2},
        {"show_8samplers", 8, 1},
        {"show_4samplers", 4, 0},
};

static const char* const kExpandKeys[] = {
        "expand_samplers_1-8",
        "expand_samplers_9-16",
        "expand_samplers_17-24",
        "expand_samplers_25-32",
        "expand_samplers_33-40",
        "expand_samplers_41-48",
        "expand_samplers_49-56",
        "expand_samplers_57-64",
};

constexpr int kNumExpandKeys = static_cast<int>(std::size(kExpandKeys));

constexpr int kMaxVisibleSamplerButtons = 64;

const QRegularExpression kUnsafeFilenameChars(
        QStringLiteral(R"([\\/:*?"<>|])"));
} // namespace

void CueMenuPushButton::mousePressEvent(QMouseEvent* e) {
    if (e->type() == QEvent::MouseButtonPress && e->button() == Qt::RightButton) {
        emit rightClicked();
        return;
    }
    QPushButton::mousePressEvent(e);
}

void WCueMenuPopup::updateTypeAndColorIfDefault(mixxx::CueType newType) {
    auto hotcueColorPalette =
            m_colorPaletteSettings.getHotcueColorPalette();
    int colorIndex;
    switch (m_pCue->getType()) {
    default:
        colorIndex = m_pConfig->getValue(kHotcueDefaultColorIndexConfigKey, -1);
        break;
    case mixxx::CueType::Loop:
        colorIndex = m_pConfig->getValue(kLoopDefaultColorIndexConfigKey, -1);
        break;
    case mixxx::CueType::Jump:
        colorIndex = m_pConfig->getValue(kJumpDefaultColorIndexConfigKey, -1);
        break;
    }
    auto defaultColor =
            (colorIndex < 0 || colorIndex >= hotcueColorPalette.size())
            ? hotcueColorPalette.defaultColor()
            : hotcueColorPalette.at(colorIndex);
    m_pCue->setType(newType);
    if (m_pCue->getColor() != defaultColor) {
        return;
    }
    switch (newType) {
    default:
        colorIndex = m_pConfig->getValue(kHotcueDefaultColorIndexConfigKey, -1);
        break;
    case mixxx::CueType::Loop:
        colorIndex = m_pConfig->getValue(kLoopDefaultColorIndexConfigKey, -1);
        break;
    case mixxx::CueType::Jump:
        colorIndex = m_pConfig->getValue(kJumpDefaultColorIndexConfigKey, -1);
        break;
    }
    if (colorIndex < 0 || colorIndex >= hotcueColorPalette.size()) {
        m_pCue->setColor(hotcueColorPalette.defaultColor());
    } else {
        m_pCue->setColor(hotcueColorPalette.at(colorIndex));
    }
}

WCueMenuPopup::WCueMenuPopup(UserSettingsPointer pConfig, QWidget* parent)
        : QWidget(parent),
          m_pConfig(pConfig),
          m_colorPaletteSettings(ColorPaletteSettings(pConfig)),
          m_pBeatLoopSize(ControlFlag::AllowMissingOrInvalid),
          m_pPlayPos(ControlFlag::AllowMissingOrInvalid),
          m_pTrackSample(ControlFlag::AllowMissingOrInvalid),
          m_pQuantizeEnabled(ControlFlag::AllowMissingOrInvalid) {
    QWidget::hide();
    setWindowFlags(Qt::Popup);
    setAttribute(Qt::WA_StyledBackground);
    setObjectName("WCueMenuPopup");

    m_pCueNumber = std::make_unique<QLabel>(this);
    m_pCueNumber->setToolTip(tr("Cue number"));
    m_pCueNumber->setObjectName("CueNumberLabel");
    m_pCueNumber->setAlignment(Qt::AlignLeft);

    m_pCuePosition = std::make_unique<QLabel>(this);
    m_pCuePosition->setToolTip(tr("Cue position"));
    m_pCuePosition->setObjectName("CuePositionLabel");
    m_pCuePosition->setAlignment(Qt::AlignRight);

    m_pEditLabel = std::make_unique<QLineEdit>(this);
    m_pEditLabel->setToolTip(tr("Edit cue label"));
    m_pEditLabel->setObjectName("CueLabelEdit");
    m_pEditLabel->setPlaceholderText(tr("Label..."));
    connect(m_pEditLabel.get(), &QLineEdit::textEdited, this, &WCueMenuPopup::slotEditLabel);
    connect(m_pEditLabel.get(), &QLineEdit::returnPressed, this, &WCueMenuPopup::hide);

    m_pColorPicker =
            std::make_unique<WColorPicker>(WColorPicker::Option::NoOptions,
                    m_colorPaletteSettings.getHotcueColorPalette(),
                    this);
    m_pColorPicker->setObjectName("CueColorPicker");
    connect(m_pColorPicker.get(),
            &WColorPicker::colorPicked,
            this,
            &WCueMenuPopup::slotChangeCueColor);

    m_pDeleteCue = std::make_unique<CueMenuPushButton>(this);
    m_pDeleteCue->setToolTip(tr("Delete this cue"));
    m_pDeleteCue->setObjectName("CueDeleteButton");
    connect(m_pDeleteCue.get(), &QPushButton::clicked, this, &WCueMenuPopup::slotDeleteCue);

    m_pStandardCue = std::make_unique<CueMenuPushButton>(this);
    m_pStandardCue->setToolTip(
            tr("Turn this cue into a regular hotcue"));
    m_pStandardCue->setObjectName("CueStandardButton");
    m_pStandardCue->setCheckable(true);
    connect(m_pStandardCue.get(),
            &CueMenuPushButton::clicked,
            this,
            &WCueMenuPopup::slotStandardCue);

    m_pSavedLoopCue = std::make_unique<CueMenuPushButton>(this);
    m_pSavedLoopCue->setToolTip(tr("Turn this cue into a saved loop") + "\n\n" +
            tr("Left-click: Use the old size if known or the current beatloop "
               "size as the loop size") +
            "\n" +
            tr("Right-click: Use the current play position as new loop end if "
               "it is after the cue"));
    m_pSavedLoopCue->setObjectName("CueSavedLoopButton");
    m_pSavedLoopCue->setCheckable(true);
    connect(m_pSavedLoopCue.get(),
            &CueMenuPushButton::clicked,
            this,
            &WCueMenuPopup::slotSavedLoopCueAuto);
    connect(m_pSavedLoopCue.get(),
            &CueMenuPushButton::rightClicked,
            this,
            &WCueMenuPopup::slotSavedLoopCueManual);

    m_pSavedJumpCue = std::make_unique<CueMenuPushButton>(this);
    m_pSavedJumpCue->setObjectName("CueSavedJumpButton");
    m_pSavedJumpCue->setCheckable(true);
    connect(m_pSavedJumpCue.get(),
            &CueMenuPushButton::clicked,
            this,
            &WCueMenuPopup::slotSavedJumpCueAuto);
    connect(m_pSavedJumpCue.get(),
            &CueMenuPushButton::rightClicked,
            this,
            &WCueMenuPopup::slotSavedJumpCueManual);

    m_pExportCue = std::make_unique<CueMenuPushButton>(this);
    m_pExportCue->setToolTip(tr("Export this loop as a sample"));
    m_pExportCue->setObjectName("CueExportButton");
    connect(m_pExportCue.get(),
            &QPushButton::clicked,
            this,
            &WCueMenuPopup::slotExportCue);

    m_pExportToSamplerButtons.reserve(kMaxVisibleSamplerButtons);
    for (int i = 0; i < kMaxVisibleSamplerButtons; ++i) {
        auto btn = std::make_unique<CueMenuPushButton>(this);
        btn->setToolTip(tr("Export this cue to Sampler %1").arg(i + 1));
        btn->setObjectName(QStringLiteral("CueExportToSampler%1").arg(i + 1));
        btn->setText(QString::number(i + 1));
        connect(btn.get(), &QPushButton::clicked, this, [this, i]() { slotExportToSampler(i); });
        btn->setVisible(false);
        m_pExportToSamplerButtons.push_back(std::move(btn));
    }

    QHBoxLayout* pLabelLayout = new QHBoxLayout();
    pLabelLayout->addWidget(m_pCueNumber.get());
    pLabelLayout->addStretch(1);
    pLabelLayout->addWidget(m_pCuePosition.get());

    QVBoxLayout* pLeftLayout = new QVBoxLayout();
    pLeftLayout->addLayout(pLabelLayout);
    pLeftLayout->addWidget(m_pEditLabel.get());
    pLeftLayout->addWidget(m_pColorPicker.get());
    m_pLeftLayout = pLeftLayout;

    QVBoxLayout* pRightLayout = new QVBoxLayout();
    pRightLayout->addWidget(m_pDeleteCue.get());
    pRightLayout->addWidget(m_pStandardCue.get());
    pRightLayout->addStretch(1);
    pRightLayout->addWidget(m_pSavedLoopCue.get());
    pRightLayout->addStretch(1);
    pRightLayout->addWidget(m_pSavedJumpCue.get());
    pRightLayout->addWidget(m_pExportCue.get());

    QHBoxLayout* pMainLayout = new QHBoxLayout();
    pMainLayout->addLayout(pLeftLayout);
    pMainLayout->addSpacing(5);
    pMainLayout->addLayout(pRightLayout);
    setLayout(pMainLayout);
    // we need to update the the layout here since the size is used to
    // calculate the positioning later
    layout()->update();
    layout()->activate();
}

SamplerLayout WCueMenuPopup::currentSamplerLayout() const {
    SamplerLayout layout;

    int blockCount = -1;
    bool isFourSamplerLayout = false;
    QString activeShowKey;

    for (const auto& k : kShowKeys) {
        PollingControlProxy proxy(
                ConfigKey("[LateNight]", k.key),
                ControlFlag::AllowMissingOrInvalid);
        if (proxy.valid() && proxy.get() != 0.0) {
            blockCount = k.blockCount;
            isFourSamplerLayout = (k.count == 4);
            activeShowKey = QString::fromLatin1(k.key);
            break;
        }
    }

    // read all expand states once so the debug line can print them.
    std::array<bool, kNumExpandKeys> expandStates{};
    for (int b = 0; b < kNumExpandKeys; ++b) {
        PollingControlProxy expandProxy(
                ConfigKey("[LateNight]", kExpandKeys[b]),
                ControlFlag::AllowMissingOrInvalid);
        expandStates[b] = expandProxy.valid() && expandProxy.get() != 0.0;
    }

    qDebug() << "[WCUEMENUPOPUP] -> currentSamplerLayout config:"
             << "activeShowKey=" << activeShowKey
             << "blockCount=" << blockCount
             << "isFourSamplerLayout=" << isFourSamplerLayout
             << "expand_1_8=" << expandStates[0]
             << "expand_9_16=" << expandStates[1]
             << "expand_17_24=" << expandStates[2]
             << "expand_25_32=" << expandStates[3]
             << "expand_33_40=" << expandStates[4]
             << "expand_41_48=" << expandStates[5]
             << "expand_49_56=" << expandStates[6]
             << "expand_57_64=" << expandStates[7];

    if (blockCount < 0) {
        return layout;
    }

    if (isFourSamplerLayout) {
        layout.samplerNumbers = {1, 2, 3, 4};
        layout.columnsPerRow = {4};
        return layout;
    }

    auto addBlock = [&layout](int first, bool expanded) {
        if (expanded) {
            layout.samplerNumbers
                    << first << first + 1 << first + 4 << first + 5
                    << first + 2 << first + 3 << first + 6 << first + 7;
            layout.columnsPerRow << 4 << 4;
        } else {
            for (int i = 0; i < 8; ++i) {
                layout.samplerNumbers << first + i;
            }
            layout.columnsPerRow << 8;
        }
    };

    for (int b = 0; b < blockCount && b < kNumExpandKeys; ++b) {
        const int firstSamplerNumber = b * 8 + 1;
        addBlock(firstSamplerNumber, expandStates[b]);
    }

    return layout;
}

void WCueMenuPopup::clearExportToSamplerButtons() {
    // remove buttons first
    for (auto& btn : m_pExportToSamplerButtons) {
        if (btn) {
            btn->setParent(nullptr);
            btn->deleteLater();
        }
    }
    m_pExportToSamplerButtons.clear();

    // remove rows from the layout and delete them
    for (QHBoxLayout* pRow : m_pSamplerButtonRows) {
        if (!pRow) {
            continue;
        }
        while (pRow->count() > 0) {
            QLayoutItem* pItem = pRow->takeAt(0);
            delete pItem;
        }
        if (m_pLeftLayout) {
            m_pLeftLayout->removeItem(pRow);
        }
        delete pRow;
    }
    m_pSamplerButtonRows.clear();
}

void WCueMenuPopup::rebuildExportToSamplerButtons() {
    qDebug() << "[WCUEMENUPOPUP] -> rebuildExportToSamplerButtons:"
             << "m_pLeftLayout=" << m_pLeftLayout;

    clearExportToSamplerButtons();

    if (!m_pLeftLayout) {
        qWarning() << "[WCUEMENUPOPUP] -> m_pLeftLayout is null, bailing";
        return;
    }

    const SamplerLayout layout = currentSamplerLayout();

    qDebug() << "[WCUEMENUPOPUP] -> layout:"
             << "samplerNumbers=" << layout.samplerNumbers
             << "columnsPerRow=" << layout.columnsPerRow;

    if (layout.samplerNumbers.isEmpty()) {
        return;
    }

    PlayerManager* pPlayerManager = PlayerManager::instance();

    int pos = 0;
    for (int cols : layout.columnsPerRow) {
        auto* pRow = new QHBoxLayout();
        for (int c = 0; c < cols && pos < layout.samplerNumbers.size();
                ++c, ++pos) {
            const int samplerNumber = layout.samplerNumbers[pos];

            auto btn = std::make_unique<CueMenuPushButton>(this);
            btn->setText(QString::number(samplerNumber));
            btn->setMinimumSize(22, 18);
            btn->setToolTip(
                    tr("Export this hotcue/loop as sample to sampler %1")
                            .arg(samplerNumber));
            btn->setObjectName(
                    QStringLiteral("CueExportToSampler%1").arg(samplerNumber));
            connect(btn.get(), &QPushButton::clicked, this, [this, samplerNumber]() {
                slotExportToSampler(samplerNumber - 1);
            });

            // the gred shows colours for the sampler state:
            // -> black = empty sampler
            // -> red text if a track is loaded in the sampler, not playing
            // -> orange background if the sampler is currently playing a NON-LOOP.
            // -> red background if the sampler is currently playing a LOOP
            if (pPlayerManager) {
                const QString group =
                        PlayerManager::groupForSampler(samplerNumber - 1);
                const bool hasTrack = samplerHasLoadedTrack(pPlayerManager, group);
                const bool isPlaying = samplerIsPlaying(group);
                const bool isLooping = isPlaying && samplerIsLooping(group);

                if (isLooping) {
                    // Playing a loop
                    btn->setStyleSheet(QStringLiteral(
                            "background-color: #800000; color: #ffffff;"));
                } else if (isPlaying) {
                    // Playing a one-shot
                    btn->setStyleSheet(QStringLiteral(
                            "background-color: #b06000; color: #ffffff;"));
                } else if (hasTrack) {
                    // Loaded but stopped
                    btn->setStyleSheet(QStringLiteral("color: #ff4040;"));
                }
            }

            pRow->addWidget(btn.get(), 1);
            m_pExportToSamplerButtons.push_back(std::move(btn));
        }
        m_pSamplerButtonRows.push_back(pRow);
        m_pLeftLayout->addLayout(pRow);
    }

    updateExportToSamplerButtons();
}

bool WCueMenuPopup::samplerIsPlaying(const QString& group) const {
    ControlObject* pPlay = ControlObject::getControl(
            ConfigKey(group, QStringLiteral("play")),
            ControlFlag::AllowMissingOrInvalid);
    return pPlay && pPlay->toBool();
}

bool WCueMenuPopup::samplerHasLoadedTrack(
        PlayerManager* pPlayerManager, const QString& group) const {
    if (!pPlayerManager) {
        return false;
    }
    BaseTrackPlayer* pPlayer = pPlayerManager->getPlayer(group);
    if (!pPlayer) {
        return false;
    }
    return pPlayer->getLoadedTrack() != nullptr;
}

bool WCueMenuPopup::samplerIsLooping(const QString& group) const {
    ControlObject* pRepeat = ControlObject::getControl(
            ConfigKey(group, QStringLiteral("repeat")),
            ControlFlag::AllowMissingOrInvalid);
    return pRepeat && pRepeat->toBool();
}

void WCueMenuPopup::setTrackCueGroup(
        TrackPointer pTrack, const CuePointer& pCue, const QString& group) {
    if (!pTrack || !pCue) {
        return;
    }

    m_pTrack = pTrack;
    m_pCue = pCue;
    m_group = group;

    if (m_pBeatLoopSize.getKey().group != group) {
        m_pBeatLoopSize = PollingControlProxy(group, "beatloop_size");
    }

    if (m_pPlayPos.getKey().group != group) {
        m_pPlayPos = PollingControlProxy(group, "playposition");
    }

    if (m_pTrackSample.getKey().group != group) {
        m_pTrackSample = PollingControlProxy(group, "track_samples");
    }

    if (m_pQuantizeEnabled.getKey().group != group) {
        m_pQuantizeEnabled = PollingControlProxy(group, "quantize");
    }
    slotUpdate();
    rebuildExportToSamplerButtons();
}

void WCueMenuPopup::slotUpdate() {
    if (m_pTrack && m_pCue) {
        int hotcueNumber = m_pCue->getHotCue();
        QString hotcueNumberText = "";
        if (hotcueNumber != Cue::kNoHotCue) {
            // Programmers count from 0, but DJs count from 1
            hotcueNumberText = QString(tr("Hotcue #%1")).arg(QString::number(hotcueNumber + 1));
        }
        m_pCueNumber->setText(hotcueNumberText);

        QString positionText = "";
        Cue::StartAndEndPositions pos = m_pCue->getStartAndEndPosition();
        if (pos.startPosition.isValid() && pos.endPosition.isValid() &&
                m_pCue->getType() != mixxx::CueType::HotCue) {
            double startPositionSeconds = pos.startPosition.value() / m_pTrack->getSampleRate();
            double endPositionSeconds = pos.endPosition.value() / m_pTrack->getSampleRate();
            QString startPositionText =
                    mixxx::Duration::formatTime(std::min(startPositionSeconds, endPositionSeconds),
                            mixxx::Duration::Precision::CENTISECONDS);
            QString endPositionText = mixxx::Duration::formatTime(
                    std::max(startPositionSeconds, endPositionSeconds),
                    mixxx::Duration::Precision::
                            CENTISECONDS);
            positionText =
                    QString("%1 %2 %3")
                            .arg(startPositionText,
                                    m_pCue->getType() == mixxx::CueType::Loop
                                            ? "-"
                                            : (startPositionSeconds < endPositionSeconds
                                                              ? "⟵"
                                                              : "⟶"),
                                    endPositionText);
        } else {
            double startPositionSeconds = pos.startPosition.value() / m_pTrack->getSampleRate();
            positionText = mixxx::Duration::formatTime(startPositionSeconds,
                    mixxx::Duration::Precision::CENTISECONDS);
        }
        m_pCuePosition->setText(positionText);

        m_pEditLabel->setText(m_pCue->getLabel());
        m_pColorPicker->setSelectedColor(m_pCue->getColor());
        m_pStandardCue->setChecked(m_pCue->getType() == mixxx::CueType::HotCue);
        m_pSavedLoopCue->setChecked(m_pCue->getType() == mixxx::CueType::Loop);
        m_pSavedJumpCue->setChecked(m_pCue->getType() == mixxx::CueType::Jump);
        QString direction;
        if (m_pCue->getType() == mixxx::CueType::HotCue) {
            // Use forward/backward icon if the playposition is before/after
            // the hotcue position
            auto cueStartEnd = m_pCue->getStartAndEndPosition();
            auto newPosition = cueStartEnd.endPosition;
            if (!newPosition.isValid()) {
                newPosition = getCurrentPlayPositionWithQuantize();
            }
            if (!newPosition.isValid() ||
                    std::abs(newPosition - cueStartEnd.startPosition) <=
                            kMinimumAudibleLoopSizeFrames) {
                direction = "impossible";
            } else if (newPosition < cueStartEnd.startPosition) {
                direction = "forward";
            } else {
                direction = "backward";
            }
        } else {
            const bool isforward = m_pCue->getType() != mixxx::CueType::Jump ||
                    m_pCue->getPosition() > m_pCue->getEndPosition();
            // Use forward icon if this is a saved loop, or forward/back if this
            // already is a jump cue
            direction = isforward
                    ? "forward"
                    : "backward";
            m_pSavedJumpCue->setToolTip(
                    //: \n is a linebreak. Try to not to extend the translation
                    //: beyond the length of the longest source line so the
                    //: tooltip remains compact.
                    (isforward ? tr("Turn this cue into a saved forward jump.")
                               : tr("Turn this cue into a saved backward jump "
                                    "(one shot loop).")) +
                    "\n\n" +
                    tr("Left-click: Use the old size if known or the current "
                       "play position as jump start position\n"
                       "If this is already a jump cue, swap the jump position "
                       "and the cue/target position.") +
                    "\n\n" +
                    tr("Right-click: use current play position as new jump "
                       "start position"));
        }
        m_pSavedJumpCue->setProperty("direction", direction);
        m_pSavedJumpCue->style()->polish(m_pSavedJumpCue.get());
        m_pSavedJumpCue->repaint();
        m_pExportCue->setEnabled(m_pCue->getStartAndEndPosition().startPosition.isValid());
    } else {
        m_pTrack.reset();
        m_pCue.reset();
        m_pCueNumber->setText(QString(""));
        m_pCuePosition->setText(QString(""));
        m_pEditLabel->setText(QString(""));
        m_pColorPicker->setSelectedColor(std::nullopt);
        m_pExportCue->setEnabled(false);
    }
    updateExportToSamplerButtons();
}

void WCueMenuPopup::updateExportToSamplerButtons() {
    const bool canExport = m_pCue != nullptr &&
            m_pCue->getStartAndEndPosition().startPosition.isValid();

    for (auto& btn : m_pExportToSamplerButtons) {
        if (btn) {
            btn->setEnabled(canExport);
        }
    }
}

void WCueMenuPopup::slotEditLabel() {
    VERIFY_OR_DEBUG_ASSERT(m_pCue != nullptr) {
        return;
    }
    m_pCue->setLabel(m_pEditLabel->text());
}

void WCueMenuPopup::slotChangeCueColor(mixxx::RgbColor::optional_t color) {
    VERIFY_OR_DEBUG_ASSERT(m_pCue != nullptr) {
        return;
    }
    VERIFY_OR_DEBUG_ASSERT(color) {
        return;
    }
    m_pCue->setColor(*color);
    m_pColorPicker->setSelectedColor(color);
    hide();
}

void WCueMenuPopup::slotDeleteCue() {
    VERIFY_OR_DEBUG_ASSERT(m_pCue != nullptr) {
        return;
    }
    VERIFY_OR_DEBUG_ASSERT(m_pTrack != nullptr) {
        return;
    }
    m_pTrack->removeCue(m_pCue);
    hide();
}

void WCueMenuPopup::slotStandardCue() {
    VERIFY_OR_DEBUG_ASSERT(m_pCue != nullptr) {
        return;
    }
    VERIFY_OR_DEBUG_ASSERT(m_pTrack != nullptr) {
        return;
    }
    if (m_pCue->getType() != mixxx::CueType::HotCue) {
        updateTypeAndColorIfDefault(mixxx::CueType::HotCue);
    }
    slotUpdate();
}

void WCueMenuPopup::slotSavedLoopCueAuto() {
    VERIFY_OR_DEBUG_ASSERT(m_pCue != nullptr) {
        return;
    }
    VERIFY_OR_DEBUG_ASSERT(m_pTrack != nullptr) {
        return;
    }
    VERIFY_OR_DEBUG_ASSERT(m_pBeatLoopSize.valid()) {
        return;
    }
    auto cueStartEnd = m_pCue->getStartAndEndPosition();
    // If we are changing the cue type from a jump, we need to permute the positions
    if (m_pCue->getType() == mixxx::CueType::Jump) {
        auto endPosition = cueStartEnd.endPosition;
        if (cueStartEnd.endPosition < cueStartEnd.startPosition) {
            // Only swap value if this is a forward jump
            cueStartEnd.endPosition = cueStartEnd.startPosition;
            cueStartEnd.startPosition = endPosition;
        }
        m_pCue->setStartAndEndPosition(cueStartEnd.startPosition, cueStartEnd.endPosition);
    }
    if (!cueStartEnd.endPosition.isValid() ||
            cueStartEnd.endPosition <= cueStartEnd.startPosition) {
        double beatloopSize = m_pBeatLoopSize.get();
        const mixxx::BeatsPointer pBeats = m_pTrack->getBeats();
        if (beatloopSize <= 0 || !pBeats) {
            return;
        }
        auto position = pBeats->findNBeatsFromPosition(
                cueStartEnd.startPosition, beatloopSize);
        if (position <= m_pCue->getPosition()) {
            return;
        }
        m_pCue->setEndPosition(position);
    }
    updateTypeAndColorIfDefault(mixxx::CueType::Loop);
    slotUpdate();
}

mixxx::audio::FramePos WCueMenuPopup::getCurrentPlayPositionWithQuantize() const {
    const mixxx::BeatsPointer pBeats = m_pTrack->getBeats();
    auto position = mixxx::audio::FramePos::fromEngineSamplePos(
            m_pPlayPos.get() * m_pTrackSample.get());
    if (m_pQuantizeEnabled.toBool() && pBeats) {
        mixxx::audio::FramePos nextBeatPosition, prevBeatPosition;
        pBeats->findPrevNextBeats(position, &prevBeatPosition, &nextBeatPosition, false);
        return (nextBeatPosition - position > position - prevBeatPosition)
                ? prevBeatPosition
                : nextBeatPosition;
    }
    return position;
}

void WCueMenuPopup::slotSavedLoopCueManual() {
    VERIFY_OR_DEBUG_ASSERT(m_pCue != nullptr) {
        return;
    }
    VERIFY_OR_DEBUG_ASSERT(m_pTrack != nullptr) {
        return;
    }
    // If we are changing the cue type from a jump, we need to permute the
    // positions if it wasn't going backward
    if (m_pCue->getType() == mixxx::CueType::Jump &&
            m_pCue->getPosition() > m_pCue->getEndPosition()) {
        auto cueStartEnd = m_pCue->getStartAndEndPosition();
        auto endPosition = cueStartEnd.endPosition;
        cueStartEnd.endPosition = cueStartEnd.startPosition;
        cueStartEnd.startPosition = endPosition;
        m_pCue->setStartAndEndPosition(cueStartEnd.startPosition, cueStartEnd.endPosition);
    }
    auto newPosition = getCurrentPlayPositionWithQuantize();
    if (newPosition <= m_pCue->getPosition()) {
        return;
    }
    m_pCue->setEndPosition(newPosition);
    updateTypeAndColorIfDefault(mixxx::CueType::Loop);
    slotUpdate();
}

void WCueMenuPopup::slotSavedJumpCueAuto() {
    VERIFY_OR_DEBUG_ASSERT(m_pCue != nullptr) {
        slotUpdate();
        return;
    }
    VERIFY_OR_DEBUG_ASSERT(m_pTrack != nullptr) {
        slotUpdate();
        return;
    }
    auto cueStartEnd = m_pCue->getStartAndEndPosition();
    // If we are changing the cue type from a loop, we need to permute the position
    // Also, if the type is already a jump, we swap to the to/from point
    if (m_pCue->getType() == mixxx::CueType::Loop || m_pCue->getType() == mixxx::CueType::Jump) {
        auto endPosition = cueStartEnd.endPosition;
        cueStartEnd.endPosition = cueStartEnd.startPosition;
        cueStartEnd.startPosition = endPosition;
    }
    if (!cueStartEnd.endPosition.isValid()) {
        auto newPosition = getCurrentPlayPositionWithQuantize();
        if (std::abs(newPosition - cueStartEnd.startPosition) <=
                kMinimumAudibleLoopSizeFrames) {
            slotUpdate();
            return;
        }
        cueStartEnd.endPosition = newPosition;
    }
    m_pCue->setStartAndEndPosition(cueStartEnd.startPosition, cueStartEnd.endPosition);
    updateTypeAndColorIfDefault(mixxx::CueType::Jump);
    slotUpdate();
}

void WCueMenuPopup::slotSavedJumpCueManual() {
    VERIFY_OR_DEBUG_ASSERT(m_pCue != nullptr) {
        return;
    }
    VERIFY_OR_DEBUG_ASSERT(m_pTrack != nullptr) {
        return;
    }
    auto cueStartEnd = m_pCue->getStartAndEndPosition();
    auto newPosition = getCurrentPlayPositionWithQuantize();
    if (newPosition == cueStartEnd.startPosition) {
        return;
    }
    cueStartEnd.endPosition = newPosition;
    m_pCue->setStartAndEndPosition(cueStartEnd.startPosition, cueStartEnd.endPosition);
    updateTypeAndColorIfDefault(mixxx::CueType::Jump);
    slotUpdate();
}

void WCueMenuPopup::closeEvent(QCloseEvent* event) {
    if (m_pTrack && m_pCue) {
        // Check if this is a hotcue, and -if yes- if it has an end position
        // from previous loop cue or temporary state.
        // If yes, remove it.
        if (m_pCue->getType() == mixxx::CueType::HotCue && m_pCue->getEndPosition().isValid()) {
            m_pCue->setEndPosition(mixxx::audio::FramePos());
        }
    }
    emit aboutToHide();
    QWidget::closeEvent(event);
}

bool WCueMenuPopup::trackHasStems() const {
    if (!m_pTrack) {
        return false;
    }
    return m_pTrack->hasStem();
}

QString WCueMenuPopup::buildExportPath(const QString& trackId,
        const QString& artist,
        const QString& title,
        const QString& tag,
        const QString& ext) const {
    const QString settingsPath = m_pConfig->getSettingsPath();
    const QString samplesDir = QDir(settingsPath).filePath(QStringLiteral("Samples"));

    if (!QDir().mkpath(samplesDir)) {
        qWarning() << "[WCUEMENUPOPUP] -> Sample Export: could not create Samples dir"
                   << samplesDir;
        return QString();
    }

    QString safeArtist = artist;
    QString safeTitle = title;
    if (safeArtist.isEmpty()) {
        safeArtist = QStringLiteral("Unknown");
    }
    if (safeTitle.isEmpty()) {
        safeTitle = QStringLiteral("Untitled");
    }

    safeArtist.replace(kUnsafeFilenameChars, QStringLiteral("_"));
    safeTitle.replace(kUnsafeFilenameChars, QStringLiteral("_"));

    const QString baseName = QStringLiteral("%1_%2-%3-[%4]")
                                     .arg(trackId,
                                             safeArtist,
                                             safeTitle,
                                             tag);

    return QDir(samplesDir).filePath(QStringLiteral("%1.%2").arg(baseName, ext));
}

void WCueMenuPopup::getStemState(std::array<double, 4>& stemGains) const {
    stemGains = {1.0, 1.0, 1.0, 1.0};

    if (m_group.isEmpty() || !trackHasStems()) {
        return;
    }

    int deckNumber = -1;
    if (!PlayerManager::isDeckGroup(m_group, &deckNumber)) {
        return;
    }

    for (int i = 0; i < 4; ++i) {
        const QString stemGroup =
                PlayerManager::groupForDeckStem(deckNumber - 1, i);

        PollingControlProxy stemMute(
                stemGroup, QStringLiteral("mute"), ControlFlag::AllowMissingOrInvalid);
        PollingControlProxy stemVolume(
                stemGroup, QStringLiteral("volume"), ControlFlag::AllowMissingOrInvalid);

        if (!stemMute.valid() || !stemVolume.valid()) {
            stemGains = {1.0, 1.0, 1.0, 1.0};
            return;
        }

        stemGains[i] = stemMute.toBool() ? 0.0 : stemVolume.get();
    }
}

bool WCueMenuPopup::exportLoopByStreamCopy(const QString& src,
        const mixxx::audio::FramePos& start,
        const mixxx::audio::FramePos& end,
        const QString& dst,
        const QString& title,
        bool isStemFile,
        bool blocking) {
    if (!m_pTrack) {
        return false;
    }
    const double sampleRate = m_pTrack->getSampleRate();
    if (sampleRate <= 0.0) {
        return false;
    }

    const double startSec = start.value() / sampleRate;
    const double durationSec =
            static_cast<double>(end - start) / sampleRate;

    if (durationSec <= 0.0) {
        return false;
    }

    QStringList args;
    args << QStringLiteral("-hide_banner")
         << QStringLiteral("-loglevel") << QStringLiteral("error")
         << QStringLiteral("-ss") << QString::number(startSec, 'f', 6)
         << QStringLiteral("-t") << QString::number(durationSec, 'f', 6)
         << QStringLiteral("-i") << src;

    if (isStemFile) {
        // stream 0 in the stem container
        args << QStringLiteral("-map") << QStringLiteral("0:0");
    } else {
        // 1st audio stream, ignore video/cover-art
        args << QStringLiteral("-map") << QStringLiteral("0:a:0");
    }

    // set metadata title to distinguish the exported sample from the original track in the library
    // add a Mixxx-genre tag to easu retrieve & delete exported samples from the library later
    args << QStringLiteral("-metadata")
         << QStringLiteral("title=%1").arg(title)
         << QStringLiteral("-metadata")
         << QStringLiteral("genre=Mixxx-Exported-HotCue-To-Sampler");

    args << QStringLiteral("-c:a") << QStringLiteral("copy")
         << QStringLiteral("-y") << dst;

    if (blocking) {
        QProcess proc;
        proc.start(QStringLiteral("ffmpeg"), args);
        if (!proc.waitForStarted(5000)) {
            qWarning() << "[WCUEMENUPOPUP] -> Sample Export: ffmpeg failed to start";
            return false;
        }
        if (!proc.waitForFinished(-1)) {
            qWarning() << "[WCUEMENUPOPUP] -> Sample Export: ffmpeg did not finish";
            return false;
        }
        if (proc.exitStatus() != QProcess::NormalExit || proc.exitCode() != 0) {
            qWarning() << "[WCUEMENUPOPUP] -> Sample Export: ffmpeg error"
                       << proc.readAllStandardError();
            return false;
        }
        return true;
    }

    auto* proc = new QProcess(this);
    QObject::connect(proc,
            QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            proc,
            [proc, dst](int exitCode, QProcess::ExitStatus status) {
                if (status != QProcess::NormalExit || exitCode != 0) {
                    qWarning() << "[WCUEMENUPOPUP] -> Sample Export: ffmpeg error"
                               << proc->readAllStandardError();
                } else {
                    qDebug() << "[WCUEMENUPOPUP] -> Sample Export: " << dst;
                }
                proc->deleteLater();
            });
    QObject::connect(proc,
            &QProcess::errorOccurred,
            proc,
            [proc](QProcess::ProcessError err) {
                qWarning() << "[WCUEMENUPOPUP] -> Sample Export: ffmpeg process error" << err
                           << proc->errorString();
            });
    proc->start(QStringLiteral("ffmpeg"), args);

    return true;
}

bool WCueMenuPopup::exportLoopByRendering(const mixxx::audio::FramePos& start,
        const mixxx::audio::FramePos& end,
        const std::array<double, 4>& stemGains,
        bool isStemFile,
        const QString& dst,
        const QString& title,
        bool blocking) {
    if (!m_pTrack || !isStemFile) {
        return false;
    }
    const double sampleRate = m_pTrack->getSampleRate();
    if (sampleRate <= 0.0) {
        return false;
    }

    const double startSec = start.value() / sampleRate;
    const double durationSec =
            static_cast<double>(end - start) / sampleRate;
    if (durationSec <= 0.0) {
        return false;
    }

    const QString src = m_pTrack->getLocation();

    QStringList args;
    args << QStringLiteral("-hide_banner")
         << QStringLiteral("-loglevel") << QStringLiteral("error")
         << QStringLiteral("-ss") << QString::number(startSec, 'f', 6)
         << QStringLiteral("-t") << QString::number(durationSec, 'f', 6)
         << QStringLiteral("-i") << src;

    // stream 0 is original premix, streams 1..4 the individual stems.
    // only use render path when 1+ stems are muted or have a volume other than 1.0.
    // -> original premix is ignored
    //
    // not all ffmpeg versions support normalize in amix,
    // -> we need to compensate for the division by the number of inputs ourselves.
    // amix divides each input by the number of inputs
    // = default in all ffmpeg versions
    // -> pre-multiply each gain by numInputs so amix
    // division cancels out and the output is the plain weighted sum.
    constexpr int numInputs = 4;
    constexpr double amixCompensation = static_cast<double>(numInputs);

    QStringList filterParts;
    for (int i = 0; i < numInputs; ++i) {
        // skip premix stream 0
        filterParts << QStringLiteral("[0:a:%1]volume=%2[a%3]")
                               .arg(i + 1)
                               .arg(stemGains[i] * amixCompensation, 0, 'f', 6)
                               .arg(i);
    }
    filterParts << QStringLiteral(
            "[a0][a1][a2][a3]amix=inputs=4[out]");

    args << QStringLiteral("-filter_complex") << filterParts.join(';')
         << QStringLiteral("-map") << QStringLiteral("[out]");

    // set metadata title to distinguish the exported sample from the original track in the library
    // add a Mixxx-genre tag to easu retrieve & delete exported samples from the library later
    args << QStringLiteral("-metadata")
         << QStringLiteral("title=%1").arg(title)
         << QStringLiteral("-metadata")
         << QStringLiteral("genre=Mixxx-Exported-HotCue-To-Sampler");

    args << QStringLiteral("-c:a") << QStringLiteral("pcm_f32le")
         << QStringLiteral("-y") << dst;

    qDebug() << "[WCUEMENUPOPUP] - ffmpeg render args:" << args;

    if (blocking) {
        QProcess proc;
        proc.start(QStringLiteral("ffmpeg"), args);
        if (!proc.waitForStarted(5000)) {
            qWarning() << "[WCUEMENUPOPUP] -> Sample Export: ffmpeg failed to start";
            return false;
        }
        if (!proc.waitForFinished(-1)) {
            qWarning() << "[WCUEMENUPOPUP] -> Sample Export: ffmpeg did not finish";
            return false;
        }
        if (proc.exitStatus() != QProcess::NormalExit || proc.exitCode() != 0) {
            qWarning() << "[WCUEMENUPOPUP] -> Sample Export: ffmpeg error"
                       << proc.readAllStandardError();
            return false;
        }
        return true;
    }

    auto* proc = new QProcess(this);
    QObject::connect(proc,
            QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            proc,
            [proc, dst](int exitCode, QProcess::ExitStatus status) {
                if (status != QProcess::NormalExit || exitCode != 0) {
                    qWarning() << "[WCUEMENUPOPUP] -> Sample Export: ffmpeg error"
                               << proc->readAllStandardError();
                } else {
                    qDebug() << "[WCUEMENUPOPUP] -> Sample Export: " << dst;
                }
                proc->deleteLater();
            });
    QObject::connect(proc,
            &QProcess::errorOccurred,
            proc,
            [proc](QProcess::ProcessError err) {
                qWarning() << "[WCUEMENUPOPUP] -> Sample Export: ffmpeg process error" << err
                           << proc->errorString();
            });
    proc->start(QStringLiteral("ffmpeg"), args);

    return true;
}

QString WCueMenuPopup::exportCueToFile(bool blocking) {
    if (!m_pCue || !m_pTrack) {
        return QString();
    }

    const auto pos = m_pCue->getStartAndEndPosition();
    if (!pos.startPosition.isValid()) {
        return QString();
    }

    // compute the export range.
    // -> loop with a valid forward range: use the loop range
    // -> savedjump with a valid (positive) range: use the jump range
    // -> else: normal hotcue =  loop with broken range
    // -> preference sample length from the cue position
    mixxx::audio::FramePos endPosition;
    if (((m_pCue->getType() == mixxx::CueType::Loop) ||
                (m_pCue->getType() == mixxx::CueType::Jump)) &&
            pos.endPosition.isValid() &&
            pos.endPosition > pos.startPosition) {
        endPosition = pos.endPosition;
    } else {
        const double sampleRate = m_pTrack->getSampleRate();
        if (sampleRate <= 0.0) {
            return QString();
        }
        const int sampleLengthSec = m_pConfig->getValue(
                ConfigKey("[Controls]", "NonLoopSampleLengthSec"), 5);
        endPosition = pos.startPosition +
                mixxx::audio::FrameDiff_t(sampleRate * sampleLengthSec);
    }

    if (endPosition <= pos.startPosition) {
        qWarning() << "[WCUEMENUPOPUP] -> Sample Export: computed end position is not after start";
        return QString();
    }

    std::array<double, 4> stemGains = {1.0, 1.0, 1.0, 1.0};
    getStemState(stemGains);

    const bool hasStems = trackHasStems();

    // untouched means all 4 stems are unmuted and at 100% volume
    // -> their sum is the original premix
    // -> we can stream-copy the original premix (stream 0) directly.
    bool untouchedMix;
    if (!hasStems) {
        untouchedMix = true;
    } else {
        untouchedMix = std::all_of(stemGains.begin(),
                stemGains.end(),
                [](double g) { return g == 1.0; });
    }

    const QString src = m_pTrack->getLocation();
    const QFileInfo srcInfo(src);

    const TrackId trackId = m_pTrack->getId();
    const QString artist = m_pTrack->getArtist();
    const QString title = m_pTrack->getTitle();
    const int hotcueNumber = m_pCue->getHotCue() + 1;

    QString tag;
    if (m_pCue->getType() == mixxx::CueType::Loop) {
        tag = QStringLiteral("HC-%1-LOOP")
                      .arg(hotcueNumber, 2, 10, QChar('0'));
    } else {
        tag = QStringLiteral("HC-%1")
                      .arg(hotcueNumber, 2, 10, QChar('0'));
    }

    // a changed cue produces a different signature, if eg the file is locked on windows
    // we can't replacethe previous exported sample.
    // To check if the existing file can be reused we add a hash,
    // if new cue parameters produce a different hash, we export a new file
    // -> all done to avoid a lot of samples being imported in the library
    QString sigInput = QStringLiteral("%1|%2|%3")
                               .arg(pos.startPosition.value())
                               .arg(endPosition.value())
                               .arg(untouchedMix ? 1 : 0);
    for (double g : stemGains) {
        sigInput += QStringLiteral("|%1").arg(g, 0, 'f', 4);
    }
    const QByteArray sigHash = QCryptographicHash::hash(
            sigInput.toUtf8(), QCryptographicHash::Sha1);
    const QString signature = QString::fromLatin1(sigHash.toHex().left(6));

    const QString tagWithSig = tag + QStringLiteral("-") + signature;

    const QString exportTitle = QStringLiteral("%1 [%2]").arg(title, tag);

    const QString dst = untouchedMix
            ? buildExportPath(trackId.toString(),
                      artist,
                      title,
                      tagWithSig,
                      srcInfo.suffix())
            : buildExportPath(trackId.toString(),
                      artist,
                      title,
                      tagWithSig,
                      QStringLiteral("wav"));

    if (dst.isEmpty()) {
        qWarning() << "[WCUEMENUPOPUP] -> Sample Export: could not resolve export path";
        return QString();
    }

    // If the file already exists with this exact signature,
    // the export is unchanged
    // -> reuse it as it is.
    // -> when the file is currently loaded in a sampler: no need to eject or reload.
    if (QFile::exists(dst)) {
        qDebug() << "[WCUEMENUPOPUP] -> Sample Export: unchanged export exists,"
                 << "reusing" << dst;
        return dst;
    }

    qDebug() << "[WCUEMENUPOPUP] -> Sample Export -> decision:"
             << "stemGains=" << stemGains[0] << stemGains[1]
             << stemGains[2] << stemGains[3]
             << "untouchedMix=" << untouchedMix
             << "hasStems=" << hasStems
             << "start=" << pos.startPosition.value()
             << "end=" << endPosition.value()
             << "signature=" << signature;

    bool ok = false;
    if (untouchedMix) {
        ok = exportLoopByStreamCopy(src,
                pos.startPosition,
                endPosition,
                dst,
                exportTitle,
                hasStems,
                blocking);
    } else {
        ok = exportLoopByRendering(pos.startPosition,
                endPosition,
                stemGains,
                hasStems,
                dst,
                exportTitle,
                blocking);
    }

    if (!ok) {
        qWarning() << "[WCUEMENUPOPUP] -> Sample Export: export failed";
        return QString();
    }

    return dst;
}

void WCueMenuPopup::slotExportCue() {
    const QString path = exportCueToFile(/*blocking=*/false);
    if (path.isEmpty()) {
        qWarning() << "[WCUEMENUPOPUP] -> Sample Export: loop export failed";
    }
    hide();
}

void WCueMenuPopup::slotExportToSampler(int samplerIndex) {
    if (!m_pCue || !m_pTrack) {
        hide();
        return;
    }

    PlayerManager* pPlayerManager = PlayerManager::instance();
    if (!pPlayerManager) {
        qWarning() << "[WCUEMENUPOPUP] -> Sample Export: No PlayerManager instance available";
        hide();
        return;
    }

    // Block export to a sampler that's currently playing.
    // Loading a track into a playing sampler stops playback, which is
    // disruptive during a live set and will make the audience booing,
    // DJs don't like boohoo.

    const QString targetGroup = PlayerManager::groupForSampler(samplerIndex);
    if (samplerIsPlaying(targetGroup)) {
        qWarning() << "[WCUEMENUPOPUP] -> Sample Export: target sampler"
                   << (samplerIndex + 1) << "is playing, refusing to export";
        hide();
        return;
    }

    const QString path = exportCueToFile(/*blocking=*/true);
    if (path.isEmpty()) {
        qWarning() << "[WCUEMENUPOPUP] -> Sample Export to sampler"
                   << (samplerIndex + 1) << "failed";
        hide();
        return;
    }

    pPlayerManager->slotLoadToSampler(path, samplerIndex + 1);

    // enable repeat on the sampler for loop cues
    const double repeatValue =
            (m_pCue->getType() == mixxx::CueType::Loop) ? 1.0 : 0.0;
    ControlObject::set(ConfigKey(targetGroup, QStringLiteral("repeat")),
            repeatValue);
    ControlObject::set(ConfigKey("[Playlist]", QStringLiteral("ToggleSelectedSidebarItem")), 1);

    hide();
}

bool WCueMenuPopup::isFileLoadedInAnySampler(const QString& path) const {
    PlayerManager* pPlayerManager = PlayerManager::instance();
    if (!pPlayerManager) {
        return false;
    }

    const unsigned int numSamplers = pPlayerManager->numberOfSamplers();
    for (unsigned int i = 1; i <= numSamplers; ++i) {
        // only call getSampler when we know the index is valid
        // -> numberOfSamplers() = configured count
        const QString group = PlayerManager::groupForSampler(i - 1);
        BaseTrackPlayer* pPlayer = pPlayerManager->getPlayer(group);
        if (!pPlayer) {
            continue;
        }
        TrackPointer pLoadedTrack = pPlayer->getLoadedTrack();
        if (pLoadedTrack && pLoadedTrack->getLocation() == path) {
            return true;
        }
    }
    return false;
}
