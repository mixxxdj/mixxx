
#include <QtCore>
#include <QLabel>
#include "midimapping.h"
#include "midiinputmappingtablemodel.h"

MidiInputMappingTableModel::MidiInputMappingTableModel(MidiMapping* mapping) :
    QAbstractTableModel(),
    m_pMapping(NULL)

{
    setMapping(mapping); //Save the mapping
}

MidiInputMappingTableModel::~MidiInputMappingTableModel()
{
}

void MidiInputMappingTableModel::slotInputMappingChanged() {

    // for now ask it to refresh the whole thing -- we can get fancy
    // with more complex signals later
    reset();
}

void MidiInputMappingTableModel::setMapping(MidiMapping* mapping)
{
    if(m_pMapping != NULL) {
        disconnect(m_pMapping, SIGNAL(inputMappingChanged()), this, SLOT(slotInputMappingChanged()));
    }
    m_pMapping = mapping;
    connect(m_pMapping, SIGNAL(inputMappingChanged()), this, SLOT(slotInputMappingChanged()));
}

QVariant MidiInputMappingTableModel::data(const QModelIndex &index, int role) const
 {     
     if (!index.isValid())
         return QVariant();

     if(!m_pMapping->isInputIndexValid(index.row()))
         return QVariant();

     if (role == Qt::DisplayRole || role == Qt::EditRole) {
         //This might be super slow, but that's the price of using a map/hash table (which give
         //us super fast lookups when MidiMessages come in).
         //Also note that QMaps are always sorted by key, whereas QHashes are not sorted and rearrange themselves.

         MidiMessage message = m_pMapping->getInputMidiMessage(index.row());
         MixxxControl control = m_pMapping->getInputMixxxControl(message);

         switch (index.column())
         {
             case MIDIINPUTTABLEINDEX_MIDISTATUS:
                 return message.getMidiStatusByte();
                 break;

             case MIDIINPUTTABLEINDEX_MIDINO:
                 return message.getMidiNo();
                 break;

             case MIDIINPUTTABLEINDEX_MIDICHANNEL:
                 return message.getMidiChannel();
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

            case MIDIINPUTTABLEINDEX_CONTROLOBJECTDESCRIPTION:
                return control.getControlObjectDescription();
                break;

             default:
                 return QVariant();
         }
     }

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
        MidiMessage message = m_pMapping->getInputMidiMessage(index.row());
        MixxxControl control = m_pMapping->getInputMixxxControl(message);
        
        //Now we actually need to remove the mapping we want to operate on,
        //because otherwise if we change the status or channel bits, we'll
        //end up inserting a new mapping instead of overwriting this one.
        //(This is because the mapping datastructure is a hash table that
        // hashes on the status byte.)
        m_pMapping->clearInputMidiMapping(message);
        
        switch (index.column())
            {
                case MIDIINPUTTABLEINDEX_MIDISTATUS:
                    message.setMidiStatusByte((MidiStatusByte)value.toInt());
                    break;

                case MIDIINPUTTABLEINDEX_MIDINO:
                    message.setMidiNo(value.toInt());
                    break;

                case MIDIINPUTTABLEINDEX_MIDICHANNEL:
                    message.setMidiChannel(value.toInt());
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

                case MIDIINPUTTABLEINDEX_CONTROLOBJECTDESCRIPTION:
                    control.setControlObjectDescription(value.toString());
                    break;
            };

        //Insert the updated control into the map.
        m_pMapping->setInputMidiMapping(message, control);

        emit dataChanged(index, index);
        return true;
    }
    return false;
}

int MidiInputMappingTableModel::rowCount(const QModelIndex& parent) const
{
    if (parent != QModelIndex()) //Some weird thing for table-based models.
        return 0;
    
    return m_pMapping->numInputMidiMessages();
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
            case MIDIINPUTTABLEINDEX_MIDISTATUS:
                return QVariant(tr("Midi Status Type"));
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

            case MIDIINPUTTABLEINDEX_CONTROLOBJECTDESCRIPTION:
                return QVariant(tr("Description"));
                break;
        }
    }

    return QVariant();
}

bool MidiInputMappingTableModel::removeRow(int row, const QModelIndex& parent)
{
    //DO NOT CALL beginRemoveRows()/endRemoveRows() in this function! You're ONLY supposed to
    //do that for removeRows() (plural!!). It causes a bug if you do it here.
    
    m_pMapping->clearInputMidiMapping(row);

    return true;
}

bool MidiInputMappingTableModel::removeRows(int row, int count, const QModelIndex& parent)
{
    beginRemoveRows(parent, row, row+count);

    m_pMapping->clearInputMidiMapping(row, count);

    //TODO: Should probably handle an invalid selection and return false.

    endRemoveRows();

    return true;
}
