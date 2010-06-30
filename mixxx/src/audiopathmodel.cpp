/**
 * @file audiopathmodel.cpp
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

#include <climits> // CHAR_MAX
#include <QtCore>
#include "audiopathmodel.h"
#include "sounddevice.h"

AudioPathModel::AudioPathModel(QObject *parent, ConfigObject<ConfigValue> *config)
    : QAbstractTableModel(parent)
    , m_config(config) {
}

int AudioPathModel::columnCount(const QModelIndex &parent /* = QModelIndex() */) const {
    Q_UNUSED(parent);
    return 4; // i/o + sounddevice + channels + (type and index string)
}

int AudioPathModel::rowCount(const QModelIndex &parent /* = QModelIndex() */) const {
    Q_UNUSED(parent);
    return m_data.length();
}

QVariant AudioPathModel::data(const QModelIndex &index, int role /* = Qt::DisplayRole */) const {
    if (!index.isValid() || index.row() >= rowCount()
            || index.column() >= columnCount() || role != Qt::DisplayRole) {
        return QVariant();
    }
    qDebug() << "data()";
    // ok, otherwise index should be valid
    AudioPathModel::AudioPathModelItem row = m_data[index.row()];
    SoundDevice *device = row.device;
    AudioPath path = row.path;
    ChannelGroup chGroup = path.getChannelGroup();
    unsigned int baseChannel = chGroup.getChannelBase();
    unsigned int channelCount = chGroup.getChannelCount();
    QVariant toReturn;
    switch (index.column()) {
    case 0: // i/o
        switch (row.type) {
        case AudioPathModel::SOURCE:
            toReturn = QString("Output");
            break;
        case AudioPathModel::RECEIVER:
            toReturn = QString("Input");
            break;
        }
        break;
    case 1: // device
        toReturn = device->getDisplayName();
        break;
    case 2: // channels
        if (channelCount == 1) {
            toReturn = baseChannel + 1;
        } else {
            toReturn = QString("%1 - %2").arg(baseChannel + 1).arg(baseChannel + channelCount);
        }
        break;
    case 3: // type string
        toReturn = path.getString();
        break;
    }
    qDebug() << "data() finished";
    return toReturn;
}

QVariant AudioPathModel::headerData(int section, Qt::Orientation orientation, int role) const {
    if (orientation != Qt::Horizontal || role != Qt::DisplayRole || section >= columnCount()) {
        return QVariant();
    }
    switch (section) {
    case 0:
        return QString("Input / Output");
        break;
    case 1:
        return QString("Device");
        break;
    case 2:
        return QString("Channels");
        break;
    case 3:
        return QString("Type");
        break;
    }
    return QVariant();
}

bool AudioPathModel::setData(const QModelIndex &index, const QVariant &value,
        int role /*= Qt::EditRole*/) {
    if (!index.isValid() || index.row() >= rowCount()
            || index.column() >= columnCount() || role != Qt::EditRole) {
        return false;
    }
    qDebug() << "setData()";
    // ok, otherwise index should be valid
    AudioPathModel::AudioPathModelItem row = m_data.takeAt(index.row());
    AudioPath::AudioPathType type = row.path.getType();
    unsigned char pathIndex = row.path.getIndex();
    unsigned char channelBase = row.path.getChannelGroup().getChannelBase();
    unsigned char channels = row.path.getChannelGroup().getChannelCount();
    switch (index.column()) {
    case 0: // i/o
        // I expect an int equal to either SOURCE or RECEIVER
        if (value == AudioPathModel::SOURCE) {
            row.type = AudioPathModel::SOURCE;
        } else if (value == AudioPathModel::RECEIVER) {
            row.type = AudioPathModel::RECEIVER;
        }
        break;
    case 1: // device
        // I expect an internal device name
        row.device = NULL;
        foreach (SoundDevice *device, m_devices) {
            if (device->getInternalName() == value) {
                row.device = device;
                break;
            }
        }
        if (row.device == NULL) {
            return false;
        }
        break;
    case 2: // channels
        // I expect a QList<uint> with two entries, chanBase followed by numChannels
        if (!value.canConvert(QVariant::List)) {
            return false;
        }
        if (value.toList().length() != 2) {
            return false;
        }
        channelBase = (unsigned char) (value.toList()[0].toUInt() & CHAR_MAX);
        channels = (unsigned char) (value.toList()[1].toUInt() & CHAR_MAX);
        break;
    case 3: // type
        // I expect a QList<int> with two entries, type followed by index
        if (!value.canConvert(QVariant::List)) {
            return false;
        }
        if (value.toList().length() != 2) {
            return false;
        }
        type = AudioPath::getTypeFromInt(value.toList()[0].toUInt());
        pathIndex = (unsigned char) (value.toList()[1].toUInt() & CHAR_MAX);
        break;
    }
    switch (row.type) {
    case SOURCE:
        row.path = AudioSource(type, channelBase, channels, pathIndex);
        break;
    case RECEIVER:
        row.path = AudioReceiver(type, channelBase, channels, pathIndex);
        break;
    }
    m_data.insert(index.row(), row);
    emit dataChanged(index, index);
    qDebug() << "setData() done";
    return true;
}

Qt::ItemFlags AudioPathModel::flags(const QModelIndex &index) const {
    Q_UNUSED(index);
    return QAbstractTableModel::flags(index) & Qt::ItemIsEditable;
}

bool AudioPathModel::insertRows(int position, int rows,
        const QModelIndex &parent /*= QModelIndex()*/) {
    Q_UNUSED(parent);
    qDebug() << "inserting";
    beginInsertRows(QModelIndex(), position, position + rows - 1);
    for (int row = 0; row < rows; ++row) {
        qDebug() << "making path";
        AudioPathModel::AudioPathModelItem item = {
            m_devices[0],
            AudioPathModel::SOURCE,
            AudioSource(AudioSource::MASTER, 0, 2, 1)
        };
        qDebug() << "and go";
        m_data.insert(position, item);
    }
    endInsertRows();
    qDebug() << "inserting done";
    return true;
}

bool AudioPathModel::removeRows(int position, int rows,
        const QModelIndex &parent /*= QModelIndex()*/) {
    qDebug() << "removing";
    Q_UNUSED(parent);
    beginRemoveRows(QModelIndex(), position, position + rows - 1);
    for (int row = 0; row < rows; ++row) {
        m_data.removeAt(position);
    }
    endRemoveRows();
    qDebug() << "removing done";
    return true;
}
void AudioPathModel::writeToConfig() {

}

void AudioPathModel::setDevices(QList<SoundDevice*> devices) {
    m_devices = devices;
}
