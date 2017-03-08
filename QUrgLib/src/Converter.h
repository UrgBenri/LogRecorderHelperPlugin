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

#ifndef CONVERTER_H
#define CONVERTER_H

#include <QPointF>

class Converter
{
public:
    Converter(int frontStep = 0, int totalSteps = 0, int firstStep = 0, int lastStep = 0, int grouping = 0);

    Converter(const Converter& rhs);
    Converter& operator=(const Converter& rhs);

#ifdef Q_COMPILER_RVALUE_REFS

    Converter(Converter&& rhs);
    Converter& operator=(Converter&& rhs);
#endif
    virtual QPointF range2point(int step,
                        long range,
                        qreal angle_offset = 0,
                        QPointF spacial_offset = QPointF(0,0),
                        long maxLength = -1) const;
    virtual long point2range(QPointF point,
                    qreal angle_offset = 0,
                    QPointF spacial_offset = QPointF(0,0)) const;

    qreal index2rad(int index) const;
    int rad2index(qreal radian) const;

    int frontStep() const;
    void setFrontStep(int value);

    int totalSteps() const;
    void setTotalSteps(int value);

    bool isValid() const;

    int firstStep() const;
    void setFirstStep(int firstStep);

    int lastStep() const;
    void setLastStep(int lastStep);

    int deg2index(qreal degree) const;
    qreal index2deg(int index) const;

    int grouping() const;
    void setGrouping(int grouping);

    int index2Step(int index) const;
    int step2Index(int step) const;

    static qreal deg2rad(qreal degree);
    static qreal rad2deg(qreal rad);

    QVector<QVector<QPointF> > getPoints(const QVector<QVector<long > > &steps
                                         , const QPointF &offset = QPointF(0, 0)
                                         , qreal rotation = 0
                                         , int max_length = -1) const;

private:
    int m_frontStep;
    int m_totalSteps;
    int m_firstStep;
    int m_lastStep;
    int m_grouping;
};

#endif // CONVERTER_H

