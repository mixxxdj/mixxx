#ifndef LIBRARYMIDICONTROL_H
#define LIBRARYMIDICONTROL_H

#include <QObject>

#include "controlobjectthread.h"

class ControlObject;
class WLibrary;
class WLibrarySidebar;
class MixxxKeyboard;

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
    LibraryControl(QObject* pParent=NULL);
    virtual ~LibraryControl();

    void bindWidget(WLibrary* pLibrary, MixxxKeyboard* pKeyboard);
    void bindSidebarWidget(WLibrarySidebar* pLibrarySidebar);

  private slots:
    void libraryWidgetDeleted();
    void sidebarWidgetDeleted();
    void slotLoadSelectedTrackToGroup(QString group, bool play);
    void slotSelectNextTrack(double v);
    void slotSelectPrevTrack(double v);
    void slotSelectTrack(double v);
    void slotSelectSidebarItem(double v);
    void slotSelectNextSidebarItem(double v);
    void slotSelectPrevSidebarItem(double v);
    void slotToggleSelectedSidebarItem(double v);
    void slotLoadSelectedIntoFirstStopped(double v);

    void maybeCreateGroupController(const QString& group);
    void slotNumDecksChanged(double v);
    void slotNumSamplersChanged(double v);
    void slotNumPreviewDecksChanged(double v);

  private:
    ControlObject* m_pSelectNextTrack;
    ControlObject* m_pSelectPrevTrack;
    ControlObject* m_pSelectTrack;

    ControlObject* m_pSelectSidebarItem;
    ControlObject* m_pSelectPrevSidebarItem;
    ControlObject* m_pSelectNextSidebarItem;

    ControlObject* m_pToggleSidebarItem;
    ControlObject* m_pLoadSelectedIntoFirstStopped;

    WLibrary* m_pLibraryWidget;
    WLibrarySidebar* m_pSidebarWidget;
    ControlObjectThread m_numDecks;
    ControlObjectThread m_numSamplers;
    ControlObjectThread m_numPreviewDecks;
    QMap<QString, LoadToGroupController*> m_loadToGroupControllers;
};

#endif //LIBRARYMIDICONTROL_H
