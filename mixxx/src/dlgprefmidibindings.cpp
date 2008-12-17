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

#ifdef __SCRIPT__
#include "script/midiscriptengine.h"
#endif

#define BINDINGS_PATH QDir::homePath().append("/").append(".MixxxMIDIBindings.xml")
const QStringList options = (QStringList() << "Normal" << "Script-Binding" << "Invert" << "Rot64" << "Rot64Inv"
		<< "Rot64Fast" << "Diff" << "Button" << "Switch" << "HercJog"
		<< "Spread64" << "SelectKnob");

QStringList controKeyOptionChoices;

const QStringList outputTypeChoices = (QStringList() << "light");


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
	connect(btnTestOutputBinding, SIGNAL(clicked()), this, SLOT(slotTestOutputBinding()));

	connect(btnOutputImportXML, SIGNAL(clicked()), this, SLOT(slotImportXML()));
	connect(btnOutputExportXML, SIGNAL(clicked()), this, SLOT(slotExportXML()));

	connect(btnClearOutputBindings, SIGNAL(clicked()), this, SLOT(slotClearOutputBindings()));
	connect(btnRemoveOutputBinding, SIGNAL(clicked()), this, SLOT(slotRemoveOutputBinding()));
	connect(btnAddOutputBinding, SIGNAL(clicked()), this, SLOT(slotAddOutputBinding()));

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
	clearOutputTable();
	if (root.isNull()) return;
	// For each controller in the DOM
	m_pBindings = root;
	QDomElement controller = m_pBindings.firstChildElement("controller");
	while (!controller.isNull()) {
		// For each controller
		// Get deviceid
		QString device = controller.attribute("id","");
		qDebug() << device << " settings found" << endl;
		QDomElement control = controller.firstChildElement("controls").firstChildElement("control");

#ifdef __SCRIPT__
		bool scriptGood=m_pMidi->getMidiScriptEngine()->evaluateScript();
		QStringList scriptFunctions;
		if (scriptGood) scriptFunctions = m_pMidi->getMidiScriptEngine()->getFunctionList();
#endif
		while (!control.isNull()) {
			// For each control
			QString group = WWidget::selectNodeQString(control, "group");
			QString key = WWidget::selectNodeQString(control, "key");
			QString controltype = WWidget::selectNodeQString(control, "controltype");
			QString miditype = WWidget::selectNodeQString(control, "miditype");
			// We convert to midino and midichan to base 10 because that his how they will be matched to midi keys internally.
			bool ok = false;
			QString midino = QString::number(WWidget::selectNodeQString(control, "midino").toUShort(&ok, 0), 10);
			QString midichan = !ok ? "" : QString::number(WWidget::selectNodeQString(control, "midichan").toUShort(&ok, 0), 10);
			if (miditype.trimmed().length() == 0 || !ok) { // Blank all values, if they one is invalid
				qDebug() << "One or more of miditype, midino, or midichan elements were omitted. The MIDI control has been cleared, you'll have to reteach it.";
				miditype = "";
				midino = "";
				midichan = "";
			}
			QDomElement optionsNode = control.firstChildElement("options");
			// At the moment, use one element, in future iterate through options
			QString option;
			if (optionsNode.hasChildNodes()) {
				option = optionsNode.firstChild().nodeName();
			} else {
				option = "Normal";
			}
			
#ifdef __SCRIPT__
			// Verify script functions are loaded
			if (scriptGood && (option=="script-binding" || option=="Script-Binding") && scriptFunctions.indexOf(key)==-1) {
				QMessageBox::warning(this, "Warning: Script function not found", "Function "+key+" not found in script "+m_pMidi->getMidiScriptEngine()->getFilepath()+".\nThis control will be unbound.  Please correct by either adding the script method or correcting the key name before relearning the control.");
				addRow(device, group, key, controltype, "", "", "", option); // Drop the midi binding bit...
			} else {
#endif
			// Construct row in table
			addRow(device, group, key, controltype, miditype, midino, midichan, option);
#ifdef __SCRIPT__
			}
#endif
			control = control.nextSiblingElement("control");
		}

		QDomNode output = controller.namedItem("outputs").toElement().firstChild();
		while (!output.isNull()) {
			QString outputType = output.nodeName();
			QString group = WWidget::selectNodeQString(output, "group");
			QString key = WWidget::selectNodeQString(output, "key");

			QString status = QString::number(WWidget::selectNodeInt(output, "status"));
			QString midino = QString::number(WWidget::selectNodeInt(output, "midino"));

			QString on = "0x7F";	// Compatible with Hercules and others
			QString off = "0x00";
			QString min = "";
			QString max = "";

			if(outputType == "light") {
				if (!output.firstChildElement("on").isNull()) {
					on = WWidget::selectNodeQString(output, "on");
				}
				if (!output.firstChildElement("off").isNull()) {
					off = WWidget::selectNodeQString(output, "off");
				}
				if (!output.firstChildElement("threshold").isNull()) {
					min = WWidget::selectNodeQString(output, "threshold");
				}
				if (!output.firstChildElement("minimum").isNull()) {
					min = WWidget::selectNodeQString(output, "minimum");
				}
				if (!output.firstChildElement("maximum").isNull()) {
					max = WWidget::selectNodeQString(output, "maximum");
				}
			}
			qDebug() << "Loaded Output type:" << outputType << " -> " << group << key << "between"<< min << "and" << max << "to midi out:" << status << midino << "on" << device << "on/off:" << on << off;

			addOutputRow(outputType, group, key, min, max, toHex(status), toHex(midino), device, on, off);

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
	if (!output.open(QIODevice::WriteOnly | QIODevice::Truncate)) return;
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

void DlgPrefMidiBindings::clearOutputTable() {
	tblOutputBindings->clearContents();
	tblOutputBindings->setRowCount(0);
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
	if (!fileName.isNull()) savePreset(fileName);
}

/* slotClear()
 * Clears the table and DOM of all bindings.
 */
void DlgPrefMidiBindings::slotClear() {
	if (QMessageBox::warning(this, "Clear Input Bindings",
			"Are you sure you want to clear all bindings?",
			QMessageBox::Ok | QMessageBox::Cancel, QMessageBox::Cancel) != QMessageBox::Ok)
		return;
	clearPreset();
	clearTable();
}

void DlgPrefMidiBindings::slotClearOutputBindings() {
	if (QMessageBox::warning(this, "Clear Output Bindings",
			"Are you sure you want to clear all bindings?",
			QMessageBox::Ok | QMessageBox::Cancel, QMessageBox::Cancel) != QMessageBox::Ok)
		return;
	clearOutputTable();
}

void DlgPrefMidiBindings::slotTestOutputBinding() {
	// If nothing selected, return
	if (tblOutputBindings->currentRow() == -1) {
		labelOutputStatus->setText("No output row selected"); btnTestOutputBinding->setChecked(false); return;
	}
	int y = tblOutputBindings->currentRow();

	QString outputType = ((QComboBox*)tblOutputBindings->cellWidget(y,1))->currentText().trimmed();
	QString device = tblOutputBindings->item(y,3)->text().trimmed();
	// FIXME: check that device is actually physically connected before trying to send output!

	qDebug() << "slotTestOutputBinding firing"<< (btnTestOutputBinding->isChecked() ? "ON" : "OFF") << "event for row:" << y << "outputType:" << outputType << "on device:" << device;

	if (outputType == "light") {
		bool ok = true;
		unsigned char status = (unsigned char) tblOutputBindings->item(y,2)->text().trimmed().split(' ').at(0).trimmed().toUShort(&ok, 0);
		// FIXME: if you blank col3 these !ok doesn't catch the problem and trigger the error check below
		if (!ok) { labelOutputStatus->setText("invalid MIDI status code (col 3, value 1)"); btnTestOutputBinding->setChecked(false); return; }
		unsigned char midino = (unsigned char) tblOutputBindings->item(y,2)->text().trimmed().split(' ').at(1).trimmed().toUShort(&ok, 0);;
		if (!ok) { labelOutputStatus->setText("invalid MIDI # code (col 3, value 2)"); btnTestOutputBinding->setChecked(false); return; }

		unsigned on = (unsigned char) tblOutputBindings->item(y,6)->text().trimmed().toUShort(&ok, 0);
		if (!ok) { on = 0x7f; }
		int off = (unsigned char) tblOutputBindings->item(y,7)->text().trimmed().toUShort(&ok, 0);
		if (!ok) { off = 0x00; }

		m_pMidi->sendShortMsg(status, midino, btnTestOutputBinding->isChecked()? on : off , device);
	}
	labelOutputStatus->setText( btnTestOutputBinding->isChecked()? outputType + " on" : outputType + " off" );
// TODO: create "outputCycleNext" checkbox which when checked will move to the next row when an off event happens
//	if (outputCycleNext && tblOutputBindings->count() > 0 && !btnTestOutputBinding->isChecked()) {
//		if (tblOutputBindings->count() - 1 > y) {
//			tblOutputBindings->setCurrentRow(y++);
//		} else { tblOutputBindings->setCurrentRow(0); }
//	}
}

void DlgPrefMidiBindings::removeSelectedBindings(QTableWidget* table) {
	// If nothing selected, return
	int count = table->selectedItems().count();
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
	QList<QTableWidgetItem *> cellList = table->selectedItems();
	for (int i = 0; i < cellList.count(); i++) {
		row = cellList[i]->row();
		rows.append(row);
		// Skip to next row
		while (i < cellList.count() && cellList[i]->row() == row) i++;
	}

	// Sort the list (does not work without this, even if the list is already ordered!)
	qSort(rows);

	// Remove elements in reverse order
	while (rows.count() != 0) table->removeRow(rows.takeLast());
}


/* slotRemoveBinding()
 * Removes one or more selected bindings from the table.
 */
void DlgPrefMidiBindings::slotRemoveBinding() {
	removeSelectedBindings(tblBindings);
}

void DlgPrefMidiBindings::slotRemoveOutputBinding() {
	removeSelectedBindings(tblOutputBindings);
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

	//	selectionList = QStringList(getControlTypeList());
	//	if (selectionList.size() == 0 && tblBindings->rowCount() > 0) { selectionList.append(tblBindings->item(tblBindings->rowCount() - 1,3)->text()); }
	//	QString controltype = QInputDialog::getItem(this, tr("7/8 Select Control"), tr("Select a function of Mixxx to control"), selectionList, 0, true, &ok);
	//        if (!ok) return;
	QString controltype = "";

	selectionList = QStringList(options);
	QString option = QInputDialog::getItem(this, tr("6/6 Select Option"), tr("Select an optional behaviour modifier"), selectionList, 0, true, &ok);
	if (!ok) return;

	// At this stage we have enough information to create a blank, learnable binding
	addRow(device, group, key, controltype ,miditype, midino, midichan, option);
	tblBindings->selectRow(tblBindings->rowCount() - 1); // Focus the row just added
}

void DlgPrefMidiBindings::slotAddOutputBinding() {
	// TODO: make pretty dialog
	bool ok = true;

	QStringList selectionList;
	selectionList = QStringList(m_pMidi->getOpenDevices());
	if (selectionList.size() == 0 && tblBindings->rowCount() > 0) { selectionList.append(tblBindings->item(tblBindings->rowCount() - 1,2)->text()); }
	QString device = QInputDialog::getItem(this, tr("Select Device"), tr("Select a device to control"), selectionList, 0, true, &ok);
	if (!ok) return;

	addOutputRow(outputTypeChoices.value(0), "[Channel1]", "play", "", "", toHex(0), toHex(0), device, "0x7F", "0x00");

	tblOutputBindings->selectRow(tblOutputBindings->rowCount() - 1); // Focus the row just added
}

/* slotChangeBinding()
 * Changes the currently selected binding(s) until the user cancels.
 */
//void DlgPrefMidiBindings::slotChangeBinding() {
// TODO: make pretty dialog
// For each field in the table
// Get new information
// If user cancels, return
// Change the selected row
//}

/* slotAdvancedOptions()
 * Changes the advanced options for the currently selected binding(s).
 */
//void DlgPrefMidiBindings::slotAdvancedOptions() {
// TODO: make pretty dialog
// Ask for string of options
// Tokenise, validate
// Update options column
//}

/* setRowBackground(int, QColor)
 * Colours the specified row of the table.
 */
/*
void DlgPrefMidiBindings::setRowBackground(int row, QColor color) {
	for (int i = 0; i < tblBindings->columnCount(); i++) {
		tblBindings->item(row, i)->setBackgroundColor(color);
	}
}
 */

/* addRow(QString, QString, QString, QString, QString, QString, QString)
 * Adds a row to the table representing one binding
 */
void DlgPrefMidiBindings::addRow(QString device, QString group, QString key, QString controltype, QString miditype, QString midino, QString midichan, QString option) {
	tblBindings->setRowCount(tblBindings->rowCount() + 1);
	int row = tblBindings->rowCount() - 1;

	QComboBox *controlKeysComboBox = new QComboBox();
	controlKeysComboBox->setEditable(true);
	controlKeysComboBox->addItems(getControlKeyList());
	QString controlKey = QString(group + " " + key).trimmed();
	if (controlKeysComboBox->findText(controlKey) == -1) {
		controlKeysComboBox->addItem(controlKey.trimmed());
	}
	controlKeysComboBox->setCurrentIndex(controlKeysComboBox->findText(controlKey));

	tblBindings->setCellWidget(row, 0, controlKeysComboBox);

	tblBindings->setItem(row, 1, new QTableWidgetItem(QString(miditype + " " + midino + " " + midichan).trimmed()));
	tblBindings->setItem(row, 2, new QTableWidgetItem(device.trimmed()));
	tblBindings->setItem(row, 3, new QTableWidgetItem(controltype.trimmed()));

	//Setup the Options combobox
	QComboBox *optionBox = new QComboBox();
	optionBox->addItems(options);
	optionBox->setCurrentIndex(optionBox->findText(option));
	tblBindings->setCellWidget(row, 4, optionBox);
}

void DlgPrefMidiBindings::addOutputRow(QString outputType, QString group, QString key, QString min, QString max, QString status, QString midino, QString device, QString on, QString off) {

	tblOutputBindings->setRowCount(tblOutputBindings->rowCount() + 1);
	int row = tblOutputBindings->rowCount() - 1;

	//Setup the Options combobox
	QComboBox *optionBox = new QComboBox();
	optionBox->addItems(outputTypeChoices);
	optionBox->setEditText(outputType);
	tblOutputBindings->setCellWidget(row, 0, optionBox);

	QComboBox *controlKeysComboBox = new QComboBox();
	controlKeysComboBox->setEditable(true);
	controlKeysComboBox->addItems(getControlKeyList());
	QString controlKey = QString(group + " " + key).trimmed();
	if (controlKeysComboBox->findText(controlKey) == -1) {
		controlKeysComboBox->addItem(controlKey);
	}
	controlKeysComboBox->setCurrentIndex(controlKeysComboBox->findText(controlKey));
	tblOutputBindings->setCellWidget(row, 1, controlKeysComboBox);

	tblOutputBindings->setItem(row, 2, new QTableWidgetItem(midino + " " + status));
	tblOutputBindings->setItem(row, 3, new QTableWidgetItem(device));
	tblOutputBindings->setItem(row, 4, new QTableWidgetItem(min));
	tblOutputBindings->setItem(row, 5, new QTableWidgetItem(max));
	tblOutputBindings->setItem(row, 6, new QTableWidgetItem(on));
	tblOutputBindings->setItem(row, 7, new QTableWidgetItem(off));
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

		QString device = tblBindings->item(y,2)->text().trimmed();

		QHash<QString, QString> controlMapping;
		QString controlKey = ((QComboBox*)tblBindings->cellWidget(y,0))->currentText().trimmed();

		if (controlKey.isEmpty() 
			|| controlKey.trimmed().split(' ').count() < 2 
			|| controlKey.indexOf("[") == -1 
			|| controlKey.indexOf("]") == -1) {
			qDebug() << "MIDI Input Row"<<y+1<<"was dropped during save because controlKey value of" << controlKey <<"is invalid."; 
			continue; // Invalid mapping, skip it.
		}
		controlMapping["group"] = controlKey.trimmed().split(' ').at(0).trimmed();
		controlMapping["key"] = controlKey.trimmed().split(' ').at(1).trimmed();
		if ( tblBindings->item(y,1)->text().trimmed().split(' ').count() == 3 ) {
			controlMapping["miditype"] = tblBindings->item(y,1)->text().trimmed().split(' ').at(0).trimmed();
			controlMapping["midino"] = tblBindings->item(y,1)->text().trimmed().split(' ').at(1).trimmed();
			controlMapping["midichan"] = tblBindings->item(y,1)->text().trimmed().split(' ').at(2).trimmed();
		} else {
			qDebug() << "MIDI value of" << tblBindings->item(y,1)->text().trimmed() << "was omitted from the mapping saved for row"<<y+1<<"."; 
		}
		controlMapping["controltype"] = tblBindings->item(y,3)->text().trimmed();
		controlMapping["options"] = ((QComboBox*)tblBindings->cellWidget(y,4))->currentText().trimmed();

		// TODO: make these output in a more human friendly order
		foreach (QString tagName, controlMapping.keys()) {
			QDomElement tagNode = nodeMaker.createElement(tagName);
			if (tagName != "options") {
				text = nodeMaker.createTextNode(controlMapping.value(tagName));
				tagNode.appendChild(text);
			} else if (tagName == "options" && controlMapping[tagName] != "Normal") {
				QDomElement singleOption = nodeMaker.createElement(controlMapping[tagName]);
				tagNode.appendChild(singleOption);
			}
			control.appendChild(tagNode);
		}

		//Add control to correct device tag - find the correct tag
		QDomElement controller = m_pBindings.firstChildElement("controller");
		while (controller.attribute("id","") != device && !controller.isNull()) {
			controller = controller.nextSiblingElement("controller");
		}
		if (controller.isNull()) {
			// No tag was found - create it
			controller = nodeMaker.createElement("controller");
			controller.setAttribute("id", device);
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

	for (int y = 0; y < tblOutputBindings->rowCount(); y++) {
		// For each row
		QString outputType = ((QComboBox*)tblOutputBindings->cellWidget(y,0))->currentText().trimmed();
		QString device = tblOutputBindings->item(y,3)->text().trimmed();

		QHash<QString, QString> outputMapping;
		QString controlKey = ((QComboBox*)tblOutputBindings->cellWidget(y,1))->currentText().trimmed();
		if (controlKey.isEmpty() 
			|| controlKey.trimmed().split(' ').count() < 2 
			|| controlKey.indexOf("[") == -1 
			|| controlKey.indexOf("]") == -1
			|| tblOutputBindings->item(y,2)->text().trimmed().split(' ').count() < 2) {
			qDebug() << "MIDI Output Row"<<y+1<<"was dropped during save because it contains invalid values."; 
			continue; // Invalid mapping, skip it.
		}
		
		outputMapping["group"] = controlKey.trimmed().split(' ').at(0).trimmed();
		outputMapping["key"] = controlKey.trimmed().split(' ').at(1).trimmed();
		outputMapping["status"] = tblOutputBindings->item(y,2)->text().trimmed().split(' ').at(0).trimmed();
		outputMapping["midino"] = tblOutputBindings->item(y,2)->text().trimmed().split(' ').at(1).trimmed();
		//		outputMapping["device"] = tblOutputBindings->item(y,3)->text().trimmed();
		outputMapping["minimum"] = tblOutputBindings->item(y,4)->text().trimmed();
		outputMapping["maximum"] = tblOutputBindings->item(y,5)->text().trimmed();
		outputMapping["on"] = tblOutputBindings->item(y,6)->text().trimmed();
		outputMapping["off"] = tblOutputBindings->item(y,7)->text().trimmed();

		// Clean up any optional values
		if (outputMapping["maximum"].isEmpty()) {
			if (!outputMapping["minimum"].isEmpty()) {
				outputMapping["threshold"] = outputMapping["minimum"];
			}
			outputMapping.remove("minimum");
			outputMapping.remove("maximum");
		}
		if (outputMapping["on"].isEmpty() || outputMapping["off"].isEmpty()) {
			outputMapping.remove("on");
			outputMapping.remove("off");
		}

		// Generate output XML
		QDomText text;
		QDomDocument nodeMaker;
		QDomElement output = nodeMaker.createElement(outputType);

		// TODO: make these output in a more human friendly order
		foreach (QString tagName, outputMapping.keys()) {
			QDomElement tagNode = nodeMaker.createElement(tagName);
			text = nodeMaker.createTextNode(outputMapping.value(tagName));
			tagNode.appendChild(text);
			output.appendChild(tagNode);
		}

		// Find the controller to attach the XML to...
		QDomElement controller = m_pBindings.firstChildElement("controller");
		while (controller.attribute("id","") != device && !controller.isNull()) {
			controller = controller.nextSiblingElement("controller");
		}
		if (controller.isNull()) {
			// No tag was found - create it
			controller = nodeMaker.createElement("controller");
			controller.setAttribute("id", device);
			m_pBindings.appendChild(controller);
		}

		// Find the outputs block
		QDomElement outputs = controller.firstChildElement("outputs");
		if (outputs.isNull()) {
			outputs = nodeMaker.createElement("outputs");
			controller.appendChild(outputs);
		}
		// attach the output to the outputs block
		outputs.appendChild(output);
	}

}

/* getControlKeyList()
 * Gets the list of control objects
 */
QStringList DlgPrefMidiBindings::getControlKeyList() {
	// midi/BindableConfigKeys.txt = grep "ConfigKey" *.cpp | grep \\[ | sed -e 's/.*ConfigKey("//g' | cut -d \) -f1 | sed -e 's/[",]/ /g' -e 's/ \+/ /g' | egrep -ve '[+:>]' | sort -u | egrep -e "\[Channel[12]\]|\[Master\]" > midi/BindableConfigKeys.txt
	if (controKeyOptionChoices.count() == 0) {
		QFile input(m_pConfig->getConfigPath() + "midi/BindableConfigKeys.txt");

		if (!input.open(QIODevice::ReadOnly)) return QStringList();

		while (!input.atEnd()) {
			QString line = input.readLine().trimmed();
			if (line.indexOf('#') == 0) continue; // ignore # hashed out comments
			if (!line.isNull()) controKeyOptionChoices.append(line);
		}
		input.close();
	}
	return controKeyOptionChoices;
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
