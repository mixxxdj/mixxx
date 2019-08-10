#include <QMessageBox>

#include "library/autodj/dlgautodj.h"

#include "library/playlisttablemodel.h"
#include "util/assert.h"
#include "util/compatibility.h"
#include "util/duration.h"
#include "widget/wtracktableview.h"

namespace {
const char* kPreferenceGroupName = "[Auto DJ]";
const char* kRepeatPlaylistPreference = "Requeue";
} // anonymous namespace

DlgAutoDJ::DlgAutoDJ(QWidget* parent,
                     UserSettingsPointer pConfig,
                     Library* pLibrary,
                     AutoDJProcessor* pProcessor,
                     TrackCollection* pTrackCollection,
                     KeyboardEventFilter* pKeyboard)
        : QWidget(parent),
          Ui::DlgAutoDJ(),
          m_pAutoDJProcessor(pProcessor),
          // no sorting
          m_pTrackTableView(new WTrackTableView(this, pConfig,
                                                pTrackCollection, false)),
          m_pAutoDJTableModel(nullptr),
          m_pConfig(pConfig) {
    setupUi(this);

    m_pTrackTableView->installEventFilter(pKeyboard);
    connect(m_pTrackTableView, SIGNAL(loadTrack(TrackPointer)),
            this, SIGNAL(loadTrack(TrackPointer)));
    connect(m_pTrackTableView, SIGNAL(loadTrackToPlayer(TrackPointer, QString, bool)),
            this, SIGNAL(loadTrackToPlayer(TrackPointer, QString, bool)));
    connect(m_pTrackTableView, SIGNAL(trackSelected(TrackPointer)),
            this, SIGNAL(trackSelected(TrackPointer)));
    connect(m_pTrackTableView, SIGNAL(trackSelected(TrackPointer)),
            this, SLOT(updateSelectionInfo()));

    connect(pLibrary, SIGNAL(setTrackTableFont(QFont)),
            m_pTrackTableView, SLOT(setTrackTableFont(QFont)));
    connect(pLibrary, SIGNAL(setTrackTableRowHeight(int)),
            m_pTrackTableView, SLOT(setTrackTableRowHeight(int)));
    connect(pLibrary, SIGNAL(setSelectedClick(bool)),
            m_pTrackTableView, SLOT(setSelectedClick(bool)));

    QBoxLayout* box = dynamic_cast<QBoxLayout*>(layout());
    VERIFY_OR_DEBUG_ASSERT(box) { //Assumes the form layout is a QVBox/QHBoxLayout!
    } else {
        box->removeWidget(m_pTrackTablePlaceholder);
        m_pTrackTablePlaceholder->hide();
        box->insertWidget(1, m_pTrackTableView);
    }

    // We do _NOT_ take ownership of this from AutoDJProcessor.
    m_pAutoDJTableModel = m_pAutoDJProcessor->getTableModel();
    m_pTrackTableView->loadTrackModel(m_pAutoDJTableModel);

    // Override some playlist-view properties:

    // Do not set this because it disables auto-scrolling
    //m_pTrackTableView->setDragDropMode(QAbstractItemView::InternalMove);

    connect(pushButtonShuffle, SIGNAL(clicked(bool)),
            this, SLOT(shufflePlaylistButton(bool)));

    connect(pushButtonSkipNext, SIGNAL(clicked(bool)),
            this, SLOT(skipNextButton(bool)));

    connect(pushButtonAddRandom, SIGNAL(clicked(bool)),
            this, SIGNAL(addRandomButton(bool)));

    connect(pushButtonFadeNow, SIGNAL(clicked(bool)),
            this, SLOT(fadeNowButton(bool)));

    connect(spinBoxTransition, SIGNAL(valueChanged(int)),
            this, SLOT(transitionSliderChanged(int)));

    connect(pushButtonAutoDJ, SIGNAL(toggled(bool)),
            this, SLOT(toggleAutoDJButton(bool)));

    fadeModeCombobox->addItem(tr("Intro/Outro (smooth transition)"),
                                static_cast<int>(AutoDJProcessor::TransitionMode::IntroOutroSmooth));
    fadeModeCombobox->addItem(tr("Intro/Outro (quick transition)"),
                                static_cast<int>(AutoDJProcessor::TransitionMode::IntroOutroQuick));
    fadeModeCombobox->addItem(tr("Fixed time (full track)"),
                                static_cast<int>(AutoDJProcessor::TransitionMode::FixedFullTrack));
    fadeModeCombobox->addItem(tr("Fixed time (skip silence)"),
                                static_cast<int>(AutoDJProcessor::TransitionMode::FixedSkipSilence));
    fadeModeCombobox->setCurrentIndex(
            fadeModeCombobox->findData(static_cast<int>(m_pAutoDJProcessor->getTransitionMode())));
    connect(fadeModeCombobox, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &DlgAutoDJ::slotTransitionModeChanged);
    QString fadeModeTooltip = tr(
    "Intro/Outro (smooth transition):\n"
    "Play the full intro and outro. The intro or outro time is\n"
    "used as the crossfade time, whichever is shorter.\n"
    "\n"
    "Intro/Outro (quick transition):\n"
    "Cut off part of the intro or outro, whichever is longer.\n"
    "The intro or outro time is used as the crossfade time,\n"
    "whichever is shorter.\n"
    "\n"
    "Fixed time modes:\n"
    "Use the selected number of seconds as the crossfade time."
    );
    fadeModeCombobox->setToolTip(fadeModeTooltip);
    fadeModeLabel->setToolTip(fadeModeTooltip);

    repeatPlaylistCheckbox->setChecked(m_pConfig->getValue<bool>(
            ConfigKey(kPreferenceGroupName, kRepeatPlaylistPreference)));
    connect(repeatPlaylistCheckbox, &QCheckBox::stateChanged,
            this, &DlgAutoDJ::slotRepeatPlaylistChanged);

    // Setup DlgAutoDJ UI based on the current AutoDJProcessor state. Keep in
    // mind that AutoDJ may already be active when DlgAutoDJ is created (due to
    // skin changes, etc.).
    spinBoxTransition->setValue(m_pAutoDJProcessor->getTransitionTime());
    connect(m_pAutoDJProcessor, SIGNAL(transitionTimeChanged(int)),
            this, SLOT(transitionTimeChanged(int)));
    connect(m_pAutoDJProcessor, SIGNAL(autoDJStateChanged(AutoDJProcessor::AutoDJState)),
            this, SLOT(autoDJStateChanged(AutoDJProcessor::AutoDJState)));
    autoDJStateChanged(m_pAutoDJProcessor->getState());

    updateSelectionInfo();
}

DlgAutoDJ::~DlgAutoDJ() {
    qDebug() << "~DlgAutoDJ()";

    // Delete m_pTrackTableView before the table model. This is because the
    // table view saves the header state using the model.
    delete m_pTrackTableView;
}

void DlgAutoDJ::onShow() {
    m_pAutoDJTableModel->select();
}

void DlgAutoDJ::onSearch(const QString& text) {
    // Do not allow filtering the Auto DJ playlist, because
    // Auto DJ will work from the filtered table
    Q_UNUSED(text);
}

void DlgAutoDJ::loadSelectedTrack() {
    m_pTrackTableView->loadSelectedTrack();
}

void DlgAutoDJ::loadSelectedTrackToGroup(QString group, bool play) {
    m_pTrackTableView->loadSelectedTrackToGroup(group, play);
}

void DlgAutoDJ::moveSelection(int delta) {
    m_pTrackTableView->moveSelection(delta);
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
    AutoDJProcessor::AutoDJError error = m_pAutoDJProcessor->toggleAutoDJ(enable);
    switch (error) {
        case AutoDJProcessor::ADJ_BOTH_DECKS_PLAYING:
            QMessageBox::warning(
                    NULL, tr("Auto DJ"),
                    tr("One deck must be stopped to enable Auto DJ mode."),
                    QMessageBox::Ok);
            // Make sure the button becomes unpushed.
            pushButtonAutoDJ->setChecked(false);
            break;
        case AutoDJProcessor::ADJ_DECKS_3_4_PLAYING:
            QMessageBox::warning(
                    NULL, tr("Auto DJ"),
                    tr("Decks 3 and 4 must be stopped to enable Auto DJ mode."),
                    QMessageBox::Ok);
            pushButtonAutoDJ->setChecked(false);
            break;
        case AutoDJProcessor::ADJ_OK:
        default:
            break;
    }
}

void DlgAutoDJ::transitionTimeChanged(int time) {
    spinBoxTransition->setValue(time);
}

void DlgAutoDJ::transitionSliderChanged(int value) {
    m_pAutoDJProcessor->setTransitionTime(value);
}

void DlgAutoDJ::autoDJStateChanged(AutoDJProcessor::AutoDJState state) {
    if (state == AutoDJProcessor::ADJ_DISABLED) {
        pushButtonAutoDJ->setChecked(false);
        pushButtonAutoDJ->setToolTip(tr("Enable Auto DJ"));
        pushButtonAutoDJ->setText(tr("Enable Auto DJ"));
        pushButtonFadeNow->setEnabled(false);
        pushButtonSkipNext->setEnabled(false);
    } else {
        // No matter the mode, you can always disable once it is enabled.
        pushButtonAutoDJ->setChecked(true);
        pushButtonAutoDJ->setToolTip(tr("Disable Auto DJ"));
        pushButtonAutoDJ->setText(tr("Disable Auto DJ"));

        // If fading, you can't hit fade now.
        if (state == AutoDJProcessor::ADJ_LEFT_FADING ||
                state == AutoDJProcessor::ADJ_RIGHT_FADING ||
                state == AutoDJProcessor::ADJ_ENABLE_P1LOADED) {
            pushButtonFadeNow->setEnabled(false);
        } else {
            pushButtonFadeNow->setEnabled(true);
        }

        // You can always skip the next track if we are enabled.
        pushButtonSkipNext->setEnabled(true);
    }
}

void DlgAutoDJ::slotTransitionModeChanged(int comboboxIndex) {
    m_pAutoDJProcessor->setTransitionMode(static_cast<AutoDJProcessor::TransitionMode>(
          fadeModeCombobox->itemData(comboboxIndex).toInt()));
}

void DlgAutoDJ::slotRepeatPlaylistChanged(int checkState) {
    m_pConfig->setValue(ConfigKey(kPreferenceGroupName, kRepeatPlaylistPreference),
            static_cast<bool>(checkState));
}

void DlgAutoDJ::updateSelectionInfo() {
    double duration = 0.0;

    QModelIndexList indices = m_pTrackTableView->selectionModel()->selectedRows();

    for (int i = 0; i < indices.size(); ++i) {
        TrackPointer pTrack = m_pAutoDJTableModel->getTrack(indices.at(i));
        if (pTrack) {
            duration += pTrack->getDuration();
        }
    }

    QString label;

    if (!indices.isEmpty()) {
        label.append(mixxx::DurationBase::formatTime(duration));
        label.append(QString(" (%1)").arg(indices.size()));
        labelSelectionInfo->setToolTip(tr("Displays the duration and number of selected tracks."));
        labelSelectionInfo->setText(label);
        labelSelectionInfo->setEnabled(true);
    } else {
        labelSelectionInfo->setText("");
        labelSelectionInfo->setEnabled(false);
    }
}

bool DlgAutoDJ::hasFocus() const {
    return QWidget::hasFocus();
}
