#ifndef LIBRARYMIDICONTROL_H
#define LIBRARYMIDICONTROL_H

#include <QObject>

#include "control/controlproxy.h"

class ControlObject;
class ControlPushButton;
class Library;
class WLibrary;
class WLibrarySidebar;
class KeyboardEventFilter;

class LoadToGroupController : public QObject {
    Q_OBJECT
  public:
    LoadToGroupController(QObject* pParent, const QString& group);
    virtual ~LoadToGroupController();

  signals:
    void loadToGroup(QString group, bool);

  public slots:
    void slotLoadToGroup(double v);
    void slotLoadToGroupAndPlay(double v);

  private:
    QString m_group;
    ControlObject* m_pLoadControl;
    ControlObject* m_pLoadAndPlayControl;
};

class LibraryControl : public QObject {
    Q_OBJECT
  public:
    LibraryControl(Library* pLibrary);
    virtual ~LibraryControl();

    void bindWidget(WLibrary* pLibrary, KeyboardEventFilter* pKeyboard);
    void bindSidebarWidget(WLibrarySidebar* pLibrarySidebar);

  private slots:
    void libraryWidgetDeleted();
    void sidebarWidgetDeleted();

    void slotMoveUp(double);
    void slotMoveDown(double);
    void slotMoveVertical(double);
    void slotMoveLeft(double);
    void slotMoveRight(double);
    void slotMoveHorizontal(double);
    void slotMoveFocusForward(double);
    void slotMoveFocusBackward(double);
    void slotMoveFocus(double);
    void slotChooseItem(double v);

    // Deprecated navigation slots
    void slotLoadSelectedTrackToGroup(QString group, bool play);
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

    void slotFontSize(double v);
    void slotIncrementFontSize(double v);
    void slotDecrementFontSize(double v);

  private:
    Library* m_pLibrary = nullptr;

    // Simulate pressing a key on the keyboard
    void emitKeyEvent(QKeyEvent&& event);

    // Controls to navigate vertically within currently focussed widget (up/down buttons)
    ControlPushButton* m_pMoveUp = nullptr;
    ControlPushButton* m_pMoveDown = nullptr;
    ControlObject* m_pMoveVertical = nullptr;

    // Controls to navigate horizontally within currently selected item (left/right buttons)
    ControlPushButton* m_pMoveLeft = nullptr;
    ControlPushButton* m_pMoveRight = nullptr;
    ControlObject* m_pMoveHorizontal = nullptr;

    // Controls to navigate between widgets (tab/shit+tab button)
    ControlPushButton* m_pMoveFocusForward = nullptr;
    ControlPushButton* m_pMoveFocusBackward = nullptr;
    ControlObject* m_pMoveFocus = nullptr;

    // Control to choose the currently selected item in focussed widget (double click)
    ControlObject* m_pChooseItem = nullptr;

    // Font sizes
    ControlPushButton* m_pFontSizeIncrement = nullptr;
    ControlPushButton* m_pFontSizeDecrement = nullptr;
    ControlObject* m_pFontSizeKnob = nullptr;

    // Direct navigation controls for specific widgets (deprecated)
    ControlObject* m_pSelectNextTrack = nullptr;
    ControlObject* m_pSelectPrevTrack = nullptr;
    ControlObject* m_pSelectTrack = nullptr;
    ControlObject* m_pSelectSidebarItem = nullptr;
    ControlObject* m_pSelectPrevSidebarItem = nullptr;
    ControlObject* m_pSelectNextSidebarItem = nullptr;
    ControlObject* m_pToggleSidebarItem = nullptr;
    ControlObject* m_pLoadSelectedIntoFirstStopped = nullptr;
    ControlObject* m_pAutoDjAddTop = nullptr;
    ControlObject* m_pAutoDjAddBottom = nullptr;

    // Library widgets
    WLibrary* m_pLibraryWidget = nullptr;
    WLibrarySidebar* m_pSidebarWidget = nullptr;

    // Other variables
    ControlProxy m_numDecks;
    ControlProxy m_numSamplers;
    ControlProxy m_numPreviewDecks;
    QMap<QString, LoadToGroupController*> m_loadToGroupControllers;
};

#endif //LIBRARYMIDICONTROL_H
