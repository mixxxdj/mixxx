#include "scriptstudio.h"

#include <QDir>
#include <QDirModel>

ScriptStudio::ScriptStudio(ScriptEngine *eng) : QMainWindow(), Ui::ScriptStudio(), m_eng(eng) {
    setupUi(this);

	connect(actionRun_Macro, SIGNAL(triggered()), this, SLOT(runPressed()));
    connect(actionImport, SIGNAL(triggered()), this, SLOT(importScript()));
    connect(actionExport, SIGNAL(triggered()), this, SLOT(exportScript()));
}

void ScriptStudio::showStudio() {
	fillTree();

	this->setVisible(true);
}

#define MIXXXMACRODIR "mixxxmacros"

void ScriptStudio::fillTree() {
        QString path = QDir::home().QDir::homePath().append("/").append(SETTINGS_PATH).append(MIXXXMACRODIR);
	QDir topdir(path);
	if (!topdir.exists()) {
                QDir().mkdir(path);
	}

	QDirModel *model = new QDirModel();
    treeView->setModel(model);
	treeView->setRootIndex(model->index(path));
}

void ScriptStudio::runPressed() {
	m_eng->executeMacro(new Macro(Macro::LANG_QTSCRIPT, "Macro", textEdit->toPlainText()));
}

void ScriptStudio::importScript() {
    QString filename = QFileDialog::getOpenFileName(this, "Select a macro...", QDir::home().QDir::homePath().append("/").append(SETTINGS_PATH).append(MIXXXMACRODIR), MIXXX_SCRIPT_NAMEFILTER);
    QFile file(filename);
    if (file.open(QFile::ReadWrite))
    {
        QTextStream stream(&file);
        textEdit->setPlainText(stream.readAll()); //FIXME: Why doesn't this work?
    }
    else
        QMessageBox::critical(this, "Script Import Error", "Failed to open the selected file.");
}

void ScriptStudio::exportScript() {
    qDebug() << "FIXME: ScriptStudio::exportScript() unimplemented in" << __FILE__ << "on line" << __LINE__;
}


