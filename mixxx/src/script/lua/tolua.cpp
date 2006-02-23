/*
** Lua binding: mixxx
** Generated automatically by tolua 5.0a on Sun Feb 19 16:28:07 2006.
*/

#ifndef __cplusplus
#include "stdlib.h"
#endif
#include "string.h"

#include "tolua.h"

/* Exported function */
TOLUA_API int tolua_mixxx_open (lua_State* tolua_S);

#include "../playinterface.h"
#include "../scriptengine.h"

/* function to register type */
static void tolua_reg_types (lua_State* tolua_S)
{
 tolua_usertype(tolua_S,"PlayInterface");
}

/* method: test of class  PlayInterface */
static int tolua_mixxx_PlayInterface_test00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
 !tolua_isusertype(tolua_S,1,"PlayInterface",0,&tolua_err) ||
 !tolua_isnoobj(tolua_S,2,&tolua_err)
 )
 goto tolua_lerror;
 else
#endif
 {
  PlayInterface* self = (PlayInterface*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in function 'test'",NULL);
#endif
 {
  self->test();
 }
 }
 return 0;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'test'.",&tolua_err);
 return 0;
#endif
}

/* method: stop of class  PlayInterface */
static int tolua_mixxx_PlayInterface_stop00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
 !tolua_isusertype(tolua_S,1,"PlayInterface",0,&tolua_err) ||
 !tolua_isnumber(tolua_S,2,0,&tolua_err) ||
 !tolua_isnoobj(tolua_S,3,&tolua_err)
 )
 goto tolua_lerror;
 else
#endif
 {
  PlayInterface* self = (PlayInterface*)  tolua_tousertype(tolua_S,1,0);
  int channel = ((int)  tolua_tonumber(tolua_S,2,0));
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in function 'stop'",NULL);
#endif
 {
  self->stop(channel);
 }
 }
 return 0;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'stop'.",&tolua_err);
 return 0;
#endif
}

/* method: play of class  PlayInterface */
static int tolua_mixxx_PlayInterface_play00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
 !tolua_isusertype(tolua_S,1,"PlayInterface",0,&tolua_err) ||
 !tolua_isnumber(tolua_S,2,0,&tolua_err) ||
 !tolua_isnoobj(tolua_S,3,&tolua_err)
 )
 goto tolua_lerror;
 else
#endif
 {
  PlayInterface* self = (PlayInterface*)  tolua_tousertype(tolua_S,1,0);
  int channel = ((int)  tolua_tonumber(tolua_S,2,0));
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in function 'play'",NULL);
#endif
 {
  self->play(channel);
 }
 }
 return 0;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'play'.",&tolua_err);
 return 0;
#endif
}

/* method: setFader of class  PlayInterface */
static int tolua_mixxx_PlayInterface_setFader00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
 !tolua_isusertype(tolua_S,1,"PlayInterface",0,&tolua_err) ||
 !tolua_isnumber(tolua_S,2,0,&tolua_err) ||
 !tolua_isnoobj(tolua_S,3,&tolua_err)
 )
 goto tolua_lerror;
 else
#endif
 {
  PlayInterface* self = (PlayInterface*)  tolua_tousertype(tolua_S,1,0);
  double fade = ((double)  tolua_tonumber(tolua_S,2,0));
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in function 'setFader'",NULL);
#endif
 {
  self->setFader(fade);
 }
 }
 return 0;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'setFader'.",&tolua_err);
 return 0;
#endif
}

/* method: setTag of class  PlayInterface */
static int tolua_mixxx_PlayInterface_setTag00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
 !tolua_isusertype(tolua_S,1,"PlayInterface",0,&tolua_err) ||
 !tolua_isnumber(tolua_S,2,0,&tolua_err) ||
 !tolua_isnoobj(tolua_S,3,&tolua_err)
 )
 goto tolua_lerror;
 else
#endif
 {
  PlayInterface* self = (PlayInterface*)  tolua_tousertype(tolua_S,1,0);
  int tag = ((int)  tolua_tonumber(tolua_S,2,0));
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in function 'setTag'",NULL);
#endif
 {
  self->setTag(tag);
 }
 }
 return 0;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'setTag'.",&tolua_err);
 return 0;
#endif
}

/* method: clearTag of class  PlayInterface */
static int tolua_mixxx_PlayInterface_clearTag00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
 !tolua_isusertype(tolua_S,1,"PlayInterface",0,&tolua_err) ||
 !tolua_isnoobj(tolua_S,2,&tolua_err)
 )
 goto tolua_lerror;
 else
#endif
 {
  PlayInterface* self = (PlayInterface*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in function 'clearTag'",NULL);
#endif
 {
  self->clearTag();
 }
 }
 return 0;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'clearTag'.",&tolua_err);
 return 0;
#endif
}

/* method: kill of class  PlayInterface */
static int tolua_mixxx_PlayInterface_kill00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
 !tolua_isusertype(tolua_S,1,"PlayInterface",0,&tolua_err) ||
 !tolua_isnoobj(tolua_S,2,&tolua_err)
 )
 goto tolua_lerror;
 else
#endif
 {
  PlayInterface* self = (PlayInterface*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in function 'kill'",NULL);
#endif
 {
  self->kill();
 }
 }
 return 0;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'kill'.",&tolua_err);
 return 0;
#endif
}

/* method: killTag of class  PlayInterface */
static int tolua_mixxx_PlayInterface_killTag00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
 !tolua_isusertype(tolua_S,1,"PlayInterface",0,&tolua_err) ||
 !tolua_isnumber(tolua_S,2,0,&tolua_err) ||
 !tolua_isnoobj(tolua_S,3,&tolua_err)
 )
 goto tolua_lerror;
 else
#endif
 {
  PlayInterface* self = (PlayInterface*)  tolua_tousertype(tolua_S,1,0);
  int tag = ((int)  tolua_tonumber(tolua_S,2,0));
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in function 'killTag'",NULL);
#endif
 {
  self->killTag(tag);
 }
 }
 return 0;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'killTag'.",&tolua_err);
 return 0;
#endif
}

/* method: getFader of class  PlayInterface */
static int tolua_mixxx_PlayInterface_getFader00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
 !tolua_isusertype(tolua_S,1,"PlayInterface",0,&tolua_err) ||
 !tolua_isnoobj(tolua_S,2,&tolua_err)
 )
 goto tolua_lerror;
 else
#endif
 {
  PlayInterface* self = (PlayInterface*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in function 'getFader'",NULL);
#endif
 {
  double tolua_ret = (double)  self->getFader();
 tolua_pushnumber(tolua_S,(lua_Number)tolua_ret);
 }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'getFader'.",&tolua_err);
 return 0;
#endif
}

/* method: getValue of class  PlayInterface */
static int tolua_mixxx_PlayInterface_getValue00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
 !tolua_isusertype(tolua_S,1,"PlayInterface",0,&tolua_err) ||
 !tolua_isstring(tolua_S,2,0,&tolua_err) ||
 !tolua_isstring(tolua_S,3,0,&tolua_err) ||
 !tolua_isnoobj(tolua_S,4,&tolua_err)
 )
 goto tolua_lerror;
 else
#endif
 {
  PlayInterface* self = (PlayInterface*)  tolua_tousertype(tolua_S,1,0);
  const char* group = ((const char*)  tolua_tostring(tolua_S,2,0));
  const char* name = ((const char*)  tolua_tostring(tolua_S,3,0));
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in function 'getValue'",NULL);
#endif
 {
  double tolua_ret = (double)  self->getValue(group,name);
 tolua_pushnumber(tolua_S,(lua_Number)tolua_ret);
 }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'getValue'.",&tolua_err);
 return 0;
#endif
}

/* method: startFadeCrossfader of class  PlayInterface */
static int tolua_mixxx_PlayInterface_startFadeCrossfader00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
 !tolua_isusertype(tolua_S,1,"PlayInterface",0,&tolua_err) ||
 !tolua_isnoobj(tolua_S,2,&tolua_err)
 )
 goto tolua_lerror;
 else
#endif
 {
  PlayInterface* self = (PlayInterface*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in function 'startFadeCrossfader'",NULL);
#endif
 {
  self->startFadeCrossfader();
 }
 }
 return 0;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'startFadeCrossfader'.",&tolua_err);
 return 0;
#endif
}

/* method: startList of class  PlayInterface */
static int tolua_mixxx_PlayInterface_startList00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
 !tolua_isusertype(tolua_S,1,"PlayInterface",0,&tolua_err) ||
 !tolua_isstring(tolua_S,2,0,&tolua_err) ||
 !tolua_isstring(tolua_S,3,0,&tolua_err) ||
 !tolua_isnoobj(tolua_S,4,&tolua_err)
 )
 goto tolua_lerror;
 else
#endif
 {
  PlayInterface* self = (PlayInterface*)  tolua_tousertype(tolua_S,1,0);
  const char* group = ((const char*)  tolua_tostring(tolua_S,2,0));
  const char* name = ((const char*)  tolua_tostring(tolua_S,3,0));
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in function 'startList'",NULL);
#endif
 {
  self->startList(group,name);
 }
 }
 return 0;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'startList'.",&tolua_err);
 return 0;
#endif
}

/* method: startFade of class  PlayInterface */
static int tolua_mixxx_PlayInterface_startFade00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
 !tolua_isusertype(tolua_S,1,"PlayInterface",0,&tolua_err) ||
 !tolua_isstring(tolua_S,2,0,&tolua_err) ||
 !tolua_isstring(tolua_S,3,0,&tolua_err) ||
 !tolua_isnoobj(tolua_S,4,&tolua_err)
 )
 goto tolua_lerror;
 else
#endif
 {
  PlayInterface* self = (PlayInterface*)  tolua_tousertype(tolua_S,1,0);
  const char* group = ((const char*)  tolua_tostring(tolua_S,2,0));
  const char* name = ((const char*)  tolua_tostring(tolua_S,3,0));
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in function 'startFade'",NULL);
#endif
 {
  self->startFade(group,name);
 }
 }
 return 0;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'startFade'.",&tolua_err);
 return 0;
#endif
}

/* method: point of class  PlayInterface */
static int tolua_mixxx_PlayInterface_point00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
 !tolua_isusertype(tolua_S,1,"PlayInterface",0,&tolua_err) ||
 !tolua_isnumber(tolua_S,2,0,&tolua_err) ||
 !tolua_isnumber(tolua_S,3,0,&tolua_err) ||
 !tolua_isnoobj(tolua_S,4,&tolua_err)
 )
 goto tolua_lerror;
 else
#endif
 {
  PlayInterface* self = (PlayInterface*)  tolua_tousertype(tolua_S,1,0);
  int time = ((int)  tolua_tonumber(tolua_S,2,0));
  double value = ((double)  tolua_tonumber(tolua_S,3,0));
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in function 'point'",NULL);
#endif
 {
  self->point(time,value);
 }
 }
 return 0;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'point'.",&tolua_err);
 return 0;
#endif
}

/* method: fadePoint of class  PlayInterface */
static int tolua_mixxx_PlayInterface_fadePoint00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
 !tolua_isusertype(tolua_S,1,"PlayInterface",0,&tolua_err) ||
 !tolua_isnumber(tolua_S,2,0,&tolua_err) ||
 !tolua_isnumber(tolua_S,3,0,&tolua_err) ||
 !tolua_isnoobj(tolua_S,4,&tolua_err)
 )
 goto tolua_lerror;
 else
#endif
 {
  PlayInterface* self = (PlayInterface*)  tolua_tousertype(tolua_S,1,0);
  int time = ((int)  tolua_tonumber(tolua_S,2,0));
  double value = ((double)  tolua_tonumber(tolua_S,3,0));
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in function 'fadePoint'",NULL);
#endif
 {
  self->fadePoint(time,value);
 }
 }
 return 0;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'fadePoint'.",&tolua_err);
 return 0;
#endif
}

/* method: endFade of class  PlayInterface */
static int tolua_mixxx_PlayInterface_endFade00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
 !tolua_isusertype(tolua_S,1,"PlayInterface",0,&tolua_err) ||
 !tolua_isnoobj(tolua_S,2,&tolua_err)
 )
 goto tolua_lerror;
 else
#endif
 {
  PlayInterface* self = (PlayInterface*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in function 'endFade'",NULL);
#endif
 {
  self->endFade();
 }
 }
 return 0;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'endFade'.",&tolua_err);
 return 0;
#endif
}

/* method: endList of class  PlayInterface */
static int tolua_mixxx_PlayInterface_endList00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
 !tolua_isusertype(tolua_S,1,"PlayInterface",0,&tolua_err) ||
 !tolua_isnoobj(tolua_S,2,&tolua_err)
 )
 goto tolua_lerror;
 else
#endif
 {
  PlayInterface* self = (PlayInterface*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in function 'endList'",NULL);
#endif
 {
  self->endList();
 }
 }
 return 0;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'endList'.",&tolua_err);
 return 0;
#endif
}

/* method: playChannel1 of class  PlayInterface */
static int tolua_mixxx_PlayInterface_playChannel100(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
 !tolua_isusertype(tolua_S,1,"PlayInterface",0,&tolua_err) ||
 !tolua_isnumber(tolua_S,2,0,&tolua_err) ||
 !tolua_isstring(tolua_S,3,0,&tolua_err) ||
 !tolua_isnoobj(tolua_S,4,&tolua_err)
 )
 goto tolua_lerror;
 else
#endif
 {
  PlayInterface* self = (PlayInterface*)  tolua_tousertype(tolua_S,1,0);
  int time = ((int)  tolua_tonumber(tolua_S,2,0));
  char* path = ((char*)  tolua_tostring(tolua_S,3,0));
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in function 'playChannel1'",NULL);
#endif
 {
  self->playChannel1(time,path);
 }
 }
 return 0;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'playChannel1'.",&tolua_err);
 return 0;
#endif
}

/* method: playChannel2 of class  PlayInterface */
static int tolua_mixxx_PlayInterface_playChannel200(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
 !tolua_isusertype(tolua_S,1,"PlayInterface",0,&tolua_err) ||
 !tolua_isnumber(tolua_S,2,0,&tolua_err) ||
 !tolua_isstring(tolua_S,3,0,&tolua_err) ||
 !tolua_isnoobj(tolua_S,4,&tolua_err)
 )
 goto tolua_lerror;
 else
#endif
 {
  PlayInterface* self = (PlayInterface*)  tolua_tousertype(tolua_S,1,0);
  int time = ((int)  tolua_tonumber(tolua_S,2,0));
  char* path = ((char*)  tolua_tostring(tolua_S,3,0));
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in function 'playChannel2'",NULL);
#endif
 {
  self->playChannel2(time,path);
 }
 }
 return 0;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'playChannel2'.",&tolua_err);
 return 0;
#endif
}

/* Open function */
TOLUA_API int tolua_mixxx_open (lua_State* tolua_S)
{
 tolua_open(tolua_S);
 tolua_reg_types(tolua_S);
 tolua_module(tolua_S,NULL,0);
 tolua_beginmodule(tolua_S,NULL);
 tolua_cclass(tolua_S,"PlayInterface","PlayInterface","",NULL);
 tolua_beginmodule(tolua_S,"PlayInterface");
 tolua_function(tolua_S,"test",tolua_mixxx_PlayInterface_test00);
 tolua_function(tolua_S,"stop",tolua_mixxx_PlayInterface_stop00);
 tolua_function(tolua_S,"play",tolua_mixxx_PlayInterface_play00);
 tolua_function(tolua_S,"setFader",tolua_mixxx_PlayInterface_setFader00);
 tolua_function(tolua_S,"setTag",tolua_mixxx_PlayInterface_setTag00);
 tolua_function(tolua_S,"clearTag",tolua_mixxx_PlayInterface_clearTag00);
 tolua_function(tolua_S,"kill",tolua_mixxx_PlayInterface_kill00);
 tolua_function(tolua_S,"killTag",tolua_mixxx_PlayInterface_killTag00);
 tolua_function(tolua_S,"getFader",tolua_mixxx_PlayInterface_getFader00);
 tolua_function(tolua_S,"getValue",tolua_mixxx_PlayInterface_getValue00);
 tolua_function(tolua_S,"startFadeCrossfader",tolua_mixxx_PlayInterface_startFadeCrossfader00);
 tolua_function(tolua_S,"startList",tolua_mixxx_PlayInterface_startList00);
 tolua_function(tolua_S,"startFade",tolua_mixxx_PlayInterface_startFade00);
 tolua_function(tolua_S,"point",tolua_mixxx_PlayInterface_point00);
 tolua_function(tolua_S,"fadePoint",tolua_mixxx_PlayInterface_fadePoint00);
 tolua_function(tolua_S,"endFade",tolua_mixxx_PlayInterface_endFade00);
 tolua_function(tolua_S,"endList",tolua_mixxx_PlayInterface_endList00);
 tolua_function(tolua_S,"playChannel1",tolua_mixxx_PlayInterface_playChannel100);
 tolua_function(tolua_S,"playChannel2",tolua_mixxx_PlayInterface_playChannel200);
 tolua_endmodule(tolua_S);
 tolua_endmodule(tolua_S);
 return 1;
}
