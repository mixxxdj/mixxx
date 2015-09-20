#include <QApplication>
#include <QtDebug>

#include "controlobjectscript.h"

ControlObjectScript::ControlObjectScript(const ConfigKey& key, QObject* pParent)
        : ControlObjectSlave(key, pParent) {
}

ControlObjectScript::~ControlObjectScript() {
    //qDebug() << "ControlObjectScript::~ControlObjectSlave()";
}
