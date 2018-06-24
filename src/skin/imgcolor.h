/***************************************************************************
                          imgcolor.h  -  description
                             -------------------
    begin                : 14 April 2007
    copyright            : (C) 2007 by Adam Davison
    email                : adamdavison@gmail.com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef IMGCOLOR_H
#define IMGCOLOR_H

#include "imgsource.h"

class ImgAdd : public ImgColorProcessor {

public:
    ImgAdd(const QSharedPointer<ImgSource>& parent, int amt);
    QColor doColorCorrection(const QColor& c) const override;

private:
    int m_amt;
};

class ImgMax : public ImgColorProcessor {

public:
    ImgMax(const QSharedPointer<ImgSource>& parent, int amt);
    QColor doColorCorrection(const QColor& c) const override;

private:
    int m_amt;
};

class ImgMonoColor : public ImgColorProcessor {
    
  public:
    ImgMonoColor(const QSharedPointer<ImgSource>& parent, const QColor &baseColor);
    QColor doColorCorrection(const QColor& c) const override;
  private:
    QColor m_baseColor;
};

class ImgScaleWhite : public ImgColorProcessor {

public:
    inline ImgScaleWhite(const QSharedPointer<ImgSource>& parent, float amt)
        : ImgColorProcessor(parent), m_amt(amt) {}
    QColor doColorCorrection(const QColor& c) const override;
private:
    float m_amt;
};

class ImgHueRot : public ImgColorProcessor {

public:
    inline ImgHueRot(QSharedPointer<ImgSource> parent, int amt)
        : ImgColorProcessor(parent), m_amt(amt) {}
    QColor doColorCorrection(const QColor& c) const override;

private:
    int m_amt;
};

class ImgHueInv : public ImgColorProcessor {

public:
    inline ImgHueInv(const QSharedPointer<ImgSource>& parent) : ImgColorProcessor(parent) {}
    QColor doColorCorrection(const QColor& c) const override;
};

class ImgHSVTweak : public ImgColorProcessor {
  public:
    inline ImgHSVTweak(const QSharedPointer<ImgSource>& parent, int hmin, int hmax, int smin,
                       int smax, int vmin, int vmax, float hfact, int hconst, float sfact,
                       int sconst, float vfact, int vconst)
            : ImgColorProcessor(parent),
              m_hmin(hmin), m_hmax(hmax),
              m_smin(smin), m_smax(smax),
              m_vmin(vmin), m_vmax(vmax),
              m_hconst(hconst), m_sconst(sconst), m_vconst(vconst),
              m_hfact(hfact), m_sfact(sfact), m_vfact(vfact) {}
    QColor doColorCorrection(const QColor& c) const override;

  private:
    int m_hmin, m_hmax,
        m_smin, m_smax,
        m_vmin, m_vmax,
        m_hconst, m_sconst, m_vconst;
    float m_hfact, m_sfact, m_vfact;
};
#endif

