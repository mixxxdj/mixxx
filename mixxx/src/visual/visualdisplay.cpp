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

    atBasepos = true;
    movement = false;

    idCount++;
    id = idCount;

    length = baselength;
    height = baseheight;
    depth  = basedepth;

    angle = 0;
    rx = 1;ry=0;rz=0;

    ox = -25;
    oy = oz = 0;

    fishEyeMode = false;

    fishEyeLengthScale = 0.15f;
    fishEyeSignalFraction = 0.01f;
    signalScaleHeight = 1.0f;
    signalScaleLength = 1.0f;

    fishEyeSignal = new VisualDisplayBuffer(m_pVisualBuffer);
    preSignal     = new VisualDisplayBuffer(m_pVisualBuffer);
    postSignal    = new VisualDisplayBuffer(m_pVisualBuffer);
    signal        = new VisualDisplayBuffer(m_pVisualBuffer);

    box = new VisualBox(id);
    m_bDrawBox = drawBox;
                        
//    setBoxWireMaterial(&m_materialBeat);

    if (QString(type)=="marks")
    {
        fishEyeSignal->setMaterial(&m_materialBeat);
        preSignal->setMaterial(&m_materialBeat);
        postSignal->setMaterial(&m_materialBeat);
        signal->setMaterial(&m_materialBeat);
        box->setMaterial(&m_materialBeat);
        playPosMarker = 0;
    }
    else
    {
        fishEyeSignal->setMaterial(&m_materialSignal);
        preSignal->setMaterial(&m_materialSignal);
        postSignal->setMaterial(&m_materialSignal);
        signal->setMaterial(&m_materialSignal);
        box->setMaterial(&m_materialFisheye);
        playPosMarker = new VisualBox(id);
        playPosMarker->setMaterial(&m_materialMarker);
    }

    doLayout();

    QString id("VisualLengthScale-");
    id.append(type);
    controlScaleLength = new ControlPotmeter(ConfigKey(group,id.latin1()),0.9f,1.1f);
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

        i1.p1 = b.p1;
        i1.p2 = b.p2;
        i1.len1 = max(0,min(b.len1, noutside));
        i1.len2 = noutside-i1.len1;
        //std::cout << "i1: len1: " << i1.len1 << ", len2: " << i1.len2 << "\n";

        i2.p1 = i1.p1+noutside*3;
        i2.p2 = i1.p2+(i1.len2*3);
        i2.len1 = max(0,min(b.len1-noutside,nfish));
        i2.len2 = nfish-i2.len1;
        //std::cout << "i2: len1: " << i2.len1 << ", len2: " << i2.len2 << "\n";

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

void VisualDisplay::setBasepos(float x, float y, float z)
{
    basex = x;
    basey = y;
    basez = z;

    if (atBasepos)
    {
        ox = x;
        oy = y;
        oz = z;
    }
}

void VisualDisplay::setZoompos(float x, float y, float z)
{
    zoomx = x;
    zoomy = y;
    zoomz = z;

    if (!atBasepos)
    {
        ox = x;
        oy = y;
        oz = z;
    }
}

void VisualDisplay::zoom()
{
    qDebug("zoom");
    if (atBasepos)
    {
        zoom(zoomx,zoomy,zoomz,zoomlength,zoomheight,zoomdepth);
        atBasepos = false;
    }
    else
    {
        zoom(basex,basey,basez,baselength,baseheight,basedepth);
        atBasepos = true;
    }
}

void VisualDisplay::move(int msec)
{
    if (movement)
    {
            float x = signal->getOrigoX();
            float y = signal->getOrigoY();
            float z = signal->getOrigoZ();
            float l = signal->getLength();
            float h = signal->getHeight();
            float d = signal->getDepth();
            float dx = msec*(destx - x)/100.;
            float dy = msec*(desty - y)/100.;
            float dz = msec*(destz - z)/100.;
            float dl = msec*(destl - l)/100.;
            float dh = msec*(desth - h)/100.;
            float dd = msec*(destd - d)/100.;
            float updx = x+dx;
            float updy = y+dy;
            float updz = z+dz;
            float updl = l+dl;
            float updh = h+dh;
            float updd = d+dd;
            signal->setOrigo(updx,updy,updz);
            signal->setLength(updl);
            signal->setHeight(updh);
//            signal->setDepth(updd);

//       qDebug("updd %f, origo %f,%f,%f, len: %f, hei: %f",updd);

//        signal->setOrigo(updx,updy,updz);
//        signal->setLength(updl);
//        signal->setHeight(updh);
//        signal->setDepth(updd);

        // Check if destination is reached
        if (updx==destx && updy==desty && updz==destz && updl==destl && updh==desth && updd==destd)
            movement = false;
    }
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
//    qDebug("scale input %f, actual %f",scale, signalScaleLength);
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
    float curLength = length*signalScaleLength*1.5;
    float curOx = ox+(length-curLength)/2;
    
    if(fishEyeMode)
    {
        float fishEyeLength = fishEyeLengthScale*length;
        float nonFishEyeLength = (curLength- fishEyeLength)/2.0f;

        float ox2 = curOx + nx*nonFishEyeLength;
        float oy2 = oy + ny*nonFishEyeLength;
        float oz2 = oz + nz*nonFishEyeLength;
        float ox3 = ox2 + nx*fishEyeLength;
        float oy3 = oy2 + ny*fishEyeLength;
        float oz3 = oz2 + nz*fishEyeLength;

        preSignal->setOrigo(curOx,oy,oz);
        preSignal->setLength(nonFishEyeLength);
        preSignal->setHeight(signalScaleHeight*height*2);
        preSignal->setRotation(angle,rx,ry,rz);

        fishEyeSignal->setOrigo(ox2,oy2,oz2);
        fishEyeSignal->setLength(fishEyeLength);
        fishEyeSignal->setHeight(signalScaleHeight*height*2);
        fishEyeSignal->setRotation(angle,rx,ry,rz);

        postSignal->setOrigo(ox3,oy3,oz3);
        postSignal->setLength(nonFishEyeLength);
        postSignal->setHeight(signalScaleHeight*height*2);
        postSignal->setRotation(angle,rx,ry,rz);


        box->setOrigo(ox2,oy2,oz2-5);
        box->setLength(fishEyeLength);
        box->setHeight(signalScaleHeight*height);
        box->setDepth(depth);
        box->setRotation(angle,rx,ry,rz);

    }
    else
    {
        signal->setOrigo(curOx,oy,oz);
        signal->setLength(curLength);
        signal->setHeight(signalScaleHeight*height*2);
        signal->setRotation(angle,rx,ry,rz);
    
        box->setOrigo(curOx,oy,oz);
        box->setLength(curLength);
        box->setHeight(height);
        box->setDepth(depth);
        box->setRotation(angle,rx,ry,rz);
    }

    float plength = 2.;
    float pheight = height*1.2;
    float pdepth = depth*1.05;

    float offset = (curLength-plength)/2.0f;

    float px = curOx + nx*offset; 
    float py = oy + ny*offset;
    float pz = oz + nz*offset;

    if (playPosMarker)
    {
        playPosMarker->setOrigo(px,py,pz);
        playPosMarker->setLength(plength);
        playPosMarker->setHeight(pheight);
        playPosMarker->setDepth(pdepth);
        playPosMarker->setRotation(angle,rx,ry,rz);
    }
}

void VisualDisplay::zoom(float ox, float oy, float oz, float _length, float _height, float _depth)
{
    destx = ox;
    desty = oy;
    destz = oz;
    destl = _length;
    desth = _height;
    destd = _depth;

    movement = true;
}

void VisualDisplay::setRotation(float angle, float rx,float ry,float rz)
{
    this->angle = angle;
    this->rx = rx;
    this->ry = ry;
    this->rz = rz;
}

void VisualDisplay::setLength(float l)
{
//    qDebug("l %f",l);
    length = l;
}

void VisualDisplay::setHeight(float h)
{
    height = h;
}


