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
#include "material.h"
#include <math.h>

// Static members
Material VisualDisplay::dblue, VisualDisplay::lblue, VisualDisplay::purple, VisualDisplay::lgreen;
int VisualDisplay::idCount = 0;

VisualDisplay::VisualDisplay(VisualBuffer *pVisualBuffer, const char *group)
{
    m_pVisualBuffer = pVisualBuffer;

    setupScene();
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

    fishEyeLengthScale = 0.5f;
    fishEyeSignalFraction = 0.2f;
    signalScale = 1.0f;

    fishEyeSignal = new VisualDisplayBuffer(m_pVisualBuffer);
    preSignal     = new VisualDisplayBuffer(m_pVisualBuffer);
    postSignal    = new VisualDisplayBuffer(m_pVisualBuffer);
    signal        = new VisualDisplayBuffer(m_pVisualBuffer);
    box = new VisualBox(id);
    playPosMarker = new VisualBox(id);
        
//    setBoxWireMaterial(&lblue);
    fishEyeSignal->setMaterial(&lgreen);
    preSignal->setMaterial(&lgreen);
    postSignal->setMaterial(&lgreen);
    signal->setMaterial(&lgreen);
    box->setMaterial(&lblue);
    playPosMarker->setMaterial(&purple);    

    setFishEyeMode(true);

    doLayout();

//    boxMaterial = boxWireMaterial = 0;
//    playPosMarkerMaterial = 0;

    sliderFishEyeSignalFraction = new ControlPotmeter(ConfigKey(group,"FishEyeSignalFraction"),0.001f,0.5f);
//  connect(sliderFishEyeSignalFraction, SIGNAL(valueChanged(FLOAT_TYPE)), this, SLOT(setFishEyeSignalFraction(FLOAT_TYPE)));

    sliderFishEyeLengthScale = new ControlPotmeter(ConfigKey(group,"FishEyeLengthScale"),0.05f,0.95f);
//  connect(sliderFishEyeLengthScale, SIGNAL(valueChanged(FLOAT_TYPE)), this, SLOT(setFishEyeLengthScale(FLOAT_TYPE)));
}

VisualDisplay::~VisualDisplay()
{
    delete box;
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
    
        int nfish =     (b.len1+b.len2)*fishEyeSignalFraction;
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

//    if (boxWireMaterial)
//        box->setMaterial(boxWireMaterial);
    box->setDrawMode(GL_LINE_LOOP);
    box->draw(mode);

//    if (boxMaterial)
//    box->setMaterial(&purple);
    box->setDrawMode(GL_POLYGON);
    box->draw(mode);


//    if (playPosMarkerMaterial)
//        playPosMarker->setMaterial(playPosMarkerMaterial);
    playPosMarker->setDrawMode(GL_POLYGON);
    playPosMarker->draw(mode);
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

void VisualDisplay::setFishEyeMode(bool value)
{
    fishEyeMode = value;
}

void VisualDisplay::setFishEyeLengthScale(FLOAT_TYPE scale)
{
    fishEyeLengthScale = scale;
}

void VisualDisplay::setFishEyeSignalFraction(FLOAT_TYPE fraction)
{
    fishEyeSignalFraction = fraction;
}

void VisualDisplay::setSignalScale(float scale)
{
    signalScale = scale;
}



void VisualDisplay::setupScene()
{
    dblue.ambient[0] = 0.f;
    dblue.ambient[1] = 0.f;
    dblue.ambient[2] = 62/255.f;
    dblue.ambient[3] = 0.5f;

    dblue.diffuse[0] = 0.f;
    dblue.diffuse[1] = 0.f;
    dblue.diffuse[2] = 62/255.f;
    dblue.diffuse[3] = 0.5f;

    dblue.specular[0] = 0.f;
    dblue.specular[1] = 0.0f;
    dblue.specular[2] = 62/255.f;
    dblue.specular[3] = 0.5f;

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

    lgreen.ambient[0] = 0.0f;
    lgreen.ambient[1] = 255.f;
    lgreen.ambient[2] = 0.0f;
    lgreen.ambient[3] = 1.f;

    lgreen.diffuse[0] = 0.f;
    lgreen.diffuse[1] = 255.f;
    lgreen.diffuse[2] = 0.f;
    lgreen.diffuse[3] = 1.f;

    lgreen.specular[0] = 0.f;
    lgreen.specular[1] = 255.f;
    lgreen.specular[2] = 0.f;
    lgreen.specular[3] = 1.f;

    lgreen.shininess = 128;

    purple.ambient[0] = 62/255.f;
    purple.ambient[1] = 0.0f;
    purple.ambient[2] = 62/255.f;
    purple.ambient[3] = 1.0f;

    purple.diffuse[0] = 62/255.f;
    purple.diffuse[1] = 0/255.f;
    purple.diffuse[2] = 62/255.f;
    purple.diffuse[3] = 1.0f;

    purple.specular[0] = 62/255.f;
    purple.specular[1] = 0.0f;
    purple.specular[2] = 62/255.f;
    purple.specular[3] = 1.0f;

    purple.shininess = 128;

}

void VisualDisplay::doLayout()
{
    //--- Argh, in order to support rotations we must rotate the n-vector!!!
    float nx = 1;
    float ny = 0;
    float nz = 0;

    if(fishEyeMode)
    {
        float fishEyeLength = fishEyeLengthScale*length;
        float nonFishEyeLength = (length- fishEyeLength)/2.0f;

        float ox2 = ox + nx*nonFishEyeLength;
        float oy2 = oy + ny*nonFishEyeLength;
        float oz2 = oz + nz*nonFishEyeLength;
        float ox3 = ox2 + nx*fishEyeLength;
        float oy3 = oy2 + ny*fishEyeLength;
        float oz3 = oz2 + nz*fishEyeLength;

        preSignal->setOrigo(ox,oy,oz);
        preSignal->setLength(nonFishEyeLength);
        preSignal->setHeight(signalScale*height*2);
        preSignal->setRotation(angle,rx,ry,rz);

        fishEyeSignal->setOrigo(ox2,oy2,oz2);
        fishEyeSignal->setLength(fishEyeLength);
        fishEyeSignal->setHeight(signalScale*height*2);
        fishEyeSignal->setRotation(angle,rx,ry,rz);

        postSignal->setOrigo(ox3,oy3,oz3);
        postSignal->setLength(nonFishEyeLength);
        postSignal->setHeight(signalScale*height*2);
        postSignal->setRotation(angle,rx,ry,rz);


        box->setOrigo(ox2,oy2,oz2);
        box->setLength(fishEyeLength);
        box->setHeight(signalScale*height);
        box->setDepth(depth);
        box->setRotation(angle,rx,ry,rz);

    }
    else
    {
        signal->setOrigo(ox,oy,oz);
        signal->setLength(length);
        signal->setHeight(signalScale*height*2);
        signal->setRotation(angle,rx,ry,rz);
    
        box->setOrigo(ox,oy,oz);
        box->setLength(length);
        box->setHeight(height);
        box->setDepth(depth);
        box->setRotation(angle,rx,ry,rz);
    }

    float plength = length/300;
    float pheight = height*1.05;
    float pdepth = depth*1.05;

    float offset = (length-plength)/2.0f;

    float px = ox + nx*offset; 
    float py = oy + ny*offset;
    float pz = oz + nz*offset;

    playPosMarker->setOrigo(px,py,pz);
    playPosMarker->setLength(plength);
    playPosMarker->setHeight(pheight);
    playPosMarker->setDepth(pdepth);
    playPosMarker->setRotation(angle,rx,ry,rz);
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

