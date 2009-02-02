
#include <QtCore>
#include <QLabel>
#include "midimapping.h"
#include "midiinputmappingtablemodel.h"

MidiInputMappingTableModel::MidiInputMappingTableModel(MidiInputMapping* mapping) : QAbstractTableModel()
{
    setMapping(mapping); //Save the mapping
}

MidiInputMappingTableModel::~MidiInputMappingTableModel()
{
}

void MidiInputMappingTableModel::setMapping(MidiInputMapping* mapping)
{
    m_pMapping = mapping;
}

QVariant MidiInputMappingTableModel::data(const QModelIndex &index, int role) const
 {
     if (!index.isValid())
         return QVariant();

     if (index.row() >= m_pMapping->size())
         return QVariant();

     if (role == Qt::DisplayRole || role == Qt::EditRole) {
         //This might be super slow, but that's the price of using a map/hash table.
         //Also note that QMaps are always sorted by key, whereas QHashes are not sorted and rearrange themselves.
         QList<MidiCommand> keys = m_pMapping->keys();
         MidiCommand command = keys.at(index.row());
         MidiControl control = (*m_pMapping)[command]; //Get the control from the map

         switch (index.column())
         {
             case MIDIINPUTTABLEINDEX_MIDITYPE:
                 return command.getMidiType();
                 break;

             case MIDIINPUTTABLEINDEX_MIDINO:
                 return command.getMidiNo();
                 break;

             case MIDIINPUTTABLEINDEX_MIDICHANNEL:
                 return command.getMidiChannel();
                 break;

             case MIDIINPUTTABLEINDEX_CONTROLOBJECTGROUP:
                 return control.getControlObjectGroup();
                 break;

             case MIDIINPUTTABLEINDEX_CONTROLOBJECTVALUE:
                 return control.getControlObjectValue();
                 break;

             case MIDIINPUTTABLEINDEX_MIDIOPTION:
                 return control.getMidiOption();
                 break;

             default:
                 return QVariant();
         }
     }
     else
         return QVariant();
}

Qt::ItemFlags MidiInputMappingTableModel::flags(const QModelIndex &index) const
{
     if (!index.isValid())
         return Qt::ItemIsEnabled;

     return QAbstractItemModel::flags(index) | Qt::ItemIsEditable;
}

bool MidiInputMappingTableModel::setData(const QModelIndex &index, const QVariant &value,
                                         int role)
{
     if (index.isValid() && role == Qt::EditRole) {
         QList<MidiCommand> keys = m_pMapping->keys();
         MidiCommand command = keys.at(index.row());
         MidiControl control = m_pMapping->take(command); //Get the control from the map

         switch (index.column())
         {
             case MIDIINPUTTABLEINDEX_MIDITYPE:
                 command.setMidiType((MidiType)value.toInt());
                 break;

             case MIDIINPUTTABLEINDEX_MIDINO:
                 command.setMidiNo(value.toInt());
                 break;

             case MIDIINPUTTABLEINDEX_MIDICHANNEL:
                 command.setMidiChannel(value.toInt());
                 break;

             case MIDIINPUTTABLEINDEX_CONTROLOBJECTGROUP:
                 control.setControlObjectGroup(value.toString());
                 break;

             case MIDIINPUTTABLEINDEX_CONTROLOBJECTVALUE:
                 control.setControlObjectValue(value.toString());
                 break;

             case MIDIINPUTTABLEINDEX_MIDIOPTION:
                 control.setMidiOption((MidiOption)value.toInt());
                 break;
         };

        //Insert the updated control into the map.
        m_pMapping->insert(command, control);

        emit dataChanged(index, index);
        return true;
     }
     return false;
}

int MidiInputMappingTableModel::rowCount(const QModelIndex& parent) const
{
    if (parent != QModelIndex()) //Some weird thing for table-based models.
        return 0;
    return m_pMapping->size();
}

int MidiInputMappingTableModel::columnCount(const QModelIndex& parent) const
{
    if (parent != QModelIndex()) //Some weird thing for table-based models.
        return 0;
    return MIDIINPUTTABLEINDEX_NUMCOLS;
}

QVariant MidiInputMappingTableModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    //Column heading labels
    if (orientation == Qt::Horizontal)
    {
        switch (section)
        {
            case MIDIINPUTTABLEINDEX_MIDITYPE:
                return QVariant(tr("Midi Type"));
                break;

            case MIDIINPUTTABLEINDEX_MIDINO:
                return QVariant(tr("Midi Note"));
                break;

            case MIDIINPUTTABLEINDEX_MIDICHANNEL:
                return QVariant(tr("Midi Channel"));
                break;

            case MIDIINPUTTABLEINDEX_CONTROLOBJECTGROUP:
                return QVariant(tr("Control Group"));
                break;

            case MIDIINPUTTABLEINDEX_CONTROLOBJECTVALUE:
                return QVariant(tr("Control Value"));
                break;

            /*  //WTF, why does the header disappear when I enable this? - Albert (1 AM)
            case MIDIINPUTTABLEINDEX_MIDIOPTION:
                return QVariant(tr("Midi Option"));
                break;
                */
        }
    }

    return QVariant();
}

void MidiInputMappingTableModel::removeRow(int row, const QModelIndex& parent)
{
    beginRemoveRows(parent, row, row);

     //This might be super slow, but that's the price of using a map/hash table.
     //Also note that QMaps are always sorted by key, whereas QHashes are not sorted and rearrange themselves.
     QList<MidiCommand> keys = m_pMapping->keys();
     MidiCommand command = keys.at(row);
     m_pMapping->take(command); //Remove the control from the map

    endRemoveRows();
}

bool MidiInputMappingTableModel::removeRows(int row, int count, const QModelIndex& parent)
{
    beginRemoveRows(parent, row, row+count);

    //This might be super slow, but that's the price of using a map/hash table.
    //Also note that QMaps are always sorted by key, whereas QHashes are not sorted and rearrange themselves.
    QList<MidiCommand> keys = m_pMapping->keys();
    for (int i = row; i < row+count; i++) {
        MidiCommand command = keys.at(i);
        m_pMapping->take(command); //Remove the control from the map
    }

    //TODO: Should probably handle an invalid selection and return false.

    endRemoveRows();

    return true;
}
