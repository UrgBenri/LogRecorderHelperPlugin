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

#ifndef QRK_RANGE_SENSOR_H
#define QRK_RANGE_SENSOR_H

/*!
  \file
  \brief Definition of laser sensor interface

  \author Satofumi KAMIMURA

  $Id: RangeSensor.h 143 2012-09-25 02:13:28Z kristou $
*/

#include "Connection.h"
#include "RangeSensorInformation.h"
#include "RangeSensorInternalInformation.h"
#include "RangeSensorParameter.h"

#include "RangeCaptureMode.h"
#include <qmath.h>
#include <QVector>
#include <string>
#include <QMap>
#include "Converter.h"

typedef struct {
    QVector<QVector<long > > steps;
    Converter converter;
    qint64 timestamp;
} SensorDataArray;

Q_DECLARE_METATYPE(SensorDataArray)
Q_DECLARE_METATYPE(Converter)

namespace qrk
{
class Connection;
class RangeSensorParameter;
class RangeSensorInformation;
class RangeSensorInternalInformation;


//! Range sensor interface
class RangeSensor
{
public:
    virtual ~RangeSensor(void) {}


    /*!
      \brief Returns internal status

      \return String that represent internal status

      <strong>Example</strong>
    \code
    UrgCtrl sensor;
    if (! sensor.connect(device, baudrate)) {
    // Displays error message when connection is failed.
    printf("connect: %s\n", sensor.what());
    exit(1);
    } \endcode
    */
    virtual const char* what(void) const = 0;


    /*!
      \brief Connection

      \param[in] device Connected device name
      \param[in] baudrate Baudrate

      \retval true Success
      \retval false Failure

      <strong>Example</strong>
    \code
    const char device[] = "/dev/ttyACM0";
    const long baudrate = 115200;
    UrgCtrl sensor;

    // Connect to specified device
    if (! sensor.connect(device, baudrate)) {
    printf("connect: %s\n", sensor.what());
    exit(1);
    } \endcode
    */
    virtual bool connect(const char* device, long baudrate) = 0;


    /*!
      \brief Specifies the connection object

      \param[in] con Connection object
    */
    virtual void setConnection(Connection* con) = 0;


    /*!
      \brief Get connection object

      \return Connection object
    */
    virtual Connection* connection(void) = 0;


    /*!
      \brief Disconnect
    */
    virtual void disconnect(void) = 0;


    /*!
      \brief Returns connection status

      \retval true If connected.
      \retval false if disconnected.
    */
    virtual bool isConnected(void) const = 0;


    /*!
      \brief Get valid minimum distance

      \return valid minimum distance
    */
    virtual long minDistance(void) const = 0;


    /*!
      \brief Get valid maximum distance

      \return Valid maximum distance
    */
    virtual long maxDistance(void) const = 0;


    /*!
    \brief Get initial minimum area

    \return Initial minimum area
    */
    virtual int minArea(void) const = 0;


    /*!
    \brief Get initial maximum area

    \return Initial maximum area
    */
    virtual int maxArea(void) const = 0;

    /*!
    \brief Get current start step

    \return Current start step
    */
    virtual int startStep(void) const = 0;


    /*!
    \brief Get current end step

    \return Current end step
    */
    virtual int endStep(void) const = 0;


    /*!
      \brief Get Max scan index

      \return Max scan index
    */
    virtual int maxScanLines(void) const = 0;

    virtual int scanMsec(void) const = 0;

    virtual void setCaptureMode(RangeCaptureMode mode) = 0;
    virtual RangeCaptureMode captureMode(void) = 0;

    virtual void setCaptureGroupSteps(size_t group_steps) = 0;
    virtual size_t captureGroupSteps() const = 0;


//    /*!
//      \brief Get data

//      Get data and store it in buffer.

//      \param[out] data Buffer to store data
//      \param[out] timestamp Time stamp

//      \return Number of data recieved.
//      \retval <0 Receiving failed
//    */
//    virtual int capture(QVector<long> &ranges, long &timestamp) = 0;


//    virtual int captureWithIntensity(QVector<long> &ranges,
//                                     QVector<long> &levels,
//                                     long &timestamp) = 0;


    /*!
      \brief Sets the timestamp value. Sensor returs this timestamp value.

      Sets the time stamp that can be acquired with capture().

      \param[in] ticks Set value of time stamp at that time
      \param[out] response_msec Response time for sending and receiving message [msec]
      \param[in] force_delay_msec Forced delay

      \retval true Success
      \retval false Failure
    */
    virtual bool setTimestamp(int timestamp = 0, int* response_msec = NULL,
                              int* force_delay_msec = NULL) = 0;


    /*!
      \brief Latest time stamp value

      \return Time stamp value
    */
    virtual long recentTimestamp(void) const {
        return 0;
    }


    virtual bool setLaserOutput(bool on) = 0;


    /*!
      \brief Radian to angle conversion of data index

      Front of sensor is considered as 0.0 in radians.

      \param[in] index Data index

      \return angle [radian]

      \image html sensor_radian.png upper view

      \see index2deg(), rad2index(), deg2index()
    */
    virtual double index2rad(const int index) const = 0;


    /*!
      \brief Degree to angle conversion of data index

      Front of sensor is considered as 0.0 in radians.

      \param[in] index Data index

      \return angle [degree]

      \image html sensor_radian.png upper view

      \see index2rad(), rad2index(), deg2index()
    */
    int index2deg(const int index) const {
        return static_cast<int>(floor((180.0 * index2rad(index) / M_PI) + 0.5));
    }


    /*!
      \brief radian Convert angle in radian to data index

       Front of sensor is considered as 0.0 in radians.

      \param[in] radian angle [radian]

      \return Data index

      \see deg2index(), index2rad(), index2deg()
    */
    virtual int rad2index(const double radian) const = 0;


    /*!
      \brief Angle conversion of data index

      Front of sensor is considered as 0.0 in radians.

      \param[in] degree angle [degree]

      \return angle [degree]

      \see rad2index(), index2rad(), index2deg()
    */
    int deg2index(const int degree) const {
        return rad2index(degree * M_PI / 180.0);
    }


    /*!
      \brief Update URG parameter

      \param[in] parameter data used to update URG parameter
    */
    virtual void setParameter(const RangeSensorParameter &parameter) = 0;


    /*!
      \brief Get URG parameter

      \return URG parameter
    */
    virtual RangeSensorParameter parameter(void) const = 0;
    virtual RangeSensorInformation information(void) const = 0;
    virtual RangeSensorInternalInformation internalInformation(void) const = 0;

    virtual RangeSensorParameter parameterNow(void) const = 0;
    virtual RangeSensorInformation informationNow(void) const = 0;
    virtual RangeSensorInternalInformation internalInformationNow(void) const = 0;

    virtual QVector<std::string> supportedCommands(void) const = 0;
    virtual QStringList supportedModes(bool force = false) const = 0;

    virtual bool isWorking() = 0;

    virtual bool setConnectionDebug(bool mode, const QString &locationPart = QCoreApplication::applicationFilePath(),
                                    const QString &sendFilePart = "SendFile",
                                    const QString &receiveFilePart = "ReceiveFile") = 0;

    virtual bool changeBaurate(long baud) = 0;

    virtual Converter getConverter() = 0;
};
}

#endif /* !QRK_RANGE_SENSOR_H */

