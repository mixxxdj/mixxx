#ifndef KEYUTILS_H
#define KEYUTILS_H

#include <QString>

#include "proto/keys.pb.h"

mixxx::track::io::key::ChromaticKey guessKeyFromText(const QString& text);


#endif /* KEYUTILS_H */
