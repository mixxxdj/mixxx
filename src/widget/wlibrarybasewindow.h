#pragma once

#include <QMap>
#include <QStackedWidget>
#include <QString>

#include "library/library_decl.h"
#include "library/libraryview.h"
#include "skin/legacy/skincontext.h"
#include "util/compatibility/qmutex.h"
#include "widget/wbasewidget.h"

class LibraryView;
class WTrackTableView;
class TrackId;
class QDomNode;
class SkinContext;

class WLibraryBaseWindow : public QStackedWidget, public WBaseWidget {
    Q_OBJECT
  public:
    explicit WLibraryBaseWindow(QWidget* parent);
    virtual ~WLibraryBaseWindow() = default;

    void setup(const QDomNode& node, const SkinContext& context);

    // registerView is used to add a view to the LibraryWidget which the widget
    // can display on request via showView(). To switch to a given view, call
    // showView with the name provided here. WLibraryPreparationWindowWidget takes ownership of
    // the view and is in charge of deleting it. Returns whether or not the
    // registration was successful. Registered widget must implement the
    // LibraryView interface.
    bool registerView(const QString& name, QWidget* view);
    bool registerViewInPreparationWindow(const QString& name, QWidget* view);

    LibraryView* getActiveView() const;
    WTrackTableView* getCurrentTrackTableView() const;
    // WLibraryPreparationWindowTrackTableView* getCurrentTrackTableView() const;
    // WLibraryPreparationWindowTrackTableView* getCurrentTrackTableViewInPreparationWindow() const;
    //  This returns true if the current view is or has a WTracksTableView and
    //  contains trackId, otherwise false.
    //  This is primarily used to disable the "Select track in library" track menu action
    //  to avoid unintended behaviour if the current view has no tracks table.
    bool isTrackInCurrentView(const TrackId& trackId);
    bool isTrackInCurrentViewInPreparationWindow(const TrackId& trackId);
    void saveCurrentViewState() const;
    void saveCurrentViewStateInPreparationWindow() const;
    void restoreCurrentViewState() const;
    void restoreCurrentViewStateInPreparationWindow() const;

    // Alpha value for row color background
    static constexpr double kDefaultTrackTableBackgroundColorOpacity = 0.125; // 12.5% opacity
    static constexpr double kMinTrackTableBackgroundColorOpacity = 0.0;       // 0% opacity
    static constexpr double kMaxTrackTableBackgroundColorOpacity = 1.0;       // 100% opacity

    double getTrackTableBackgroundColorOpacity() const {
        return m_trackTableBackgroundColorOpacity;
    }

    bool getShowButtonText() const {
        return m_bShowButtonText;
    }
    // bool dropAccept(const QModelIndex& index, const QList<QUrl>& urls, QObject* pSource);

  signals:
    FocusWidget setLibraryFocus(FocusWidget newFocus);

  public slots:
    // Show the view registered with the given name. Does nothing if the current
    // view is the specified view, or if the name does not specify any
    // registered view.
    void switchToView(const QString& name);
    void switchToViewInPreparationWindow(const QString& name);
    void slotSelectTrackInActiveTrackView(const TrackId& trackId);
    void slotSelectTrackInActiveTrackViewInPreparationWindow(const TrackId& trackId);
    void pasteFromSidebar();
    void pasteFromSidebarInPreparationWindow();
    void search(const QString&);
    void searchInPreparationWindow(const QString&);

  protected:
    bool event(QEvent* pEvent) override;
    //    bool eventInPreparationWindow(QEvent* pEvent) override;
    void keyPressEvent(QKeyEvent* event) override;
    //    void keyPressEventInPreparationWindow(QKeyEvent* event) override;
    const QString m_callingParent;

  private:
    QT_RECURSIVE_MUTEX m_mutex;
    QMap<QString, QWidget*> m_viewMap;
    double m_trackTableBackgroundColorOpacity;
    bool m_bShowButtonText;
};
