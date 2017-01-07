#include "library/librarycontrol.h"

#include <QApplication>
#include <QItemSelectionModel>
#include <QModelIndex>
#include <QModelIndexList>
#include <QtDebug>

#include "control/controlobject.h"
#include "control/controlpushbutton.h"
#include "mixer/playermanager.h"
#include "widget/wlibrary.h"
#include "widget/wlibrarysidebar.h"
#include "library/library.h"
#include "library/libraryview.h"
#include "util/container.h"

LoadToGroupController::LoadToGroupController(QObject* pParent, const QString& group)
        : QObject(pParent),
          m_group(group) {
    m_pLoadControl = new ControlPushButton(ConfigKey(group, "LoadSelectedTrack"));
    connect(m_pLoadControl, SIGNAL(valueChanged(double)),
            this, SLOT(slotLoadToGroup(double)));

    m_pLoadAndPlayControl = new ControlPushButton(ConfigKey(group, "LoadSelectedTrackAndPlay"));
    connect(m_pLoadAndPlayControl, SIGNAL(valueChanged(double)),
            this, SLOT(slotLoadToGroupAndPlay(double)));

    connect(this, SIGNAL(loadToGroup(QString, bool)),
            pParent, SLOT(slotLoadSelectedTrackToGroup(QString, bool)));
}

LoadToGroupController::~LoadToGroupController() {
    delete m_pLoadControl;
    delete m_pLoadAndPlayControl;
}

void LoadToGroupController::slotLoadToGroup(double v) {
    if (v > 0) {
        emit(loadToGroup(m_group, false));
    }
}

void LoadToGroupController::slotLoadToGroupAndPlay(double v) {
    if (v > 0) {
        emit(loadToGroup(m_group, true));
    }
}

LibraryControl::LibraryControl(Library* pLibrary)
        : QObject(pLibrary),
          m_pLibrary(pLibrary),
          m_numDecks("[Master]", "num_decks", this),
          m_numSamplers("[Master]", "num_samplers", this),
          m_numPreviewDecks("[Master]", "num_preview_decks", this) {

    slotNumDecksChanged(m_numDecks.get());
    slotNumSamplersChanged(m_numSamplers.get());
    slotNumPreviewDecksChanged(m_numPreviewDecks.get());
    m_numDecks.connectValueChanged(SLOT(slotNumDecksChanged(double)));
    m_numSamplers.connectValueChanged(SLOT(slotNumSamplersChanged(double)));
    m_numPreviewDecks.connectValueChanged(SLOT(slotNumPreviewDecksChanged(double)));

    // Controls to navigate vertically within currently focussed widget (up/down buttons)
    m_pMoveUp = new ControlPushButton(ConfigKey("[Playlist]", "MoveUp"));
    m_pMoveDown = new ControlPushButton(ConfigKey("[Playlist]", "MoveDown"));
    m_pMoveVertical = new ControlObject(ConfigKey("[Playlist]", "MoveVertical"), false);
    connect(m_pMoveUp, SIGNAL(valueChanged(double)),this, SLOT(slotMoveUp(double)));
    connect(m_pMoveDown, SIGNAL(valueChanged(double)),this, SLOT(slotMoveDown(double)));
    connect(m_pMoveVertical, SIGNAL(valueChanged(double)),this, SLOT(slotMoveVertical(double)));

    // Controls to navigate horizontally within currently selected item (left/right buttons)
    m_pMoveLeft = new ControlPushButton(ConfigKey("[Playlist]", "MoveLeft"));
    m_pMoveRight = new ControlPushButton(ConfigKey("[Playlist]", "MoveRight"));
    m_pMoveHorizontal = new ControlObject(ConfigKey("[Playlist]", "MoveHorizontal"), false);
    connect(m_pMoveLeft, SIGNAL(valueChanged(double)),this, SLOT(slotMoveLeft(double)));
    connect(m_pMoveRight, SIGNAL(valueChanged(double)),this, SLOT(slotMoveRight(double)));
    connect(m_pMoveHorizontal, SIGNAL(valueChanged(double)),this, SLOT(slotMoveHorizontal(double)));

    // Control to navigate between widgets (tab/shit+tab button)
    m_pMoveFocusForward = new ControlPushButton(ConfigKey("[Playlist]", "MoveFocusForward"));
    m_pMoveFocusBackward = new ControlPushButton(ConfigKey("[Playlist]", "MoveFocusBackward"));
    m_pMoveFocus = new ControlObject(ConfigKey("[Playlist]", "MoveFocus"), false);
    connect(m_pMoveFocusForward, SIGNAL(valueChanged(double)),this, SLOT(slotMoveFocusForward(double)));
    connect(m_pMoveFocusBackward, SIGNAL(valueChanged(double)),this, SLOT(slotMoveFocusBackward(double)));
    connect(m_pMoveFocus, SIGNAL(valueChanged(double)),this, SLOT(slotMoveFocus(double)));

    // Control to choose the currently selected item in focussed widget (double click)
    m_pChooseItem = new ControlPushButton(ConfigKey("[Playlist]", "ChooseItem"));
    connect(m_pChooseItem, SIGNAL(valueChanged(double)), this, SLOT(slotChooseItem(double)));


    // Font sizes
    m_pFontSizeKnob = new ControlObject(
            ConfigKey("[Library]", "font_size_knob"), false);
    connect(m_pFontSizeKnob, SIGNAL(valueChanged(double)),
            this, SLOT(slotFontSize(double)));

    m_pFontSizeDecrement = new ControlPushButton(
            ConfigKey("[Library]", "font_size_decrement"));
    connect(m_pFontSizeDecrement, SIGNAL(valueChanged(double)),
            this, SLOT(slotDecrementFontSize(double)));

    m_pFontSizeIncrement = new ControlPushButton(
            ConfigKey("[Library]", "font_size_increment"));
    connect(m_pFontSizeIncrement, SIGNAL(valueChanged(double)),
            this, SLOT(slotIncrementFontSize(double)));


    /// Deprecated controls
    m_pSelectNextTrack = new ControlPushButton(ConfigKey("[Playlist]", "SelectNextTrack"));
    connect(m_pSelectNextTrack, SIGNAL(valueChanged(double)),
            this, SLOT(slotSelectNextTrack(double)));


    m_pSelectPrevTrack = new ControlPushButton(ConfigKey("[Playlist]", "SelectPrevTrack"));
    connect(m_pSelectPrevTrack, SIGNAL(valueChanged(double)),
            this, SLOT(slotSelectPrevTrack(double)));

    // Ignoring no-ops is important since this is for +/- tickers.
    m_pSelectTrack = new ControlObject(ConfigKey("[Playlist]","SelectTrackKnob"), false);
    connect(m_pSelectTrack, SIGNAL(valueChanged(double)),
            this, SLOT(slotSelectTrack(double)));


    m_pSelectNextSidebarItem = new ControlPushButton(ConfigKey("[Playlist]", "SelectNextPlaylist"));
    connect(m_pSelectNextSidebarItem, SIGNAL(valueChanged(double)),
            this, SLOT(slotSelectNextSidebarItem(double)));

    m_pSelectPrevSidebarItem = new ControlPushButton(ConfigKey("[Playlist]", "SelectPrevPlaylist"));
    connect(m_pSelectPrevSidebarItem, SIGNAL(valueChanged(double)),
            this, SLOT(slotSelectPrevSidebarItem(double)));

    // Ignoring no-ops is important since this is for +/- tickers.
    m_pSelectSidebarItem = new ControlObject(ConfigKey("[Playlist]", "SelectPlaylist"), false);
    connect(m_pSelectSidebarItem, SIGNAL(valueChanged(double)),
            this, SLOT(slotSelectSidebarItem(double)));

    m_pToggleSidebarItem = new ControlPushButton(ConfigKey("[Playlist]", "ToggleSelectedSidebarItem"));
    connect(m_pToggleSidebarItem, SIGNAL(valueChanged(double)),
            this, SLOT(slotToggleSelectedSidebarItem(double)));

    m_pLoadSelectedIntoFirstStopped = new ControlPushButton(ConfigKey("[Playlist]","LoadSelectedIntoFirstStopped"));
    connect(m_pLoadSelectedIntoFirstStopped, SIGNAL(valueChanged(double)),
            this, SLOT(slotLoadSelectedIntoFirstStopped(double)));

    m_pAutoDjAddTop = new ControlPushButton(ConfigKey("[Playlist]","AutoDjAddTop"));
    connect(m_pAutoDjAddTop, SIGNAL(valueChanged(double)),
            this, SLOT(slotAutoDjAddTop(double)));

    m_pAutoDjAddBottom = new ControlPushButton(ConfigKey("[Playlist]","AutoDjAddBottom"));
    connect(m_pAutoDjAddBottom, SIGNAL(valueChanged(double)),
            this, SLOT(slotAutoDjAddBottom(double)));

}

LibraryControl::~LibraryControl() {
   delete m_pSelectNextTrack;
   delete m_pSelectPrevTrack;
   delete m_pSelectTrack;
   delete m_pSelectNextSidebarItem;
   delete m_pSelectPrevSidebarItem;
   delete m_pSelectSidebarItem;
   delete m_pToggleSidebarItem;
   delete m_pLoadSelectedIntoFirstStopped;
   delete m_pAutoDjAddTop;
   delete m_pAutoDjAddBottom;
   delete m_pFontSizeKnob;
   delete m_pFontSizeDecrement;
   delete m_pFontSizeIncrement;
   deleteMapValues(&m_loadToGroupControllers);
}

void LibraryControl::maybeCreateGroupController(const QString& group) {
    LoadToGroupController* pGroup = m_loadToGroupControllers.value(group, nullptr);
    if (!pGroup) {
        pGroup = new LoadToGroupController(this, group);
        m_loadToGroupControllers[group] = pGroup;
    }
}

void LibraryControl::slotNumDecksChanged(double v) {
    int iNumDecks = v;

    if (iNumDecks < 0) {
        return;
    }

    for (int i = 0; i < iNumDecks; ++i) {
        maybeCreateGroupController(PlayerManager::groupForDeck(i));
    }
}

void LibraryControl::slotNumSamplersChanged(double v) {
    int iNumSamplers = v;

    if (iNumSamplers < 0) {
        return;
    }

    for (int i = 0; i < iNumSamplers; ++i) {
        maybeCreateGroupController(PlayerManager::groupForSampler(i));
    }
}

void LibraryControl::slotNumPreviewDecksChanged(double v) {
    int iNumPreviewDecks = v;

    if (iNumPreviewDecks < 0) {
        return;
    }

    for (int i = 0; i < iNumPreviewDecks; ++i) {
        maybeCreateGroupController(PlayerManager::groupForPreviewDeck(i));
    }
}

void LibraryControl::bindSidebarWidget(WLibrarySidebar* pSidebarWidget) {
    if (m_pSidebarWidget) {
        disconnect(m_pSidebarWidget, 0, this, 0);
    }
    m_pSidebarWidget = pSidebarWidget;
    connect(m_pSidebarWidget, SIGNAL(destroyed()),
            this, SLOT(sidebarWidgetDeleted()));
}

void LibraryControl::bindWidget(WLibrary* pLibraryWidget, KeyboardEventFilter* pKeyboard) {
    Q_UNUSED(pKeyboard);
    if (m_pLibraryWidget) {
        disconnect(m_pLibraryWidget, 0, this, 0);
    }
    m_pLibraryWidget = pLibraryWidget;
    connect(m_pLibraryWidget, SIGNAL(destroyed()),
            this, SLOT(libraryWidgetDeleted()));
}

void LibraryControl::libraryWidgetDeleted() {
    m_pLibraryWidget = nullptr;
}

void LibraryControl::sidebarWidgetDeleted() {
    m_pSidebarWidget = nullptr;
}

void LibraryControl::slotLoadSelectedTrackToGroup(QString group, bool play) {
    if (!m_pLibraryWidget) {
        return;
    }

    LibraryView* activeView = m_pLibraryWidget->getActiveView();
    if (!activeView) {
        return;
    }
    activeView->loadSelectedTrackToGroup(group, play);
}

void LibraryControl::slotLoadSelectedIntoFirstStopped(double v) {
    if (!m_pLibraryWidget) {
        return;
    }

    if (v > 0) {
        LibraryView* activeView = m_pLibraryWidget->getActiveView();
        if (!activeView) {
            return;
        }
        activeView->loadSelectedTrack();
    }
}

void LibraryControl::slotAutoDjAddTop(double v) {
    if (!m_pLibraryWidget) {
        return;
    }

    if (v > 0) {
        LibraryView* activeView = m_pLibraryWidget->getActiveView();
        if (!activeView) {
            return;
        }
        activeView->slotSendToAutoDJTop();
    }
}

void LibraryControl::slotAutoDjAddBottom(double v) {
    if (!m_pLibraryWidget) {
        return;
    }

    if (v > 0) {
        LibraryView* activeView = m_pLibraryWidget->getActiveView();
        if (!activeView) {
            return;
        }
        activeView->slotSendToAutoDJ();
    }
}

void LibraryControl::slotSelectNextTrack(double v) {
    if (v > 0) {
        slotSelectTrack(1);
    }
}

void LibraryControl::slotSelectPrevTrack(double v) {
    if (v > 0) {
        slotSelectTrack(-1);
    }
}

void LibraryControl::slotSelectTrack(double v) {
    if (!m_pLibraryWidget) {
        return;
    }

    int i = (int)v;

    LibraryView* activeView = m_pLibraryWidget->getActiveView();
    if (!activeView) {
        return;
    }
    activeView->moveSelection(i);
}



void LibraryControl::slotMoveUp(double v) {
    if (v > 0) {
        emitKeyEvent(QKeyEvent{QEvent::KeyPress, Qt::Key_Up, Qt::NoModifier});
    }
}

void LibraryControl::slotMoveDown(double v) {
    if (v > 0) {
        emitKeyEvent(QKeyEvent{QEvent::KeyPress, Qt::Key_Down, Qt::NoModifier});
    }
}

void LibraryControl::slotMoveVertical(double v) {
    const auto key = (v < 0) ? Qt::Key_Up: Qt::Key_Down;
    const auto times = static_cast<unsigned short>(v);
    emitKeyEvent(QKeyEvent{QEvent::KeyPress, key, Qt::NoModifier, QString(), false, times});
}

void LibraryControl::slotMoveLeft(double v) {
    if (v > 0) {
        emitKeyEvent(QKeyEvent{QEvent::KeyPress, Qt::Key_Left, Qt::NoModifier});
    }
}

void LibraryControl::slotMoveRight(double v) {
    if (v > 0) {
        emitKeyEvent(QKeyEvent{QEvent::KeyPress, Qt::Key_Right, Qt::NoModifier});
    }
}

void LibraryControl::slotMoveHorizontal(double v) {
    const auto key = (v < 0) ? Qt::Key_Left: Qt::Key_Right;
    const auto times = static_cast<unsigned short>(v);
    emitKeyEvent(QKeyEvent{QEvent::KeyPress, key, Qt::NoModifier, QString(), false, times});
}

void LibraryControl::slotMoveFocusForward(double v) {
    if (v > 0) {
        emitKeyEvent(QKeyEvent{QEvent::KeyPress, Qt::Key_Tab, Qt::NoModifier});
    }
}

void LibraryControl::slotMoveFocusBackward(double v) {
    if (v > 0) {
        emitKeyEvent(QKeyEvent{QEvent::KeyPress, Qt::Key_Tab, Qt::ShiftModifier});
    }
}

void LibraryControl::slotMoveFocus(double v) {
    const auto shift = (v < 0) ? Qt::ShiftModifier: Qt::NoModifier;
    const auto times = static_cast<unsigned short>(v);
    emitKeyEvent(QKeyEvent{QEvent::KeyPress, Qt::Key_Tab, shift, QString(), false, times});
}

void LibraryControl::emitKeyEvent(QKeyEvent&& event) {
    auto focusWidget = QApplication::focusWidget();
    if (!focusWidget) {
        qWarning() << "LibraryControl::emitKeyPress() failed due to no widget having focus";
        return;
    }
    QApplication::sendEvent(focusWidget, &event);
}

void LibraryControl::slotSelectSidebarItem(double v) {
    if (v != 0) {
        const auto key = (v < 0) ? Qt::Key_Up : Qt::Key_Down;
        emitKeyEvent(QKeyEvent{QEvent::KeyPress, key, Qt::NoModifier});
        emitKeyEvent(QKeyEvent{QEvent::KeyRelease, key, Qt::NoModifier});
    }
}

void LibraryControl::slotSelectNextSidebarItem(double v) {
    if (v > 0) {
        slotSelectSidebarItem(1);
    }
}

void LibraryControl::slotSelectPrevSidebarItem(double v) {
    if (v > 0) {
        slotSelectSidebarItem(-1);
    }
}

void LibraryControl::slotToggleSelectedSidebarItem(double v) {
    if (m_pSidebarWidget && v > 0) {
        m_pSidebarWidget->toggleSelectedItem();
    }
}

void LibraryControl::slotChooseItem(double v) {
    // XXX: Make this more generic? If Enter key is mapped correctly maybe we can use that
    if (!m_pLibraryWidget) {
        return;
    }
    // Load current track if a LibraryView object has focus
    const auto activeView = m_pLibraryWidget->getActiveView();
    if (activeView && activeView->hasFocus()) {
        return slotLoadSelectedIntoFirstStopped(v);
    }
    // Otherwise toggle the sidebar item expanded state (like a double-click)
    slotToggleSelectedSidebarItem(v);
}

void LibraryControl::slotFontSize(double v) {
    if (v == 0.0) {
        return;
    }
    QFont font = m_pLibrary->getTrackTableFont();
    font.setPointSizeF(font.pointSizeF() + v);
    m_pLibrary->slotSetTrackTableFont(font);
}

void LibraryControl::slotIncrementFontSize(double v) {
    if (v > 0.0) {
        slotFontSize(1);
    }
}

void LibraryControl::slotDecrementFontSize(double v) {
    if (v > 0.0) {
        slotFontSize(-1);
    }
}
