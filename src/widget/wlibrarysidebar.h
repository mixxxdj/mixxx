#pragma once

#include <QBasicTimer>
#include <QModelIndex>
#include <QTreeView>

#include "library/library_decl.h"
#include "widget/wbasewidget.h"

class LibraryFeature;
class QPoint;

class WLibrarySidebar : public QTreeView, public WBaseWidget {
    Q_OBJECT
  public:
    explicit WLibrarySidebar(QWidget* parent = nullptr);
    ~WLibrarySidebar() override;

    void setup(UserSettingsPointer pConfig);

    void contextMenuEvent(QContextMenuEvent* pEvent) override;
    void dragMoveEvent(QDragMoveEvent* pEvent) override;
    void dragEnterEvent(QDragEnterEvent* pEvent) override;
    void dragLeaveEvent(QDragLeaveEvent* pEvent) override;
    void dropEvent(QDropEvent* pEvent) override;
    void keyPressEvent(QKeyEvent* pEvent) override;
    void mousePressEvent(QMouseEvent* pEvent) override;
    void focusInEvent(QFocusEvent* pEvent) override;
    void timerEvent(QTimerEvent* pEvent) override;
    void toggleSelectedItem();
    void renameSelectedItem();
    bool isLeafNodeSelected();
    bool isChildIndexSelected(const QModelIndex& index);
    bool isFeatureRootIndexSelected(LibraryFeature* pFeature);

  public slots:
    void selectIndex(const QModelIndex& index, bool scrollTo = true);
    void saveScrollPosition();
    void restoreScrollPosition();
    void selectChildIndex(const QModelIndex& index, bool selectItem = true);
    void slotSetFont(const QFont& font);

  signals:
    void rightClicked(const QPoint&, const QModelIndex&);
    void renameItem(const QModelIndex&);
    void deleteItem(const QModelIndex&);
    FocusWidget setLibraryFocus(FocusWidget newFocus,
            Qt::FocusReason focusReason = Qt::OtherFocusReason);

  protected:
    bool event(QEvent* pEvent) override;

  private:
    void focusSelectedIndex();
    QModelIndex selectedIndex();

    void toggleDragHoverPropertyAndUpdateStyle(bool enabled);
    void resetHoverIndexAndDragMoveResult();

    QBasicTimer m_expandTimer;
    QModelIndex m_hoverIndex;
    bool m_lastDragMoveAccepted;
    UserSettingsPointer m_pConfig;
};
