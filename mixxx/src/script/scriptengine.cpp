#include "luainterface.h"
#include "scriptengine.h"
#include "scripttest.h"
#include "scriptcontrolqueue.h"
#include "scriptrecorder.h"
#include "scriptstudio.h"

#include <qfile.h>
#include <qdom.h>

ScriptEngine::ScriptEngine(MixxxApp* parent) {
	ScriptControlQueue* q = new ScriptControlQueue();
	m_lua = new LuaInterface(q); 

	m_macros = new QPtrList<Macro>();

	loadMacros();
	
	//ScriptTest* stest = new ScriptTest(this);
	m_studio = new ScriptStudio(this);
	ScriptRecorder* bobby = new ScriptRecorder();
}

ScriptStudio* ScriptEngine::getStudio() {
	return m_studio;
}

void ScriptEngine::addMacro(Macro* macro) {
	m_macros->append(macro);
}

void ScriptEngine::newMacro() {
	addMacro(new Macro("New Macro"));
}

int ScriptEngine::macroCount() {
	return m_macros->count();
}

Macro* ScriptEngine::getMacro(int index) {
	return m_macros->at(index);
}

ScriptEngine::~ScriptEngine() {
}

void ScriptEngine::executeScript(const char* script) {
	// Don't execute a null script
	if (script == 0x0) {
		return;
	}
	// For now just call lua, but this layer is here in anticipation of not
	// necessarily wanting to use it
	m_lua->executeScript(script);
}

void ScriptEngine::deleteMacro(Macro* macro) {
	if (!m_macros->remove(macro)) {
		qDebug("Something pretty bad happened deleting a macro");
	}
}

void ScriptEngine::loadMacros() {
	QDomDocument doc("MacroStore");
	QFile *file = getMacroFile();
	
	
	if (!file->open(IO_ReadOnly)) {
		qDebug("Problem loading macros from disk (file)");
		return;
	}
	QString* errorMsg = new QString();;
	int line;
	int col;
	if (!doc.setContent(file, errorMsg, &line, &col)) {
		file->close();
		qDebug("Problem loading macros from disk (xml)");
		qDebug("At %i, %i: %s", line, col, (const char*)(*errorMsg));
		return;
	}
	file->close();
	delete errorMsg;
	QDomElement docElem = doc.documentElement();

	QDomNode n = docElem.firstChild();
	while (!n.isNull()) {
		QDomNode name = n.firstChild();
		QDomNode script = name.nextSibling();
		QDomText ntext = name.firstChild().toText();
		QDomText stext = script.firstChild().toText();
		QString nstr = ntext.data();
		QString sstr = stext.data();
		Macro* macro = new Macro(nstr, sstr);
		addMacro(macro);
		n = n.nextSibling();
	}
}

void ScriptEngine::saveMacros() {
	qDebug("Saving Macros to disk");
	QFile* file = getMacroFile();
	QDomDocument doc("MacroStore");
	
	QDomElement root = doc.createElement("Macros");
	doc.appendChild(root);

	for (int i = 0; i < macroCount(); i++) {
		Macro* m = getMacro(i);
		QDomElement macro = doc.createElement("Macro");
		QDomElement name = doc.createElement("Name");
		QDomElement script = doc.createElement("Script");
		QDomText ntext = doc.createTextNode(m->getName());
		QDomText stext = doc.createTextNode(m->getScript());
		macro.appendChild(name);
		macro.appendChild(script);
		name.appendChild(ntext);
		script.appendChild(stext);
		root.appendChild(macro);
	}

	QString xml = doc.toString();

	if (file->open(IO_WriteOnly)) {
		QTextStream stream(file);
		stream << xml << "\n";
		file->close();
	} else {
		qDebug("Everything went horribly wrong writing macros to disk");
	}
	
}

QFile* ScriptEngine::getMacroFile() {
	QString path = QDir::home().absFilePath(".mixxxmacro.xml");
	return new QFile(path);
}
