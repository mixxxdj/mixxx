#ifndef SCRIPT_SCRIPTSTUDIO_H
#define SCRIPT_SCRIPTSTUDIO_H

// Has to be included with wierd path so that it finds this generated in .obj
#include "script/ui_scriptstudio.h"

#include "scriptengine.h"

#include <QMainWindow>
#include <QStringList>

class ScriptStudio : public QMainWindow {

	Q_OBJECT

public:
	ScriptStudio(ScriptEngine* eng);

public slots:
	void showStudio();
	void runPressed();

private:
	void fillTree();

	Ui::ScriptStudio ui;

	ScriptEngine* m_eng;
	QStringList m_namefilters;
};

#endif
