#include "library/librarycontrol.h"

#include <QApplication>
#include <QItemSelectionModel>
#include <QModelIndex>
#include <QModelIndexList>
#include <QWindow>
#include <QtDebug>

#include "control/controlobject.h"
#include "control/controlpushbutton.h"
#include "library/library.h"
#include "library/libraryview.h"
#include "mixer/playermanager.h"
#include "moc_librarycontrol.cpp"
#include "util/cmdlineargs.h"
#include "widget/wlibrary.h"
#include "widget/wlibrarysidebar.h"
#include "widget/wsearchlineedit.h"
#include "widget/wtracktableview.h"

namespace {
const QString kAppGroup = QStringLiteral("[App]");
} // namespace

LoadToGroupController::LoadToGroupController(LibraryControl* pParent, const QString& group)
        : QObject(pParent),
          m_group(group) {
    m_pLoadControl = std::make_unique<ControlPushButton>(ConfigKey(group, "LoadSelectedTrack"));
    connect(m_pLoadControl.get(),
            &ControlObject::valueChanged,
            this,
            &LoadToGroupController::slotLoadToGroup);

    m_pLoadAndPlayControl = std::make_unique<ControlPushButton>(ConfigKey(group, "LoadSelectedTrackAndPlay"));
    connect(m_pLoadAndPlayControl.get(),
            &ControlObject::valueChanged,
            this,
            &LoadToGroupController::slotLoadToGroupAndPlay);

    connect(this,
            &LoadToGroupController::loadToGroup,
            pParent,
            &LibraryControl::slotLoadSelectedTrackToGroup);
}

LoadToGroupController::~LoadToGroupController() = default;

void LoadToGroupController::slotLoadToGroup(double v) {
    if (v > 0) {
        emit loadToGroup(m_group, false);
    }
}

void LoadToGroupController::slotLoadToGroupAndPlay(double v) {
    if (v > 0) {
        emit loadToGroup(m_group, true);
    }
}

LibraryControl::LibraryControl(Library* pLibrary)
        : QObject(pLibrary),
          m_pLibrary(pLibrary),
          m_focusedWidget(FocusWidget::None),
          m_prevFocusedWidget(FocusWidget::None),
          m_pLibraryWidget(nullptr),
          m_pSidebarWidget(nullptr),
          m_pSearchbox(nullptr),
          m_numDecks(kAppGroup, QStringLiteral("num_decks"), this),
          m_numSamplers(kAppGroup, QStringLiteral("num_samplers"), this),
          m_numPreviewDecks(kAppGroup, QStringLiteral("num_preview_decks"), this) {
    qRegisterMetaType<FocusWidget>("FocusWidget");

    slotNumDecksChanged(m_numDecks.get());
    slotNumSamplersChanged(m_numSamplers.get());
    slotNumPreviewDecksChanged(m_numPreviewDecks.get());
    m_numDecks.connectValueChanged(this, &LibraryControl::slotNumDecksChanged);
    m_numSamplers.connectValueChanged(this, &LibraryControl::slotNumSamplersChanged);
    m_numPreviewDecks.connectValueChanged(this, &LibraryControl::slotNumPreviewDecksChanged);

    // Controls to navigate vertically within currently focused widget (up/down buttons)
    m_pMoveUp = std::make_unique<ControlPushButton>(ConfigKey("[Library]", "MoveUp"));
    m_pMoveDown = std::make_unique<ControlPushButton>(ConfigKey("[Library]", "MoveDown"));
    m_pMoveVertical = std::make_unique<ControlEncoder>(ConfigKey("[Library]", "MoveVertical"), false);
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    connect(m_pMoveUp.get(),
            &ControlPushButton::valueChanged,
            this,
            &LibraryControl::slotMoveUp);
    connect(m_pMoveDown.get(),
            &ControlPushButton::valueChanged,
            this,
            &LibraryControl::slotMoveDown);
    connect(m_pMoveVertical.get(),
            &ControlEncoder::valueChanged,
            this,
            &LibraryControl::slotMoveVertical);
#endif

    // Controls to navigate vertically within currently focused widget (up/down buttons)
    m_pScrollUp = std::make_unique<ControlPushButton>(ConfigKey("[Library]", "ScrollUp"));
    m_pScrollDown = std::make_unique<ControlPushButton>(ConfigKey("[Library]", "ScrollDown"));
    m_pScrollVertical = std::make_unique<ControlEncoder>(ConfigKey("[Library]", "ScrollVertical"), false);
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    connect(m_pScrollUp.get(),
            &ControlPushButton::valueChanged,
            this,
            &LibraryControl::slotScrollUp);
    connect(m_pScrollDown.get(),
            &ControlPushButton::valueChanged,
            this,
            &LibraryControl::slotScrollDown);
    connect(m_pScrollVertical.get(),
            &ControlEncoder::valueChanged,
            this,
            &LibraryControl::slotScrollVertical);
#endif

    // Controls to navigate horizontally within currently selected item (left/right buttons)
    m_pMoveLeft = std::make_unique<ControlPushButton>(ConfigKey("[Library]", "MoveLeft"));
    m_pMoveRight = std::make_unique<ControlPushButton>(ConfigKey("[Library]", "MoveRight"));
    m_pMoveHorizontal = std::make_unique<ControlEncoder>(ConfigKey("[Library]", "MoveHorizontal"), false);
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    connect(m_pMoveLeft.get(),
            &ControlPushButton::valueChanged,
            this,
            &LibraryControl::slotMoveLeft);
    connect(m_pMoveRight.get(),
            &ControlPushButton::valueChanged,
            this,
            &LibraryControl::slotMoveRight);
    connect(m_pMoveHorizontal.get(),
            &ControlEncoder::valueChanged,
            this,
            &LibraryControl::slotMoveHorizontal);
#endif

    // Controls to navigate between widgets
    // Relative focus controls (emulate Tab/Shift+Tab button press)
    m_pMoveFocusForward = std::make_unique<ControlPushButton>(ConfigKey("[Library]", "MoveFocusForward"));
    m_pMoveFocusBackward = std::make_unique<ControlPushButton>(ConfigKey("[Library]", "MoveFocusBackward"));
    m_pMoveFocus = std::make_unique<ControlEncoder>(ConfigKey("[Library]", "MoveFocus"), false);
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    connect(m_pMoveFocusForward.get(),
            &ControlPushButton::valueChanged,
            this,
            &LibraryControl::slotMoveFocusForward);
    connect(m_pMoveFocusBackward.get(),
            &ControlPushButton::valueChanged,
            this,
            &LibraryControl::slotMoveFocusBackward);
    connect(m_pMoveFocus.get(),
            &ControlEncoder::valueChanged,
            this,
            &LibraryControl::slotMoveFocus);
#endif

    // Direct focus control, read/write
    m_pFocusedWidgetCO = std::make_unique<ControlPushButton>(
            ConfigKey("[Library]", "focused_widget"));
    m_pFocusedWidgetCO->setStates(static_cast<int>(FocusWidget::Count));
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    m_pFocusedWidgetCO->connectValueChangeRequest(
            this,
            [this](double value) {
                // Focus can not be removed from a widget just moved to another one.
                // Thus, to keep the CO and QApplication::focusWidget() in sync we
                // have to prevent scripts or GUI buttons setting the CO to 'None'.
                // It's only set to 'None' internally when one of the library widgets
                // receives a FocusOutEvent(), e.g. when the focus is moved to another
                // widget, or when the main window loses focus.
                const int valueInt = static_cast<int>(value);
                if (valueInt != static_cast<int>(FocusWidget::None) &&
                        valueInt < static_cast<int>(FocusWidget::Count)) {
                    setLibraryFocus(static_cast<FocusWidget>(valueInt));
                }
            });
#endif

    // Pure trigger control. Alternative for signal/slot since widgets that want
    // to call refocusPrevLibraryWidget() are cumbersome to connect to.
    // This CO is never actually set or read so the value just needs to be not 0
    m_pRefocusPrevWidgetCO = std::make_unique<ControlPushButton>(
            ConfigKey("[Library]", "refocus_prev_widget"));
    m_pRefocusPrevWidgetCO->setButtonMode(ControlPushButton::TRIGGER);
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    m_pRefocusPrevWidgetCO->connectValueChangeRequest(this,
            &LibraryControl::refocusPrevLibraryWidget);
#endif

    // Control to "goto" the currently selected item in focused widget (context dependent)
    m_pGoToItem = std::make_unique<ControlPushButton>(ConfigKey("[Library]", "GoToItem"));
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    connect(m_pGoToItem.get(),
            &ControlPushButton::valueChanged,
            this,
            &LibraryControl::slotGoToItem);
#endif

    // Auto DJ controls
    m_pAutoDjAddTop = std::make_unique<ControlPushButton>(ConfigKey("[Library]","AutoDjAddTop"));
    m_pAutoDjAddTop->addAlias(ConfigKey(
            QStringLiteral("[Playlist]"), QStringLiteral("AutoDjAddTop")));
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    connect(m_pAutoDjAddTop.get(),
            &ControlPushButton::valueChanged,
            this,
            &LibraryControl::slotAutoDjAddTop);
#endif

    m_pAutoDjAddBottom = std::make_unique<ControlPushButton>(ConfigKey("[Library]","AutoDjAddBottom"));
    m_pAutoDjAddBottom->addAlias(ConfigKey(
            QStringLiteral("[Playlist]"), QStringLiteral("AutoDjAddBottom")));
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    connect(m_pAutoDjAddBottom.get(),
            &ControlPushButton::valueChanged,
            this,
            &LibraryControl::slotAutoDjAddBottom);
#endif

    m_pAutoDjAddReplace = std::make_unique<ControlPushButton>(
            ConfigKey("[Library]", "AutoDjAddReplace"));
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    connect(m_pAutoDjAddReplace.get(),
            &ControlPushButton::valueChanged,
            this,
            &LibraryControl::slotAutoDjAddReplace);
#endif

    // Sort controls
    m_pSortColumn = std::make_unique<ControlEncoder>(ConfigKey("[Library]", "sort_column"));
    m_pSortOrder = std::make_unique<ControlPushButton>(ConfigKey("[Library]", "sort_order"));
    m_pSortOrder->setButtonMode(ControlPushButton::TOGGLE);
    m_pSortColumnToggle = std::make_unique<ControlEncoder>(ConfigKey("[Library]", "sort_column_toggle"), false);
    m_pSortFocusedColumn = std::make_unique<ControlPushButton>(
            ConfigKey("[Library]", "sort_focused_column"));
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    connect(m_pSortColumn.get(),
            &ControlEncoder::valueChanged,
            this,
            &LibraryControl::slotSortColumn);
    connect(m_pSortColumnToggle.get(),
            &ControlEncoder::valueChanged,
            this,
            &LibraryControl::slotSortColumnToggle);
    connect(m_pSortFocusedColumn.get(),
            &ControlObject::valueChanged,
            this,
            [this](double value) {
                if (value > 0.0) {
                    slotSortColumnToggle(static_cast<int>(TrackModel::SortColumnId::CurrentIndex));
                }
            });

    // Font sizes
    m_pFontSizeKnob = std::make_unique<ControlObject>(
            ConfigKey("[Library]", "font_size_knob"), false);
    connect(m_pFontSizeKnob.get(),
            &ControlObject::valueChanged,
            this,
            &LibraryControl::slotFontSize);

    m_pFontSizeDecrement = std::make_unique<ControlPushButton>(
            ConfigKey("[Library]", "font_size_decrement"));
    connect(m_pFontSizeDecrement.get(),
            &ControlPushButton::valueChanged,
            this,
            &LibraryControl::slotDecrementFontSize);

    m_pFontSizeIncrement = std::make_unique<ControlPushButton>(
            ConfigKey("[Library]", "font_size_increment"));
    connect(m_pFontSizeIncrement.get(),
            &ControlPushButton::valueChanged,
            this,
            &LibraryControl::slotIncrementFontSize);
#endif

    // Track Color controls
    m_pTrackColorPrev = std::make_unique<ControlPushButton>(ConfigKey("[Library]", "track_color_prev"));
    m_pTrackColorNext = std::make_unique<ControlPushButton>(ConfigKey("[Library]", "track_color_next"));
    connect(m_pTrackColorPrev.get(),
            &ControlPushButton::valueChanged,
            this,
            &LibraryControl::slotTrackColorPrev);
    connect(m_pTrackColorNext.get(),
            &ControlPushButton::valueChanged,
            this,
            &LibraryControl::slotTrackColorNext);

    // Controls to select saved searchbox queries and to clear the searchbox
    m_pSelectHistoryNext = std::make_unique<ControlPushButton>(
            ConfigKey("[Library]", "search_history_next"));
    connect(m_pSelectHistoryNext.get(),
            &ControlPushButton::valueChanged,
            this,
            [this](double value) {
                VERIFY_OR_DEBUG_ASSERT(m_pSearchbox) {
                    return;
                }
                if (value > 0.0) {
                    m_pSearchbox->slotMoveSelectedHistory(1);
                }
            });
    m_pSelectHistoryPrev = std::make_unique<ControlPushButton>(
            ConfigKey("[Library]", "search_history_prev"));
    connect(m_pSelectHistoryPrev.get(),
            &ControlPushButton::valueChanged,
            this,
            [this](double value) {
                VERIFY_OR_DEBUG_ASSERT(m_pSearchbox) {
                    return;
                }
                if (value > 0.0) {
                    m_pSearchbox->slotMoveSelectedHistory(-1);
                }
            });
    m_pSelectHistorySelect = std::make_unique<ControlEncoder>(
            ConfigKey("[Library]", "search_history_selector"), false);
    connect(m_pSelectHistorySelect.get(),
            &ControlEncoder::valueChanged,
            this,
            [this](double steps) {
                VERIFY_OR_DEBUG_ASSERT(m_pSearchbox) {
                    return;
                }
                int iSteps = static_cast<int>(steps);
                if (iSteps) {
                    m_pSearchbox->slotMoveSelectedHistory(static_cast<int>(steps));
                }
            });
    m_pClearSearch = std::make_unique<ControlPushButton>(
            ConfigKey("[Library]", "clear_search"));
    connect(m_pClearSearch.get(),
            &ControlPushButton::valueChanged,
            this,
            [this](double value) {
                VERIFY_OR_DEBUG_ASSERT(m_pSearchbox) {
                    return;
                }
                if (value > 0.0) {
                    m_pSearchbox->slotClearSearch();
                }
            });
    m_pDeleteSearchQuery = std::make_unique<ControlPushButton>(
            ConfigKey("[Library]", "delete_search_query"));
    connect(m_pDeleteSearchQuery.get(),
            &ControlPushButton::valueChanged,
            this,
            [this](double value) {
                VERIFY_OR_DEBUG_ASSERT(m_pSearchbox) {
                    return;
                }
                if (value > 0.0) {
                    m_pSearchbox->slotDeleteCurrentItem();
                }
            });

    // Show the track context menu for selected tracks, or hide it
    // if it is the current active window
    // The control is updated in slotUpdateTrackMenuControl with the actual state
    // sent from WTrackMenu via WTrackTableView
    m_pShowTrackMenu = std::make_unique<ControlPushButton>(
            ConfigKey("[Library]", "show_track_menu"));
    m_pShowTrackMenu->setStates(2);
    m_pShowTrackMenu->connectValueChangeRequest(this,
            [this](double value) {
                VERIFY_OR_DEBUG_ASSERT(m_pLibraryWidget) {
                    return;
                }
                bool show = static_cast<bool>(value);
                emit showHideTrackMenu(show);
            });

    // Deprecated controls
    m_pSelectNextTrack = std::make_unique<ControlPushButton>(ConfigKey("[Playlist]", "SelectNextTrack"));
    connect(m_pSelectNextTrack.get(),
            &ControlPushButton::valueChanged,
            this,
            &LibraryControl::slotSelectNextTrack);

    m_pSelectPrevTrack = std::make_unique<ControlPushButton>(ConfigKey("[Playlist]", "SelectPrevTrack"));
    connect(m_pSelectPrevTrack.get(),
            &ControlPushButton::valueChanged,
            this,
            &LibraryControl::slotSelectPrevTrack);

    // Ignoring no-ops is important since this is for +/- tickers.
    m_pSelectTrack = std::make_unique<ControlObject>(ConfigKey("[Playlist]","SelectTrackKnob"), false);
    connect(m_pSelectTrack.get(),
            &ControlObject::valueChanged,
            this,
            &LibraryControl::slotSelectTrack);

    m_pSelectNextSidebarItem = std::make_unique<ControlPushButton>(ConfigKey("[Playlist]", "SelectNextPlaylist"));
    connect(m_pSelectNextSidebarItem.get(),
            &ControlPushButton::valueChanged,
            this,
            &LibraryControl::slotSelectNextSidebarItem);

    m_pSelectPrevSidebarItem = std::make_unique<ControlPushButton>(ConfigKey("[Playlist]", "SelectPrevPlaylist"));
    connect(m_pSelectPrevSidebarItem.get(),
            &ControlPushButton::valueChanged,
            this,
            &LibraryControl::slotSelectPrevSidebarItem);

    // Ignoring no-ops is important since this is for +/- tickers.
    m_pSelectSidebarItem = std::make_unique<ControlObject>(ConfigKey("[Playlist]", "SelectPlaylist"), false);
    connect(m_pSelectSidebarItem.get(),
            &ControlObject::valueChanged,
            this,
            &LibraryControl::slotSelectSidebarItem);

    m_pToggleSidebarItem = std::make_unique<ControlPushButton>(ConfigKey("[Playlist]", "ToggleSelectedSidebarItem"));
    connect(m_pToggleSidebarItem.get(),
            &ControlPushButton::valueChanged,
            this,
            &LibraryControl::slotToggleSelectedSidebarItem);

    m_pLoadSelectedIntoFirstStopped = std::make_unique<ControlPushButton>(ConfigKey("[Playlist]","LoadSelectedIntoFirstStopped"));
    connect(m_pLoadSelectedIntoFirstStopped.get(),
            &ControlPushButton::valueChanged,
            this,
            &LibraryControl::slotLoadSelectedIntoFirstStopped);

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    QApplication* app = qApp;
    // Update controls if any widget in any Mixxx window gets or loses focus
    connect(app,
            &QApplication::focusChanged,
            this,
            &LibraryControl::slotFocusedWidgetChanged);
    // Also update controls if the window focus changed.
    // Even though any new menu window has focus and will receive keypress events
    // it does NOT have a focused widget before the first click or keypress.
    // Thus a QMenu popping up is not reported by focusChanged(oldWidget, newWidget).
    // QApplication::focusWidget() is still that in the previously focused
    // window (MixxxMainWindow for example).
    connect(app,
            &QGuiApplication::focusWindowChanged,
            this,
            &LibraryControl::updateFocusedWidgetControls);
#endif
}

LibraryControl::~LibraryControl() = default;

void LibraryControl::maybeCreateGroupController(const QString& group) {
    if (m_loadToGroupControllers.find(group) == m_loadToGroupControllers.end()) {
        m_loadToGroupControllers.emplace(group, std::make_unique<LoadToGroupController>(this, group));
    }
}

void LibraryControl::slotNumDecksChanged(double v) {
    int iNumDecks = static_cast<int>(v);

    if (iNumDecks < 0) {
        return;
    }

    for (int i = 0; i < iNumDecks; ++i) {
        maybeCreateGroupController(PlayerManager::groupForDeck(i));
    }
}

void LibraryControl::slotNumSamplersChanged(double v) {
    int iNumSamplers = static_cast<int>(v);

    if (iNumSamplers < 0) {
        return;
    }

    for (int i = 0; i < iNumSamplers; ++i) {
        maybeCreateGroupController(PlayerManager::groupForSampler(i));
    }
}

void LibraryControl::slotNumPreviewDecksChanged(double v) {
    int iNumPreviewDecks = static_cast<int>(v);

    if (iNumPreviewDecks < 0) {
        return;
    }

    for (int i = 0; i < iNumPreviewDecks; ++i) {
        maybeCreateGroupController(PlayerManager::groupForPreviewDeck(i));
    }
}

void LibraryControl::bindSidebarWidget(WLibrarySidebar* pSidebarWidget) {
    if (m_pSidebarWidget) {
        disconnect(m_pSidebarWidget, nullptr, this, nullptr);
    }
    m_pSidebarWidget = pSidebarWidget;
    connect(m_pSidebarWidget,
            &WLibrarySidebar::destroyed,
            this,
            &LibraryControl::sidebarWidgetDeleted);
}

void LibraryControl::bindLibraryWidget(WLibrary* pLibraryWidget, KeyboardEventFilter* pKeyboard) {
    Q_UNUSED(pKeyboard);
    if (m_pLibraryWidget) {
        disconnect(m_pLibraryWidget, nullptr, this, nullptr);
    }
    m_pLibraryWidget = pLibraryWidget;
    connect(m_pLibraryWidget,
            &WLibrary::destroyed,
            this,
            &LibraryControl::libraryWidgetDeleted);
}

void LibraryControl::bindSearchboxWidget(WSearchLineEdit* pSearchbox) {
    if (m_pSearchbox) {
        disconnect(m_pSearchbox, nullptr, this, nullptr);
    }
    m_pSearchbox = pSearchbox;
    connect(m_pSearchbox,
            &WSearchLineEdit::destroyed,
            this,
            &LibraryControl::searchboxWidgetDeleted);
}

void LibraryControl::libraryWidgetDeleted() {
    m_pLibraryWidget = nullptr;
}

void LibraryControl::sidebarWidgetDeleted() {
    m_pSidebarWidget = nullptr;
}

void LibraryControl::searchboxWidgetDeleted() {
    m_pSearchbox = nullptr;
}

void LibraryControl::slotUpdateTrackMenuControl(bool visible) {
    m_pShowTrackMenu->setAndConfirm(visible ? 1.0 : 0.0);
}

void LibraryControl::slotLoadSelectedTrackToGroup(const QString& group, bool play) {
    if (!m_pLibraryWidget) {
        return;
    }

    LibraryView* pActiveView = m_pLibraryWidget->getActiveView();
    if (!pActiveView) {
        return;
    }
    pActiveView->loadSelectedTrackToGroup(group, play);
}

void LibraryControl::slotLoadSelectedIntoFirstStopped(double v) {
    if (!m_pLibraryWidget) {
        return;
    }

    if (v > 0) {
        LibraryView* pActiveView = m_pLibraryWidget->getActiveView();
        if (!pActiveView) {
            return;
        }
        pActiveView->activateSelectedTrack();
    }
}

void LibraryControl::slotAutoDjAddTop(double v) {
    if (!m_pLibraryWidget) {
        return;
    }

    if (v > 0) {
        LibraryView* pActiveView = m_pLibraryWidget->getActiveView();
        if (!pActiveView) {
            return;
        }
        pActiveView->slotAddToAutoDJTop();
    }
}

void LibraryControl::slotAutoDjAddBottom(double v) {
    if (!m_pLibraryWidget) {
        return;
    }
    if (v > 0) {
        LibraryView* pActiveView = m_pLibraryWidget->getActiveView();
        if (!pActiveView) {
            return;
        }
        pActiveView->slotAddToAutoDJBottom();
    }
}

void LibraryControl::slotAutoDjAddReplace(double v) {
    if (!m_pLibraryWidget) {
        return;
    }
    if (v > 0) {
        LibraryView* pActiveView = m_pLibraryWidget->getActiveView();
        if (!pActiveView) {
            return;
        }
        pActiveView->slotAddToAutoDJReplace();
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

    LibraryView* pActiveView = m_pLibraryWidget->getActiveView();
    if (!pActiveView) {
        return;
    }

    int i = (int)v;
    pActiveView->moveSelection(i);
}

void LibraryControl::slotMoveUp(double v) {
    if (v > 0) {
        slotMoveVertical(-1);
    }
}

void LibraryControl::slotMoveDown(double v) {
    if (v > 0) {
        slotMoveVertical(1);
    }
}

void LibraryControl::slotMoveVertical(double v) {
    if (v == 0) {
        return;
    }

    switch (m_focusedWidget) {
    case FocusWidget::Sidebar: {
        int i = static_cast<int>(v);
        slotSelectSidebarItem(i);
        return;
    }
    case FocusWidget::TracksTable: {
        // `WLibraryTableView`'s cursor movement function has been overridden to
        // wrap the selection around at the top/bottom of the tracks list. This
        // behavior is thus shared between `[Library],MoveVertical` and Up/Down
        // cursor key presses. See `WLibraryTableView::moveCursor()` for an
        // explanation on why this is useful.
        break;
    }
    case FocusWidget::Dialog: {
        // For navigating dialogs map up/down to Tab/BackTab
        // Don't use Shift + Tab! (see moveFocus())
        const auto key = (v > 0) ? Qt::Key_Tab : Qt::Key_Backtab;
        const auto times = static_cast<unsigned short>(std::abs(v));
        emitKeyEvent(QKeyEvent{
                QEvent::KeyPress, key, Qt::NoModifier, QString(), false, times});
        return;
    }
    case FocusWidget::ContextMenu: {
        // To navigate menus (and activate menus that were just opened) send the
        // keyEvent to focusWindow() (not focusWidget() like emitKeyEvent() does)
        const auto key = (v < 0) ? Qt::Key_Up : Qt::Key_Down;
        const auto times = static_cast<unsigned short>(std::abs(v));
        QKeyEvent event = QKeyEvent{
                QEvent::KeyPress, key, Qt::NoModifier, QString(), false, times};
        QApplication::sendEvent(QApplication::focusWindow(), &event);
        return;
    }
    case FocusWidget::Searchbar:
        // There's also m_pSearchbox->slotMoveSelectedHistory but that wraps around
        // at top/bottom. Doesn't match Up/Down key behaviour and may not be desired.
        // Proceed and let emitkeyEvent deal with it.
        break;
    case FocusWidget::None:
    case FocusWidget::Unknown:
    default:
        // 'Unknown' uncategorized widget like a QComboBox. Return to not alter
        // any WBeatSpinBox or WEffectSelector
        setLibraryFocus(FocusWidget::TracksTable);
        return;
    }
    const auto key = (v < 0) ? Qt::Key_Up : Qt::Key_Down;
    const auto times = static_cast<unsigned short>(std::abs(v));
    emitKeyEvent(QKeyEvent{
            QEvent::KeyPress, key, Qt::NoModifier, QString(), false, times});
}

void LibraryControl::slotScrollUp(double v) {
    if (v > 0) {
        slotScrollVertical(-1);
    }
}

void LibraryControl::slotScrollDown(double v) {
    if (v > 0) {
        slotScrollVertical(1);
    }
}

void LibraryControl::slotScrollVertical(double v) {
    const auto key = (v < 0) ? Qt::Key_PageUp: Qt::Key_PageDown;
    const auto times = static_cast<unsigned short>(std::abs(v));
    emitKeyEvent(QKeyEvent{QEvent::KeyPress, key, Qt::NoModifier, QString(), false, times});
}

void LibraryControl::slotMoveLeft(double v) {
    if (v > 0) {
        slotMoveHorizontal(-1);
    }
}

void LibraryControl::slotMoveRight(double v) {
    if (v > 0) {
        slotMoveHorizontal(1);
    }
}

void LibraryControl::slotMoveHorizontal(double v) {
    const auto key = (v < 0) ? Qt::Key_Left: Qt::Key_Right;
    const auto times = static_cast<unsigned short>(std::abs(v));
    emitKeyEvent(QKeyEvent{QEvent::KeyPress, key, Qt::NoModifier, QString(), false, times});
}

void LibraryControl::slotMoveFocusForward(double v) {
    if (v > 0) {
        slotMoveFocus(1);
    }
}

void LibraryControl::slotMoveFocusBackward(double v) {
    if (v > 0) {
        slotMoveFocus(-1);
    }
}

void LibraryControl::slotMoveFocus(double v) {
    // Don't use Key_Tab + ShiftModifier for moving focus backwards!
    // This would indeed move the focus, though it has a significant side-effect
    // compared to pressing Shift + Tab on a real keyboard:
    // Shift would remain 'pressed' in the previously focused widget until it
    // receives any keyEvent with Qt::NoModifier.
    const auto key = (v > 0) ? Qt::Key_Tab : Qt::Key_Backtab;
    const auto times = static_cast<unsigned short>(std::abs(v));
    emitKeyEvent(QKeyEvent{
            QEvent::KeyPress, key, Qt::NoModifier, QString(), false, times});
}

void LibraryControl::emitKeyEvent(QKeyEvent&& event) {
    if (!QApplication::focusWindow()) {
        qInfo() << "No Mixxx window, popup or menu has focus."
                << "Don't send key events.";
        return;
    }

    if (m_focusedWidget == FocusWidget::None) {
        setLibraryFocus(FocusWidget::TracksTable);
        return;
    }

    // Send the event pointer to the currently focused widget
    auto* focusWidget = QApplication::focusWidget();
    if (focusWidget) {
        for (auto i = 0; i < event.count(); ++i) {
            QApplication::sendEvent(focusWidget, &event);
        }
    }
}

FocusWidget LibraryControl::getFocusedWidget() {
    auto* focusWindow = QApplication::focusWindow();
    if (!focusWindow) {
        return FocusWidget::None;
    }

    // Any QMenu is focusWindow() but NOT focusWidget() before any menu item
    // is highlighted, though it can already receive keypress events.
    // Thus, test for focus window type first to catch open popups.
    if (focusWindow->type() == Qt::Popup) {
        // WMainMenuBar
        // WTrackMenuClassWindow = WTrackMenu + submenus
        // QMenuClassWindow      = e.g. sidebar context menu
        // qt_edit_menuWindow    = QLineEdit/QCombobox context menu
        // QComboBoxListView of WEffectSelector, WSearchLineEdit, ...
        return FocusWidget::ContextMenu;
    } else if (focusWindow->type() == Qt::Dialog) {
        // DlgPreferencesDlgWindow
        // DlgDeveloperToolsWindow
        // DlgAboutDlgWindow
        // DlgKeywheelWindow
        // QInputDialogClassWindow (file dialogs, rename/create dialogs)
        // error messages and Close Mixxx confirmation dialog
        // ToDo(ronso0) handle CoverArt:
        // - refocus tracks view?
        // DlgCoverArtFullSizeWindow
        return FocusWidget::Dialog;
    }

    // Now we assume MixxxMainWindow is focused
    if (!QApplication::focusWidget()) {
        return FocusWidget::None;
    }

    if (m_pSearchbox && m_pSearchbox->hasFocus()) {
        return FocusWidget::Searchbar;
    } else if (m_pSidebarWidget && m_pSidebarWidget->hasFocus()) {
        return FocusWidget::Sidebar;
    } else if (m_pLibraryWidget && m_pLibraryWidget->getActiveView()->hasFocus()) {
        return FocusWidget::TracksTable;
    } else {
        // Unknown widget, for example Clear button in WSearcLineEdit,
        // some drop-down view, WBeatSpinBox or QLineEdit in WtrackTableView
        return FocusWidget::Unknown;
    }
}

void LibraryControl::setLibraryFocus(FocusWidget newFocusWidget) {
    if (!QApplication::focusWindow()) {
        qInfo() << "No Mixxx window, popup or menu has focus."
                << "Don't attempt to focus a specific widget.";
        return;
    }

    // ignore no-op
    if (newFocusWidget == m_focusedWidget) {
        return;
    }

    switch (newFocusWidget) {
    case FocusWidget::Searchbar:
        VERIFY_OR_DEBUG_ASSERT(m_pSearchbox) {
            return;
        }
        m_pSearchbox->setFocus();
        return;
    case FocusWidget::Sidebar:
        VERIFY_OR_DEBUG_ASSERT(m_pSidebarWidget) {
            return;
        }
        m_pSidebarWidget->setFocus();
        return;
    case FocusWidget::TracksTable:
        VERIFY_OR_DEBUG_ASSERT(m_pLibraryWidget) {
            return;
        }
        m_pLibraryWidget->getActiveView()->setFocus();
        return;
    case FocusWidget::None:
        // What could be the goal, what are the consequences of manually
        // removing focus from a widget?
    default:
        // Ignore invalid requests and don't allow focussing any other widget
        // manually, like QDialog or QMenu.
        return;
    }
    // Done. QApplication::focusChanged will invoke updateFocusControl()
    // to update [Library],focused_widget
}

void LibraryControl::slotFocusedWidgetChanged(QWidget* oldW, QWidget* newW) {
    Q_UNUSED(newW);

    // If one of the library widgets had focus store it so we can return to it,
    // for example when we finish editing a WBeatSizeSpinBox.
    if (m_pSearchbox && oldW == m_pSearchbox) {
        m_prevFocusedWidget = FocusWidget::Searchbar;
    } else if (m_pSidebarWidget && oldW == m_pSidebarWidget) {
        m_prevFocusedWidget = FocusWidget::Sidebar;
    } else if (m_pLibraryWidget && oldW == m_pLibraryWidget->currentWidget()) {
        m_prevFocusedWidget = FocusWidget::TracksTable;
    }
    updateFocusedWidgetControls();
}

void LibraryControl::updateFocusedWidgetControls() {
    m_focusedWidget = getFocusedWidget();
    // Update "[Library], focused_widget" control
    double newVal = static_cast<double>(m_focusedWidget);
    m_pFocusedWidgetCO->setAndConfirm(newVal);
}

void LibraryControl::refocusPrevLibraryWidget() {
    setLibraryFocus(m_prevFocusedWidget);
}

void LibraryControl::slotSelectSidebarItem(double v) {
    if (!m_pSidebarWidget) {
        return;
    }
    if (v == 0) {
        return;
    }

    const auto key = (v < 0) ? Qt::Key_Up : Qt::Key_Down;
    const auto times = static_cast<unsigned short>(std::abs(v));
    QKeyEvent event = QKeyEvent{
            QEvent::KeyPress, key, Qt::NoModifier, QString(), false, times};
    QApplication::sendEvent(m_pSidebarWidget, &event);
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

void LibraryControl::slotGoToItem(double v) {
    if (v <= 0) {
        return;
    }

    switch (m_focusedWidget) {
    case FocusWidget::Sidebar:
        // Focus the library if this is a leaf node in the tree
        // Note that Tracks and AutoDJ always return 'false':
        // expanding those root items via controllers is considered dispensable
        // because the subfeatures' actions can't be accessed by controllers anyway.
        if (m_pSidebarWidget->isLeafNodeSelected()) {
            setLibraryFocus(FocusWidget::TracksTable);
        } else {
            // Otherwise toggle the sidebar item expanded state
            m_pSidebarWidget->toggleSelectedItem();
        }
        return;
    case FocusWidget::TracksTable:
        m_pLibraryWidget->getActiveView()->activateSelectedTrack();
        return;
    case FocusWidget::Dialog: {
        // press & release Space (QAbstractButton::clicked() is emitted on release)
        QKeyEvent pressSpace = QKeyEvent{QEvent::KeyPress, Qt::Key_Space, Qt::NoModifier};
        QKeyEvent releaseSpace = QKeyEvent{QEvent::KeyRelease, Qt::Key_Space, Qt::NoModifier};
        QApplication::sendEvent(QApplication::focusWindow(), &pressSpace);
        QApplication::sendEvent(QApplication::focusWindow(), &releaseSpace);
        return;
    }
    case FocusWidget::ContextMenu:
    case FocusWidget::Unknown: {
        // press Return to
        // * expand submenus or select highlighted menu item
        // * click Clear button in WSearcLineEdit (Note: even though this QToolButton
        //   inherits QAbstractButton, clicked is emitted on keypress if Return is used)
        // * confirm and WBeatSpinBox and move focus to tracks table
        // * ) and some more.
        // If Unknown is some other 'untrained' or unresponsive widget
        // GoToItem is inappropriate and we can't do much about that.
        QKeyEvent event = QKeyEvent{QEvent::KeyPress, Qt::Key_Return, Qt::NoModifier};
        QApplication::sendEvent(QApplication::focusWindow(), &event);
        return;
    }
    case FocusWidget::Searchbar:
    case FocusWidget::None:
    default:
        setLibraryFocus(FocusWidget::TracksTable);
    }
}

void LibraryControl::slotSortColumn(double v) {
    m_pSortColumnToggle->set(v);
}

void LibraryControl::slotSortColumnToggle(double v) {
    int sortColumnId = static_cast<int>(v);
    if (sortColumnId == static_cast<int>(TrackModel::SortColumnId::CurrentIndex)) {
        if (!m_pLibraryWidget) {
            return;
        }
        // Get the ID of the column with the cursor
        sortColumnId =
                static_cast<int>(m_pLibraryWidget->getActiveView()
                                         ->getColumnIdFromCurrentIndex());
    }

    if (static_cast<int>(m_pSortColumn->get()) == sortColumnId) {
        m_pSortOrder->set((m_pSortOrder->get() == 0) ? 1.0 : 0.0);
    } else {
        m_pSortColumn->set(sortColumnId);
        m_pSortOrder->set(0.0);
    }
}

void LibraryControl::slotFontSize(double v) {
    if (v == 0.0) {
        return;
    }
    QFont font = m_pLibrary->getTrackTableFont();
    font.setPointSizeF(font.pointSizeF() + v);
    m_pLibrary->setFont(font);
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

void LibraryControl::slotTrackColorPrev(double v) {
    if (!m_pLibraryWidget) {
        return;
    }

    if (v > 0) {
        LibraryView* pActiveView = m_pLibraryWidget->getActiveView();
        if (!pActiveView) {
            return;
        }
        pActiveView->assignPreviousTrackColor();
    }
}

void LibraryControl::slotTrackColorNext(double v) {
    if (!m_pLibraryWidget) {
        return;
    }

    if (v > 0) {
        LibraryView* pActiveView = m_pLibraryWidget->getActiveView();
        if (!pActiveView) {
            return;
        }
        pActiveView->assignNextTrackColor();
    }
}
