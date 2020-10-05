#ifndef PREVIEWBUTTONDELEGATE_H
#define PREVIEWBUTTONDELEGATE_H

#include <QPushButton>
#include <QStyleOptionButton>

#include "library/tableitemdelegate.h"
#include "track/track_decl.h"
#include "util/parented_ptr.h"

class ControlProxy;
class WLibraryTableView;

// A QPushButton for rendering the library preview button within the
// PreviewButtonDelegate.
class LibraryPreviewButton : public QPushButton {
    Q_OBJECT
  public:
    LibraryPreviewButton(QWidget* parent=nullptr) : QPushButton(parent) {
        setObjectName("LibraryPreviewButton");
    }

    void paint(QPainter* painter) {
        // This matches the implementation of QPushButton::paintEvent, except it
        // does not create a new QStylePainter, and it is simpler and more
        // direct than QWidget::render(QPainter*, ...).
        QStyleOptionButton option;
        initStyleOption(&option);
        auto pStyle = style();
        if (pStyle) {
            pStyle->drawControl(QStyle::CE_PushButton, &option, painter, this);
        }
    }
};

class PreviewButtonDelegate : public TableItemDelegate {
  Q_OBJECT

  public:
    explicit PreviewButtonDelegate(WLibraryTableView* parent, int column);
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
    void loadTrackToPlayer(TrackPointer track, QString group, bool play);
    void buttonSetChecked(bool);

  public slots:
    void cellEntered(const QModelIndex& index);
    void buttonClicked();
    void previewDeckPlayChanged(double v);

  private:
    QTableView* m_pTableView;
    ControlProxy* m_pPreviewDeckPlay;
    ControlProxy* m_pCueGotoAndPlay;
    parented_ptr<LibraryPreviewButton> m_pButton;
    bool m_isOneCellInEditMode;
    QPersistentModelIndex m_currentEditedCellIndex;
    int m_column;
};

#endif // PREVIEWBUTTONDELEGATE_H
