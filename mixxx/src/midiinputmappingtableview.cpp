
#include <QtCore>
#include <QtGui>
#include "midiinputmappingtablemodel.h"
#include "midiinputmappingtableview.h"

MidiInputMappingTableView::MidiInputMappingTableView(QWidget* parent) : QTableView(parent)
{
    //Setup properties for table
    setSelectionBehavior(SelectRows);
    setSelectionMode(QAbstractItemView::ExtendedSelection);
    verticalHeader()->hide();
}

MidiInputMappingTableView::~MidiInputMappingTableView()
{
    
}
