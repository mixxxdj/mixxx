#include <QtCore>
#include <QtXml>

#include "mixxxcontrol.h"

#ifdef __MIDISCRIPT__
#include <QScriptValue>
#endif

#include "controllers/mixxxcontrol.h"

MixxxControl::MixxxControl(QString controlobject_group, QString controlobject_item,
                           QString controlobject_description)
        : m_sGroup(controlobject_group),
          m_sItem(controlobject_item),
          m_sDescription(controlobject_description) {
    m_scriptFunction = QScriptValue();
}

/** Constructor that deserializes a MixxxControl object from a <control> or <output>
    node block in our MIDI mapping XML file.
*/
/*
MixxxControl::MixxxControl(QDomElement& parentNode, bool isOutputNode) {
    // TODO
}

void MixxxControl::serializeToXML(QDomElement& parentNode, bool isOutputNode) const
{
    // TODO
}
*/

uint qHash(const MixxxControl& key) {
    return qHash(key.group() + key.item());
}
