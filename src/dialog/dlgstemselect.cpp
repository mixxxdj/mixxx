#include "dialog/dlgstemselect.h"

#include <QDir>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QSvgRenderer>

#include "control/controlobject.h"
#include "library/coverartcache.h"
#include "library/library_prefs.h"
#include "moc_dlgstemselect.cpp"
#include "track/track.h"
#include "widget/wcoverartlabel.h"

namespace {

} // namespace

DlgStemSelect::DlgStemSelect(QWidget* parent)
        : QDialog(parent) {
    setupUi(this);

    // setWindowFlags(Qt::Popup);
    // setAttribute(Qt::WA_StyledBackground);

    m_pWCoverArtLabel = make_parented<WCoverArtLabel>(this);

    // Cover art
    CoverArtCache* pCache = CoverArtCache::instance();
    if (pCache) {
        connect(pCache,
                &CoverArtCache::coverFound,
                this,
                &DlgStemSelect::slotCoverFound);
    }

    coverLayout->setAlignment(Qt::AlignRight | Qt::AlignTop);
    coverLayout->setSpacing(0);
    coverLayout->setContentsMargins(0, 0, 0, 0);
    coverLayout->insertWidget(0, m_pWCoverArtLabel.get());

    installEventFilter(this);

    connect(buttonAbort,
            &QAbstractButton::clicked,
            this,
            &QDialog::accept);

    connect(radioButtonStem,
            &QRadioButton::toggled,
            this,
            &DlgStemSelect::slotStemMixToggled);
    connect(radioButtonStereo,
            &QRadioButton::toggled,
            this,
            &DlgStemSelect::slotStemMixToggled);

    connect(checkBoxStem1,
            &QCheckBox::stateChanged,
            this,
            &DlgStemSelect::slotStemChecked);
    connect(checkBoxStem2,
            &QCheckBox::stateChanged,
            this,
            &DlgStemSelect::slotStemChecked);
    connect(checkBoxStem3,
            &QCheckBox::stateChanged,
            this,
            &DlgStemSelect::slotStemChecked);
    connect(checkBoxStem4,
            &QCheckBox::stateChanged,
            this,
            &DlgStemSelect::slotStemChecked);
}

bool DlgStemSelect::eventFilter(QObject* obj, QEvent* event) {
    if (event->type() == QEvent::KeyPress) {
        QKeyEvent* keyEvent = static_cast<QKeyEvent*>(event);
        if (keyEvent->key() == Qt::Key_Tab) {
            return true;
        } else if (keyEvent->key() == Qt::Key_Backtab) {
            return true;
        }
    } else if (event->type() == QEvent::MouseButtonPress) {
        // QMouseEvent* mouseEvent = static_cast<QMouseEvent*>(event);
        // return true;
    }
    // standard event processing
    return QDialog::eventFilter(obj, event);
}

void DlgStemSelect::show(TrackPointer pTrack) {
    m_pTrack = pTrack;

    DEBUG_ASSERT(pTrack->hasStem());

    QString colorStyle;
    auto stemInfoList = pTrack->getStemInfo();
    for (int stemIdx = 0; stemIdx < stemInfoList.count(); stemIdx++) {
        switch (stemIdx) {
        case 0:
            checkBoxStem1->setText(stemInfoList.at(stemIdx).getLabel());
            colorStyle =
                    QString("QCheckBox { color: %1; }")
                            .arg(stemInfoList.at(stemIdx).getColor().name());
            checkBoxStem1->setStyleSheet(colorStyle);
            break;
        case 1:
            checkBoxStem2->setText(stemInfoList.at(stemIdx).getLabel());
            colorStyle =
                    QString("QCheckBox { color: %1; }")
                            .arg(stemInfoList.at(stemIdx).getColor().name());
            checkBoxStem2->setStyleSheet(colorStyle);
            break;
        case 2:
            checkBoxStem3->setText(stemInfoList.at(stemIdx).getLabel());
            colorStyle =
                    QString("QCheckBox { color: %1; }")
                            .arg(stemInfoList.at(stemIdx).getColor().name());
            checkBoxStem3->setStyleSheet(colorStyle);
            break;
        case 3:
            checkBoxStem4->setText(stemInfoList.at(stemIdx).getLabel());
            colorStyle =
                    QString("QCheckBox { color: %1; }")
                            .arg(stemInfoList.at(stemIdx).getColor().name());
            checkBoxStem4->setStyleSheet(colorStyle);
            break;
        }
    }

    m_pWCoverArtLabel->loadTrack(pTrack);

    const auto coverInfo = CoverInfo(
            m_pTrack->getCoverInfo(),
            m_pTrack->getLocation());
    m_pWCoverArtLabel->setCoverArt(coverInfo, QPixmap());
    // Executed concurrently
    CoverArtCache::requestCover(this, coverInfo);

    QDialog::show();
}

void DlgStemSelect::slotStemMixToggled(bool checked) {
    if (checked) {
        checkBoxStem1->setChecked(false);
        checkBoxStem2->setChecked(false);
        checkBoxStem3->setChecked(false);
        checkBoxStem4->setChecked(false);
    }
}

void DlgStemSelect::slotStemChecked(int state) {
    if (state == Qt::Checked) {
        blockSignals(true);
        buttonGroup->setExclusive(false);
        radioButtonStem->setChecked(false);
        radioButtonStereo->setChecked(false);
        buttonGroup->setExclusive(true);
        blockSignals(false);
    }
}

void DlgStemSelect::slotCoverFound(
        const QObject* pRequester,
        const CoverInfo& coverInfo,
        const QPixmap& pixmap) {
    if (pRequester == this &&
            m_pTrack &&
            m_pTrack->getLocation() == coverInfo.trackLocation) {
        m_pWCoverArtLabel->setCoverArt(coverInfo, pixmap);
    }
}
