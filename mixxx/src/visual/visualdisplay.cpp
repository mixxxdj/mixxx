/***************************************************************************
                          visualdisplay.cpp  -  description
                             -------------------
    copyright            : (C) 2002 by Tue and Ken Haste Andersen and Kenny
                                       Erleben
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

#include "visualdisplay.h"
#include "../controlpotmeter.h"
#include <math.h>

// Static members
int VisualDisplay::idCount = 0;

VisualDisplay::VisualDisplay(VisualBuffer *pVisualBuffer, const char *type, const char *group, bool drawBox)
{
    m_pVisualBuffer = pVisualBuffer;

    idCount++;
    id = idCount;
    
    // Rotation variables
    angle = 0; rx = 1;ry=0;rz=0;

    // Origo of visual signal
    ox = oy = oz = 0;

    // Fish eye mode
    fishEyeMode = false;

    // Relative length of the fish eye signal
    fishEyeLengthScale = 0.25f;
    // Fraction of the signal shown in the fish eye
    fishEyeSignalFraction = 0.02f;
    // Relative height of signal
    signalScaleHeight = 0.9f;
    // Relative length of signal
    signalScaleLength = 1.0f;

    fishEyeSignal = new VisualDisplayBuffer(m_pVisualBuffer);
    preSignal     = new VisualDisplayBuffer(m_pVisualBuffer);
    postSignal    = new VisualDisplayBuffer(m_pVisualBuffer);
    signal        = new VisualDisplayBuffer(m_pVisualBuffer);

    box = new VisualBox(id);
    m_bDrawBox = drawBox;

    if (QString(type)=="marks")
    {
        fishEyeSignal->setMaterial(&m_materialBeat);
        preSignal->setMaterial(&m_materialBeat);
        postSignal->setMaterial(&m_materialBeat);
        signal->setMaterial(&m_materialBeat);
        box->setMaterial(&m_materialBeat);
        playPosMarker = 0;
    }
    else if (QString(type)=="signal")
    {
        fishEyeSignal->setMaterial(&m_materialSignal);
        preSignal->setMaterial(&m_materialSignal);
        postSignal->setMaterial(&m_materialSignal);
        signal->setMaterial(&m_materialSignal);
        box->setMaterial(&m_materialFisheye);
        playPosMarker = new VisualBox(id);
        playPosMarker->setMaterial(&m_materialMarker);
    }
    else if (QString(type)=="hfc")
    {
        qDebug("hfc");
        fishEyeSignal->setMaterial(&m_materialHfc);
        preSignal->setMaterial(&m_materialHfc);
        postSignal->setMaterial(&m_materialHfc);
        signal->setMaterial(&m_materialHfc);
        box->setMaterial(&m_materialHfc);
        playPosMarker = 0;
    }
    qDebug("type %s", type);

    doLayout();

    QString id("VisualLengthScale-");
    id.append(type);
    controlScaleLength = new ControlPotmeter(ConfigKey(group,id.latin1()),-5.f,5.f);
    connect(controlScaleLength, SIGNAL(signalUpdateApp(double)), this, SLOT(setSignalScaleLength(double)));

}

VisualDisplay::~VisualDisplay()
{
    delete box;
    if (playPosMarker)
        delete playPosMarker;
}

int VisualDisplay::getId()
{
    return id;
}

void VisualDisplay::draw(GLenum mode)
{
    doLayout();

    bufInfo b = m_pVisualBuffer->getVertexArray();


    if(fishEyeMode)
    {
        // TOTAL LENGTH SHOULD EQUAL b.len1+b.len2 but seems to give b-1!!!!

        int nfish = (int)((b.len1+b.len2)*fishEyeSignalFraction);
        int noutside = ((b.len1+b.len2)-nfish)/2;
//      int n = ((b.len1+b.len2)/3)+1;
        bufInfo i1, i2, i3;

        i1.corr = b.corr;
        i1.p1 = b.p1;
        i1.p2 = b.p2;
        i1.len1 = max(0,min(b.len1, noutside));
        i1.len2 = noutside-i1.len1;
        //std::cout << "i1: len1: " << i1.len1 << ", len2: " << i1.len2 << "\n";

        i2.corr = 0.;//b.corr;
        i2.p1 = i1.p1+noutside*3;
        i2.p2 = i1.p2+(i1.len2*3);
        i2.len1 = max(0,min(b.len1-noutside,nfish));
        i2.len2 = nfish-i2.len1;
        //std::cout << "i2: len1: " << i2.len1 << ", len2: " << i2.len2 << "\n";

        i3.corr = b.corr;
        i3.p1 = i2.p1+nfish*3;
        i3.p2 = i2.p2+(i2.len2*3);
        i3.len1 = max(0,min(b.len1-nfish-noutside,noutside));
        i3.len2 = noutside-i3.len1;
        //std::cout << "i3: len1: " << i3.len1 << ", len2: " << i3.len2 << "\n";

        preSignal->setBuffer(i1);
        fishEyeSignal->setBuffer(i2);
        postSignal->setBuffer(i3);

        preSignal->draw(mode);
        postSignal->draw(mode);
        fishEyeSignal->draw(mode);
    }
    else
    {
        signal->setBuffer(b);
        signal->draw(mode);
    }

/*
//    if (boxWireMaterial)
//        box->setMaterial(boxWireMaterial);
    box->setDrawMode(GL_LINE_LOOP);
    box->draw(mode);

//    if (boxMaterial)
//    box->setMaterial(&m_materialMarker);
    box->setDrawMode(GL_POLYGON);
    box->draw(mode);
*/

    if (playPosMarker && m_bDrawBox)
    {
        playPosMarker->setDrawMode(GL_POLYGON);
        playPosMarker->draw(mode);
    }

    // Only draw box in fish eye mode
    if (fishEyeMode && m_bDrawBox)
    {
        box->setMaterial(&m_materialFisheye);
        box->setDrawMode(GL_POLYGON);
        box->draw(mode);
    }
//    if (playPosMarkerMaterial)

}

void VisualDisplay::draw()
{
}

void VisualDisplay::toggleFishEyeMode()
{
    fishEyeMode = !fishEyeMode;
}

void VisualDisplay::setFishEyeLengthScale(double scale)
{
    fishEyeLengthScale = 1.+scale;
}

void VisualDisplay::setFishEyeSignalFraction(double fraction)
{
    fishEyeSignalFraction = fraction;
}

void VisualDisplay::setSignalScaleHeight(double scale)
{
    signalScaleHeight = scale;
}

void VisualDisplay::setSignalScaleLength(double scale)
{


    signalScaleLength = 1./(1.+(ControlObject::getControl(ConfigKey("[Channel1]","rate_dir"))->getValue()*scale));
    //qDebug("scale input %f, actual %f",scale, signalScaleLength);
}

void VisualDisplay::setColorSignal(float r, float g, float b)
{
    m_materialSignal.ambient[0] = r;
    m_materialSignal.ambient[1] = g;
    m_materialSignal.ambient[2] = b;
    m_materialSignal.ambient[3] = 1.0f;

    m_materialSignal.diffuse[0] = r;
    m_materialSignal.diffuse[1] = g;
    m_materialSignal.diffuse[2] = b;
    m_materialSignal.diffuse[3] = 1.0f;

    m_materialSignal.specular[0] = r;
    m_materialSignal.specular[1] = g;
    m_materialSignal.specular[2] = b;
    m_materialSignal.specular[3] = 1.0f;

    m_materialSignal.shininess = 128;
}

void VisualDisplay::setColorHfc(float r, float g, float b)
{
    m_materialHfc.ambient[0] = r;
    m_materialHfc.ambient[1] = g;
    m_materialHfc.ambient[2] = b;
    m_materialHfc.ambient[3] = 1.0f;

    m_materialHfc.diffuse[0] = r;
    m_materialHfc.diffuse[1] = g;
    m_materialHfc.diffuse[2] = b;
    m_materialHfc.diffuse[3] = 1.0f;

    m_materialHfc.specular[0] = r;
    m_materialHfc.specular[1] = g;
    m_materialHfc.specular[2] = b;
    m_materialHfc.specular[3] = 1.0f;

    m_materialHfc.shininess = 128;
}

void VisualDisplay::setColorBeat(float r, float g, float b)
{
    m_materialBeat.ambient[0] = r;
    m_materialBeat.ambient[1] = g;
    m_materialBeat.ambient[2] = b;
    m_materialBeat.ambient[3] = 1.0f;

    m_materialBeat.diffuse[0] = r;
    m_materialBeat.diffuse[1] = g;
    m_materialBeat.diffuse[2] = b;
    m_materialBeat.diffuse[3] = 1.0f;

    m_materialBeat.specular[0] = r;
    m_materialBeat.specular[1] = g;
    m_materialBeat.specular[2] = b;
    m_materialBeat.specular[3] = 1.0f;

    m_materialBeat.shininess = 128;
}

void VisualDisplay::setColorMarker(float r, float g, float b)
{
    m_materialMarker.ambient[0] = r;
    m_materialMarker.ambient[1] = g;
    m_materialMarker.ambient[2] = b;
    m_materialMarker.ambient[3] = 1.0f;

    m_materialMarker.diffuse[0] = r;
    m_materialMarker.diffuse[1] = g;
    m_materialMarker.diffuse[2] = b;
    m_materialMarker.diffuse[3] = 1.0f;

    m_materialMarker.specular[0] = r;
    m_materialMarker.specular[1] = g;
    m_materialMarker.specular[2] = b;
    m_materialMarker.specular[3] = 1.0f;

    m_materialMarker.shininess = 128;
}

void VisualDisplay::setColorFisheye(float r, float g, float b)
{
    m_materialFisheye.ambient[0] = r;
    m_materialFisheye.ambient[1] = g;
    m_materialFisheye.ambient[2] = b;
    m_materialFisheye.ambient[3] = 1.0f;

    m_materialFisheye.diffuse[0] = r;
    m_materialFisheye.diffuse[1] = g;
    m_materialFisheye.diffuse[2] = b;
    m_materialFisheye.diffuse[3] = 1.0f;

    m_materialFisheye.specular[0] = r;
    m_materialFisheye.specular[1] = g;
    m_materialFisheye.specular[2] = b;
    m_materialFisheye.specular[3] = 1.0f;

    m_materialFisheye.shininess = 128;
}

void VisualDisplay::doLayout()
{
    //--- Argh, in order to support rotations we must rotate the n-vector!!!
    float nx = 1;
    float ny = 0;
    float nz = 0;

    // Set length, position and offset of signal to be displayed
    float curOx = ox-(kfVisualDisplayLength*signalScaleLength/2.);

    if(fishEyeMode)
    {
        float fishEyeLength = fishEyeLengthScale*kfVisualDisplayLength;
        float nonFishEyeLength = (signalScaleLength*kfVisualDisplayLength-fishEyeLength)/2.0f;

        float ox2 = curOx + nx*nonFishEyeLength;
        float oy2 = oy + ny*nonFishEyeLength;
        float oz2 = oz + nz*nonFishEyeLength;
        float ox3 = ox2 + nx*fishEyeLength;
        float oy3 = oy2 + ny*fishEyeLength;
        float oz3 = oz2 + nz*fishEyeLength;

        preSignal->setOrigo(curOx,oy,oz);
        preSignal->setLength(nonFishEyeLength);
        preSignal->setHeight(kfVisualDisplayHeight*signalScaleHeight);
        preSignal->setRotation(angle,rx,ry,rz);

        fishEyeSignal->setOrigo(ox2,oy2,oz2);
        fishEyeSignal->setLength(fishEyeLength);
        fishEyeSignal->setHeight(kfVisualDisplayHeight*signalScaleHeight);
        fishEyeSignal->setRotation(angle,rx,ry,rz);

        postSignal->setOrigo(ox3,oy3,oz3);
        postSignal->setLength(nonFishEyeLength);
        postSignal->setHeight(kfVisualDisplayHeight*signalScaleHeight);
        postSignal->setRotation(angle,rx,ry,rz);


        box->setOrigo(ox2,oy2,oz2-kfVisualDisplayDepth);
        box->setLength(fishEyeLength);
        box->setHeight(kfVisualDisplayHeight*2.);
        box->setDepth(kfVisualDisplayDepth);
        box->setRotation(angle,rx,ry,rz);
    }
    else
    {
        signal->setOrigo(curOx,oy,oz);
        signal->setLength(kfVisualDisplayLength*signalScaleLength);
        signal->setHeight(kfVisualDisplayHeight*signalScaleHeight);
        signal->setRotation(angle,rx,ry,rz);
    
        box->setOrigo(curOx,oy,oz);
        box->setLength(0.);
        box->setHeight(kfVisualDisplayHeight*2.);
        box->setDepth(kfVisualDisplayDepth);
        box->setRotation(angle,rx,ry,rz);
    }

    if (playPosMarker)
    {
        playPosMarker->setOrigo(ox,oy,oz);
        playPosMarker->setLength(kfVisualDisplayLength/400.);
        playPosMarker->setHeight(kfVisualDisplayHeight*2.);
        playPosMarker->setDepth(kfVisualDisplayDepth);
        playPosMarker->setRotation(angle,rx,ry,rz);
    }
}

void VisualDisplay::setRotation(float angle, float rx,float ry,float rz)
{
    this->angle = angle;
    this->rx = rx;
    this->ry = ry;
    this->rz = rz;
}

