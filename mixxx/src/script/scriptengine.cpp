#ifdef __PYTHON__
#include "python/pythoninterface.h"
#endif

#include "scriptengine.h"
#include "scripttest.h"
#include "scriptcontrolqueue.h"
#include "scriptrecorder.h"
#include "scriptstudio.h"

#include <qfile.h>
#include <qdom.h>

ScriptEngine::ScriptEngine(MixxxApp* parent, Track* track) {
	ScriptControlQueue* q = new ScriptControlQueue(this);

	m_pcount = 0;
	m_parent = parent;
	m_track = track;

	m_pi = new PlayInterface(q);
#ifdef __LUA__
	m_lua = new LuaInterface(m_pi); 
#endif

#ifdef __PYTHON__
	PythonInterface::initInterface(m_pi);	
#endif
	
	m_macros = new QPtrList<Macro>();

	loadMacros();
	m_rec = new ScriptRecorder(track);
	
	//ScriptTest* stest = new ScriptTest(this);
	m_studio = new ScriptStudio(this);
}

void ScriptEngine::playTrack(int channel, QString filename) {
	if (channel == 1) {
		m_track->slotLoadPlayer1(filename);
	} else if (channel == 2) { 
		m_track->slotLoadPlayer2(filename);
	} else {
		// This shouldn't happen
		qDebug("Asked for channel %i which doesn't exist", channel);
	}
}

ScriptStudio* ScriptEngine::getStudio() {
	return m_studio;
}

void ScriptEngine::addMacro(Macro* macro) {
	m_macros->append(macro);
}

void ScriptEngine::newMacro(int lang) {
	addMacro(new Macro(lang, "New Macro"));
}

int ScriptEngine::macroCount() {
	return m_macros->count();
}

ScriptRecorder* ScriptEngine::getRecorder() {
	return m_rec;
}

Macro* ScriptEngine::getMacro(int index) {
	return m_macros->at(index);
}

ScriptEngine::~ScriptEngine() {
}

void ScriptEngine::executeMacro(Macro* macro) {
	if (macro->getLang() == Macro::LANG_LUA) {
#ifdef __LUA__
		m_lua->executeScript(macro->getScript(), m_pcount);
#else
		qDebug("Lua support not available!");
#endif
	} else if (macro->getLang() == Macro::LANG_PYTHON) {
#ifdef __PYTHON__
		PythonInterface::executeScript(macro->getScript(), m_pcount);
#else
		qDebug("Python support not available!");
#endif
	} else {
		return;
	}
	m_pcount++;
}

void ScriptEngine::executeScript(const char* script) {
	// Don't execute a null script
	qDebug("This function is obsolete and breaks everything!");
	if (script == 0x0) {
		return;
	}
	// For now just call lua, but this layer is here in anticipation of not
	// necessarily wanting to use it
#ifdef __LUA__
	m_lua->executeScript(script, -1);
#else
	qDebug("Lua support not available!");
#endif
}

void ScriptEngine::deleteMacro(Macro* macro) {
	if (!m_macros->remove(macro)) {
		qDebug("Something pretty bad happened deleting a macro");
	}
}

void ScriptEngine::loadMacros() {
	QDomDocument doc("MacroStore");
	QFile* file = getMacroFile();
	
	
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
		delete file;
		return;
	}
	file->close();
	delete file;
	delete errorMsg;
	QDomElement docElem = doc.documentElement();

	QDomNode n = docElem.firstChild();
	while (!n.isNull()) {
		int lang = Macro::LANG_LUA;
		QDomNode lnode = n.firstChild();
		QDomNode name = lnode.nextSibling();
		if (lnode.nodeName() == "Name") {
			qDebug("Converting from old macro file format...");
			name = lnode;
		} else {
			QDomText ltext = lnode.firstChild().toText();
			QString lstr = ltext.data();
			lang = lstr.toInt();
		}
		QDomNode script = name.nextSibling();
		QDomText ntext = name.firstChild().toText();
		QDomText stext = script.firstChild().toText();
		QString nstr = ntext.data();
		QString sstr = stext.data();
		Macro* macro = new Macro(lang, nstr, sstr);
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
		QDomElement lang = doc.createElement("Lang");
		QDomElement name = doc.createElement("Name");
		QDomElement script = doc.createElement("Script");
		QDomText ntext = doc.createTextNode(m->getName());
		QDomText stext = doc.createTextNode(m->getScript());
		QDomText ltext = doc.createTextNode(QString::number(\
					m->getLang()));
		macro.appendChild(lang);
		macro.appendChild(name);
		macro.appendChild(script);
		name.appendChild(ntext);
		script.appendChild(stext);
		lang.appendChild(ltext);
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
	delete file;
	
}

QFile* ScriptEngine::getMacroFile() {
	QString path = QDir::home().absFilePath(".mixxxmacro.xml");
	return new QFile(path);
}
