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
    Library* m_pLibrary;

    // Simulate pressing a key on the keyboard
    void emitKeyEvent(QKeyEvent&& event);

    // Controls to navigate vertically within currently focussed widget (up/down buttons)
    ControlPushButton* m_pMoveUp;
    ControlPushButton* m_pMoveDown;
    ControlObject* m_pMoveVertical;

    // Controls to navigate horizontally within currently selected item (left/right buttons)
    ControlPushButton* m_pMoveLeft;
    ControlPushButton* m_pMoveRight;
    ControlObject* m_pMoveHorizontal;

    // Controls to navigate between widgets (tab/shit+tab button)
    ControlPushButton* m_pMoveFocusForward;
    ControlPushButton* m_pMoveFocusBackward;
    ControlObject* m_pMoveFocus;

    // Control to choose the currently selected item in focussed widget (double click)
    ControlObject* m_pChooseItem;

    // Font sizes
    ControlPushButton* m_pFontSizeIncrement;
    ControlPushButton* m_pFontSizeDecrement;
    ControlObject* m_pFontSizeKnob;

    // Direct navigation controls for specific widgets (deprecated)
    ControlObject* m_pSelectNextTrack;
    ControlObject* m_pSelectPrevTrack;
    ControlObject* m_pSelectTrack;
    ControlObject* m_pSelectSidebarItem;
    ControlObject* m_pSelectPrevSidebarItem;
    ControlObject* m_pSelectNextSidebarItem;
    ControlObject* m_pToggleSidebarItem;
    ControlObject* m_pLoadSelectedIntoFirstStopped;
    ControlObject* m_pAutoDjAddTop;
    ControlObject* m_pAutoDjAddBottom;

    // Library widgets
    WLibrary* m_pLibraryWidget;
    WLibrarySidebar* m_pSidebarWidget;

    // Other variables
    ControlProxy m_numDecks;
    ControlProxy m_numSamplers;
    ControlProxy m_numPreviewDecks;
    QMap<QString, LoadToGroupController*> m_loadToGroupControllers;
};

#endif //LIBRARYMIDICONTROL_H
