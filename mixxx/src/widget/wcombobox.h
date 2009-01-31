#ifndef WCOMBOBOX_H
#define WCOMBOBOX_H

#include <QComboBox>
#include <qdom.h>

class QComboBox;
class QWidget;
class QDomNode;
class QDomElement;

class WComboBox : public QComboBox
{
  Q_OBJECT
  public:
	  WComboBox(QWidget *parent=0, const char *name=0);
      ~WComboBox();
      void setup(QDomNode node);
};
#endif