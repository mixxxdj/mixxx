/***************************************************************************
                          controlobject.h  -  description
                             -------------------
    begin                : Wed Feb 20 2002
    copyright            : (C) 2002 by Tue and Ken Haste Andersen
    email                : 
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef CONTROLOBJECT_H
#define CONTROLOBJECT_H

#include <qobject.h>
#include <qwidget.h>
#include <qevent.h>
#include <qptrlist.h>
#include "configobject.h"
#include "defs.h"

class ControlEngine;
class ControlEngineQueue;
/**
  *@author Tue and Ken Haste Andersen
  */

class ControlObject : public QObject
{
  Q_OBJECT
public:
  ControlObject();
  ControlObject(ConfigKey key);
  ~ControlObject();
  
  QString *print();
  static void setConfig(ConfigObject<ConfigValueMidi> *_config);
  static void setControlEngineQueue(ControlEngineQueue *_queue);
  void setControlEngine(int _controlEngineNo);
  void setWidget(QWidget *widget);
  FLOAT_TYPE getValue();
public slots:
  virtual void slotSetPosition(int) = 0;
  void slotSetPositionMidi(int);
signals:
  void updateGUI(int);
  void valueChanged(FLOAT_TYPE);
protected:
  /** Set the value of the object. Called from event handler when receiving ControlEventEngine. */
  void setValue(FLOAT_TYPE);
  /** Forces the gui to be updated with the value of the controller */
  virtual void forceGUIUpdate() = 0;  
  void emitValueChanged(FLOAT_TYPE);

  FLOAT_TYPE value;
  static ConfigObject<ConfigValueMidi> *config;
  static ControlEngineQueue *queue;
  ConfigOption<ConfigValueMidi> *cfgOption;
  /** Internal number of associated ControlEngine object */
  int controlEngineNo;
private:
  void midi(char channel, char control, char value);
  bool eventFilter(QObject *, QEvent *);

  /** List of ControlObject instantiations */
  static QPtrList<ControlObject> list;
};

#endif

