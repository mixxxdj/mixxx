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
#include "dlgprefmidibindings.h"
#include "wwidget.h"
#include "configobject.h"

#define BINDINGS_PATH QDir::homePath().append("/").append(".MixxxMIDIBindings.xml")
const QStringList options = (QStringList() << "Normal" << "Invert" << "Rot64" << "Rot64Inv"
		<< "Rot64Fast" << "Diff" << "Button" << "Switch" << "HercJog"
		<< "Spread64" << "SelectKnob");

const QStringList outputChoices = (QStringList() << "light");


static QString toHex(QString numberStr) {
  return "0x" + QString("0" + QString::number(numberStr.toUShort(), 16).toUpper()).right(2);
}

DlgPrefMidiBindings::DlgPrefMidiBindings(QWidget *parent, MidiObject *midi, ConfigObject<ConfigValue> *pConfig) :  QWidget(parent), Ui::DlgPrefMidiBindingsDlg() {
    setupUi(this);
    m_pConfig = pConfig;
    singleLearning = false;
    groupLearning = false;
    m_pMidi = midi;

    // Connect buttons to slots
    connect(btnSingleLearn, SIGNAL(clicked()), this, SLOT(slotSingleLearnToggle()));
    connect(btnGroupLearn, SIGNAL(clicked()), this, SLOT(slotGroupLearnToggle()));
    connect(btnImportXML, SIGNAL(clicked()), this, SLOT(slotImportXML()));
    connect(btnExportXML, SIGNAL(clicked()), this, SLOT(slotExportXML()));

    connect(btnClearBindings, SIGNAL(clicked()), this, SLOT(slotClear()));
    connect(btnRemoveBinding, SIGNAL(clicked()), this, SLOT(slotRemoveBinding()));
    connect(btnAddBinding, SIGNAL(clicked()), this, SLOT(slotAddBinding()));

    // Output tab
    connect(btnOutputImportXML, SIGNAL(clicked()), this, SLOT(slotImportXML()));
    connect(btnOutputExportXML, SIGNAL(clicked()), this, SLOT(slotExportXML()));


    // Try to read in the current XML bindings file, or create one if nothing is available
    loadPreset(BINDINGS_PATH);
    applyPreset();
    m_pMidi->disableMidiLearn();
}

DlgPrefMidiBindings::~DlgPrefMidiBindings() {
	//delete m_pMidiConfig;
}

/* loadPreset(QString)
 * Overloaded function for convenience
 */
void DlgPrefMidiBindings::loadPreset(QString path) {
	loadPreset(WWidget::openXMLFile(path, "controller"));	
}

/* loadPreset(QDomElement)
 * Loads a set of MIDI bindings from a QDomElement structure.
 */
void DlgPrefMidiBindings::loadPreset(QDomElement root) {
	clearTable();
	if (root.isNull()) return;
	// For each controller in the DOM
	m_pBindings = root;
	QDomElement controller = m_pBindings.firstChildElement("controller");
	while (!controller.isNull()) {
		// For each controller
		// Get deviceid
		QString deviceid = controller.attribute("id","");
		qDebug() << deviceid << " settings found" << endl;
		QDomElement control = controller.firstChildElement("controls").firstChildElement("control");
		while (!control.isNull()) {
			// For each control
			QString group = WWidget::selectNodeQString(control, "group");
			QString key = WWidget::selectNodeQString(control, "key");
			QString controltype = WWidget::selectNodeQString(control, "controltype");
			QString miditype = WWidget::selectNodeQString(control, "miditype");
			// We convert to midino and midichan to base 10 because that his how they will be matched to midi keys internally.
			QString midino = QString::number(WWidget::selectNodeQString(control, "midino").toUShort(NULL, 0), 10);
			QString midichan = QString::number(WWidget::selectNodeQString(control, "midichan").toUShort(NULL, 0), 10);

			QDomElement optionsNode = control.firstChildElement("options");
			// At the moment, use one element, in future iterate through options
			QString option;
			if (optionsNode.hasChildNodes()) {
				 option = optionsNode.firstChild().nodeName();
			} else {
				option = "Normal";
			}
			// Construct row in table
			addRow(deviceid, group, key, controltype, miditype, midino, midichan, option);
			control = control.nextSiblingElement("control");
		}

		QDomNode output = controller.namedItem("outputs").toElement().firstChild();
		while (!output.isNull()) {
			QString outputType = output.nodeName();
	                QString group = WWidget::selectNodeQString(output, "group");
	                QString key = WWidget::selectNodeQString(output, "key");

	                QString status = QString::number(WWidget::selectNodeInt(output, "status"));
	                QString midino = QString::number(WWidget::selectNodeInt(output, "midino"));

	                QString on = "0x7f";	// Compatible with Hercules and others
	                QString off = "0x00";
	                QString min = "";
	                QString max = "";

		        if(outputType == "light") {
	                if (!output.firstChildElement("on").isNull()) {
	                    on = QString::number(WWidget::selectNodeInt(output, "on"));
	                }
	                if (!output.firstChildElement("off").isNull()) {
	                    off = QString::number(WWidget::selectNodeInt(output, "off"));
	                }
	                if (!output.firstChildElement("threshold").isNull()) {
	                    min = WWidget::selectNodeFloat(output, "threshold");
	                }
	                if (!output.firstChildElement("minimum").isNull()) {
	                    min = WWidget::selectNodeQString(output, "minimum");
	                }
	                if (!output.firstChildElement("maximum").isNull()) {
	                    max = WWidget::selectNodeQString(output, "maximum");
	                }
			}
                	qDebug() << "Loaded Output type:" << outputType << " -> " << group << key << "between"<< min << "and" << max << "to midi out:" << status << midino << "on" << deviceid << "on/off:" << on << off;

			addOutputRow(outputType, group, key, min, max, toHex(status), toHex(midino), deviceid, on, off);

			output = output.nextSibling();
		}

		controller = controller.nextSiblingElement("controller");
	}
}

/* savePreset(QString)
 * Given a path, saves the current table of bindings to an XML file.
 */
void DlgPrefMidiBindings::savePreset(QString path) {
	QFile output(path);
	if (!output.open(QIODevice::WriteOnly | QIODevice::Truncate))
	         return;
	QTextStream outputstream(&output);
	// Construct the DOM from the table
	buildDomElement();
	// Save the DOM to the XML file
	m_pBindings.save(outputstream, 4);
	output.close();
}

/* applyPreset()
 * Load the current bindings set into the MIDI handler, and the outputs info into
 * the LED handler.
 */
void DlgPrefMidiBindings::applyPreset() {
	MidiLedHandler::destroyHandlers();

        QDomElement controller = m_pBindings.firstChildElement("controller");
        // For each device
        while (!controller.isNull()) {
		// Device Outputs - LEDs
		QString deviceId = controller.attribute("id","");

		qDebug() << "Processing MIDI Control Bindings for" << deviceId;
                m_pMidiConfig = new ConfigObject<ConfigValueMidi>(controller.namedItem("controls"));

		qDebug() << "Processing MIDI Output Bindings for" << deviceId;
		MidiLedHandler::createHandlers(controller.namedItem("outputs").firstChild(), m_pMidi, deviceId);

                // Next device
                controller = controller.nextSiblingElement("controller");
	}
        m_pMidi->setMidiConfig(m_pMidiConfig);
}

/* clearPreset()
 * Creates a blank bindings preset.
 */
void DlgPrefMidiBindings::clearPreset() {
	// Create a new blank DomNode
	QString blank = "<MixxxMIDIPreset version=\"" + QString(VERSION) + "\">\n"
		"</MixxxMIDIPreset>\n";
	QDomDocument doc("Bindings");
	doc.setContent(blank);
	m_pBindings = doc.documentElement();
}

/* clearTable()
 * Clears the contents of the bindings table.
 */
void DlgPrefMidiBindings::clearTable() {
	tblBindings->clearContents();
	tblBindings->setRowCount(0);
}

/* slotUpdate()
 * Called when the dialog is loaded.
 */
void DlgPrefMidiBindings::slotUpdate() {

}

/* slotApply()
 * Called when the OK button is pressed.
 */
void DlgPrefMidiBindings::slotApply() {
	/* User has pressed OK, so write the controls to the DOM, reload the MIDI
	 * bindings, and save the default XML file. */
	savePreset(BINDINGS_PATH);
	applyPreset();
	m_pMidi->disableMidiLearn();
}

/*
 * singleLearn(ConfigValueMidi *, QString)
 * Sets the currently row(s) to the MIDI value and Device. Typically called by
 * the MIDI device handler. Prompts the user to confirm if more than one row is
 * selected.
 */
void DlgPrefMidiBindings::singleLearn(ConfigValueMidi *value, QString device) {
	if (!singleLearning || tblBindings->rowCount() == 0) return;
	singleLearning = false; // Stop message queue
	// Get list of currently selected rows
	QList<QTableWidgetItem *> cellList = tblBindings->selectedItems();
	// For each row
	// If more than one row selected, confirm with user
	int row = cellList.front()->row(), rowcount = 0;
	for (int i = 0; i < cellList.count(); i++) {
		//For each selected item, check for the same row number
		if (cellList[i]->row() != row) {
			// Multiple rows have been selected, confirm with the user
			if (QMessageBox::question(this, "Warning: Single MIDI Learn With Multiple Rows", "Multiple rows are selected. If you continue, the rows will all be set to the same MIDI binding.", QMessageBox::Ok | QMessageBox::Cancel, QMessageBox::Cancel) != QMessageBox::Ok) {
				return;
			} else {
				//OK to proceed, no need to check any more items
				break;
			}
		}
	}
	// For each row
	for (int i = 0; i < cellList.count(); i++) {
		row = cellList[i]->row();
		rowcount++;
		// Set MIDI info in row
		QString midiInfo;
		QTextStream out(&midiInfo);
		// Pitch bend should use '0' as the value
		int midino = (value->miditype == MIDI_PITCH) ? 0 : value->midino;
		out << value->getType() << " " << midino << " " << value->midichannel;
		// Skip to next row
		while (i < cellList.count() && cellList[i]->row() == row) i++;
		// Update row info
		tblBindings->setItem(row, 1, new QTableWidgetItem(midiInfo));
		tblBindings->setItem(row, 2, new QTableWidgetItem(device));
		// Keep the row selected
		tblBindings->item(row, 1)->setSelected(true);
		tblBindings->item(row, 2)->setSelected(true);
		// Fetch updated table and continue where we left off
		cellList = tblBindings->selectedItems();
	}
	// Update the status with the number of rows affected
	QString newStatus;
	QTextStream out(&newStatus);
	out << "MIDI binding set (" << rowcount << " rows); waiting";
	labelStatus->setText(newStatus);
	singleLearning = true; // Enable messages again
}

/* groupLearn(ConfigValueMidi *, QString)
 * A midi learn message has been received during group learn mode. Sets the
 * current row in the group to the given MIDI message, and waits to be called
 * again for the next row. If the end of the selection is reached, group learn
 * is disabled.
 */
void DlgPrefMidiBindings::groupLearn(ConfigValueMidi *value, QString device) {
	if (!groupLearning || tblBindings->rowCount() == 0) return;
	// Get list of currently selected rows
	QList<QTableWidgetItem *> cellList = tblBindings->selectedItems();
	// Get the row of the control to be learned
	int row;
	for (QList<QTableWidgetItem *>::iterator it = cellList.begin(); it != cellList.end(); ++it) {
		row = (*it)->row();
		if (row > currentGroupRow) {
			// Next row found, Set MIDI info in row
			QString midiInfo;
			QTextStream out(&midiInfo);
			out << value->miditype << " " << value->midino << " " << value->midichannel;
			tblBindings->setItem(row, 1, new QTableWidgetItem(midiInfo));
			tblBindings->setItem(row, 2, new QTableWidgetItem(device));
		}
		// Skip to next row
		while ((*it)->row() == row) {
			if (it != cellList.end()) {
				//If there are no more rows selected, disable group learn
				groupLearning = false;
				//Update status
				QString newStatus;
				QTextStream out(&newStatus);
				out << "Updated row " << row << "; Group learn finished";
				labelStatus->setText(newStatus);
				btnGroupLearn->setChecked(false);
				btnSingleLearn->setEnabled(true);
			}
			++it;
		}
	}
	currentGroupRow = row; // Set the next row to be learned
}

/* slotSingleLearnToggle()
 * The user has pressed the Single Learn toggle button. Enable or disable the
 * single learning mode and update the status accordingly.
 */
void DlgPrefMidiBindings::slotSingleLearnToggle() {
	// Toggle the status variable
	singleLearning = !singleLearning;
	if (singleLearning) {
		// Enable MIDI Hook
		m_pMidi->enableMidiLearn(this);
		labelStatus->setText("Single MIDI Learn: waiting...");
		// Can't do group learning while single learning
		btnGroupLearn->setEnabled(false);
	} else {
		// Disable MIDI Hook
		m_pMidi->disableMidiLearn();
		labelStatus->setText("Single MIDI Learn disabled");
		btnGroupLearn->setEnabled(true);
	}
}

/* slotGroupLearnToggle()
 * The user has pressed the Group Learn button. Midi Learn is enabled, and the
 * status is updated, indicating Mixxx is waiting for a Message. */
void DlgPrefMidiBindings::slotGroupLearnToggle() {
	groupLearning = !groupLearning;
	if (groupLearning) {
		// Enable MIDI Hook
		m_pMidi->enableMidiLearn(this);
		labelStatus->setText("Group MIDI Learn: waiting...");
		// Can't do group learning while single learning
		btnSingleLearn->setEnabled(false);
	} else {
		// Disable MIDI Hook
		m_pMidi->disableMidiLearn();
		labelStatus->setText("Group MIDI Learn disabled");
		btnSingleLearn->setEnabled(true);
	}
}

/* slotImportXML()
 * Prompts the user for an XML preset and loads it.
 */
void DlgPrefMidiBindings::slotImportXML() {
	QString fileName = QFileDialog::getOpenFileName(this,
		     "Import Mixxx MIDI Preset", m_pConfig->getConfigPath().append("midi/"),
		     "Preset Files (*.xml)");
	if (!fileName.isNull()) {
		loadPreset(fileName);
		applyPreset();
	}
}

/* slotExportXML()
 * Prompts the user for an XML preset and saves it.
 */
void DlgPrefMidiBindings::slotExportXML() {
	QString fileName = QFileDialog::getSaveFileName(this,
		     "Export Mixxx MIDI Preset", m_pConfig->getConfigPath().append("midi/"),
		     "Preset Files (*.xml)");
	if (!fileName.isNull())
		savePreset(fileName);
}

/* slotClear()
 * Clears the table and DOM of all bindings.
 */
void DlgPrefMidiBindings::slotClear() {
	if (QMessageBox::warning(this, "Clear Bindings",
			"Are you sure you want to clear all bindings?",
			QMessageBox::Ok | QMessageBox::Cancel, QMessageBox::Cancel)
			!= QMessageBox::Ok)
		return;
	clearPreset();
	clearTable();
}

/* slotRemoveBinding()
 * Removes one or more selected bindings from the table.
 */
void DlgPrefMidiBindings::slotRemoveBinding() {
	// If nothing selected, return
	int count = tblBindings->selectedItems().count();
	if (count == 0) return;

	// Prompt user for confirmation
	if (QMessageBox::warning(this, "Remove Binding",
				"Are you sure you want to remove the selected binding(s)?",
				QMessageBox::Ok | QMessageBox::Cancel, QMessageBox::Cancel)
				!= QMessageBox::Ok)
			return;

	// Generate list of rows
	QList<int> rows;
	int row;
	QList<QTableWidgetItem *> cellList = tblBindings->selectedItems();
	for (int i = 0; i < cellList.count(); i++) {
		row = cellList[i]->row();
		rows.append(row);
		// Skip to next row
		while (i < cellList.count() && cellList[i]->row() == row) i++;
	}

	// Sort the list (does not work without this, even if the list is already ordered!)
	qSort(rows);

	// Remove elements in reverse order
	while (rows.count() != 0) tblBindings->removeRow(rows.takeLast());
}

/* slotAddBinding()
 * Adds a binding to the table, with a minimum of the control and device info.
 */
void DlgPrefMidiBindings::slotAddBinding() {
	// TODO: make pretty dialog
	bool ok = true;

	QStringList selectionList;

	// Try to load values, when that fails pick up last entry values from existing table where present.
	selectionList = QStringList(m_pMidi->getOpenDevices());
	if (selectionList.size() == 0 && tblBindings->rowCount() > 0) { selectionList.append(tblBindings->item(tblBindings->rowCount() - 1,2)->text()); }
	QString device = QInputDialog::getItem(this, tr("1/8 Select Device"), tr("Select a device to control"), selectionList, 0, true, &ok);
        if (!ok) return;

	selectionList = QStringList();
	selectionList << "[Master]" << "[Channel1]" << "[Channel2]";
	QString group = QInputDialog::getItem(this, tr("2/8 Select Control Group"), tr("Select Control Group"), selectionList, 0, false,  &ok);
        if (!ok) return;

	selectionList = QStringList();
	selectionList << "play" << "nextTrack" << "prevTrack";
	QString key = QInputDialog::getItem(this, tr("3/8 Select Control Key"), tr("Select Control Key"), selectionList, 0, true,  &ok);
        if (!ok) return;

	selectionList = QStringList();
	selectionList << "Ctrl" << "Key" << "Pitch";
	QString miditype = QInputDialog::getItem(this, tr("4/8 Select MIDI Type"), tr("Select MIDI Type"), selectionList, 0, false,  &ok);
        if (!ok) return;

	QString midino = QString::number(QInputDialog::getInteger(this, tr("5/8 Select Midi No."), tr("Type in the MIDI number value"), 1, 0, 127, 1, &ok));
        if (!ok) return;

	QString midichan = QString::number(QInputDialog::getInteger(this, tr("6/8 Select Midi Channel"), tr("Type in the MIDI channel value (ch)"), 1, 0, 127, 1, &ok));
        if (!ok) return;

	selectionList = QStringList(getControlList());
	if (selectionList.size() == 0 && tblBindings->rowCount() > 0) { selectionList.append(tblBindings->item(tblBindings->rowCount() - 1,3)->text()); }
	QString controltype = QInputDialog::getItem(this, tr("7/8 Select Control"), tr("Select a function of Mixxx to control"), selectionList, 0, true, &ok);
        if (!ok) return;

	selectionList = QStringList(options);
	QString option = QInputDialog::getItem(this, tr("8/8 Select Option"), tr("Select an optional behaviour modifier"), selectionList, 0, true, &ok);
        if (!ok) return;

	// At this stage we have enough information to create a blank, learnable binding
	addRow(device, group, key, controltype ,miditype, midino, midichan, option);
}

/* slotChangeBinding()
 * Changes the currently selected binding(s) until the user cancels.
 */
void DlgPrefMidiBindings::slotChangeBinding() {
	// TODO: make pretty dialog
	// For each field in the table
	// Get new information
	// If user cancels, return
	// Change the selected row
}

/* slotAdvancedOptions()
 * Changes the advanced options for the currently selected binding(s).
 */
void DlgPrefMidiBindings::slotAdvancedOptions() {
	// TODO: make pretty dialog
	// Ask for string of options
	// Tokenise, validate
	// Update options column
}

/* setRowBackground(int, QColor)
 * Colours the specified row of the table.
 */
void DlgPrefMidiBindings::setRowBackground(int row, QColor color) {
	for (int i = 0; i < tblBindings->columnCount(); i++) {
		tblBindings->item(row, i)->setBackgroundColor(color);
	}
}

/* addRow(QString, QString, QString, QString, QString, QString, QString)
 * Adds a row to the table representing one binding
 */
void DlgPrefMidiBindings::addRow(QString device, QString group, QString key, QString controltype, QString miditype, QString midino, QString midichan, QString option) {
	tblBindings->setRowCount(tblBindings->rowCount() + 1);
	int row = tblBindings->rowCount() - 1;
	tblBindings->setItem(row, 0, new QTableWidgetItem(group + " " + key));
	tblBindings->setItem(row, 1, new QTableWidgetItem(miditype + " " + midino + " " + midichan));
	tblBindings->setItem(row, 2, new QTableWidgetItem(device));
	tblBindings->setItem(row, 3, new QTableWidgetItem(controltype));
	//tblBindings->setItem(row, 4, new QTableWidgetItem("none"));

	//Setup the Options combobox
	QComboBox *optionBox = new QComboBox();
	optionBox->addItems(options);
	optionBox->setCurrentText(option);
	tblBindings->setCellWidget(row, 4, optionBox);

}

void DlgPrefMidiBindings::addOutputRow(QString outputType, QString group, QString key, QString min, QString max, QString status, QString midino, QString device, QString on, QString off) { 

	tblOutputBindings->setRowCount(tblOutputBindings->rowCount() + 1);
	int row = tblOutputBindings->rowCount() - 1;

	//Setup the Options combobox
	QComboBox *optionBox = new QComboBox();
	optionBox->addItems(outputChoices);
	optionBox->setCurrentText(outputType);
	tblOutputBindings->setCellWidget(row, 0, optionBox);

	tblOutputBindings->setItem(row, 1, new QTableWidgetItem(group + " " + key));
	tblOutputBindings->setItem(row, 2, new QTableWidgetItem(midino + " " + status));
	tblOutputBindings->setItem(row, 3, new QTableWidgetItem(device));
	tblOutputBindings->setItem(row, 4, new QTableWidgetItem(min));
	tblOutputBindings->setItem(row, 5, new QTableWidgetItem(min));
	tblOutputBindings->setItem(row, 6, new QTableWidgetItem(on));
	tblOutputBindings->setItem(row, 7, new QTableWidgetItem(off));
	//tblOutputBindings->setItem(row, 4, new QTableWidgetItem("none"));

}


/* buildDomElement()
 * Updates the DOM with what is currently in the table
 */
void DlgPrefMidiBindings::buildDomElement() {
	clearPreset(); // Create blank document
	for (int y = 0; y < tblBindings->rowCount(); y++) {
		// For each row

		QDomText text;
		QDomDocument nodeMaker;
		QDomElement control = nodeMaker.createElement("control");

		QDomElement group = nodeMaker.createElement("group");
		text = nodeMaker.createTextNode(tblBindings->item(y,0)->text().split(' ',
				QString::SkipEmptyParts, Qt::CaseInsensitive).at(0));
		group.appendChild(text);

		QDomElement key = nodeMaker.createElement("key");
		text = nodeMaker.createTextNode(tblBindings->item(y,0)->text().split(' ',
				QString::SkipEmptyParts, Qt::CaseInsensitive).at(1));
		key.appendChild(text);

		QDomElement midiType = nodeMaker.createElement("miditype");
		text = nodeMaker.createTextNode(tblBindings->item(y,1)->text().split(' ',
						QString::SkipEmptyParts, Qt::CaseInsensitive).at(0));
		midiType.appendChild(text);
		QDomElement midiNo = nodeMaker.createElement("midino");
		text = nodeMaker.createTextNode(toHex(tblBindings->item(y,1)->text().split(' ',
						QString::SkipEmptyParts, Qt::CaseInsensitive).at(1)));
		midiNo.appendChild(text);

		QDomElement midiChan = nodeMaker.createElement("midichan");
		text = nodeMaker.createTextNode(toHex(tblBindings->item(y,1)->text().split(' ',
						QString::SkipEmptyParts, Qt::CaseInsensitive).at(2)));
		midiChan.appendChild(text);

		QDomElement controlType = nodeMaker.createElement("controltype");
		text = nodeMaker.createTextNode(tblBindings->item(y,3)->text());
		controlType.appendChild(text);

		QDomElement options = nodeMaker.createElement("options");
		QString comboText = ((QComboBox*)tblBindings->cellWidget(y,4))->currentText();
		if (comboText != "Normal") {
			QDomElement singleOption = nodeMaker.createElement(comboText);
			options.appendChild(singleOption);
		}

		control.appendChild(group);
		control.appendChild(key);
		control.appendChild(midiType);
		control.appendChild(midiNo);
		control.appendChild(midiChan);
		control.appendChild(controlType);
		control.appendChild(options);

		//Add control to correct device tag - find the correct tag
		QDomElement controller = m_pBindings.firstChildElement("controller");
		while (controller.attribute("id","") != tblBindings->item(y,2)->text() && !controller.isNull()) {
			controller = controller.nextSiblingElement("controller");
		}
		if (controller.isNull()) {
			// No tag was found - create it
			controller = nodeMaker.createElement("controller");
			controller.setAttribute("id", tblBindings->item(y,2)->text());
			m_pBindings.appendChild(controller);
		}
		// Check for controls tag
		QDomElement controls = controller.firstChildElement("controls");
		if (controls.isNull()) {
			controls = nodeMaker.createElement("controls");
			controller.appendChild(controls);
		}
		controls.appendChild(control);
	}
	// FIXME: outputs block!!!!

	
}

/* getControlList()
 * Gets the list of control objects
 */
QStringList DlgPrefMidiBindings::getControlList() {
/*
	ConfigOption<ValueType> *it;
	for (it = m_pConfig->list.first(); it; it = m_pConfig->list.next())
	    {
	        qDebug() << "group:" << it->key->group << "item:" << it->key->item;
	    }
	return QStringList();
*/
/*
    ConfigOption<ValueType> * it;
    for (it = list.first(); it; it = list.next())
    {
		qDebug() << "key:"<< it->key <<"value:" << it->val->value;
	}
	return;
*/
	QFile input("midi/controls.txt");
	if (!input.open(QIODevice::ReadOnly))
	         return QStringList(); //Error opening the file, return blank list
	QTextStream inputstream(&input);

	QStringList list;
	while (!inputstream.atEnd()) list.append(inputstream.readLine(1024));
	input.close();

	return list;
}

/* TODO:
 * - Extract the MIDI device handlers from the device dialog and put them in
 * 	 another file. This way the bindings manager can interact with the devices.
 * - Create small dialogs for the dialog functions
 *   - Initially QInputDialog
 *   - Create custom dialog later
 * - Functionality
 *   - MIDI Learn
 *     - slotSingleLearn
 *       - If waiting for a single learn, change the selected row
 *         - Add visual confirmation
 *       - If waiting for a group learn, change the next row
 * - BuildDomElement should not return the newly constructed DOM, not replace it
 * - Same with clearpreset?
 * - OPTIONS!!!
 * - Comboboxes in Table
 * - Better way of 'for each row'? iterate over row numbers and see if item is
 *   in row. Nicer looking code, complexity?
 * - Device type functionality
 */
