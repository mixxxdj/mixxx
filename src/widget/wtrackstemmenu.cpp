#include "widget/wtrackstemmenu.h"

#include <QAction>

#include "moc_wtrackstemmenu.cpp"

namespace {
const QList<mixxx::StemChannel> stemTracks = {
        mixxx::StemChannel::PreMix,
        mixxx::StemChannel::First,
        mixxx::StemChannel::Second,
        mixxx::StemChannel::Third,
        mixxx::StemChannel::Fourth,
};
} // namespace

WTrackStemMenu::WTrackStemMenu(const QString& label,
        QWidget* parent,
        bool primaryDeck,
        const QString& group,
        const QList<StemInfo>& stemInfo)
        : QMenu(label, parent),
          m_group(group),
          m_selectMode(false),
          m_stemInfo(stemInfo),
          m_currentSelection() {
    if (primaryDeck) {
        QAction* pAction = new QAction(tr("Load for stem mixing"), this);
        addAction(pAction);
        connect(pAction, &QAction::triggered, this, [this, group] {
            emit selectedStem(group, {mixxx::StemChannelSelection()});
        });
    }

    QAction* pAction = new QAction(tr("Load pre-mixed stereo track"), this);
    addAction(pAction);
    connect(pAction, &QAction::triggered, this, [this, group] {
        emit selectedStem(group, mixxx::StemChannel::PreMix);
    });
    addSeparator();

    // DEBUG_ASSERT(stemTracks.count() == mixxx::kMaxSupportedStems);
    // int stemIdx = 0;
    // for (const auto& stemTrack : stemTracks) {
    //     m_stemActions.emplace_back(
    //             make_parented<QAction>(tr("Load the \"%1\" stem")
    //                                            .arg(m_stemInfo.at(stemIdx).getLabel()),
    //                     this));
    //     addAction(m_stemActions.back().get());
    //     connect(m_stemActions.back().get(), &QAction::triggered, this, [this, stemTrack] {
    //         emit selectedStem(m_group, stemTrack);
    //     });
    //     connect(m_stemActions.back().get(),
    //             &QAction::toggled,
    //             this,
    //             [this, stemTrack](bool checked) {
    //                 m_currentSelection.setFlag(stemTrack, checked);
    //             });
    //     stemIdx++;
    // }
    m_selectAction = make_parented<QAction>(this);
    m_selectAction->setToolTip(tr("Load multiple stem into a stereo deck"));
    m_selectAction->setDisabled(true);
    addAction(m_selectAction.get());
    installEventFilter(this);
}

// WTrackStemMenu::WTrackStemMenu(const QString& label,
//         QWidget* parent,
//         bool primaryDeck,
//         const QString& group,
//         const QList<StemInfo>& stemInfo)
//         : QMenu(label, parent),
//           m_group(group),
//           m_selectMode(false),
//           m_stemInfo(stemInfo),
//           m_currentSelection() {
//     if (primaryDeck) {
//         QAction* pAction = new QAction(tr("Load for stem mixing"), this);
//         addAction(pAction);
//         connect(pAction, &QAction::triggered, this, [this, group] {
//             emit selectedStem(group, {mixxx::StemChannelSelection()});
//         });
//     }
//
//     // Pre-mixed stream (index 0)
//     QAction* pAction = new QAction(tr("Load pre-mixed stereo track"), this);
//     addAction(pAction);
//     connect(pAction, &QAction::triggered, this, [this, group] {
//         emit selectedStem(group, mixxx::StemChannel::PreMix);
//     });
//     addSeparator();
//
////// Expect m_stemInfo to contain the labels for streams 1..(kMaxSupportedStems-1)
////    const int numStems = m_stemInfo.count(); // typically 4
////    // Defensive: allow 0..(kMaxSupportedStems-1)
////    VERIFY_OR_DEBUG_ASSERT(numStems >= 0 && numStems <= mixxx::kMaxSupportedStems - 1) {
////        // bail out if we got something unexpected
////        return;
////    }
////
////    for (int idx = 0; idx < numStems; ++idx) {
////        // streamIndex = 1..4  (stream 0 is PreMix and already has its action)
////        const int streamIndex = idx + 1;
////
////        const auto& info = m_stemInfo.at(idx); // <-- correct: use idx (0..3) for labels
////        m_stemActions.push_back(make_parented<QAction>(
////                tr("Load the \"%1\" stem").arg(info.getLabel()), this));
////
////        QAction* action = m_stemActions.back().get();
////        addAction(action);
////
////        // create a local copy of the StemChannel to capture by value in the lambdas
////        const mixxx::StemChannel stemChannel =
////                static_cast<mixxx::StemChannel>(streamIndex);
////
////        // capture stemChannel by value so each lambda gets the correct one
////        connect(action, &QAction::triggered, this, [this, stemChannel] {
////            emit selectedStem(m_group, stemChannel);
////        });
////
////        connect(action, &QAction::toggled, this, [this, stemChannel](bool checked) {
////            m_currentSelection.setFlag(stemChannel, checked);
////        });
////    }
////
//
//
//    m_selectAction = make_parented<QAction>(this);
//    m_selectAction->setToolTip(tr("Load multiple stem into a stereo deck"));
//    m_selectAction->setDisabled(true);
//    addAction(m_selectAction.get());
//    installEventFilter(this);
//}

// WTrackStemMenu::WTrackStemMenu(const QString& label,
//         QWidget* parent,
//         bool primaryDeck,
//         const QString& group,
//         const QList<StemInfo>& stemInfo)
//         : QMenu(label, parent),
//           m_group(group),
//           m_selectMode(false),
//           m_stemInfo(stemInfo),
//           m_currentSelection() {
//     if (primaryDeck) {
//         QAction* pAction = new QAction(tr("Load for stem mixing"), this);
//         addAction(pAction);
//         connect(pAction, &QAction::triggered, this, [this, group] {
//             emit selectedStem(group, {mixxx::StemChannelSelection()});
//         });
//     }
//
//     QAction* pAction = new QAction(tr("Load pre-mixed stereo track"), this);
//     addAction(pAction);
//     connect(pAction, &QAction::triggered, this, [this, group] {
//         emit selectedStem(group, mixxx::StemChannel::PreMix);
//     });
//     addSeparator();
//
//     DEBUG_ASSERT(stemTracks.count() == mixxx::kMaxSupportedStems);
//     int stemIdx = 0;
//     for (const auto& stemTrack : stemTracks) {
//         m_stemActions.emplace_back(
//                 make_parented<QAction>(tr("Load the \"%1\" stem")
//                                                .arg(m_stemInfo.at(stemIdx).getLabel()),
//                         this));
//         addAction(m_stemActions.back().get());
//         connect(m_stemActions.back().get(), &QAction::triggered, this, [this, stemTrack] {
//             emit selectedStem(m_group, stemTrack);
//         });
//         connect(m_stemActions.back().get(),
//                 &QAction::toggled,
//                 this,
//                 [this, stemTrack](bool checked) {
//                     m_currentSelection.setFlag(stemTrack, checked);
//                 });
//         stemIdx++;
//     }
//     m_selectAction = make_parented<QAction>(this);
//     m_selectAction->setToolTip(tr("Load multiple stem into a stereo deck"));
//     m_selectAction->setDisabled(true);
//     addAction(m_selectAction.get());
//     installEventFilter(this);
// }

bool WTrackStemMenu::eventFilter(QObject* pObj, QEvent* e) {
    QInputEvent* pInputEvent = dynamic_cast<QInputEvent*>(e);
    if (pInputEvent != nullptr) {
        bool selectMode = pInputEvent->modifiers().testFlag(Qt::ControlModifier);
        if (selectMode != m_selectMode) {
            m_selectMode = selectMode;
            updateActions();
        }
    }

    if (m_selectMode && (e->type() == QEvent::MouseButtonRelease)) {
        QAction* pAction = activeAction();
        if (pAction && pAction->isCheckable()) {
            pAction->setChecked(!pAction->isChecked());
            updateActions();
            return true;
        }
    }
    return QObject::eventFilter(pObj, e);
}
void WTrackStemMenu::updateActions() {
    for (const auto& pAction : m_stemActions) {
        pAction->setCheckable(m_selectMode);
    }
    m_selectAction->setText(m_selectMode
                    ? !m_currentSelection ? tr("Select stems to load")
                                          : tr("Release \"CTRL\" to load the "
                                               "current selection")
                    : tr("Use \"CTRL\" to select multiple stems"));
}

void WTrackStemMenu::showEvent(QShowEvent* pQEvent) {
    updateActions();
    QMenu::showEvent(pQEvent);
}

void WTrackStemMenu::keyPressEvent(QKeyEvent* pQEvent) {
    m_selectMode = pQEvent->modifiers() & Qt::ControlModifier;
    updateActions();
    pQEvent->accept();
}

void WTrackStemMenu::keyReleaseEvent(QKeyEvent* pQEvent) {
    bool selectMode = pQEvent->modifiers() & Qt::ControlModifier;
    if (!selectMode && m_selectMode && m_currentSelection) {
        emit selectedStem(m_group, m_currentSelection);
        m_currentSelection = mixxx::StemChannelSelection();
    }
    m_selectMode = selectMode;
    updateActions();
    pQEvent->accept();
}
