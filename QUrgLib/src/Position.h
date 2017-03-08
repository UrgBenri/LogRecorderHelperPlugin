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

#ifndef QRK_POSITION_H
#define QRK_POSITION_H

#include "Angle.h"
#include <iostream>


namespace qrk
{
/*!
  \brief 位置
*/
template<class T>
class Position
{
public:
    T x;
    T y;
    Angle angle;


    Position(void) : x(0), y(0), angle(deg(0)) {
    }


    Position(const Position &rhs) : x(rhs.x), y(rhs.y), angle(rhs.angle) {
    }


    Position(T x_, T y_, const Angle &angle_) : x(x_), y(y_), angle(angle_) {
    }


    int to_deg(void) const {
        return angle.to_deg();
    }


    double to_rad(void) const {
        return angle.to_rad();
    }


    bool operator == (const Position &rhs) const {
        if ((this->x == rhs.x) && (this->y == rhs.y) &&
            (this->angle.to_rad() == rhs.angle.to_rad())) {
            return true;
        } else {
            return false;
        }
    }


    Position &operator = (const Position<T> &rhs) {
        this->x = rhs.x;
        this->y = rhs.y;
        this->angle = rhs.angle;

        return *this;
    }

    Position<T> &operator += (const Position<T> &rhs) {
        this->x += rhs.x;
        this->y += rhs.y;
        this->angle += rhs.angle;

        return *this;
    }


    const Position<T> operator + (const Position<T> &rhs) const {
        return Position<T>(*this) += rhs;
    }


    Position<T> &operator -= (const Position<T> &rhs) {
        this->x -= rhs.x;
        this->y -= rhs.y;
        this->angle -= rhs.angle;

        return *this;
    }


    const Position<T> operator - (const Position<T> &rhs) const {
        return Position<T>(*this) -= rhs;
    }


    Position<T> &operator *= (const T &rhs) {
        this->x *= rhs;
        this->y *= rhs;
        this->angle *= rhs;

        return *this;
    }


    Position<T> operator * (const T &rhs) const {
        Position<T> ret(*this);
        return ret *= rhs;
    }


    friend const Position<T> operator * (const T &lhs,
                                         const Position<T> &rhs) {
        return Position<T>(rhs) * lhs;
    }


    friend std::ostream &operator << (std::ostream &out,
                                      const Position<T> &rhs) {
        out << '(' << rhs.x << ", " << rhs.y
            << ", deg(" << rhs.angle.to_deg() << "))";

        return out;
    }
};
}

#endif /* !QRK_POSITION_H */

