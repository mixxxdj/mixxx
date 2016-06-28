#include <QMessageBox>

#include "library/autodj/dlgautodj.h"

#include "library/playlisttablemodel.h"
#include "widget/wtracktableview.h"
#include "util/assert.h"
#include "util/time.h"

DlgAutoDJ::DlgAutoDJ(QWidget* parent,
                     Library* pLibrary,
                     AutoDJProcessor* pProcessor)
        : QWidget(parent),
          Ui::DlgAutoDJ(),
          m_pAutoDJProcessor(pProcessor),
          // no sorting
          m_pAutoDJTableModel(nullptr),
          m_pLibrary(pLibrary),
          m_focusedPane(-1) {
    setupUi(this);

    // We do _NOT_ take ownership of this from AutoDJProcessor.
    m_pAutoDJTableModel = m_pAutoDJProcessor->getTableModel();

    // Override some playlist-view properties:

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

    // Setup DlgAutoDJ UI based on the current AutoDJProcessor state. Keep in
    // mind that AutoDJ may already be active when DlgAutoDJ is created (due to
    // skin changes, etc.).
    spinBoxTransition->setValue(m_pAutoDJProcessor->getTransitionTime());
    connect(m_pAutoDJProcessor, SIGNAL(transitionTimeChanged(int)),
            this, SLOT(transitionTimeChanged(int)));
    connect(m_pAutoDJProcessor, SIGNAL(autoDJStateChanged(AutoDJProcessor::AutoDJState)),
            this, SLOT(autoDJStateChanged(AutoDJProcessor::AutoDJState)));
    autoDJStateChanged(m_pAutoDJProcessor->getState());
}

DlgAutoDJ::~DlgAutoDJ() {
    qDebug() << "~DlgAutoDJ()";
}

void DlgAutoDJ::onShow() {
    m_pAutoDJTableModel->select();
}

void DlgAutoDJ::setTrackTableView(WTrackTableView* pTrackTableView, int paneId) {
    pTrackTableView->loadTrackModel(m_pAutoDJTableModel);
    
    m_trackTables[paneId] = pTrackTableView;
    
    connect(pTrackTableView, SIGNAL(loadTrack(TrackPointer)),
            this, SIGNAL(loadTrack(TrackPointer)));
    connect(pTrackTableView, SIGNAL(loadTrackToPlayer(TrackPointer, QString, bool)),
            this, SIGNAL(loadTrackToPlayer(TrackPointer, QString, bool)));
    connect(pTrackTableView, SIGNAL(trackSelected(TrackPointer)),
            this, SIGNAL(trackSelected(TrackPointer)));
    connect(pTrackTableView, SIGNAL(trackSelected(TrackPointer)),
            this, SLOT(updateSelectionInfo()));
    
    updateSelectionInfo();
}

void DlgAutoDJ::setFocusedPane(int focusedPane) {
    m_focusedPane = focusedPane;
}

void DlgAutoDJ::shufflePlaylistButton(bool) {    
    LibraryView* pView = m_pLibrary->getActiveView();
    WTrackTableView* pTrackTable = dynamic_cast<WTrackTableView*>(pView);
    
    if (pView) {
        QModelIndexList indexList = pTrackTable->selectionModel()->selectedRows();
        // Activate regardless of button being checked
        m_pAutoDJProcessor->shufflePlaylist(indexList);
    }
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
        if (state == AutoDJProcessor::ADJ_P1FADING ||
                state == AutoDJProcessor::ADJ_P2FADING ||
                state == AutoDJProcessor::ADJ_ENABLE_P1LOADED) {
            pushButtonFadeNow->setEnabled(false);
        } else {
            pushButtonFadeNow->setEnabled(true);
        }

        // You can always skip the next track if we are enabled.
        pushButtonSkipNext->setEnabled(true);
    }
}

void DlgAutoDJ::updateSelectionInfo() {
    if (!m_trackTables.contains(m_focusedPane)) {
        labelSelectionInfo->setText("");
        labelSelectionInfo->setEnabled(false);
        return;
    }
    
    int duration = 0;
    WTrackTableView* pTrackTable = m_trackTables[m_focusedPane];
    if (!pTrackTable) {
        return;
    }
    
    QModelIndexList indices = pTrackTable->selectionModel()->selectedRows();
    for (int i = 0; i < indices.size(); ++i) {
        TrackPointer pTrack = m_pAutoDJTableModel->getTrack(indices.at(i));
        if (pTrack) {
            duration += pTrack->getDuration();
        }
    }

    QString label;
    if (!indices.isEmpty()) {
        label.append(Time::formatSeconds(duration));
        label.append(QString(" (%1)").arg(indices.size()));
        labelSelectionInfo->setText(label);
        labelSelectionInfo->setEnabled(true);
    } else {
        labelSelectionInfo->setText("");
        labelSelectionInfo->setEnabled(false);
    }
}
