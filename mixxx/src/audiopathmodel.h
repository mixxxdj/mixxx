/**
 * @file audiopathmodel.h
 * @author Bill Good <bkgood at gmail dot com>
 * @date 20100628
 */

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef AUDIOPATHMODEL_H
#define AUDIOPATHMODEL_H

#include <QtCore>
#include "audiopath.h"
#include "configobject.h"

class SoundDevice;

class AudioPathModel : public QAbstractTableModel {
    Q_OBJECT;
public:
    AudioPathModel(QObject *parent, ConfigObject<ConfigValue> *config);
    int columnCount(const QModelIndex &parent = QModelIndex()) const;
    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
    QVariant headerData(int section, Qt::Orientation orientation,
            int role = Qt::DisplayRole) const;
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole);
    Qt::ItemFlags flags(const QModelIndex &index) const;
    bool insertRows(int position, int rows, const QModelIndex &parent = QModelIndex());
    bool removeRows(int position, int rows, const QModelIndex &parent = QModelIndex()); 
    void writeToConfig();
    void setDevices(QList<SoundDevice*> devices);
private:
    enum AudioPathModelItemType {
        RECEIVER,
        SOURCE
    };
    struct AudioPathModelItem {
        SoundDevice *device;
        AudioPathModelItemType type;
        AudioPath path;
    };
    QList<AudioPathModelItem> m_data;
    ConfigObject<ConfigValue> *m_config;
    QList<SoundDevice*> m_devices;
};

#endif
