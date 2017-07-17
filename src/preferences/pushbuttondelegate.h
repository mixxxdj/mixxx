// pushbuttondelegate.h
// Created on July 17, 2017 by Stéphane Lepin (Palakis)

#ifndef SRC_PREFERENCES_PUSHBUTTONDELEGATE_H
#define SRC_PREFERENCES_PUSHBUTTONDELEGATE_H

#include <QItemDelegate>
#include <QObject>
#include <QWidget>
#include <QStyleOptionViewItem>
#include <QModelIndex>

class PushButtonDelegate : public QItemDelegate {
  Q_OBJECT
  public:
	PushButtonDelegate(QObject* parent = nullptr);

	QWidget* createEditor(QWidget* parent, const QStyleOptionViewItem& option,
			const QModelIndex& index) const;
	void updateEditorGeometry(QWidget* editor, const QStyleOptionViewItem& option,
			const QModelIndex& index) const;
	void setEditorData(QWidget* editor, const QModelIndex& index) const;
	void setModelData(QWidget* editor, QAbstractItemModel* model,
			const QModelIndex& index);

  signals:
    void clicked(int column, int row);

  private slots:
    void buttonClicked(bool enabled);
};

#endif // SRC_PREFERENCES_PUSHBUTTONDELEGATE_H
