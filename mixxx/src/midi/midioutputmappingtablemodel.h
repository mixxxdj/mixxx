
#ifndef _MIDIOUTPUTMAPPINGTABLEMODEL_H_
#define _MIDIOUTPUTMAPPINGTABLEMODEL_H_

#include "midimapping.h" //gives typedef for MidiInputMapping

enum MidiOutputTableIndices {
    MIDIOUTPUTTABLEINDEX_CONTROLOBJECTGROUP = 0,
    MIDIOUTPUTTABLEINDEX_CONTROLOBJECTVALUE,
    MIDIOUTPUTTABLEINDEX_THRESHOLDMIN,
    MIDIOUTPUTTABLEINDEX_THRESHOLDMAX,
    MIDIOUTPUTTABLEINDEX_MIDISTATUS,
    MIDIOUTPUTTABLEINDEX_MIDINO,
    MIDIOUTPUTTABLEINDEX_MIDICHANNEL,
    MIDIOUTPUTTABLEINDEX_MIDIOPTION,
    MIDIOUTPUTTABLEINDEX_CONTROLOBJECTDESCRIPTION,
    MIDIOUTPUTTABLEINDEX_NUMCOLS //Number of columns
};

class MidiOutputMappingTableModel : public QAbstractTableModel
{
Q_OBJECT
public:
    MidiOutputMappingTableModel(MidiMapping* mapping);
    ~MidiOutputMappingTableModel();
    void setMapping(MidiMapping* mapping);
    QVariant data(const QModelIndex &index, int role) const;
    Qt::ItemFlags flags(const QModelIndex &index) const;
    bool setData(const QModelIndex &index, const QVariant &value,
                 int role = Qt::EditRole);
    int rowCount(const QModelIndex& parent=QModelIndex()) const;
    int columnCount(const QModelIndex& parent=QModelIndex()) const;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const;
    bool removeRow(int row, const QModelIndex& parent=QModelIndex());
    bool removeRows(int row, int count, const QModelIndex& parent=QModelIndex());

public slots:
    void slotOutputMappingChanged();
private:
    MidiMapping* m_pMapping;
    
};

#endif
