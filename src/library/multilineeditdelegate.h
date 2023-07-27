#pragma once

#include <QPlainTextEdit>

#include "library/tableitemdelegate.h"

/// A QPlainTextEdit to show all content lines in a scrollable view.
/// * finish editing with Return key, like QLineEdit used for other text columns
/// * add new line with Shift+Return
/// Horizontal scrollbar is hidden as long as content is just one line.
/// Note: QTextEdit is no option here since it seems to transform content with
/// line breaks to html doc when committing data.
class MultiLineEditor : public QPlainTextEdit {
    Q_OBJECT
  public:
    MultiLineEditor(QWidget* pParent);

    bool eventFilter(QObject* obj, QEvent* event) override;

  signals:
    void editingFinished();
};

/// A delegate for text value columns that allows editing content
/// content in a multi-line editor instead of default QLineEdit
class MultiLineEditDelegate : public TableItemDelegate {
    Q_OBJECT
  public:
    explicit MultiLineEditDelegate(QTableView* pTrackTable);
    ~MultiLineEditDelegate() override = default;

    // called when the user starts editing an item
    QWidget* createEditor(QWidget* parent,
            const QStyleOptionViewItem& option,
            const QModelIndex& index) const override;

  private slots:
    void commitAndCloseEditor();

  private:
    void adjustEditor(MultiLineEditor* pEditor, const QSizeF size, const QRect iRect) const;
};
