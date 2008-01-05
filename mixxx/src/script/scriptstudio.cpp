#include "scriptstudio.h"

#include <QDir>
#include <QDirModel>

ScriptStudio::ScriptStudio(ScriptEngine *eng) : m_eng(eng) {
	ui.setupUi(this);

	connect(ui.actionRun_Macro, SIGNAL(activated()), this, SLOT(runPressed()));

	m_namefilters << "*.mxm";
}

void ScriptStudio::showStudio() {
	fillTree();

	this->setVisible(true);
}

#define MIXXXMACRODIR ".mixxxmacros"

void ScriptStudio::fillTree() {
	QString path = QDir::home().absoluteFilePath(MIXXXMACRODIR);
	QDir topdir(path);
	if (!topdir.exists()) {
		QDir::home().mkdir(MIXXXMACRODIR);
	}
	        
	QDirModel *model = new QDirModel();
    ui.treeView->setModel(model);
	ui.treeView->setRootIndex(model->index(path));
}

void ScriptStudio::runPressed() {
	m_eng->executeMacro(new Macro(Macro::LANG_QTSCRIPT, "Macro", ui.textEdit->toPlainText()));
}