/***************************************************************************
                          main.cpp  -  description
                             -------------------
    begin                : Mon Feb 18 09:48:17 CET 2002
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

#include <qapplication.h>
#include <qfont.h>
#include <qstring.h>
#include <qtextcodec.h>
#include <qtranslator.h>

#include "mixxx.h"

#include "portaudio.h"
#define TABLE_SIZE   (200)
typedef struct {
	float sine[TABLE_SIZE];
	int left_phase;
	int right_phase;
}paTestData;
/* This routine will be called by the PortAudio engine when audio is needed.
** It may called at interrupt level on some machines so don't do anything
** that could mess up the system like calling malloc() or free().
*/
static int patestCallback(   void *inputBuffer, void *outputBuffer,
                             unsigned long framesPerBuffer,
                             PaTimestamp outTime, void *userData )
{
	paTestData *data = (paTestData*)userData;
	float *out = (float*)outputBuffer;
	unsigned long i;
	int finished = 0;
	(void) outTime; /* Prevent unused variable warnings. */
	(void) inputBuffer;
	for( i=0; i<framesPerBuffer; i++ )
	{
		*out++ = data->sine[data->left_phase];		/* left */
		*out++ = data->sine[data->right_phase];		/* right */
		data->left_phase += 1;
		if( data->left_phase >= TABLE_SIZE ) data->left_phase -= TABLE_SIZE;
		data->right_phase += 3; /* higher pitch so we can distinguish left and right. */
		if( data->right_phase >= TABLE_SIZE ) data->right_phase -= TABLE_SIZE;
	}
	return finished;
}

int main(int argc, char *argv[])
{

  QApplication a(argc, argv);
  a.setFont(QFont("helvetica", 12));
  QTranslator tor( 0 );
  // set the location where your .qm files are in load() below as the last parameter instead of "."
  // for development, use "/" to use the english original as
  // .qm files are stored in the base project directory.
  tor.load( QString("mixxx.") + QTextCodec::locale(), "." );
  a.installTranslator( &tor );
  /* uncomment the following line, if you want a Windows 95 look*/
  // a.setStyle(WindowsStyle);
    
  MixxxApp *mixxx=new MixxxApp();
  a.setMainWidget(mixxx);
  mixxx->resize( 150+310, 310+210+70);
  mixxx->show();
  return a.exec();
}


