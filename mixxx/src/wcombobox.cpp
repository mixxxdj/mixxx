#include "wcombobox.h"
#include <qwidget.h>
#include <qstring.h>
WComboBox::WComboBox(QWidget *parent) : QComboBox(parent)
{
     insertItem( "Library" );
     insertItem( "Play Queue" );
}
void WComboBox::setup(QDomNode node)
{
	if (!WWidget::selectNode(node, "Pos").isNull())
    {
		 QString pos = WWidget::selectNodeQString(node, "Pos");
		 int x = pos.left(pos.find(",")).toInt();
		 int y = pos.mid(pos.find(",")+1).toInt();
		 move(x,y);
	}
	if (!WWidget::selectNode(node, "Size").isNull())
    {
		 // Size
		 QString size = WWidget::selectNodeQString(node, "Size");
		 int x = size.left(size.find(",")).toInt();
		 int y = size.mid(size.find(",")+1).toInt();
		 setFixedSize(x,y);
	}
}