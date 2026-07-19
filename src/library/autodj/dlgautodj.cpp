#include "library/autodj/dlgautodj.h"

#include <QDateTime>
#include <QKeyEvent>
#include <QLineEdit>
#include <QLocale>
#include <QMessageBox>

#include "controllers/keyboard/keyboardeventfilter.h"
#include "library/library.h"
#include "library/playlisttablemodel.h"
#include "moc_dlgautodj.cpp"
#include "track/track.h"
#include "util/assert.h"
#include "util/duration.h"
#include "widget/wlibrary.h"
#include "widget/wtracktableview.h"

namespace {
const char* kPreferenceGroupName = "[Auto DJ]";
const char* kRepeatPlaylistPreference = "Requeue";
const char* kQueueInfoShowEndTime = "QueueInfoShowEndTime";
} // anonymous namespace

DlgAutoDJ::DlgAutoDJ(WLibrary* parent,
        UserSettingsPointer pConfig,
        Library* pLibrary,
        AutoDJProcessor* pProcessor,
        KeyboardEventFilter* pKeyboard)
        : QWidget(parent),
          Ui::DlgAutoDJ(),
          m_pConfig(pConfig),
          m_pAutoDJProcessor(pProcessor),
          m_pTrackTableView(new WTrackTableView(this,
                  m_pConfig,
                  pLibrary,
                  parent->getTrackTableBackgroundColorOpacity())),
          m_bShowButtonText(parent->getShowButtonText()),
          m_pAutoDJTableModel(nullptr),
          m_queueSeconds(0.0),
          m_showEndTime(m_pConfig->getValue(
                  ConfigKey(kPreferenceGroupName, kQueueInfoShowEndTime), false)),
          m_autoDJWasActive(false) {
    setupUi(this);

    labelSelectionInfo->hide();
    connect(pushButtonQueueInfo, &QPushButton::clicked, this, [this]() {
        m_showEndTime = !m_showEndTime;
        m_pConfig->setValue(
                ConfigKey(kPreferenceGroupName, kQueueInfoShowEndTime), m_showEndTime);
        slotUpdateQueueDuration();
    });

    m_pTrackTableView->installEventFilter(pKeyboard);

    connect(m_pTrackTableView,
            &WTrackTableView::loadTrack,
            this,
            &DlgAutoDJ::loadTrack);
    connect(m_pTrackTableView,
            &WTrackTableView::loadTrackToPlayer,
            this,
            &DlgAutoDJ::loadTrackToPlayer);
    connect(m_pTrackTableView,
            &WTrackTableView::trackSelected,
            this,
            &DlgAutoDJ::trackSelected);
    connect(pLibrary,
            &Library::setTrackTableFont,
            m_pTrackTableView,
            &WTrackTableView::setTrackTableFont);
    connect(pLibrary,
            &Library::setTrackTableRowHeight,
            m_pTrackTableView,
            &WTrackTableView::setTrackTableRowHeight);
    connect(pLibrary,
            &Library::setSelectedClick,
            m_pTrackTableView,
            &WTrackTableView::setSelectedClick);

    QBoxLayout* box = qobject_cast<QBoxLayout*>(layout());
    VERIFY_OR_DEBUG_ASSERT(box) { // Assumes the form layout is a QVBox/QHBoxLayout!
    } else {
        box->removeWidget(m_pTrackTablePlaceholder);
        m_pTrackTablePlaceholder->hide();
        box->insertWidget(1, m_pTrackTableView);
    }

    // We do _NOT_ take ownership of this from AutoDJProcessor.
    m_pAutoDJTableModel = m_pAutoDJProcessor->getTableModel();
    m_pTrackTableView->loadTrackModel(m_pAutoDJTableModel);

    connect(m_pAutoDJTableModel,
            &QAbstractItemModel::rowsInserted,
            this,
            [this](const QModelIndex&, int, int) { slotRecalcQueueDuration(); });
    connect(m_pAutoDJTableModel,
            &QAbstractItemModel::rowsRemoved,
            this,
            [this](const QModelIndex&, int, int) { slotRecalcQueueDuration(); });
    connect(m_pAutoDJTableModel,
            &QAbstractItemModel::modelReset,
            this,
            &DlgAutoDJ::slotRecalcQueueDuration);

    // Do not set this because it disables auto-scrolling
    // m_pTrackTableView->setDragDropMode(QAbstractItemView::InternalMove);

    connect(pushButtonAutoDJ,
            &QPushButton::clicked,
            this,
            &DlgAutoDJ::toggleAutoDJButton);

    setupActionButton(pushButtonFadeNow, &DlgAutoDJ::fadeNowButton, tr("Fade"));
    setupActionButton(pushButtonSkipNext, &DlgAutoDJ::skipNextButton, tr("Skip"));
    setupActionButton(pushButtonShuffle, &DlgAutoDJ::shufflePlaylistButton, tr("Shuffle"));
    setupActionButton(pushButtonAddRandomTrack, &DlgAutoDJ::addRandomTrackButton, tr("Random"));
    setupActionButton(pushButtonAddEndMarker, &DlgAutoDJ::slotAddEndMarker, tr("Add End"));

    m_enableBtnTooltip = tr(
            "Enable Auto DJ\n"
            "\n"
            "Shortcut: Shift+F12");
    m_disableBtnTooltip = tr(
            "Disable Auto DJ\n"
            "\n"
            "Shortcut: Shift+F12");
    QString fadeBtnTooltip = tr(
            "Trigger the transition to the next track\n"
            "\n"
            "Shortcut: Shift+F11");
    QString skipBtnTooltip = tr(
            "Skip the next track in the Auto DJ queue\n"
            "\n"
            "Shortcut: Shift+F10");
    QString shuffleBtnTooltip = tr(
            "Shuffle the content of the Auto DJ queue\n"
            "\n"
            "Shortcut: Shift+F9");
    QString addRandomTrackBtnTooltip = tr(
            "Adds a random track from track sources (crates) to the Auto DJ queue.\n"
            "If no track sources are configured, the track is added from the library instead.");
    QString addEndMarkerBtnTooltip = tr(
            "Inserts an end marker below the selected track.\n"
            "Auto DJ will be disabled when the marker reaches the top of the queue.");
    QString repeatBtnTooltip = tr(
            "Repeat the playlist");
    QString spinBoxTransitionTooltip = tr(
            "Determines the duration of the transition");
    QString labelTransitionTooltip = tr(
            // "sec" as in seconds
            "Seconds");
    QString fadeModeTooltip = tr(
            "Auto DJ Fade Modes\n"
            "\n"
            "Full Intro + Outro:\n"
            "Play the full intro and outro. Use the intro or outro length as the\n"
            "crossfade time, whichever is shorter. If no intro or outro are marked,\n"
            "use the selected crossfade time.\n"
            "\n"
            "Fade At Outro Start:\n"
            "Start crossfading at the outro start. If the outro is longer than the\n"
            "intro, cut off the end of the outro. Use the intro or outro length as\n"
            "the crossfade time, whichever is shorter. If no intro or outro are\n"
            "marked, use the selected crossfade time.\n"
            "\n"
            "Full Track:\n"
            "Play the whole track. Begin crossfading from the selected number of\n"
            "seconds before the end of the track. A negative crossfade time adds\n"
            "silence between tracks.\n"
            "\n"
            "Skip Silence:\n"
            "Play the whole track except for silence at the beginning and end.\n"
            "Begin crossfading from the selected number of seconds before the\n"
            "last sound.\n"
            "\n"
            "Skip Silence Start Full Volume:\n"
            "The same as Skip Silence, but starting transitions with a centered\n"
            "crossfader, so that the intro starts at full volume.\n");

    pushButtonFadeNow->setToolTip(fadeBtnTooltip);
    pushButtonSkipNext->setToolTip(skipBtnTooltip);
    pushButtonShuffle->setToolTip(shuffleBtnTooltip);
    pushButtonAddRandomTrack->setToolTip(addRandomTrackBtnTooltip);
    pushButtonAddEndMarker->setToolTip(addEndMarkerBtnTooltip);
    pushButtonRepeatPlaylist->setToolTip(repeatBtnTooltip);
    spinBoxTransition->setToolTip(spinBoxTransitionTooltip);
    labelTransitionAppendix->setToolTip(labelTransitionTooltip);
    fadeModeCombobox->setToolTip(fadeModeTooltip);

    // Prevent the interactive widgets from being focused with Tab or Shift+Tab
    fadeModeCombobox->setFocusPolicy(Qt::ClickFocus);
    spinBoxTransition->setFocusPolicy(Qt::ClickFocus);
    // work around QLineEdit being protected
    QLineEdit* lineEditTransition(spinBoxTransition->findChild<QLineEdit*>());
    lineEditTransition->setFocusPolicy(Qt::ClickFocus);
    // Needed to catch Enter, Return and Escape keypresses
    lineEditTransition->installEventFilter(this);

    connect(spinBoxTransition,
            QOverload<int>::of(&QSpinBox::valueChanged),
            this,
            &DlgAutoDJ::transitionSliderChanged);

    fadeModeCombobox->addItem(tr("Full Intro + Outro"),
            static_cast<int>(AutoDJProcessor::TransitionMode::FullIntroOutro));
    fadeModeCombobox->addItem(tr("Fade At Outro Start"),
            static_cast<int>(AutoDJProcessor::TransitionMode::FadeAtOutroStart));
    fadeModeCombobox->addItem(tr("Full Track"),
            static_cast<int>(AutoDJProcessor::TransitionMode::FixedFullTrack));
    fadeModeCombobox->addItem(tr("Skip Silence"),
            static_cast<int>(AutoDJProcessor::TransitionMode::FixedSkipSilence));
    fadeModeCombobox->addItem(tr("Skip Silence Start Full Volume"),
            static_cast<int>(AutoDJProcessor::TransitionMode::FixedStartCenterSkipSilence));
    fadeModeCombobox->setCurrentIndex(
            fadeModeCombobox->findData(static_cast<int>(m_pAutoDJProcessor->getTransitionMode())));
    connect(fadeModeCombobox,
            QOverload<int>::of(&QComboBox::activated),
            this,
            &DlgAutoDJ::slotTransitionModeChanged);

    connect(pushButtonRepeatPlaylist,
            &QPushButton::clicked,
            this,
            &DlgAutoDJ::slotRepeatPlaylistChanged);
    if (m_bShowButtonText) {
        pushButtonRepeatPlaylist->setText(tr("Repeat"));
    }
    bool repeatPlaylist = m_pConfig->getValue<bool>(
            ConfigKey(kPreferenceGroupName, kRepeatPlaylistPreference));
    pushButtonRepeatPlaylist->setChecked(repeatPlaylist);
    slotRepeatPlaylistChanged(repeatPlaylist);

    // Setup DlgAutoDJ UI based on the current AutoDJProcessor state. Keep in
    // mind that AutoDJ may already be active when DlgAutoDJ is created (due to
    // skin changes, etc.).
    spinBoxTransition->setValue(static_cast<int>(m_pAutoDJProcessor->getTransitionTime()));
    connect(m_pAutoDJProcessor,
            &AutoDJProcessor::transitionTimeChanged,
            this,
            &DlgAutoDJ::transitionTimeChanged);

    // 1-second timer keeps "Ends at" clock current without requiring queue changes
    m_pQueueDurationTimer = make_parented<QTimer>(this);
    m_pQueueDurationTimer->setInterval(1000);
    connect(m_pQueueDurationTimer,
            &QTimer::timeout,
            this,
            &DlgAutoDJ::slotUpdateQueueDuration);

    connect(m_pAutoDJProcessor,
            &AutoDJProcessor::autoDJError,
            this,
            &DlgAutoDJ::autoDJError);

    connect(m_pAutoDJProcessor,
            &AutoDJProcessor::autoDJStateChanged,
            this,
            &DlgAutoDJ::autoDJStateChanged);
    autoDJStateChanged(m_pAutoDJProcessor->getState());
}

DlgAutoDJ::~DlgAutoDJ() {
    qDebug() << "~DlgAutoDJ()";

    // Delete m_pTrackTableView before the table model. This is because the
    // table view saves the header state using the model.
    delete m_pTrackTableView;
}

void DlgAutoDJ::setupActionButton(QPushButton* pButton,
        void (DlgAutoDJ::*pSlot)(bool),
        const QString& fallbackText) {
    connect(pButton, &QPushButton::clicked, this, pSlot);
    if (m_bShowButtonText) {
        pButton->setText(fallbackText);
    }
}

void DlgAutoDJ::onShow() {
    m_pAutoDJTableModel->select();
    slotRecalcQueueDuration();
    m_pQueueDurationTimer->stop();
    m_pQueueDurationTimer->start();
}

void DlgAutoDJ::hideEvent(QHideEvent* event) {
    m_pQueueDurationTimer->stop();
    QWidget::hideEvent(event);
}

void DlgAutoDJ::onSearch(const QString& text) {
    // Do not allow filtering the Auto DJ playlist, because
    // Auto DJ will work from the filtered table
    Q_UNUSED(text);
}

void DlgAutoDJ::shufflePlaylistButton(bool) {
    QModelIndexList indexList = m_pTrackTableView->selectionModel()->selectedRows();

    // Activate regardless of button being checked
    m_pAutoDJProcessor->shufflePlaylist(indexList);
}

void DlgAutoDJ::skipNextButton(bool) {
    // Activate regardless of button being checked
    m_pAutoDJProcessor->skipNext();
}

void DlgAutoDJ::fadeNowButton(bool) {
    // Activate regardless of button being checked
    m_pAutoDJProcessor->fadeNow();
}

void DlgAutoDJ::toggleAutoDJButton(bool enable) {
    m_pAutoDJProcessor->toggleAutoDJ(enable);
}

// TODO If there's a way to migrate the translations move this
// to AutoDJProcessor in order to keep this class minimal
void DlgAutoDJ::autoDJError(AutoDJProcessor::AutoDJError error) {
    switch (error) {
    case AutoDJProcessor::ADJ_NOT_TWO_DECKS:
        QMessageBox::warning(nullptr,
                tr("Auto DJ"),
                tr("Auto DJ requires two decks assigned to opposite sides of the crossfader."),
                QMessageBox::Ok);
        break;
    case AutoDJProcessor::ADJ_BOTH_DECKS_PLAYING:
        QMessageBox::warning(nullptr,
                tr("Auto DJ"),
                tr("One deck must be stopped to enable Auto DJ mode."),
                QMessageBox::Ok);
        break;
    case AutoDJProcessor::ADJ_UNUSED_DECK_PLAYING:
        QMessageBox::warning(nullptr,
                tr("Auto DJ"),
                tr("Decks not used for Auto DJ must be stopped to enable Auto DJ mode."),
                QMessageBox::Ok);
        break;
    case AutoDJProcessor::ADJ_OK:
    default:
        break;
    }
}

void DlgAutoDJ::transitionTimeChanged(int time) {
    spinBoxTransition->setValue(time);
    slotRecalcQueueDuration();
}

void DlgAutoDJ::transitionSliderChanged(int value) {
    m_pAutoDJProcessor->setTransitionTime(value);
}

void DlgAutoDJ::autoDJStateChanged(AutoDJProcessor::AutoDJState state) {
    if (state == AutoDJProcessor::ADJ_DISABLED) {
        pushButtonAutoDJ->setChecked(false);
        pushButtonAutoDJ->setToolTip(m_enableBtnTooltip);
        if (m_bShowButtonText) {
            pushButtonAutoDJ->setText(tr("Enable"));
        }
        pushButtonFadeNow->setEnabled(false);
        pushButtonSkipNext->setEnabled(false);
    } else {
        m_autoDJWasActive = true;
        // No matter the mode, you can always disable once it is enabled.
        pushButtonAutoDJ->setChecked(true);
        pushButtonAutoDJ->setToolTip(m_disableBtnTooltip);
        if (m_bShowButtonText) {
            pushButtonAutoDJ->setText(tr("Disable"));
        }

        // If fading, you can't hit fade now.
        if (state == AutoDJProcessor::ADJ_LEFT_FADING ||
                state == AutoDJProcessor::ADJ_RIGHT_FADING ||
                state == AutoDJProcessor::ADJ_ENABLE_P1LOADED) {
            pushButtonFadeNow->setEnabled(false);
        } else {
            pushButtonFadeNow->setEnabled(true);
        }

        pushButtonSkipNext->setEnabled(true);
    }
    slotRecalcQueueDuration();
}

void DlgAutoDJ::slotTransitionModeChanged(int newIndex) {
    m_pAutoDJProcessor->setTransitionMode(
            static_cast<AutoDJProcessor::TransitionMode>(
                    fadeModeCombobox->itemData(newIndex).toInt()));
    // Clicking on a transition mode item moves keyboard focus to the list widget.
    // Move focus back to the previously focused library widget.
    refocusPrevWidget();
}

void DlgAutoDJ::slotRepeatPlaylistChanged(bool checked) {
    m_pConfig->setValue(ConfigKey(kPreferenceGroupName, kRepeatPlaylistPreference),
            checked);
}

void DlgAutoDJ::refocusPrevWidget() {
    ControlObject::set(ConfigKey("[Library]", "refocus_prev_widget"), 1);
}

bool DlgAutoDJ::eventFilter(QObject* pObj, QEvent* pEvent) {
    Q_UNUSED(pObj);
    // Catch Enter, Return and Escape from the transition spinbox line edit
    if (pEvent->type() == QEvent::KeyPress) {
        const auto* keyEvent = static_cast<QKeyEvent*>(pEvent);
        if (keyEvent->key() == Qt::Key_Return || keyEvent->key() == Qt::Key_Enter ||
                keyEvent->key() == Qt::Key_Escape) {
            refocusPrevWidget();
            return true;
        }
    }
    return QWidget::eventFilter(pObj, pEvent);
}

void DlgAutoDJ::slotRecalcQueueDuration() {
    m_queueSeconds = 0.0;
    if (m_pAutoDJTableModel) {
        // Collect row indices up to (not including) any end marker
        QModelIndexList indices;
        const int rows = m_pAutoDJTableModel->rowCount();
        for (int row = 0; row < rows; ++row) {
            const QModelIndex idx = m_pAutoDJTableModel->index(row, 0);
            if (m_pAutoDJTableModel->isEndMarker(idx)) {
                break;
            }
            indices.append(idx);
        }
        if (!indices.isEmpty()) {
            // N tracks have N-1 transition overlaps
            const double rawSeconds =
                    m_pAutoDJTableModel->getTotalDuration(indices).toDoubleSeconds();
            const double transitionSecs = m_pAutoDJProcessor->getTransitionTime();
            m_queueSeconds = std::max(
                    0.0, rawSeconds - (indices.size() - 1) * transitionSecs);
        }
    }
    slotUpdateQueueDuration();
}

void DlgAutoDJ::slotUpdateQueueDuration() {
    const bool autoDJActive =
            m_pAutoDJProcessor->getState() != AutoDJProcessor::ADJ_DISABLED;
    double deckSeconds = 0.0;
    if (autoDJActive) {
        deckSeconds = m_pAutoDJProcessor->getActiveDeckRemainingSeconds();
    } else if (m_autoDJWasActive) {
        deckSeconds = m_pAutoDJProcessor->getRemainingDeckSeconds();
    }
    // Subtract one crossfade overlap: the first queued track starts T seconds
    // before the active deck ends, so the queue formula over-counts by T.
    const double transitionSecs = m_pAutoDJProcessor->getTransitionTime();
    const double totalSeconds = m_queueSeconds + deckSeconds -
            (autoDJActive && m_queueSeconds > 0.0 ? transitionSecs : 0.0);
    const bool shouldBeEnabled = autoDJActive || (m_autoDJWasActive && totalSeconds > 0.0);
    if (!shouldBeEnabled) {
        m_autoDJWasActive = false;
    }
    if (pushButtonQueueInfo->isEnabled() != shouldBeEnabled) {
        pushButtonQueueInfo->setEnabled(shouldBeEnabled);
    }
    QString newText;
    if (totalSeconds <= 0.0) {
        newText = m_showEndTime ? tr("Ends: --:-- --") : tr("Queue: -:--:--");
    } else if (m_showEndTime) {
        const QDateTime endTime =
                QDateTime::currentDateTime().addSecs(static_cast<qint64>(totalSeconds));
        newText = tr("Ends: %1").arg(QLocale().toString(endTime.time(), QLocale::ShortFormat));
    } else {
        newText = tr("Queue: %1").arg(mixxx::DurationBase::formatTime(totalSeconds));
    }
    if (newText != pushButtonQueueInfo->text()) {
        pushButtonQueueInfo->setText(newText);
    }
}

bool DlgAutoDJ::hasFocus() const {
    return m_pTrackTableView->hasFocus();
}

void DlgAutoDJ::setFocus() {
    m_pTrackTableView->setFocus();
}

void DlgAutoDJ::pasteFromSidebar() {
    m_pTrackTableView->pasteFromSidebar();
}

void DlgAutoDJ::keyPressEvent(QKeyEvent* pEvent) {
    // If we receive key events either the mode selector or the spinbox are focused.
    // Return, Enter and Escape move focus back to the previously focused
    // library widget in order to immediately allow keyboard shortcuts again.
    if (pEvent->key() == Qt::Key_Return ||
            pEvent->key() == Qt::Key_Enter ||
            pEvent->key() == Qt::Key_Escape) {
        refocusPrevWidget();
        return;
    }
    QWidget::keyPressEvent(pEvent);
}

void DlgAutoDJ::saveCurrentViewState() {
    m_pTrackTableView->saveCurrentViewState();
}

bool DlgAutoDJ::restoreCurrentViewState() {
    return m_pTrackTableView->restoreCurrentViewState();
}

void DlgAutoDJ::slotAddEndMarker(bool) {
    QModelIndexList selection = m_pTrackTableView->selectionModel()->selectedRows();
    int afterRow = -1;
    if (!selection.isEmpty()) {
        afterRow = selection.last().row();
    }
    m_pAutoDJTableModel->insertEndMarker(afterRow);
}
