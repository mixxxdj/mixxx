/***************************************************************************
                          errordialoghandler.h  -  description
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

#ifndef ERRORDIALOGHANDLER_H
#define ERRORDIALOGHANDLER_H

#include <QObject>
#include <QMessageBox>
#include <QSignalMapper>
#include <QMutex>

#include "util.h"

/**
  * Class used to allow all threads to display message boxes on error conditions
  *     with a custom list of standard buttons and to be able to react to them
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

class ErrorDialogProperties {
  public:
    /** Set the window title. ("Mixxx" is always prepended.) */
    void setTitle(QString title);

    /** Set a key to prevent multiple dialogs until the first is closed */
    inline void setKey(QString key) {
        m_key = key;
    }

    QString getKey() const {
        return m_key;
    }

    /** Set the primary window text */
    void setText(QString text);
    QString getText() const {
        return m_text;
    }

    /** Set additional window text */
    inline void setInfoText(QString text) {
        m_infoText = text;
    }

    /** Set detailed text (causes "Show Details" button to appear.) */
    inline void setDetails(QString text) {
        m_details = text;
    }

    /** Set whether the box is modal (blocks the GUI) or not */
    inline void setModal(bool modal) {
        m_modal = modal;
    }

    /** Set whether Mixxx should quit after the user acknowledges the error. */
    inline void setShouldQuit(bool shouldQuit) {
        m_shouldQuit = shouldQuit;
    }

    /** Automatically sets the icon to match the type for convenience. */
    void setType(DialogType typeToSet);

    /** Set the box's icon to one of the standard ones */
    inline void setIcon(QMessageBox::Icon icon) {
        m_icon = icon;
    }

    /** Add a standard button to the box */
    void addButton(QMessageBox::StandardButton);

    /** Set the default button to highlight */
    inline void setDefaultButton(QMessageBox::StandardButton button) {
        m_defaultButton = button;
    }

    /** Set the button to click if the Escape key is pressed */
    inline void setEscapeButton(QMessageBox::StandardButton button) {
        m_escapeButton = button;
    }

  private:
    // Only ErrorDialogHandler should instantiate this
    ErrorDialogProperties();

    QString m_title;
    QString m_key;
    QString m_text;
    QString m_infoText;
    QString m_details;
    bool m_modal;
    bool m_shouldQuit;
    DialogType m_type;
    QMessageBox::Icon m_icon;
    /** List of standard buttons to add to the box, in order
        Note that a QMessageBox::Ignore button, if clicked, will suppress
        error boxes with the same key for the duration of the application. */
    QList<QMessageBox::StandardButton> m_buttons;
    /** The default button to highlight, if any. */
    QMessageBox::StandardButton m_defaultButton;
    /** The button that's clicked if the Escape key is pressed. */
    QMessageBox::StandardButton m_escapeButton;

    // Befriending ErrorDialogHandler allows it to have cleaner code since
    // the two are closely related anyway
    friend class ErrorDialogHandler;
};

/** Singleton class because we only need one Handler to manage all error dialogs */
class ErrorDialogHandler : public QObject {
   Q_OBJECT
 public:
    static ErrorDialogHandler* instance() {
        if (!s_pInstance)
            s_pInstance = new ErrorDialogHandler();
        return s_pInstance;
    }

    virtual ~ErrorDialogHandler();
    // Call this to get a new instance of ErrorDialogProperties to populate with
    // data
    ErrorDialogProperties* newDialogProperties();

    // Any thread may call either of these to emit a signal to display the
    // requested message box. They return false if a dialog with the same key
    // (or title if no key) is already displayed. If shouldQuit is true, Mixxx
    // will shut down.
    bool requestErrorDialog(DialogType type, QString message,
                            bool shouldQuit=false);
    bool requestErrorDialog(ErrorDialogProperties* props);

    // Allows a means for main() to skip exec() if there was a critical or fatal
    // error dialog displayed on app initialization.
    bool checkError();

  signals:
    void showErrorDialog(ErrorDialogProperties* props);
    void stdButtonClicked(QString key, QMessageBox::StandardButton whichStdButton);

  private slots:
    /** Actually displays the box */
    void errorDialog(ErrorDialogProperties* props);
    void boxClosed(QString key);

  private:
    // Private constructor
    ErrorDialogHandler();

    static ErrorDialogHandler *s_pInstance;

    bool m_errorCondition;
    QList<QString> m_dialogKeys;
    QSignalMapper m_signalMapper;
    QMutex m_mutex;

    DISALLOW_COPY_AND_ASSIGN(ErrorDialogHandler);
};

#endif
