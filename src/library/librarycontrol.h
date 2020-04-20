#ifndef LIBRARYMIDICONTROL_H
#define LIBRARYMIDICONTROL_H

#include <QObject>

#include "control/controlencoder.h"
#include "control/controlproxy.h"
#include "util/memory.h"

class ControlObject;
class ControlPushButton;
class Library;
class LibraryControl;
class WLibrary;
class WLibrarySidebar;
class KeyboardEventFilter;

class LoadToGroupController : public QObject {
    Q_OBJECT
  public:
    LoadToGroupController(LibraryControl* pParent, const QString& group);
    virtual ~LoadToGroupController();

  signals:
    void loadToGroup(QString group, bool);

  public slots:
    void slotLoadToGroup(double v);
    void slotLoadToGroupAndPlay(double v);

  private:
    QString m_group;
    std::unique_ptr<ControlObject> m_pLoadControl;
    std::unique_ptr<ControlObject> m_pLoadAndPlayControl;
};

class LibraryControl : public QObject {
    Q_OBJECT
  public:
    LibraryControl(Library* pLibrary);
    virtual ~LibraryControl();

    void bindLibraryWidget(WLibrary* pLibrary, KeyboardEventFilter* pKeyboard);
    void bindSidebarWidget(WLibrarySidebar* pLibrarySidebar);

  public slots:
    // Deprecated navigation slots
    void slotLoadSelectedTrackToGroup(QString group, bool play);

  private slots:
    void libraryWidgetDeleted();
    void sidebarWidgetDeleted();

    void slotMoveUp(double);
    void slotMoveDown(double);
    void slotMoveVertical(double);
    void slotScrollUp(double);
    void slotScrollDown(double);
    void slotScrollVertical(double);
    void slotMoveLeft(double);
    void slotMoveRight(double);
    void slotMoveHorizontal(double);
    void slotMoveFocusForward(double);
    void slotMoveFocusBackward(double);
    void slotMoveFocus(double);
    void slotGoToItem(double v);

    void slotTrackColorPrev(double v);
    void slotTrackColorNext(double v);

    // Deprecated navigation slots
    void slotSelectNextTrack(double v);
    void slotSelectPrevTrack(double v);
    void slotSelectTrack(double v);
    void slotSelectSidebarItem(double v);
    void slotSelectNextSidebarItem(double v);
    void slotSelectPrevSidebarItem(double v);
    void slotToggleSelectedSidebarItem(double v);
    void slotLoadSelectedIntoFirstStopped(double v);
    void slotAutoDjAddTop(double v);
    void slotAutoDjAddBottom(double v);

    void maybeCreateGroupController(const QString& group);
    void slotNumDecksChanged(double v);
    void slotNumSamplersChanged(double v);
    void slotNumPreviewDecksChanged(double v);

    void slotSortColumn(double v);
    void slotSortColumnToggle(double v);

    void slotFontSize(double v);
    void slotIncrementFontSize(double v);
    void slotDecrementFontSize(double v);

  private:
    Library* m_pLibrary;

    // Simulate pressing a key on the keyboard
    void emitKeyEvent(QKeyEvent&& event);
    // Give the keyboard focus to the main library pane
    void setLibraryFocus();

    // Controls to navigate vertically within currently focused widget (up/down buttons)
    std::unique_ptr<ControlPushButton> m_pMoveUp;
    std::unique_ptr<ControlPushButton> m_pMoveDown;
    std::unique_ptr<ControlEncoder> m_pMoveVertical;

    // Controls to QUICKLY navigate vertically within currently focused widget (pageup/pagedown buttons)
    std::unique_ptr<ControlPushButton> m_pScrollUp;
    std::unique_ptr<ControlPushButton> m_pScrollDown;
    std::unique_ptr<ControlEncoder> m_pScrollVertical;

    // Controls to navigate horizontally within currently selected item (left/right buttons)
    std::unique_ptr<ControlPushButton> m_pMoveLeft;
    std::unique_ptr<ControlPushButton> m_pMoveRight;
    std::unique_ptr<ControlEncoder> m_pMoveHorizontal;

    // Controls to navigate between widgets (tab/shit+tab button)
    std::unique_ptr<ControlPushButton> m_pMoveFocusForward;
    std::unique_ptr<ControlPushButton> m_pMoveFocusBackward;
    std::unique_ptr<ControlEncoder> m_pMoveFocus;

    // Control to choose the currently selected item in focused widget (double click)
    std::unique_ptr<ControlObject> m_pGoToItem;

    // Add to Auto-Dj Cueue
    std::unique_ptr<ControlObject> m_pAutoDjAddTop;
    std::unique_ptr<ControlObject> m_pAutoDjAddBottom;

    // Controls to sort the track view
    std::unique_ptr<ControlEncoder> m_pSortColumn;
    std::unique_ptr<ControlEncoder> m_pSortColumnToggle;
    std::unique_ptr<ControlPushButton> m_pSortOrder;

    // Controls to change track color
    std::unique_ptr<ControlPushButton> m_pTrackColorPrev;
    std::unique_ptr<ControlPushButton> m_pTrackColorNext;

    // Font sizes
    std::unique_ptr<ControlPushButton> m_pFontSizeIncrement;
    std::unique_ptr<ControlPushButton> m_pFontSizeDecrement;
    std::unique_ptr<ControlObject> m_pFontSizeKnob;

    // Direct navigation controls for specific widgets (deprecated)
    std::unique_ptr<ControlObject> m_pSelectNextTrack;
    std::unique_ptr<ControlObject> m_pSelectPrevTrack;
    std::unique_ptr<ControlObject> m_pSelectTrack;
    std::unique_ptr<ControlObject> m_pSelectSidebarItem;
    std::unique_ptr<ControlObject> m_pSelectPrevSidebarItem;
    std::unique_ptr<ControlObject> m_pSelectNextSidebarItem;
    std::unique_ptr<ControlObject> m_pToggleSidebarItem;
    std::unique_ptr<ControlObject> m_pLoadSelectedIntoFirstStopped;

    // Library widgets
    WLibrary* m_pLibraryWidget;
    WLibrarySidebar* m_pSidebarWidget;

    // Other variables
    ControlProxy m_numDecks;
    ControlProxy m_numSamplers;
    ControlProxy m_numPreviewDecks;
    std::map<QString, std::unique_ptr<LoadToGroupController>> m_loadToGroupControllers;
};

#endif //LIBRARYMIDICONTROL_H
