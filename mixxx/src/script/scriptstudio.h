#ifndef SCRIPT_SCRIPTSTUDIO_H
#define SCRIPT_SCRIPTSTUDIO_H

// Has to be included with wierd path so that it finds this generated in .obj
#ifdef QMAKE
    #include "ui_scriptstudio.h"
#else
    #include "script/ui_scriptstudio.h"
#endif

#include "scriptengine.h"

#include <QMainWindow>
#include <QStringList>

#define MIXXX_SCRIPT_NAMEFILTER "*.mxm"

class ScriptStudio : public QMainWindow, public Ui::ScriptStudio {

	Q_OBJECT

public:
	ScriptStudio(ScriptEngine* eng);

public slots:
	void showStudio();
	void runPressed();
    void importScript();
    void exportScript();
private:
	void fillTree();

	ScriptEngine* m_eng;
};

#endif
