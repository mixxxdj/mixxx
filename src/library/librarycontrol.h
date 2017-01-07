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
    void slotLoadSelectedTrackToGroup(QString group, bool play);
    void slotSelectNextTrack(double v);
    void slotSelectPrevTrack(double v);
    void slotSelectTrack(double v);
    void slotSelectSidebarItem(double v);
    void slotSelectItem(double v);
    void slotSelectNextSidebarItem(double v);
    void slotSelectPrevSidebarItem(double v);
    void slotToggleSelectedSidebarItem(double v);
    void slotToggleFocusWidget(double v);
    void slotChooseItem(double v);
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

    ControlObject* m_pSelectNextTrack = nullptr;
    ControlObject* m_pSelectPrevTrack = nullptr;
    ControlObject* m_pSelectTrack = nullptr;

    ControlObject* m_pSelectSidebarItem = nullptr;
    ControlObject* m_pToggleFocusWidget = nullptr;
    ControlObject* m_pSelectItem = nullptr;
    ControlObject* m_pSelectPrevSidebarItem = nullptr;
    ControlObject* m_pSelectNextSidebarItem = nullptr;

    ControlObject* m_pToggleSidebarItem = nullptr;
    ControlObject* m_pChooseItem = nullptr;
    ControlObject* m_pLoadSelectedIntoFirstStopped = nullptr;
    ControlObject* m_pAutoDjAddTop = nullptr;
    ControlObject* m_pAutoDjAddBottom = nullptr;

    ControlObject* m_pFontSizeKnob = nullptr;
    ControlPushButton* m_pFontSizeIncrement = nullptr;
    ControlPushButton* m_pFontSizeDecrement = nullptr;

    WLibrary* m_pLibraryWidget = nullptr;
    WLibrarySidebar* m_pSidebarWidget = nullptr;
    ControlProxy m_numDecks;
    ControlProxy m_numSamplers;
    ControlProxy m_numPreviewDecks;
    QMap<QString, LoadToGroupController*> m_loadToGroupControllers;
};

#endif //LIBRARYMIDICONTROL_H
