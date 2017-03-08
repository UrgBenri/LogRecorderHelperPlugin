/*
	This file is part of the UrgBenri application.

	Copyright (c) 2016 Mehrez Kristou.
	All rights reserved.

	Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

	1. Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.

	2. Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.

	3. Neither the name of the copyright holder nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

	THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

	Please contact kristou@hokuyo-aut.jp for more details.

*/

#ifndef QRK_MATH_UTILS_H
#define QRK_MATH_UTILS_H

/*!
  \file
  \brief 数学関数の補助ファイル

  \author Satofumi KAMIMURA

  $Id: MathUtils.h 131 2012-09-11 06:23:04Z kristou $
*/
#include <QPoint>

//#include "DetectOS.h"
#if defined(Q_OS_WIN)
#define _USE_MATH_DEFINES
#endif
#include <math.h>
#include <vector>



using namespace std;

#ifndef M_PI
//! 円周率 (Visual C++ 6.0 用)
#define M_PI 3.14159265358979323846
#endif

#if defined(Q_CC_MSVC)
extern long lrint(double x);
#endif

/*:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::*/
/*::  This function converts decimal degrees to radians             :*/
/*:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::*/
double deg2rad(double deg);

/*:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::*/
/*::  This function converts radians to decimal degrees             :*/
/*:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::*/
double rad2deg(double rad);

// modulu - similar to matlab's mod()
// result is always possitive. not similar to fmod()
// Mod(-3,4)= 1   fmod(-3,4)= -3
double inline Mod(double x, double y)
{
    if (0 == y)
        return x;

    return x - y * floor(x / y);
}

// wrap [rad] angle to [-PI..PI)
inline double WrapPosNegPI(double fAng)
{
    return Mod(fAng + M_PI, M_PI * 2) - M_PI;
}

// wrap [rad] angle to [0..TWO_PI)
inline double WrapTwoPI(double fAng)
{
    return Mod(fAng, M_PI * 2);
}

// wrap [deg] angle to [-180..180)
inline double WrapPosNeg180(double fAng)
{
    return Mod(fAng + 180., 360.) - 180.;
}

// wrap [deg] angle to [0..360)
inline double Wrap360(double fAng)
{
    return Mod(fAng , 360.);
}



#endif /* !QRK_MATH_UTILS_H */

