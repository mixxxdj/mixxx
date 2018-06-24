#ifndef PREVIEWBUTTONDELEGATE_H
#define PREVIEWBUTTONDELEGATE_H

#include <QPushButton>

#include "library/tableitemdelegate.h"
#include "track/track.h"

class ControlProxy;

class PreviewButtonDelegate : public TableItemDelegate {
  Q_OBJECT

  public:
    explicit PreviewButtonDelegate(QTableView* parent, int column);
    virtual ~PreviewButtonDelegate();

    QWidget* createEditor(QWidget* parent,
                          const QStyleOptionViewItem& option,
                          const QModelIndex& index) const;

    void setEditorData(QWidget* editor, const QModelIndex& index) const;
    void setModelData(QWidget* editor, QAbstractItemModel* model,
                      const QModelIndex &index) const;
    void paintItem(QPainter* painter, const QStyleOptionViewItem& option,
               const QModelIndex& index) const;
    QSize sizeHint(const QStyleOptionViewItem &option,
                   const QModelIndex& index) const;
    void updateEditorGeometry(QWidget* editor,const QStyleOptionViewItem& option,
                              const QModelIndex& index) const;

  signals:
    void loadTrackToPlayer(TrackPointer Track, QString group, bool play);
    void buttonSetChecked(bool);

  public slots:
    void cellEntered(const QModelIndex& index);
    void buttonClicked();
    void previewDeckPlayChanged(double v);

  private:
    QTableView* m_pTableView;
    ControlProxy* m_pPreviewDeckPlay;
    ControlProxy* m_pCueGotoAndPlay;
    QPushButton* m_pButton;
    bool m_isOneCellInEditMode;
    QPersistentModelIndex m_currentEditedCellIndex;
    int m_column;
};

#endif // PREVIEWBUTTONDELEGATE_H
