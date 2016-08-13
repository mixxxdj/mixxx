#include <QRegExp>
#include <QString>

#include "stringhelper.h"

StringHelper::StringHelper() {

}

QChar StringHelper::getFirstCharForGrouping(const QString& text) {
    if (text.size() <= 0) {
        return QChar();
    }
    
    QChar c = text.at(0);
    if (!c.isLetter()) {
        return c;
    }
    c = c.toUpper();
    QString letter(c);
    
    // This removes the accents of the characters
    QString s1 = letter.normalized(QString::NormalizationForm_KD);    
    if (s1.size() > 0) {
        if (QString::localeAwareCompare(s1, letter) != 0) {
            // The letter does not sort together with it's normalized form,
            // it is due to Chinese letters or Finnish sorting. In Finnish the 
            // correct sorting is a-z, å, ä, ö and the user will expect these 
            // letters at the end and not like this a, ä, å, b-o, ö, p-z
            return c;
        }
        
        c = s1.at(0).toUpper();
    }
    return c;
}

