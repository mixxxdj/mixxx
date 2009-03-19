/***************************************************************************
                          dlgprefmidibindings.cpp  -  description
                             -------------------
    begin                : Sat Jun 21 2008
    copyright            : (C) 2008 by Tom Care
    email                : psyc0de@gmail.com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include <QtGui>
#include <QDebug>
#include "midiinputmappingtablemodel.h"
#include "midioutputmappingtablemodel.h"
#include "midichanneldelegate.h"
#include "miditypedelegate.h"
#include "midinodelegate.h"
#include "controlgroupdelegate.h"
#include "controlvaluedelegate.h"
#include "dlgprefmidibindings.h"
#include "widget/wwidget.h"
#include "configobject.h"
#include "midimapping.h"

#ifdef __MIDISCRIPT__
#include "script/midiscriptengine.h"
#endif

const QStringList options = (QStringList() << "Normal" << "Script-Binding" << "Invert" << "Rot64" << "Rot64Inv"
        << "Rot64Fast" << "Diff" << "Button" << "Switch" << "HercJog"
        << "Spread64" << "SelectKnob");

QStringList controKeyOptionChoices;

const QStringList outputTypeChoices = (QStringList() << "light");

DlgPrefMidiBindings::DlgPrefMidiBindings(QWidget *parent, MidiObject &midi, QString deviceName,
										 ConfigObject<ConfigValue> *pConfig) :
							QWidget(parent), Ui::DlgPrefMidiBindingsDlg(), m_rMidi(midi) {
    setupUi(this);
    m_pConfig = pConfig;
    m_deviceName = deviceName;

    m_pDlgMidiLearning = NULL;

    labelDeviceName->setText(m_deviceName);

    //Tell the input mapping table widget which data model it should be viewing
    //(note that m_pInputMappingTableView is defined in the .ui file!)
    m_pInputMappingTableView->setModel((QAbstractItemModel*)m_rMidi.getMidiMapping()->getMidiInputMappingTableModel());

    m_pInputMappingTableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_pInputMappingTableView->setSelectionMode(QAbstractItemView::ContiguousSelection); //The model won't like ExtendedSelection, probably.
    m_pInputMappingTableView->verticalHeader()->hide();

    //Set up "delete" as a shortcut key to remove a row for the MIDI input table.
    m_deleteMIDIInputRowAction = new QAction(m_pInputMappingTableView);
    /*m_deleteMIDIInputRowAction->setShortcut(QKeySequence::Delete);
    m_deleteMIDIInputRowAction->setShortcutContext(Qt::WidgetWithChildrenShortcut);
    connect(m_deleteMIDIInputRowAction, SIGNAL(triggered()), this, SLOT(slotRemoveInputBinding()));
    */
    //The above shortcut doesn't work yet, not quite sure why. -- Albert Feb 1 / 2009

    //Set up the cool item delegates for the input mapping table
    m_pMidiChannelDelegate = new MidiChannelDelegate();
    m_pMidiTypeDelegate = new MidiTypeDelegate();
    m_pMidiNoDelegate = new MidiNoDelegate();
    m_pControlGroupDelegate = new ControlGroupDelegate();
    m_pControlValueDelegate = new ControlValueDelegate();
    m_pInputMappingTableView->setItemDelegateForColumn(MIDIINPUTTABLEINDEX_MIDITYPE, m_pMidiTypeDelegate);
    m_pInputMappingTableView->setItemDelegateForColumn(MIDIINPUTTABLEINDEX_MIDICHANNEL, m_pMidiChannelDelegate);
    m_pInputMappingTableView->setItemDelegateForColumn(MIDIINPUTTABLEINDEX_MIDINO, m_pMidiNoDelegate);
    m_pInputMappingTableView->setItemDelegateForColumn(MIDIINPUTTABLEINDEX_CONTROLOBJECTGROUP, m_pControlGroupDelegate);
    m_pInputMappingTableView->setItemDelegateForColumn(MIDIINPUTTABLEINDEX_CONTROLOBJECTVALUE, m_pControlValueDelegate);

    //Tell the output mapping table widget which data model it should be viewing 
    //(note that m_pOutputMappingTableView is defined in the .ui file!)
    m_pOutputMappingTableView->setModel((QAbstractItemModel*)m_rMidi.getMidiMapping()->getMidiOutputMappingTableModel());
    m_pOutputMappingTableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_pOutputMappingTableView->setSelectionMode(QAbstractItemView::ContiguousSelection);
    m_pOutputMappingTableView->verticalHeader()->hide();

    //Set up the cool item delegates for the output mapping table
    m_pOutputMappingTableView->setItemDelegateForColumn(MIDIOUTPUTTABLEINDEX_MIDITYPE, m_pMidiTypeDelegate);
    m_pOutputMappingTableView->setItemDelegateForColumn(MIDIOUTPUTTABLEINDEX_MIDICHANNEL, m_pMidiChannelDelegate);
    m_pOutputMappingTableView->setItemDelegateForColumn(MIDIOUTPUTTABLEINDEX_MIDINO, m_pMidiNoDelegate);

    // Connect buttons to slots
    connect(btnExportXML, SIGNAL(clicked()), this, SLOT(slotExportXML()));

    //Input bindings
    connect(btnMidiLearnWizard, SIGNAL(clicked()), this, SLOT(slotShowMidiLearnDialog()));
    connect(btnClearAllInputBindings, SIGNAL(clicked()), this, SLOT(slotClearAllInputBindings()));
    connect(btnRemoveInputBinding, SIGNAL(clicked()), this, SLOT(slotRemoveInputBinding()));
    connect(btnAddInputBinding, SIGNAL(clicked()), this, SLOT(slotAddInputBinding()));

    //Output bindings
    connect(btnClearAllOutputBindings, SIGNAL(clicked()), this, SLOT(slotClearAllOutputBindings()));
    connect(btnRemoveOutputBinding, SIGNAL(clicked()), this, SLOT(slotRemoveOutputBinding()));
    connect(btnAddOutputBinding, SIGNAL(clicked()), this, SLOT(slotAddOutputBinding()));

    //Connect the activate button. One day this will be replaced with an "Enabled" checkbox.
    connect(btnActivateDevice, SIGNAL(clicked()), this, SLOT(slotEnableDevice()));
    
    connect(comboBoxPreset, SIGNAL(activated(const QString&)), this, SLOT(slotLoadMidiMapping(const QString&)));
    
    //Load the list of presets into the presets combobox.
    enumeratePresets();
}

DlgPrefMidiBindings::~DlgPrefMidiBindings() {
    //delete m_pMidiConfig;
    delete m_pMidiChannelDelegate;
    delete m_pMidiNoDelegate;
    delete m_pMidiTypeDelegate;

    delete m_deleteMIDIInputRowAction;
}

void DlgPrefMidiBindings::enumeratePresets()
{
    comboBoxPreset->clear();
    
    //Insert a dummy "..." item at the top to try to make it less confusing.
    //(For example, we don't want "Akai MPD24" showing up as the default item
    // when a user has their controller plugged in)
    comboBoxPreset->addItem("...");
    
    QString midiDirPath = m_pConfig->getConfigPath().append("midi/");
    QDirIterator it(midiDirPath, QDirIterator::Subdirectories);
    while (it.hasNext())
    {
        it.next(); //Advance iterator. We get the filename from the next line. (It's a bit weird.)
        QString curMapping = it.fileName();
        if (curMapping.endsWith(MIDI_MAPPING_EXTENSION)) //blah, thanks for nothing Qt
        {
            curMapping.chop(QString(MIDI_MAPPING_EXTENSION).length()); //chop off the .midi.xml
            comboBoxPreset->addItem(curMapping);
        }
    }
}

/* loadPreset(QString)
 * Asks MidiMapping to load a set of MIDI bindings from an XML file
 */
void DlgPrefMidiBindings::loadPreset(QString path) {
    m_rMidi.getMidiMapping()->loadPreset(path);
}


/* slotUpdate()
 * Called when the dialog is displayed.
 */
void DlgPrefMidiBindings::slotUpdate() {

    //Check if the device that this dialog is for is already enabled...
    if (m_rMidi.getOpenDevice() == m_deviceName)
    {
        btnActivateDevice->setEnabled(false); //Disable activate button
        toolBox->setEnabled(true); //Enable MIDI in/out toolbox.
        groupBoxPresets->setEnabled(true); //Enable presets group box.
    }
    else {
        btnActivateDevice->setEnabled(true); //Enable activate button
        toolBox->setEnabled(false); //Disable MIDI in/out toolbox.
        groupBoxPresets->setEnabled(false); //Disable presets group box.
    }
}

/* slotApply()
 * Called when the OK button is pressed.
 */
void DlgPrefMidiBindings::slotApply() {
    /* User has pressed OK, so write the controls to the DOM, reload the MIDI
     * bindings, and save the default XML file. */
    m_rMidi.getMidiMapping()->savePreset();   // use default bindings path
    m_rMidi.getMidiMapping()->applyPreset();
    m_rMidi.disableMidiLearn();
}

void DlgPrefMidiBindings::slotShowMidiLearnDialog() {
    //Note that DlgMidiLearning is set to delete itself on
    //close using the Qt::WA_DeleteOnClose attribute (so this "new" doesn't leak memory)
    m_pDlgMidiLearning = new DlgMidiLearning(this, m_rMidi.getMidiMapping());
    m_pDlgMidiLearning->show();
}

/* slotImportXML()
 * Prompts the user for an XML preset and loads it.
 */
void DlgPrefMidiBindings::slotLoadMidiMapping(const QString &name) {
    
    /*QString fileName = QFileDialog::getOpenFileName(this,
            "Import Mixxx MIDI Bindings", m_pConfig->getConfigPath().append("midi/"),
            "Preset Files (*.xml)");*/
    
    //TODO: Ask for confirmation if the MIDI tables aren't empty...
    MidiMapping* mapping = m_rMidi.getMidiMapping();
    if (mapping->numInputMidiMessages() > 0 ||
        mapping->numOutputMixxxControls() > 0)
    {
         QMessageBox::StandardButton result = QMessageBox::question(this, "Overwrite existing mapping?", tr("Are you sure you'd like to load the " + name + " mapping?\n"
                                                                      "This will overwrite your existing MIDI mapping."),  QMessageBox::Yes | QMessageBox::No);
                              
         if (result == QMessageBox::No) {
            //Select the "..." item again in the combobox.
            comboBoxPreset->setCurrentIndex(0);
            return;                     
         }
    }
    
    QString filename = m_pConfig->getConfigPath().append("midi/") + name + MIDI_MAPPING_EXTENSION;
    if (!filename.isNull()) {
        loadPreset(filename);
        m_rMidi.getMidiMapping()->applyPreset();
    }
    m_pInputMappingTableView->update();
    
    //Select the "..." item again in the combobox.
    comboBoxPreset->setCurrentIndex(0);
}

/* slotExportXML()
 * Prompts the user for an XML preset and saves it.
 */
void DlgPrefMidiBindings::slotExportXML() {
    QString fileName = QFileDialog::getSaveFileName(this,
            "Export Mixxx MIDI Bindings", m_pConfig->getConfigPath().append("midi/"),
            "Preset Files (*.midi.xml)");
    if (!fileName.isNull()) m_rMidi.getMidiMapping()->savePreset(fileName);
}

void DlgPrefMidiBindings::slotEnableDevice()
{
	//Just tell MidiObject to close the old device and open this device
	m_rMidi.devClose();
	m_rMidi.devOpen(m_deviceName);
	m_pConfig->set(ConfigKey("[Midi]","Device"), m_deviceName);
	btnActivateDevice->setEnabled(false);
	toolBox->setEnabled(true); //Enable MIDI in/out toolbox.
	groupBoxPresets->setEnabled(true); //Enable presets group box.
	
	//TODO: Should probably check if devOpen() actually succeeded.
}

void DlgPrefMidiBindings::slotAddInputBinding() {
    // TODO: This function is totally broken.

    bool ok = true;

    QStringList selectionList;

    QString device = QInputDialog::getItem(this, tr("1/6 Select Device"), tr("Select a device to control"), selectionList, 0, true, &ok);
    if (!ok) return;

    QString controlKey = QInputDialog::getItem(this, tr("2/6 Select Control Group + Key"), tr("Select Control Group + Key"), getControlKeyList(), 0, false,  &ok);
    if (!ok) return;
    QString group = controlKey.trimmed().split(" ").at(0).trimmed();
    QString key = controlKey.trimmed().split(" ").at(1).trimmed();

    selectionList = QStringList();
    selectionList << "Ctrl" << "Key" << "Pitch";
    QString miditype = QInputDialog::getItem(this, tr("3/6 Select MIDI Type"), tr("Select MIDI Type"), selectionList, 0, false,  &ok);
    if (!ok) return;

    QString midino = QString::number(QInputDialog::getInteger(this, tr("4/6 Select Midi No."), tr("Type in the MIDI number value"), 1, 0, 127, 1, &ok));
    if (!ok) return;

    QString midichan = QString::number(QInputDialog::getInteger(this, tr("5/6 Select Midi Channel"), tr("Type in the MIDI channel value (ch)"), 1, 0, 127, 1, &ok));
    if (!ok) return;

    //    selectionList = QStringList(getControlTypeList());
    //    if (selectionList.size() == 0 && tblBindings->rowCount() > 0) { selectionList.append(tblBindings->item(tblBindings->rowCount() - 1,3)->text()); }
    //    QString controltype = QInputDialog::getItem(this, tr("7/8 Select Control"), tr("Select a function of Mixxx to control"), selectionList, 0, true, &ok);
    //        if (!ok) return;
    QString controltype = "";

    selectionList = QStringList(options);
    QString option = QInputDialog::getItem(this, tr("6/6 Select Option"), tr("Select an optional behaviour modifier"), selectionList, 0, true, &ok);
    if (!ok) return;

    // At this stage we have enough information to create a blank, learnable binding
    m_rMidi.getMidiMapping()->addInputControl((MidiType)miditype.toInt(), midino.toInt(), midichan.toInt(),
                                              group, key, (MidiOption)option.toInt());
                        //FUCK! The "option" thing above will be garbage when converted to an int, since it's
                        //        some string describing the midi option in words, not a string number like "2".
                        //Solution: Use a delegate class for the MIDI Option column

    //tblBindings->selectRow(tblBindings->rowCount() - 1); // Focus the row just added
}

void DlgPrefMidiBindings::slotRemoveInputBinding()
{
	QModelIndexList selectedIndices = m_pInputMappingTableView->selectionModel()->selectedRows();
	if (selectedIndices.size() > 0)
	{
		MidiInputMappingTableModel* tableModel = dynamic_cast<MidiInputMappingTableModel*>(m_pInputMappingTableView->model());
		if (tableModel) {
			QModelIndex curIndex;
			//The model indices are sorted so that we remove the rows from the table
            //in ascending order. This is necessary because if row A is above row B in
            //the table, and you remove row A, the model index for row B will change.
            //Sorting the indices first means we don't have to worry about this.
            //qSort(selectedIndices);

            //Going through the model indices in descending order (see above comment for explanation).
			QListIterator<QModelIndex> it(selectedIndices);
			it.toBack();
			while (it.hasPrevious())
			{
				curIndex = it.previous();
				tableModel->removeRow(curIndex.row());
			}
		}
	}
}

void DlgPrefMidiBindings::slotClearAllInputBindings() {
    if (QMessageBox::warning(this, "Clear Input Bindings",
            "Are you sure you want to clear all bindings?",
            QMessageBox::Ok | QMessageBox::Cancel, QMessageBox::Cancel) != QMessageBox::Ok)
        return;

    //Remove all the rows from the data model (ie. the MIDI mapping).
    MidiInputMappingTableModel* tableModel = dynamic_cast<MidiInputMappingTableModel*>(m_pInputMappingTableView->model());
    if (tableModel) {
        tableModel->removeRows(0, tableModel->rowCount());
    }
}


void DlgPrefMidiBindings::slotAddOutputBinding() {
    qDebug() << "STUB: DlgPrefMidiBindings::slotAddOutputBinding()";

    m_rMidi.getMidiMapping()->setOutputMidiMapping(MixxxControl(), MidiMessage());
}

void DlgPrefMidiBindings::slotRemoveOutputBinding()
{
	QModelIndexList selectedIndices = m_pOutputMappingTableView->selectionModel()->selectedRows();
	if (selectedIndices.size() > 0)
	{
		MidiOutputMappingTableModel* tableModel =
		                    dynamic_cast<MidiOutputMappingTableModel*>(m_pOutputMappingTableView->model());
		if (tableModel) {
			QModelIndex curIndex;
			//The model indices are sorted so that we remove the rows from the table
            //in ascending order. This is necessary because if row A is above row B in
            //the table, and you remove row A, the model index for row B will change.
            //Sorting the indices first means we don't have to worry about this.
            //qSort(selectedIndices);

            //Going through the model indices in descending order (see above comment for explanation).
			QListIterator<QModelIndex> it(selectedIndices);
			it.toBack();
			while (it.hasPrevious())
			{
				curIndex = it.previous();
				qDebug() << "Dlg: removing row" << curIndex.row();
				tableModel->removeRow(curIndex.row());
			}
		}
	}
}

void DlgPrefMidiBindings::slotClearAllOutputBindings() {
    if (QMessageBox::warning(this, "Clear Output Bindings",
            "Are you sure you want to clear all output bindings?",
            QMessageBox::Ok | QMessageBox::Cancel, QMessageBox::Cancel) != QMessageBox::Ok)
        return;

    //Remove all the rows from the data model (ie. the MIDI mapping).
    MidiOutputMappingTableModel* tableModel = dynamic_cast<MidiOutputMappingTableModel*>(m_pOutputMappingTableView->model());
    if (tableModel) {
        tableModel->removeRows(0, tableModel->rowCount());
    }
}

/* getControlKeyList()
 * Gets the list of control objects
 */
QStringList DlgPrefMidiBindings::getControlKeyList() {
    // midi/BindableConfigKeys.txt = grep "ConfigKey" *.cpp | grep \\[ | sed -e 's/.*ConfigKey("//g' | cut -d \) -f1 | sed -e 's/[",]/ /g' -e 's/ \+/ /g' | egrep -ve '[+:>]' | sort -u | egrep -e "\[Channel[12]\]|\[Master\]|\[Playlist\]" > midi/BindableConfigKeys.txt
    if (controKeyOptionChoices.count() == 0) {
        QFile input(m_pConfig->getConfigPath() + "midi/BindableConfigKeys.txt");

        if (!input.open(QIODevice::ReadOnly)) return QStringList();

        while (!input.atEnd()) {
            QString line = input.readLine().trimmed();
            if (line.isEmpty() || line.indexOf('#') == 0) continue; // ignore # hashed out comments
            if (!line.isNull()) controKeyOptionChoices.append(line);
        }
        input.close();
    }
    return controKeyOptionChoices;
}
