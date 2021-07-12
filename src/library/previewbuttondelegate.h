#pragma once

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
    explicit LibraryPreviewButton(QWidget* parent)
            : QPushButton(parent) {
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
    PreviewButtonDelegate(WLibraryTableView* parent, int column);
    ~PreviewButtonDelegate() override;

    QWidget* createEditor(
            QWidget* parent,
            const QStyleOptionViewItem& option,
            const QModelIndex& index) const override;

    void setEditorData(
            QWidget* editor,
            const QModelIndex& index) const override;
    void setModelData(
            QWidget* editor,
            QAbstractItemModel* model,
            const QModelIndex& index) const override;
    void paintItem(
            QPainter* painter,
            const QStyleOptionViewItem& option,
            const QModelIndex& index) const override;
    QSize sizeHint(
            const QStyleOptionViewItem& option,
            const QModelIndex& index) const override;
    void updateEditorGeometry(QWidget* editor,
            const QStyleOptionViewItem& option,
            const QModelIndex& index) const override;

  signals:
    void loadTrackToPlayer(const TrackPointer& pTrack, const QString& group, bool play);
    void buttonSetChecked(bool);

  public slots:
    void cellEntered(const QModelIndex& index);
    void buttonClicked();
    void previewDeckPlayChanged(double v);

  private:
    QTableView* parentTableView() const {
        return qobject_cast<QTableView*>(parent());
    }
    bool isPreviewDeckPlaying() const;
    bool isTrackLoadedInPreviewDeck(
            const QModelIndex& index) const;
    bool isTrackLoadedInPreviewDeckAndPlaying(
            const QModelIndex& index) const {
        if (!isPreviewDeckPlaying()) {
            // No need to query additional data from the table
            return false;
        }
        return isTrackLoadedInPreviewDeck(index);
    }

    const int m_column;

    const parented_ptr<ControlProxy> m_pPreviewDeckPlay;
    const parented_ptr<ControlProxy> m_pCueGotoAndPlay;

    const parented_ptr<LibraryPreviewButton> m_pButton;

    QPersistentModelIndex m_currentEditedCellIndex;
};
