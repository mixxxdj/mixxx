        
#include <QtCore>
#include <QLabel>
#include "midimapping.h"
#include "midioutputmappingtablemodel.h"

MidiOutputMappingTableModel::MidiOutputMappingTableModel(MidiMapping* mapping) :
    QAbstractTableModel(),
    m_pMapping(NULL)
{
    setMapping(mapping); //Save the mapping
}

MidiOutputMappingTableModel::~MidiOutputMappingTableModel()
{
}

void MidiOutputMappingTableModel::slotOutputMappingChanged() {

    // for now ask it to refresh the whole thing -- we can get fancy
    // with more complex signals later
    reset();
}

void MidiOutputMappingTableModel::setMapping(MidiMapping* mapping)
{
    if(m_pMapping != NULL) {
        disconnect(m_pMapping, SIGNAL(outputMappingChanged()), this, SLOT(slotOutputMappingChanged()));
    }
    m_pMapping = mapping;
    connect(m_pMapping, SIGNAL(outputMappingChanged()), this, SLOT(slotOutputMappingChanged()));
}

QVariant MidiOutputMappingTableModel::data(const QModelIndex &index, int role) const
 {
     if (!index.isValid())
         return QVariant();

     if(!m_pMapping->isOutputIndexValid(index.row()))
         return QVariant();

     if (role == Qt::DisplayRole || role == Qt::EditRole) {
         //This might be super slow, but that's the price of using a map/hash table.
         //Also note that QMaps are always sorted by key, whereas QHashes are not sorted and rearrange themselves.
         
         MixxxControl control = m_pMapping->getOutputMixxxControl(index.row());
         MidiMessage command = m_pMapping->getOutputMidiMessage(control);
         
         switch (index.column())
         {
             case MIDIOUTPUTTABLEINDEX_MIDISTATUS:
                 return command.getMidiStatusByte();
                 break;

             case MIDIOUTPUTTABLEINDEX_MIDINO:
                 return command.getMidiNo();
                 break;

             case MIDIOUTPUTTABLEINDEX_MIDICHANNEL:
                 return command.getMidiChannel();
                 break;

             case MIDIOUTPUTTABLEINDEX_CONTROLOBJECTGROUP:
                 return control.getControlObjectGroup();
                 break;

             case MIDIOUTPUTTABLEINDEX_CONTROLOBJECTVALUE:
                 return control.getControlObjectValue();
                 break;

             case MIDIOUTPUTTABLEINDEX_CONTROLOBJECTDESCRIPTION:
                 return control.getControlObjectDescription();
                 break;

             case MIDIOUTPUTTABLEINDEX_MIDIOPTION:
                 return control.getMidiOption();
                 break;
                 
             case MIDIOUTPUTTABLEINDEX_THRESHOLDMIN:
                 return control.getThresholdMinimum();
                 break; 

             case MIDIOUTPUTTABLEINDEX_THRESHOLDMAX:
                 return control.getThresholdMaximum();
                 break; 

             default:
                 return QVariant();
         }
     }
     
     return QVariant();
}

Qt::ItemFlags MidiOutputMappingTableModel::flags(const QModelIndex &index) const
{
     if (!index.isValid())
         return Qt::ItemIsEnabled;

     return QAbstractItemModel::flags(index) | Qt::ItemIsEditable;
}

bool MidiOutputMappingTableModel::setData(const QModelIndex &index, const QVariant &value,
                                         int role)
{
    if (index.isValid() && role == Qt::EditRole) {
        MixxxControl control = m_pMapping->getOutputMixxxControl(index.row());
        MidiMessage command = m_pMapping->getOutputMidiMessage(control);
        
        //Now we actually need to remove the mapping we want to operate on,
        //because otherwise if we change the status or channel bits, we'll
        //end up inserting a new mapping instead of overwriting this one.
        //(This is because the mapping datastructure is a hash table that
        // hashes on the status byte.)
        m_pMapping->clearOutputMidiMapping(control);
                  
        switch (index.column())
            {
                case MIDIOUTPUTTABLEINDEX_MIDISTATUS:
                    command.setMidiStatusByte((MidiStatusByte)value.toInt());
                    break;
                    
                case MIDIOUTPUTTABLEINDEX_MIDINO:
                    command.setMidiNo(value.toInt());
                    break;
                    
                case MIDIOUTPUTTABLEINDEX_MIDICHANNEL:
                    command.setMidiChannel(value.toInt());
                    break;
                    
                case MIDIOUTPUTTABLEINDEX_CONTROLOBJECTGROUP:
                    control.setControlObjectGroup(value.toString());
                    break;
                    
                case MIDIOUTPUTTABLEINDEX_CONTROLOBJECTVALUE:
                    control.setControlObjectValue(value.toString());
                    break;

                case MIDIOUTPUTTABLEINDEX_CONTROLOBJECTDESCRIPTION:
                    control.setControlObjectDescription(value.toString());
                    break;
                    
                case MIDIOUTPUTTABLEINDEX_MIDIOPTION:
                    control.setMidiOption((MidiOption)value.toInt());
                    break;
                    
                case MIDIOUTPUTTABLEINDEX_THRESHOLDMIN:
                    control.setThresholdMinimum((float)value.toDouble());
                    break;
                    
                case MIDIOUTPUTTABLEINDEX_THRESHOLDMAX:
                    control.setThresholdMaximum((float)value.toDouble());
                    break;                    
            };
        
        //Insert the updated control into the map.
        m_pMapping->setOutputMidiMapping(control, command);
        
        emit dataChanged(index, index);
        return true;
    }
    return false;
}

int MidiOutputMappingTableModel::rowCount(const QModelIndex& parent) const
{
    if (parent != QModelIndex()) //Some weird thing for table-based models.
        return 0;
    return m_pMapping->numOutputMixxxControls();
}

int MidiOutputMappingTableModel::columnCount(const QModelIndex& parent) const
{
    if (parent != QModelIndex()) //Some weird thing for table-based models.
        return 0;
    return MIDIOUTPUTTABLEINDEX_NUMCOLS;
}

QVariant MidiOutputMappingTableModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    //Column heading labels
    if (orientation == Qt::Horizontal)
    {
        switch (section)
        {
            case MIDIOUTPUTTABLEINDEX_MIDISTATUS:
                return QVariant(tr("Midi Status"));
                break;

            case MIDIOUTPUTTABLEINDEX_MIDINO:
                return QVariant(tr("Midi Note"));
                break;

            case MIDIOUTPUTTABLEINDEX_MIDICHANNEL:
                return QVariant(tr("Midi Channel"));
                break;

            case MIDIOUTPUTTABLEINDEX_CONTROLOBJECTGROUP:
                return QVariant(tr("Control Group"));
                break;

            case MIDIOUTPUTTABLEINDEX_CONTROLOBJECTVALUE:
                return QVariant(tr("Control Value"));
                break;
                
            case MIDIOUTPUTTABLEINDEX_CONTROLOBJECTDESCRIPTION:
                return QVariant(tr("Description"));
                break;
                
            case MIDIOUTPUTTABLEINDEX_THRESHOLDMIN:
                return QVariant(tr("Threshold Min"));
                break;
                    
            case MIDIOUTPUTTABLEINDEX_THRESHOLDMAX:
                return QVariant(tr("Threshold Max"));
                break; 
                
            /*  //WTF, why does the header disappear when I enable this? - Albert (1 AM)
            case MIDIOUTPUTTABLEINDEX_MIDIOPTION:
                return QVariant(tr("Midi Option"));
                break;
                */
        }
    }

    return QVariant();
}

bool MidiOutputMappingTableModel::removeRow(int row, const QModelIndex& parent)
{
    m_pMapping->clearOutputMidiMapping(row);
    
    return true;
}

bool MidiOutputMappingTableModel::removeRows(int row, int count, const QModelIndex& parent)
{
    beginRemoveRows(parent, row, row+count);
    qDebug() << "MidiOutputMappingTableModel::removeRows()";

    m_pMapping->clearOutputMidiMapping(row, count);

    //TODO: Should probably handle an invalid selection and return false.

    endRemoveRows();

    return true;
}

/*
bool MidiOutputMappingTableModel::insertRow ( int row, const QModelIndex & parent = QModelIndex() )
{
    return insertRows(row, 1, parent);
}

bool MidiOutputMappingTableModel::insertRows ( int row, int count, const QModelIndex & parent = QModelIndex() )
{
    emit beginInsertRows(parent, row, row+count-1);
    m_pMapping->setMapping(
    emit endInsertRows();
    
    return true;
}*/
