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

#include "Angle.h"
#include <math.h>

#ifndef M_PI
    #define M_PI 3.1415926535897932384626433832795
#endif

using namespace qrk;


Angle::Angle(void) : radian_angle_(0.0)
{
}


Angle::Angle(const Angle &rhs) : radian_angle_(rhs.radian_angle_)
{
}


Angle &Angle::operator = (const Angle &rhs)
{
    this->radian_angle_ = rhs.radian_angle_;

    return *this;
}


Angle &Angle::operator += (const Angle &rhs)
{
    this->radian_angle_ += rhs.radian_angle_;

    return *this;
}


const Angle Angle::operator + (const Angle &rhs) const
{
    return Angle(*this) += rhs;
}


Angle &Angle::operator -= (const Angle &rhs)
{
    this->radian_angle_ -= rhs.radian_angle_;

    return *this;
}


const Angle Angle::operator - (const Angle &rhs) const
{
    return Angle(*this) -= rhs;
}


Angle &Angle::operator *= (const double rhs)
{
    this->radian_angle_ *= rhs;

    return *this;
}


Angle Angle::operator * (double rhs) const
{
    return Angle(*this) *= rhs;
}


void Angle::normalize(void)
{
    while (radian_angle_ < -M_PI) {
        radian_angle_ += 2.0 * M_PI;
    }
    while (radian_angle_ > M_PI) {
        radian_angle_ -= 2.0 * M_PI;
    }
}


int Angle::to_deg(void) const
{
    return static_cast<int>(180 * radian_angle_ / M_PI);
}


double Angle::to_rad(void) const
{
    return radian_angle_;
}


Angle qrk::rad(double radian)
{
    Angle angle;
    angle.radian_angle_ = radian;

    return angle;
}


Angle qrk::deg(int degree)
{
    Angle angle;
    angle.radian_angle_ = M_PI * degree / 180.0;

    return angle;
}


const Angle operator * (double lhs, const Angle &rhs)
{
    return Angle(rhs) * lhs;
}

