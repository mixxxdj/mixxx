#include <QDebug>
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
    QString normChar(s1.at(0));
    QString s1_1 = normChar + "d";
    QString s1_2 = normChar + "f";
    QString letter_1 = s1 + "e";
    if (s1.size() > 0) {
        // We do the following checking: Ad < Äe < Af
        // because locale aware compare takes in account the next character.
        // This is due to Chinese letters or Finnish sorting. In Finnish the 
        // correct sorting is a-z, å, ä, ö and the user will expect these 
        // letters at the end and not like this a, ä, å, b-o, ö, p-z
        
        //qDebug() << "Checking " << s1_1 << "<" << letter_1 << "<" << s1_2;
        if (s1_1.localeAwareCompare(letter_1) < 0 && 
            letter_1.localeAwareCompare(s1_2) < 0) {
            c = s1.at(0).toUpper();
        }
    }
    return c;
}

