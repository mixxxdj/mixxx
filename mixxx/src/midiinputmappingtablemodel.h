
#ifndef _MIDIINPUTMAPPINGTABLEMODEL_H_
#define _MIDIINPUTMAPPINGTABLEMODEL_H_

#include "midimapping.h" //gives typedef for MidiInputMapping

enum MidiInputTableIndices {
    MIDIINPUTTABLEINDEX_MIDITYPE = 0,
    MIDIINPUTTABLEINDEX_MIDINO,
    MIDIINPUTTABLEINDEX_MIDICHANNEL,
    MIDIINPUTTABLEINDEX_CONTROLOBJECTGROUP,
    MIDIINPUTTABLEINDEX_CONTROLOBJECTVALUE,
    MIDIINPUTTABLEINDEX_MIDIOPTION,
    MIDIINPUTTABLEINDEX_NUMCOLS //Number of columns
};

class MidiInputMappingTableModel : public QAbstractTableModel
{
    public:
        MidiInputMappingTableModel(MidiInputMapping* mapping);
        ~MidiInputMappingTableModel();
        void setMapping(MidiInputMapping* mapping);
        QVariant data(const QModelIndex &index, int role) const;
         Qt::ItemFlags flags(const QModelIndex &index) const;
         bool setData(const QModelIndex &index, const QVariant &value,
                      int role = Qt::EditRole);
        int rowCount(const QModelIndex& parent) const;
        int columnCount(const QModelIndex& parent) const;
        QVariant headerData(int section, Qt::Orientation orientation, int role) const;
        void removeRow(int row, const QModelIndex& parent=QModelIndex());
    private:
        MidiInputMapping* m_pMapping;
    
};

#endif _MIDIINPUTMAPPINGTABLEMODEL_H_
