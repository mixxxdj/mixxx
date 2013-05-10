/*****************************************************************************
 * Author: Lukáš Lalinský <info@acoustid.org>                                *
 *****************************************************************************/

#ifndef FPSUBMIT_GZIP_H_
#define FPSUBMIT_GZIP_H_

#include <QByteArray>

QByteArray gzipCompress(const QByteArray &data);

#endif
