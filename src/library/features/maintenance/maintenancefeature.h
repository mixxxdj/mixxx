#ifndef MAINTENANCEFEATURE_H
#define MAINTENANCEFEATURE_H

#include <QHash>
#include <QPointer>
#include <QTabWidget>

#include "library/libraryfeature.h"

class DlgHidden;
class DlgMissing;
class HiddenTableModel;
class MissingTableModel;

class MaintenanceFeature : public LibraryFeature
{
    Q_OBJECT
  public:
    MaintenanceFeature(UserSettingsPointer pConfig,
                       Library* pLibrary, QObject* parent, 
                       TrackCollection* pTrackCollection);

    QVariant title() override;
    QString getIconPath() override;
    QString getSettingsName() const override;
    TreeItemModel* getChildModel();

  public slots:
    void activate();
    void selectionChanged(const QItemSelection&, const QItemSelection&);
    void selectAll();

  protected:

    QWidget* createInnerSidebarWidget(KeyboardEventFilter* pKeyboard);
    QWidget* createPaneWidget(KeyboardEventFilter* pKeyboard, int paneId);

  private:

    enum Pane {
        Hidden = 1,
        Missing = 2
    };

    const QString kMaintenanceTitle;
    const QString kHiddenTitle;
    const QString kMissingTitle;
    
  private slots:
    
    void slotTabIndexChanged(int index);    
    void slotUnhideHidden();
    void slotPurge();

  private:

    HiddenTableModel* getHiddenTableModel();
    MissingTableModel* getMissingTableModel();

    QPointer<DlgHidden> m_pHiddenView;
    QPointer<DlgMissing> m_pMissingView;
    QPointer<QTabWidget> m_pTab;
    QHash<int, Pane> m_idPaneCurrent;

    int m_idExpandedHidden;
    int m_idExpandedMissing;

    QPointer<HiddenTableModel> m_pHiddenTableModel;
    QPointer<MissingTableModel> m_pMissingTableModel;
};

#endif // MAINTENANCEFEATURE_H
