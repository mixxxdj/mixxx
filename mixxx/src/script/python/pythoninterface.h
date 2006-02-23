#include <Python.h>
#include "../playinterface.h"

/**
 * Be bloody careful where you include this. How the Python people managed to
 * write a header which is incompatible with everything else in the universe
 * I'm not quite sure. This static stuff has been rather fortuitous in that it
 * means you can generally get away with not including this in the .h file and
 * then you can put it first in the .cpp file which avoids problems.
 */

/**
 * Ok, so this stupid static stuff needs some explaining...
 * You can only have one Python interpreter per application as far as I can
 * tell. Since the way this stuff works is to poke functions into the
 * interpreter-space, we can't have more than one script interface per
 * interpreter or which set of functions are we calling? So this basically
 * means only one Python based script type thing per application.
 * Which is good because binding an object instance into the interpreter is a
 * lot harder with Python than with lua :)
 */

class PythonInterface {

	public:
		static PythonInterface* initInterface(PlayInterface* pi);
		static PythonInterface* getInterface();
		static void executeScript(const char* script, int process);
		
		static PyObject* play(PyObject* self, PyObject* args);
		static PyObject* stop(PyObject* self, PyObject* args);
		static PyObject* setFader(PyObject* self, PyObject* args);
                static PyObject* test(PyObject* self, PyObject* args);

		static PyObject* setTag(PyObject* self, PyObject* args);
		static PyObject* clearTag(PyObject* self, PyObject* args);
		static PyObject* kill(PyObject* self, PyObject* args);
		static PyObject* killTag(PyObject* self, PyObject* args);
		
		static PyObject* getFader(PyObject* self, PyObject* args);
		static PyObject* getValue(PyObject* self, PyObject* args);
		static PyObject* startFadeCrossfader(PyObject* self, PyObject* args);
		static PyObject* startList(PyObject* self, PyObject* args);
		static PyObject* startFade(PyObject* self, PyObject* args);
		static PyObject* point(PyObject* self, PyObject* args);
		static PyObject* fadePoint(PyObject* self, PyObject* args);
		static PyObject* endFade(PyObject* self, PyObject* args);
		static PyObject* endList(PyObject* self, PyObject* args);

		static PyObject* playChannel1(PyObject* self, PyObject* args);
		static PyObject* playChannel2(PyObject* self, PyObject* args);
		
		PlayInterface* getPlayInterface();
	protected:
		static PythonInterface* iface;
		static PyMethodDef EmbMethods[];
		
		PythonInterface(PlayInterface* pi);
		~PythonInterface();

		PlayInterface* m_pi;
};
