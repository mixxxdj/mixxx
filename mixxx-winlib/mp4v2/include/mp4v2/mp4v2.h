/*
 * The contents of this file are subject to the Mozilla Public
 * License Version 1.1 (the "License"); you may not use this file
 * except in compliance with the License. You may obtain a copy of
 * the License at http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS
 * IS" basis, WITHOUT WARRANTY OF ANY KIND, either express or
 * implied. See the License for the specific language governing
 * rights and limitations under the License.
 *
 * The Original Code is MPEG4IP.
 *
 * The Initial Developer of the Original Code is Cisco Systems Inc.
 * Portions created by Cisco Systems Inc. are
 * Copyright (C) Cisco Systems Inc. 2001 - 2005.  All Rights Reserved.
 *
 * 3GPP features implementation is based on 3GPP's TS26.234-v5.60,
 * and was contributed by Ximpo Group Ltd.
 *
 * Portions created by Ximpo Group Ltd. are
 * Copyright (C) Ximpo Group Ltd. 2003, 2004.  All Rights Reserved.
 *
 * Contributor(s):
 *      Dave Mackie                 dmackie@cisco.com
 *      Alix Marchandise-Franquet   alix@cisco.com
 *      Ximpo Group Ltd.            mp4v2@ximpo.com
 *      Bill May                    wmay@cisco.com
 */
#ifndef MP4V2_MP4V2_H
#define MP4V2_MP4V2_H

/*****************************************************************************/

#include <mp4v2/platform.h>
#include <mp4v2/project.h>

/*****************************************************************************/

/* exploit C++ ability of default values for function parameters */
#if defined( DEFAULT )
#   define __MP4V2_SAVE_DEFAULT DEFAULT
#endif
#undef DEFAULT
#if defined( __cplusplus )
#   define DEFAULT(x) =x
#else
#   define DEFAULT(x)
#endif

#ifdef __cplusplus
extern "C" {
#endif

/*****************************************************************************/

#include <mp4v2/general.h>
#include <mp4v2/file.h>
#include <mp4v2/file_prop.h>
#include <mp4v2/track.h>
#include <mp4v2/track_prop.h>
#include <mp4v2/sample.h>
#include <mp4v2/chapter.h>
#include <mp4v2/meta.h>
#include <mp4v2/streaming.h>
#include <mp4v2/isma.h>

/*****************************************************************************/

/* restore macro DEFAULT to state prior to mp4v2 headers */
#undef DEFAULT
#if defined( __MP4V2_SAVE_DEFAULT )
#   define DEFAULT __MP4V2_SAVE_DEFAULT
#endif

#ifdef __cplusplus
} // extern "C"
#endif

/*****************************************************************************/

#endif /* MP4V2_MP4V2_H */
