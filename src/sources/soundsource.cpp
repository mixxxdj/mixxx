/***************************************************************************
 soundsource.cpp  -  description
 -------------------
 begin                : Wed Feb 20 2002
 copyright            : (C) 2002 by Tue and Ken Haste Andersen
 email                :
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "sources/soundsource.h"
#include "metadata/trackmetadata.h"

namespace Mixxx {

/*static*/ QString SoundSource::getTypeFromUrl(QUrl url) {
    return url.toString().section(".", -1).toLower().trimmed();
}

SoundSource::SoundSource(QUrl url)
        : m_url(url),
          m_type(getTypeFromUrl(url)) {
    DEBUG_ASSERT(getUrl().isValid());
}

SoundSource::SoundSource(QUrl url, QString type)
        : m_url(url),
          m_type(type) {
    DEBUG_ASSERT(getUrl().isValid());
}

SoundSource::~SoundSource() {
}

} //namespace Mixxx
