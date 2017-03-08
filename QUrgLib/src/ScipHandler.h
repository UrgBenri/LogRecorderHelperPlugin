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

#ifndef QRK_SCIP_HANDLER_H
#define QRK_SCIP_HANDLER_H

/*!
  \file
  \brief Process SCIP communication

  \author Satofumi KAMIMURA

  $Id: ScipHandler.h 142 2012-09-25 00:47:50Z kristou $
*/

#include "CaptureSettings.h"
#include <memory>
#include <QVector>
#include <QStringList>

using namespace std;


namespace qrk
{
class Connection;
class RangeSensorParameter;
class RangeSensorInformation;
class RangeSensorInternalInformation;


//! Handler to SCIP protocol.
class ScipHandler
{
public:
    static const bool Off = false;
    static const bool On = true;
    static const bool Force = true;

    ScipHandler(void);
    virtual ~ScipHandler(void);

    const char* what(void) const;

    static unsigned long decode(const char* data, size_t size);
    static bool checkSum(const char* buffer, int size, char actual_sum);

    void setConnection(Connection* con);
    Connection* connection(void);

    // Match the baudrate and then connect the device.
    bool connect(const char* device, long baudrate);

    int send(const char data[], int size);
    int recv(char data[], int size, int timeout);

    // Read the parameter from URG device.
    bool loadParameter(RangeSensorParameter &parameters);
    bool loadInformation(RangeSensorInformation &informations);
    bool loadInternalInformation(RangeSensorInternalInformation &internalInformations);

    QVector<string> supportedCommands();
    QStringList supportedModes();

    bool commandLines(string &cmd, int &status, QVector<string>* lines = NULL, bool all = false);
    bool commandLines(string &cmd, QVector<string >* lines = NULL, bool all = false);
    bool commandLines(int &status, QVector<string>* lines = NULL, bool all = false);
    bool commandLines(QVector<string >* lines = NULL, bool all = false);
    bool versionLines(QVector<string> &lines);
    bool parameterLines(QVector<string> &lines);
    bool informationLines(QVector<string> &lines);

    //! Transit to timestamp mode
    bool setRawTimestampMode(bool on);

    bool rawTimestamp(long &timestamp);

    bool setLaserOutput(bool on, bool force = false);

    CaptureType receiveCaptureData(QVector<QVector<long> > &ranges, QVector<QVector<long> > &levels,
                                   CaptureSettings &settings, long &timestamp,
                                   int* remain_times = NULL,
                                   int* total_times = NULL);
    bool isContiniousMode();

private:
    ScipHandler(const ScipHandler &rhs);
    ScipHandler &operator = (const ScipHandler &rhs);

    struct pImpl;
    auto_ptr<pImpl> pimpl;
};
}

#endif /* !QRK_SCIP_HANDLER_H */

