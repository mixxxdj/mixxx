/***************************************************************************
                          guicontainer.cpp  -  description
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

#include "guicontainer.h"
#include "fastvertexarray.h"
#include "signalvertexbuffer.h"
#include "guisignal.h"
#include "visualsignal.h"
#include "../defs.h"
#include "../readerextract.h"

// Static members:
Light GUIContainer::mylight;
Material GUIContainer::dblue, GUIContainer::lblue, GUIContainer::purple, GUIContainer::lgreen;

/**
 * Default Consructor.
 */
GUIContainer::GUIContainer(ReaderExtract *readerExtract, ControlPotmeter *playpos)
{
    qDebug("-1");
    setupScene();
    atBasepos = true;
    movement = false;

    // Calculate resampling factor
    CSAMPLE signalRate = (CSAMPLE)readerExtract->getRate();
    CSAMPLE factor = 1.;
    if (DISPLAYRATE<signalRate)
        factor = DISPLAYRATE/signalRate;

    // Chunk size used in vertex buffer****************
    int chunkSize = factor*readerExtract->getBufferSize()/READCHUNK_NO; // = (int)(soundBuffer->getChunkSize()*factor);
    
    vertex = new FastVertexArray();
    vertex->init(chunkSize, READCHUNK_NO);

    // Create objects
    buffer = new SignalVertexBuffer(readerExtract, playpos, vertex);
    //***********************
    signal = new GUISignal(buffer,vertex,"" /*engineBuffer->getGroup()*/);
    
    signal->setBoxMaterial(&dblue);
    signal->setBoxWireMaterial(&lblue);
    signal->setSignalMaterial(&lgreen);
    signal->setFishEyeSignalMaterial(&lgreen);
    signal->setPlayPosMarkerMaterial(&purple);
    signal->setFishEyeMode(true);
}

GUIContainer::~GUIContainer()
{
}

int GUIContainer::getId()
{
    return signal->getId();
}

GUISignal *GUIContainer::getSignal()
{
    return signal;
}

SignalVertexBuffer *GUIContainer::getBuffer()
{
    return buffer;
}

void GUIContainer::setBasepos(float x, float y, float z)
{
    basex = x;
    basey = y;
    basez = z;

    if (atBasepos)
    {
        signal->setOrigo(basex, basey, basez);
        signal->setLength(baselength);
        signal->setHeight(baseheight);
        signal->setDepth(basedepth);
    }
}

void GUIContainer::setZoompos(float x, float y, float z)
{
    zoomx = x;
    zoomy = y;
    zoomz = z;

    if (!atBasepos)
    {
        signal->setOrigo(zoomx, zoomy, zoomz);
        signal->setLength(zoomlength);
        signal->setHeight(zoomheight);
        signal->setDepth(zoomdepth);
    }
}

void GUIContainer::zoom()
{
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

void GUIContainer::zoom(float ox, float oy, float oz, float _length, float _height, float _depth)
{
    destx = ox;
    desty = oy;
    destz = oz;
    destl = _length;
    desth = _height;
    destd = _depth;
    
    movement = true;
}


void GUIContainer::move(int msec)
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
            signal->setDepth(updd);

        signal->setOrigo(updx,updy,updz);
        signal->setLength(updl);
        signal->setHeight(updh);
        signal->setDepth(updd);

        // Check if destination is reached
        if (updx==destx && updy==desty && updz==destz && updl==destl && updh==desth && updd==destd)
            movement = false;
    }
}

void GUIContainer::setupScene()
{
    mylight.ambient[0]  = 1.f;
    mylight.ambient[1]  = 1.f;
    mylight.ambient[2]  = 1.f;
    mylight.ambient[3]  = 1.f;
    
    mylight.diffuse[0]  = 1.f;
    mylight.diffuse[1]  = 1.f;
    mylight.diffuse[2]  = 1.f;
    mylight.diffuse[3]  = 1.f;

    mylight.specular[0] = 1.f;
    mylight.specular[1] = 1.f;
    mylight.specular[2] = 1.f;
    mylight.specular[3] = 1.f;

    mylight.position[0] = 100.0f;
    mylight.position[1] = 100.0f;
    mylight.position[2] = 180.0f;
    mylight.position[3] = 1.0f;

    mylight.enable();

    dblue.ambient[0] = 0.0f;
    dblue.ambient[1] = 0.0f;
    dblue.ambient[2] = 0.0f;
    dblue.ambient[3] = 1.0f;
    
    dblue.diffuse[0] = 0./255.;
    dblue.diffuse[1] = 0./255.;
    dblue.diffuse[2] = 255./255.;
    dblue.diffuse[3] = 0.1f;
    
    dblue.specular[0] = 0.4f;
    dblue.specular[1] = 0.4f;
    dblue.specular[2] = 0.4f;
    dblue.specular[3] = 1.0f;
    
    dblue.shininess = 128;

    lblue.ambient[0] = 0.0f;
    lblue.ambient[1] = 0.0f;
    lblue.ambient[2] = 0.0f;
    lblue.ambient[3] = 1.0f;
    
    lblue.diffuse[0] = 10/255.f;
    lblue.diffuse[1] = 20/255.f;
    lblue.diffuse[2] = 130/255.f;
    lblue.diffuse[3] = 1.0f;
   
    lblue.specular[0] = 0.0f;
    lblue.specular[1] = 0.0f;
    lblue.specular[2] = 0.01f;
    lblue.specular[3] = 1.0f;
    
    lblue.shininess = 0;

    purple.ambient[0] = 62/255.f;
    purple.ambient[1] = 0.0f;
    purple.ambient[2] = 62/255.f;
    purple.ambient[3] = 1.0f;
    
    purple.diffuse[0] = 62/255.f;
    purple.diffuse[1] = 0/255.f;
    purple.diffuse[2] = 62/255.f;
    purple.diffuse[3] = 0.f;
    
    purple.specular[0] = 62/255.f;
    purple.specular[1] = 0.0f;
    purple.specular[2] = 62/255.f;
    purple.specular[3] = 1.0f;
    
    purple.shininess = 128.f;
    
    lgreen.ambient[0] = 0.0f;
    lgreen.ambient[1] = 0.0f;
    lgreen.ambient[2] = 0.0f;
    lgreen.ambient[3] = 1.0f;
    
    lgreen.diffuse[0] = 90/255.f;
    lgreen.diffuse[1] = 255/255.f;
    lgreen.diffuse[2] = 90/255.f;
    lgreen.diffuse[3] = 1.0f;
    
    lgreen.specular[0] = 0.3f;
    lgreen.specular[1] = 0.3f;
    lgreen.specular[2] = 0.3f;
    lgreen.specular[3] = 1.0f;
    
    lgreen.shininess = 128;
};

