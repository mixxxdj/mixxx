#include <QApplication>
#include <QtDebug>

#include "controlobjectscript.h"


ControlObjectScript::ControlObjectScript(const QString& g, const QString& i, QObject* pParent)
        : ControlObjectSlave(g, i, pParent) {
}

ControlObjectScript::~ControlObjectScript() {
    //qDebug() << "ControlObjectScript::~ControlObjectSlave()";
}
