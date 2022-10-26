#include "library/tracksuggestion/dlgtracksuggestion.h"

#include "control/controlobject.h"
#include "library/trackcollectionmanager.h"
#include "util/assert.h"
#include "widget/wlibrary.h"
#include "widget/wskincolor.h"
#include "widget/wtracktableview.h"
#include "widget/wwidget.h"

DlgTrackSuggestion::DlgTrackSuggestion(WLibrary* parent,
        UserSettingsPointer pConfig,
        Library* pLibrary,
        KeyboardEventFilter* pKeyboard,
        SuggestionFetcher* pSuggestionFetcher,
        BaseExternalPlaylistModel*
                pSuggestionTableModel) // This is temporaraly added for to have better dlg.
        : QWidget(parent),
          m_pConfig(pConfig),
          m_pTrackTableView(new WTrackTableView(this,
                  pConfig,
                  pLibrary,
                  parent->getTrackTableBackgroundColorOpacity(),
                  true)) {
    setupUi(this);
    fetchProgressBar->setVisible(false);
    fetchProgressBar->setMinimum(0);
    fetchProgressBar->setMaximum(0);
    m_pTrackTableView->installEventFilter(pKeyboard);

    connect(btnFindSimilar,
            &QPushButton::clicked,
            this,
            &DlgTrackSuggestion::slotStartFetching);

    connect(pSuggestionFetcher,
            &SuggestionFetcher::changeButtonLabel,
            this,
            &DlgTrackSuggestion::slotChangeButtonLabel);

    connect(pSuggestionFetcher,
            &SuggestionFetcher::fetchSuggestionProgress,
            this,
            &DlgTrackSuggestion::slotShowFetchingProgress);

    connect(pSuggestionFetcher,
            &SuggestionFetcher::suggestionResults,
            this,
            &DlgTrackSuggestion::suggestionResults);

    connect(pSuggestionFetcher,
            &SuggestionFetcher::networkError,
            this,
            &DlgTrackSuggestion::slotNetworkError);

    QBoxLayout* box = qobject_cast<QBoxLayout*>(layout());
    VERIFY_OR_DEBUG_ASSERT(box) {
    }
    else {
        box->removeWidget(m_pTrackTablePlaceholder);
        m_pTrackTablePlaceholder->hide();
        box->insertWidget(1, m_pTrackTableView);
    }

    m_pTrackTableView->loadTrackModel(pSuggestionTableModel);
}

DlgTrackSuggestion::~DlgTrackSuggestion() {
    delete m_pTrackTableView;
}

bool DlgTrackSuggestion::hasFocus() const {
    return m_pTrackTableView->hasFocus();
}

void DlgTrackSuggestion::loadSelectedTrackToGroup(const QString& group, bool play) {
    m_pTrackTableView->loadSelectedTrackToGroup(group, play);
}

//Couldn't pass the track properties (Track Pointer) into this class.
//Instead there is a little hacky solution. For the initial PR, Pressing button signal emitted from here.
//But actual fetching starts in the TrackSuggestion Feature.
void DlgTrackSuggestion::slotStartFetching() {
    emit buttonPressed();
}

void DlgTrackSuggestion::slotChangeButtonLabel(const QString& buttonLabel) {
    fetchProgressBar->setVisible(false);
    btnFindSimilar->setEnabled(true);
    btnFindSimilar->setVisible(true);
    btnFindSimilar->setText(tr("Look Track Suggestions For %1").arg(buttonLabel));
}

// Message is not written on the progress bar since it shows busy indicator
// Later on GUI can be changed, so message can be used.
void DlgTrackSuggestion::slotShowFetchingProgress(const QString& message) {
    fetchProgressBar->setVisible(true);
    //fetchProgressBar->setFormat(message);
    qDebug() << message;
    btnFindSimilar->setVisible(false);
}

void DlgTrackSuggestion::slotNetworkError(
        int httpStatus, const QString& app, const QString& message, int code) {
    QString cantConnect = tr("Mixxx can't connect to %1.");
    fetchProgressBar->setVisible(false);
    btnFindSimilar->setVisible(true);
    btnFindSimilar->setText(cantConnect.arg(app));
    btnFindSimilar->setEnabled(false);
}
