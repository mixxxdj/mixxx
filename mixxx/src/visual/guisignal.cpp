#include "guisignal.h"
#include "buffer.h"
#include "../controlpotmeter.h"
#include <math.h>

/**
 * Default Consructor.
 */
CGUISignal::CGUISignal(CSignalVertexBuffer *_buffer, CFastVertexArray *vertex, const char *group) : fishEyeSignal(_buffer->buffer,vertex), preSignal(_buffer->buffer,vertex), postSignal(_buffer->buffer,vertex), signal(_buffer->buffer,vertex)
{
  buffer = _buffer;
  length = 50;
  height = 20;
  depth  = 10;

  angle = 0;
  rx = 1;ry=0;rz=0;

  ox = -25;
  oy = oz = 0;

  fishEyeMode = false;

  fishEyeLengthScale = 0.5f;
  fishEyeSignalFraction = 0.2f;
  signalScale = 1.0f;

  doLayout();

  boxMaterial = boxWireMaterial = 0;
  playPosMarkerMaterial = 0;

  sliderFishEyeSignalFraction = new ControlPotmeter(ConfigKey(group,"FishEyeSignalFraction"),0.001,0.5);
  connect(sliderFishEyeSignalFraction, SIGNAL(valueChanged(FLOAT_TYPE)), this, SLOT(setFishEyeSignalFraction(FLOAT_TYPE)));

  sliderFishEyeLengthScale = new ControlPotmeter(ConfigKey(group,"FishEyeLengthScale"),0.05,0.95);
  connect(sliderFishEyeLengthScale, SIGNAL(valueChanged(FLOAT_TYPE)), this, SLOT(setFishEyeLengthScale(FLOAT_TYPE)));

  
};


/**
 *
 */
void CGUISignal::setPlayPosMarkerMaterial(CMaterial * material)
{
	playPosMarkerMaterial = material;
};


/**
 *
 */
void CGUISignal::setBoxMaterial(CMaterial * material)
{
	boxMaterial = material;
};

void CGUISignal::setBoxWireMaterial(CMaterial * material)
{
    boxWireMaterial = material;
};

/**
 *
 */
void CGUISignal::setSignalMaterial(CMaterial * material){
  preSignal.setMaterial(material);
  postSignal.setMaterial(material);
  signal.setMaterial(material);
};

/**
 *
 */
void CGUISignal::setFishEyeSignalMaterial(CMaterial * material){
  fishEyeSignal.setMaterial(material);
};

/**
 * Specialized Drawing Routine.
 * This method overrides the oridnary behavior of a visual
 * object, because the GUI signal is composed of several
 * smaller visual and/or pickable objects.
 *
 * @param mode     The render mode.
 */
void CGUISignal::draw(GLenum mode){

  doLayout();

  bufInfo b = buffer->getVertexArray();

  
  if(fishEyeMode)
  {
	  int nfish =     (b.len1+b.len2)*fishEyeSignalFraction;
	  int noutside = ((b.len1+b.len2)-nfish)/2;
//	  int n = ((b.len1+b.len2)/3)+1;
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

    preSignal.setVertexArray(i1);
    fishEyeSignal.setVertexArray(i2);
    postSignal.setVertexArray(i3);

    preSignal.draw(mode);
    postSignal.draw(mode);
    fishEyeSignal.draw(mode);
  }
  else
  {
    signal.setVertexArray(b);
    signal.draw(mode);
  }

  if (boxWireMaterial)
	  box.setMaterial(boxWireMaterial);
  box.setDrawMode(GL_LINE_LOOP);
  box.draw(mode);

  if (boxMaterial)
	  box.setMaterial(boxMaterial);
  box.setDrawMode(GL_POLYGON);
  box.draw(mode);


  if (playPosMarkerMaterial)
	  playPosMarker.setMaterial(playPosMarkerMaterial);
  playPosMarker.setDrawMode(GL_POLYGON);
  playPosMarker.draw(mode);



}

/**
 * The Drawing Method.
 * Currently there is no need for any drawing.
 */
void CGUISignal::draw(){};


/**
 * Set Origo.
 */
void CGUISignal::setOrigo(float ox, float oy,float oz){
  this->ox = ox;
  this->oy = oy;
  this->oz = oz;
};

float CGUISignal::getOrigoX()
{
	return ox;
}

float CGUISignal::getOrigoY()
{
	return oy;
}

float CGUISignal::getOrigoZ()
{
	return oz;
}

/**
 * Set Fish Eye Mode.
 *
 * @param value   Boolean value indicating what ever
 *                fish eye mode is on or off.
 */
void CGUISignal::setFishEyeMode(bool value)
{
    fishEyeMode = value;
};

/**
 * Set Fish Eye Scale.
 *
 * @param scale    A non zero value indicating how long the fish
 *                 eye box should be stretched. 0 means no fisheye, 1 means full length fish eye
 */
void CGUISignal::setFishEyeLengthScale(FLOAT_TYPE scale)
{
    fishEyeLengthScale = scale;
};

/** Controls the fraction of the signal display in fish eye */
void CGUISignal::setFishEyeSignalFraction(FLOAT_TYPE fraction)
{
    fishEyeSignalFraction = fraction;
};


/**
 * Set Signal Scale
 *
 * @param scale   A non zero value indicating how heigh signals
 *                in the fish eye box should stretched. 
 */
void CGUISignal::setSignalScale(float scale)
{
    signalScale = scale;
};

/**
 *
 */
void CGUISignal::setLength(float length)
{
    this->length = length;
};

float CGUISignal::getLength()
{
    return length;
};
/**
 *
 */
void CGUISignal::setHeight(float height)
{
    this->height = height;
};

float CGUISignal::getHeight()
{
	return height;
}

/**
 *
 */
void CGUISignal::setDepth(float depth)
{
    this->depth = depth;
};

float CGUISignal::getDepth()
{
	return depth;
}


/**
 *
 */
void CGUISignal::doLayout()
{

  //--- Argh, in order to support rotations we must rotate the n-vector!!!
  float nx = 1;
  float ny = 0;
  float nz = 0;

  if(fishEyeMode){
    
    float fishEyeLength = fishEyeLengthScale*length;
    float nonFishEyeLength = (length- fishEyeLength)/2.0f;

    float ox2 = ox + nx*nonFishEyeLength;
    float oy2 = oy + ny*nonFishEyeLength;
    float oz2 = oz + nz*nonFishEyeLength;
    float ox3 = ox2 + nx*fishEyeLength;
    float oy3 = oy2 + ny*fishEyeLength;
    float oz3 = oz2 + nz*fishEyeLength;

    preSignal.setOrigo(ox,oy,oz);
    preSignal.setLength(nonFishEyeLength);
    preSignal.setHeight(signalScale*height*2);
    preSignal.setRotation(angle,rx,ry,rz);

    fishEyeSignal.setOrigo(ox2,oy2,oz2);
    fishEyeSignal.setLength(fishEyeLength);
    fishEyeSignal.setHeight(signalScale*height*2);
    fishEyeSignal.setRotation(angle,rx,ry,rz);

    postSignal.setOrigo(ox3,oy3,oz3);
    postSignal.setLength(nonFishEyeLength);
    postSignal.setHeight(signalScale*height*2);
    postSignal.setRotation(angle,rx,ry,rz);


    box.setOrigo(ox2,oy2,oz2);
    box.setLength(fishEyeLength);
    box.setHeight(signalScale*height);
    box.setDepth(depth);
    box.setRotation(angle,rx,ry,rz);

  }else{

    signal.setOrigo(ox,oy,oz);
    signal.setLength(length);
    signal.setHeight(signalScale*height*2);
    signal.setRotation(angle,rx,ry,rz);
    
    box.setOrigo(ox,oy,oz);
    box.setLength(length);
    box.setHeight(height);
    box.setDepth(depth);
    box.setRotation(angle,rx,ry,rz);
  }

  float plength = length/300;
  float pheight = height*1.05;
  float pdepth = depth*1.05;

  float offset = (length-plength)/2.0f;

  float px = ox + nx*offset; 
  float py = oy + ny*offset;
  float pz = oz + nz*offset;

  playPosMarker.setOrigo(px,py,pz);
  playPosMarker.setLength(plength);
  playPosMarker.setHeight(pheight);
  playPosMarker.setDepth(pdepth);
  playPosMarker.setRotation(angle,rx,ry,rz);

};

/**
 * Set Rotation.
 * Rotations are currently not fully implemented.
 *
 */
void CGUISignal::setRotation(float angle, float rx,float ry,float rz)
{
    this->angle = angle;
    this->rx = rx;
    this->ry = ry;
    this->rz = rz;
};

