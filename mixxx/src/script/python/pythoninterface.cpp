#include "pythoninterface.h"

#define PYCHECK(x) PlayInterface *x; \
	if (iface == NULL || iface->getPlayInterface() == NULL) {\
	return Py_BuildValue("i",0); } else { x = iface->getPlayInterface(); }

PythonInterface* PythonInterface::iface;

PyMethodDef PythonInterface::EmbMethods[] = {
	 {"stop", &PythonInterface::stop, METH_VARARGS, ""},
         {"play", &PythonInterface::play, METH_VARARGS, ""},
	 {"setFader", &PythonInterface::setFader, METH_VARARGS, ""},
	 {"test", &PythonInterface::test, METH_VARARGS, ""},
	 {"setTag", &PythonInterface::setTag, METH_VARARGS, ""},
	 {"clearTag", &PythonInterface::clearTag, METH_VARARGS, ""},
 	 {"kill", &PythonInterface::kill, METH_VARARGS, ""},
 	 {"killTag", &PythonInterface::killTag, METH_VARARGS, ""},
	 {"getFader", &PythonInterface::getFader, METH_VARARGS, ""},
	 {"getValue", &PythonInterface::getValue, METH_VARARGS, ""},
	 {"startFadeCrossfader", &PythonInterface::startFadeCrossfader, METH_VARARGS, ""},
	 {"startList", &PythonInterface::startList, METH_VARARGS, ""},
	 {"startFade", &PythonInterface::startFade, METH_VARARGS, ""},
	 {"point", &PythonInterface::point, METH_VARARGS, ""},
	 {"fadePoint", &PythonInterface::fadePoint, METH_VARARGS, ""},
	 {"endFade", &PythonInterface::endFade, METH_VARARGS, ""},
	 {"endList", &PythonInterface::endList, METH_VARARGS, ""},

	 {"playChannel1", &PythonInterface::playChannel1, METH_VARARGS, ""},
	 {"playChannel2", &PythonInterface::playChannel2, METH_VARARGS, ""},

         {NULL, NULL, 0, NULL}
};


PythonInterface::PythonInterface(PlayInterface* pi) {
	m_pi = pi;

	Py_Initialize();
	PyEval_InitThreads();
	PyRun_SimpleString("print 'Python: Python Interpreter created'\n");

	Py_InitModule("mixxx", EmbMethods);
//	PyRun_SimpleString("import mixxx\nmixxx.test()\nmixxx.test()\n");
//	PyRun_SimpleString("mixxx.test()\n");
	PyEval_ReleaseLock();
}

PythonInterface::~PythonInterface() {
	Py_Finalize();
}

void PythonInterface::executeScript(const char* script, int process) {
//	printf(script);
	PyEval_AcquireLock();
	iface->m_pi->setProcess(process);
	PyRun_SimpleString(script);
	iface->m_pi->clearProcess();
	PyEval_ReleaseLock();
}

PythonInterface* PythonInterface::getInterface() {
	return iface;
}

PythonInterface* PythonInterface::initInterface(PlayInterface* pi) {
	if (iface != NULL) {
		printf("Python already initialised!\n");
		return iface;
	}
	iface = new PythonInterface(pi);
	return iface;
}

PlayInterface* PythonInterface::getPlayInterface() {
	return m_pi;
}

PyObject* PythonInterface::play(PyObject* self, PyObject* args) {
	int chan;
	PyArg_ParseTuple(args, "i", &chan);
	PYCHECK(pi);
	pi->play(chan);
	return Py_BuildValue("i", 0);
}

PyObject* PythonInterface::stop(PyObject* self, PyObject* args) {
	int chan;
	PyArg_ParseTuple(args, "i", &chan);
	PYCHECK(pi);
	pi->stop(chan);
	return Py_BuildValue("i", 0);
}

PyObject* PythonInterface::setFader(PyObject* self, PyObject* args) {
	float pos;
	PyArg_ParseTuple(args, "d", &pos);
	PYCHECK(pi);
	pi->setFader(pos);
	return Py_BuildValue("i", 0);
}

PyObject* PythonInterface::test(PyObject* self, PyObject* args) {
	printf("Python: Python Test\n");
	PYCHECK(pi);
	pi->test();
	return Py_BuildValue("i", 0);
}

PyObject* PythonInterface::setTag(PyObject* self, PyObject* args) {
	PYCHECK(pi);
	int tag;
	PyArg_ParseTuple(args, "i", &tag);
	pi->setTag(tag);
	return Py_BuildValue("i", 0);
}

PyObject* PythonInterface::clearTag(PyObject* self, PyObject* args) {
	PYCHECK(pi);
	pi->clearTag();
	return Py_BuildValue("i", 0);
}

PyObject* PythonInterface::killTag(PyObject* self, PyObject* args) {
        PYCHECK(pi);
        int tag;
        PyArg_ParseTuple(args, "i", &tag);
        pi->killTag(tag);
        return Py_BuildValue("i", 0);
}

PyObject* PythonInterface::kill(PyObject* self, PyObject* args) {
        PYCHECK(pi);
        pi->kill();
        return Py_BuildValue("i", 0);
}

PyObject* PythonInterface::getFader(PyObject* self, PyObject* args) {
	PYCHECK(pi);
	return Py_BuildValue("d", pi->getFader());
}

PyObject* PythonInterface::getValue(PyObject* self, PyObject* args) {
	const char* group;
	const char* name;
	PyArg_ParseTuple(args, "ss", &group, &name);
	PYCHECK(pi);
	return Py_BuildValue("d", pi->getValue(group, name));
}

PyObject* PythonInterface::startFadeCrossfader(PyObject* self, PyObject* args) {
	PYCHECK(pi);
	pi->startFadeCrossfader();
	return Py_BuildValue("i", 0);
}

PyObject* PythonInterface::startList(PyObject* self, PyObject* args) {
        const char* group;
        const char* name;
        PyArg_ParseTuple(args, "ss", &group, &name);
        PYCHECK(pi);
	pi->startList(group, name);
	return Py_BuildValue("i", 0);
}

PyObject* PythonInterface::startFade(PyObject* self, PyObject* args) {
	const char* group;
        const char* name;
        PyArg_ParseTuple(args, "ss", &group, &name);
	PYCHECK(pi);
	pi->startFade(group, name);
	return Py_BuildValue("i", 0);
}

PyObject* PythonInterface::point(PyObject* self, PyObject* args) {
	int time;
	double value;
	PyArg_ParseTuple(args, "id", &time, &value);
	PYCHECK(pi);
	pi->point(time, value);
	return Py_BuildValue("i", 0);
}

PyObject* PythonInterface::fadePoint(PyObject* self, PyObject* args) {
        int time;
        double value;
        PyArg_ParseTuple(args, "id", &time, &value);
        PYCHECK(pi);
        pi->point(time, value);
        return Py_BuildValue("i", 0);
}

PyObject* PythonInterface::endFade(PyObject* self, PyObject* args) {
	PYCHECK(pi);
	pi->endFade();
	return Py_BuildValue("i", 0);
}

PyObject* PythonInterface::endList(PyObject* self, PyObject* args) {
	PYCHECK(pi);
	pi->endList();
	return Py_BuildValue("i", 0);
}

PyObject* PythonInterface::playChannel1(PyObject* self, PyObject* args) {
        int time;
        char *path;
        PyArg_ParseTuple(args, "is", &time, &path);
        PYCHECK(pi);
        pi->playChannel1(time, path);
        return Py_BuildValue("i", 0);
}

PyObject* PythonInterface::playChannel2(PyObject* self, PyObject* args) {
        int time;
        char* path;
        PyArg_ParseTuple(args, "is", &time, &path);
        PYCHECK(pi);
        pi->playChannel2(time, path);
        return Py_BuildValue("i", 0);
}

