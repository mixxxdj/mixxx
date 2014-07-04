// libraryfeature.cpp
// Created 8/17/2009 by RJ Ryan (rryan@mit.edu)

#include "library/libraryfeature.h"
#include "skin/skinloader.h"

// KEEP THIS cpp file to tell scons that moc should be called on the class!!!
// The reason for this is that LibraryFeature uses slots/signals and for this
// to work the code has to be precompiles by moc
LibraryFeature::LibraryFeature(QObject* parent, ConfigObject<ConfigValue>* pConfig)
              : QObject(parent),
                m_pConfig(pConfig) {
}

LibraryFeature::~LibraryFeature() {
    
}

QIcon LibraryFeature::getIcon() {
    QString current_icon = getIconName();
    SkinLoader sl(m_pConfig);
    qDebug() << "++++++++++ getIcon()";
    QString current_skin = sl.getConfiguredSkinPath();
    current_skin.append(current_icon);
    QFile Fout(current_skin);
    if(Fout.exists()){
        return QIcon(current_skin);
    }else{
        return QIcon(":/images/library/"+current_icon);
    }
}
