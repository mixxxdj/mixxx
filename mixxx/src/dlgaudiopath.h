/**
 * @file dlgaudiopath.h
 * @author Bill Good <bkgood at gmail dot com>
 * @date 20100626
 */

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef DLGAUDIOPATH_H
#define DLGAUDIOPATH_H

#include <QtCore>
#include "ui_dlgaudiopathdlg.h"
#include "configobject.h"
#include "audiopath.h"

class AudioPath;

class DlgAudioPath : public QDialog, public Ui::DlgAudioPathDlg  {
    Q_OBJECT
public:
    DlgAudioPath(QWidget *parent);
    ~DlgAudioPath();
    AudioPath getPath() const;
signals:
public slots:
private:
    enum IO {
        INPUT,
        OUTPUT
    };
    void populateDevices();
    void populateChannels();
    void populateTypes();
    IO m_io;
    AudioPath::AudioPathType m_type;
    unsigned int m_index;
    unsigned int m_channelBase;
    unsigned int m_channels;
private slots:
    void ioChanged();
    void deviceChanged();
    void typeChanged();
};

#endif
