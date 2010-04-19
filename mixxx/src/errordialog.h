/***************************************************************************
                          errordialog.h  -  description
                             -------------------
    begin                : Fri Feb 20 2009
    copyright            : (C) 2009 by Sean M. Pappalardo
    email                : pegasus@c64.org
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef ERRORDIALOG_H
#define ERRORDIALOG_H

#include <QObject>
#include <QMessageBox>
#include <QSignalMapper>

/**
  * Class used to allow all threads to display message boxes on error conditions
  *
  *@author Sean M. Pappalardo
  */

typedef enum {
    DLG_FATAL       = 5,
    DLG_CRITICAL    = 4,
    DLG_WARNING     = 3,
    DLG_INFO        = 2,
    DLG_QUESTION    = 1,
    DLG_NONE        = 0 // No icon (default)
} DialogType;

class DialogProperties {
    public:
        DialogProperties();

        void setTitle(QString title);
        QString getTitle();
        /** Automatically sets the icon to match the type for convenience. */
        void setType(DialogType typeToSet);
        
        DialogType type;
        QMessageBox::Icon icon;
        /** Set a key to prevent multiple dialogs until the first is closed */
        QString key;
        QString text;
        QString infoText;
        QString details;
        bool modal;
        /** Note that a QMessageBox::Ignore button, if clicked, will suppress
            error boxes with the same key for the duration of the application. */
        QList<QMessageBox::StandardButton> buttons;
        /** The default button to highlight, if any. */
        QMessageBox::StandardButton defaultButton;
        /** The button that's clicked if the Escape key is pressed. */
        QMessageBox::StandardButton escapeButton;
        
    private:
        /** We keep control of this so "Mixxx" is always part of the title */
        QString m_title;
};


class ErrorDialog : public QObject {
   Q_OBJECT
public:
    ErrorDialog();
    ~ErrorDialog();
    /** A qMessageHandler or any thread calls either of these to emit a signal to display the requested message box */
    /** They return false if a dialog with the same key (or title if no key) is already displayed */
    bool requestErrorDialog(DialogType type, QString message);
    bool requestErrorDialog(DialogProperties* props);
    /** Allows a means for main() to skip exec() if there was a critical or fatal error dialog displayed on app initialization */
    bool checkError();
    
signals:
    void showErrorDialog(DialogProperties* props);
    void stdButtonClicked(QString key, QMessageBox::StandardButton whichStdButton);

private:
    bool m_errorCondition;
    QList<QString> m_dialogKeys;
    QSignalMapper* m_pSignalMapper;

private slots:
    /** Actually displays the box */
    void errorDialog(DialogProperties* props);
    void boxClosed(QString key);
};

extern ErrorDialog* g_pDialogHelper;

#endif
