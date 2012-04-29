/***************************************************************************
                          wwidget.h  -  description
                             -------------------
    begin                : Wed Jun 18 2003
    copyright            : (C) 2003 by Tue & Ken Haste Andersen
    email                : haste@diku.dk
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef WWIDGET_H
#define WWIDGET_H

#include <QtGui>

#include "configobject.h"

/**
  * Abstract class used in widgets connected to ControlObjects. Derived
  * widgets can implement the signal and slot for manipulating the widgets
  * value. The widgets internal value should match that of a MIDI control.
  * The ControlObject can contain another mapping of the MIDI/Widget
  * value, but the mapping should always be done in the ControlObject.
  *
  *@author Tue & Ken Haste Andersen
  */

class WWidget : public QWidget  {
   Q_OBJECT
public:
    WWidget(QWidget *parent=0, Qt::WFlags flags=0);
    virtual ~WWidget();

    /** Sets the path used to find pixmaps */
    static void setPixmapPath(QString qPath);
    static QDomNode selectNode(const QDomNode &nodeHeader, const QString sNode);
    static int selectNodeInt(const QDomNode &nodeHeader, const QString sNode);
    static float selectNodeFloat(const QDomNode &nodeHeader, const QString sNode);
    static QString selectNodeQString(const QDomNode &nodeHeader, const QString sNode);

    /** Given a filename of a pixmap, returns its path */
    static const QString getPath(QString location);
    double getValue();
public slots:
    virtual void setValue(double fValue);
    void updateValue(double fValue);
    void setOnOff(double);
private slots:
    void slotReEmitValueDown(double);
    void slotReEmitValueUp(double);
signals:
    void valueReset();
    void valueChangedDown(double);
    void valueChangedUp(double);
    void valueChangedLeftDown(double);
    void valueChangedLeftUp(double);
    void valueChangedRightDown(double);
    void valueChangedRightUp(double);
protected:
    /** Value/state of widget */
    double m_fValue;
    /** Is true if widget is off */
    bool m_bOff;

private:
    /** Variable containing the path to the pixmaps */
    static QString m_qPath;
    /** Property used when connecting to ControlObject */
    bool m_bEmitOnDownPress;
};

#endif
