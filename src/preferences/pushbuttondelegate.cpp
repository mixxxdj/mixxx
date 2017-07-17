// pushbuttondelegate.cpp
// Created on July 17, 2017 by Stéphane Lepin (Palakis)

#include <QPushButton>
#include <QVariant>

#include <preferences/pushbuttondelegate.h>

namespace {
const char* kPropertyColumn = "item_column";
const char* kPropertyRow = "item_row";
}

PushButtonDelegate::PushButtonDelegate(QObject* parent)
	: QItemDelegate(parent) {
}

QWidget* PushButtonDelegate::createEditor(QWidget* parent, const QStyleOptionViewItem& option,
			const QModelIndex& index) const {
	QPushButton* editor = new QPushButton(tr("Remove"), parent);
	editor->setProperty(kPropertyColumn, index.column());
	editor->setProperty(kPropertyRow, index.row());

	connect(editor, SIGNAL(clicked(bool)),
			this, SLOT(buttonClicked(bool)));

	return editor;
}

void PushButtonDelegate::updateEditorGeometry(QWidget* editor, const QStyleOptionViewItem& option,
			const QModelIndex& index) const {
	editor->setGeometry(option.rect);
}

void PushButtonDelegate::setEditorData(QWidget* editor, const QModelIndex& index) const {
}

void PushButtonDelegate::setModelData(QWidget* editor, QAbstractItemModel* model,
			const QModelIndex& index) {
}

void PushButtonDelegate::buttonClicked(bool enabled) {
	QObject* editor = sender();
	QVariant column = editor->property(kPropertyColumn);
	QVariant row = editor->property(kPropertyRow);

	if(column.isValid() && row.isValid()) {
		emit clicked(column.toInt(), row.toInt());
	}
}

