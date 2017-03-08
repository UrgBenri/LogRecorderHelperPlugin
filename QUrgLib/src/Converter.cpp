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

#include "Converter.h"
#include <QDebug>
#include <QLineF>
#include <cmath>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif
Converter::Converter(int frontStep, int totalSteps, int firstStep, int lastStep, int grouping):
    m_frontStep(frontStep),
    m_totalSteps(totalSteps),
    m_firstStep(firstStep),
    m_lastStep(lastStep),
    m_grouping(grouping > 0 ? grouping : 1)
{
}

Converter::Converter(const Converter &rhs)
{
    m_frontStep = rhs.m_frontStep;
    m_totalSteps = rhs.m_totalSteps;
    m_firstStep = rhs.m_firstStep;
    m_lastStep = rhs.m_lastStep;
    m_grouping = rhs.m_grouping;
}

Converter &Converter::operator=(const Converter &rhs)
{
    if(this != &rhs)
    {
        m_frontStep = rhs.m_frontStep;
        m_totalSteps = rhs.m_totalSteps;
        m_firstStep = rhs.m_firstStep;
        m_lastStep = rhs.m_lastStep;
        m_grouping = rhs.m_grouping;
    }
    return *this;
}

#ifdef Q_COMPILER_RVALUE_REFS

Converter::Converter(Converter &&rhs)
{
    m_frontStep = std::move(rhs.m_frontStep);
    m_totalSteps = std::move(rhs.m_totalSteps);
    m_firstStep = std::move(rhs.m_firstStep);
    m_lastStep = std::move(rhs.m_lastStep);
    m_grouping = std::move(rhs.m_grouping);
}

Converter &Converter::operator=(Converter &&rhs)
{
    if(this != &rhs)
    {
        m_frontStep = std::move(rhs.m_frontStep);
        m_totalSteps = std::move(rhs.m_totalSteps);
        m_firstStep = std::move(rhs.m_firstStep);
        m_lastStep = std::move(rhs.m_lastStep);
        m_grouping = std::move(rhs.m_grouping);
    }
    return *this;
}
#endif

qreal Converter::index2rad(int index) const
{
    index = qBound(m_firstStep, index, m_lastStep);
    int index_from_front = index - m_frontStep;
    return index_from_front * (2.0 * M_PI) / m_totalSteps;
}

qreal Converter::index2deg(int index) const
{
    return rad2deg(index2rad(index));
}
int Converter::grouping() const
{
    return m_grouping;
}

void Converter::setGrouping(int grouping)
{
    m_grouping = grouping;
}

int Converter::rad2index(qreal radian) const
{
    qreal resolution = m_totalSteps / (2.0 * M_PI);
    int step = qRound((resolution * radian) + 0.5);
    return qBound(m_firstStep, step + m_frontStep, m_lastStep);
}

int Converter::deg2index(qreal degree) const
{
    qreal angle = deg2rad(degree);
    angle = angle > M_PI ? angle - (2 * M_PI) : angle;
    return rad2index(angle);
}

qreal Converter::deg2rad(qreal degree)
{
    const qreal ratio = M_PI / 180.0;
    return degree * ratio;
}

qreal Converter::rad2deg(qreal rad)
{
    const qreal ratio = 180.0 / M_PI;
    return rad * ratio;
}

QPointF Converter::range2point(int step,
                               long range,
                               qreal angle_offset,
                               QPointF spacial_offset,
                               long maxLength) const
{
    long distance = range > 0 ? range : 0;
    if(maxLength > 0)
    {
        if(distance > maxLength)
        {
            distance = maxLength;
        }
    }

    qreal angle = index2rad(step) + angle_offset;
    qreal x = static_cast<qreal>(distance * cos(angle)) + spacial_offset.x();
    qreal y = static_cast<qreal>(distance * sin(angle)) + spacial_offset.y();

    return QPointF(x, y);
}

QVector<QVector<QPointF> > Converter::getPoints(const QVector<QVector<long> > &steps
                                                , const QPointF &offset
                                                , qreal rotation
                                                , int max_length) const
{

    QVector<QVector<QPointF> > points;

    for (int i = 0; i < steps.size(); ++i) {
        QVector<QPointF> point;
        for (int j = 0; j < steps[i].size(); ++j) {
            point << range2point(index2Step(i)
                                 , steps[i][j]
                                 , rotation
                                 , offset
                                 , max_length);
        }
        points << point;
    }

    return points;
}
int Converter::index2Step(int index) const
{
    return m_firstStep + (index * m_grouping);
}

int Converter::step2Index(int step) const
{
    return (step - m_firstStep) / m_grouping;
}

long Converter::point2range(QPointF point,
                            qreal angle_offset,
                            QPointF spacial_offset) const
{
    QLineF line(QPointF(0, 0), point - spacial_offset);
    qreal pointAngle = (line.angle() * M_PI / 180.0) - angle_offset;

    int step = qRound((pointAngle / m_totalSteps) + 0.5) +
            m_frontStep - m_firstStep;

    step = qMin(qMax(0, step), m_lastStep);
    return step;
}

int Converter::frontStep() const
{
    return m_frontStep;
}

void Converter::setFrontStep(int value)
{
    m_frontStep = value;
}

int Converter::totalSteps() const
{
    return m_totalSteps;
}

void Converter::setTotalSteps(int value)
{
    m_totalSteps = value;
}

bool Converter::isValid() const
{
    return m_frontStep > 0 && m_totalSteps > 0;
}


int Converter::firstStep() const
{
    return m_firstStep;
}

int Converter::lastStep() const
{
    return m_lastStep;
}

void Converter::setLastStep(int lastStep)
{
    m_lastStep = lastStep;
}

void Converter::setFirstStep(int firstDtep)
{
    m_firstStep = firstDtep;
}

