
#ifndef _MIDIINPUTMAPPINGSTABLEVIEW_H_
#define _MIDIINPUTMAPPINGSTABLEVIEW_H_

#include <QtCore>
#include <QtGui>

class MidiInputMappingTableView : public QTableView
{
	Q_OBJECT
	
    public:
        MidiInputMappingTableView(QWidget* parent=NULL);
        ~MidiInputMappingTableView();
};

#endif