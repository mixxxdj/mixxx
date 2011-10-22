
#ifndef _MIDIINPUTMAPPINGTABLEMODEL_H_
#define _MIDIINPUTMAPPINGTABLEMODEL_H_

#include "midimapping.h" //gives typedef for MidiInputMapping

enum MidiInputTableIndices {
    MIDIINPUTTABLEINDEX_MIDISTATUS = 0,
    MIDIINPUTTABLEINDEX_MIDINO,
    MIDIINPUTTABLEINDEX_MIDICHANNEL,
    MIDIINPUTTABLEINDEX_CONTROLOBJECTGROUP,
    MIDIINPUTTABLEINDEX_CONTROLOBJECTVALUE,
    MIDIINPUTTABLEINDEX_MIDIOPTION,
    MIDIINPUTTABLEINDEX_CONTROLOBJECTDESCRIPTION,
    MIDIINPUTTABLEINDEX_NUMCOLS //Number of columns
};

class MidiInputMappingTableModel : public QAbstractTableModel
{
Q_OBJECT
public:
    MidiInputMappingTableModel(MidiMapping* mapping);
    ~MidiInputMappingTableModel();
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
    void slotInputMappingChanged();
private:
    MidiMapping* m_pMapping;
    
};

#endif
