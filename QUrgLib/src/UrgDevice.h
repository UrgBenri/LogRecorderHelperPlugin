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

#ifndef QRK_URG_DEVICE_H
#define QRK_URG_DEVICE_H

/*!
  \file
  \brief URG sensor control

  \author Satofumi KAMIMURA

  $Id: UrgDevice.h 143 2012-09-25 02:13:28Z kristou $
*/

#include "RangeSensor.h"
//#include "Coordinate.h"
#include <QVector>
#include <memory>

using namespace std;

namespace qrk
{

class Connection;

//! URG sensor control
class UrgDevice : public RangeSensor
{
public:

    //! Parameter of options
    enum {
        DefaultBaudrate = 115200, //!< [bps]
        DefaultRetryTimes = 8,
        Infinity = 0,

        Off = 0,                  //!< Laser is off
        On = 1,                   //!< Laser is on
    };

    typedef enum {
        Normal,
        Basis,
    } AreaType;

    UrgDevice(void);
    virtual ~UrgDevice(void);

    const char* what(void) const;

    bool connect(const char* device, long baudrate = DefaultBaudrate);
    void setConnection(Connection* con);
    Connection* connection(void);
    void disconnect(void);
    bool isConnected(void) const;

    long minDistance(void) const;
    long maxDistance(void) const;
    int minArea(void) const;
    int maxArea(void) const;
    int maxScanLines(void) const;

    int startStep(void) const;
    int endStep(void) const;


    /*!
      \brief Set number of scan data stored internally

      \param[in] size Number of scanning data to be stored
    */
    void setCapturesSize(size_t size);


    /*!
      \brief Time taken for 1scan.

      \return Time taken for 1scan.[msec]
    */
    int scanMsec(void) const;


    /*!
      \brief Specification of data acquisition mode

      \param[in] mode The mode defined by #RangeCaptureMode can be specified

      \see captureMode()
    */
    void setCaptureMode(RangeCaptureMode mode);


    /*!
      \brief Data acquisition mode

      \return Present data acquisition mode

      \see setCaptureMode()
    */
    RangeCaptureMode captureMode(void);


    /*!
      \brief Specifies the range of data acquisition

      \ref Refer to scip_capture_parameter_section.

      \param[in] begin_index Measurement beginning position
      \param[in] end_index Measurement end position
    */
    void setCaptureRange(int begin_index, int end_index);


    /*!
      \brief Acquisition interval of scanning data

      The data traffic between URG and the library can be decreased by lowering the frequency of data acquisition.
      \ref refer to scip_capture_parameter_section

      \param[in] interval capture interval
    */
    void setCaptureFrameInterval(size_t interval);


    /*!
      \brief Specify the frequency of data acquisition

      \ref Refer to scip_capture_parameter_section

      \param[in] times Frequency of data acquisition

      \attention Specify #UrgInfinityTimes to acquire data infinity times.
      \attention Data acquisition mode is valid only at #AutoCapture, #IntensityCapture.
    */
    void setCaptureTimes(size_t times);


    /*!
      \brief Get number of remaining capture times

      \return number of remaining capture times

      \attention Data acquisition mode is valid only at #AutoCapture, #IntensityCapture
    */
    size_t remainCaptureTimes(void);


    /*!
      \brief Get sensor type

      \return sensor type
    */
    const char* getType(void);


    /*!
      \brief Get sensor serial number

      \return sensor serial number
    */
    const char* getSerialNumber(void);

    const char* getFirmwareVersion(void);


    void setCaptureGroupSteps(size_t group_steps);
    size_t captureGroupSteps() const;

    int capture(SensorDataArray &ranges, SensorDataArray &levels, long &timestamp);


    /*!
      \brief Stop data acquisition

      \attention Data acquisition mode is valid only at #AutoCapture, #IntensityCapture
    */
    void stop(void);


    bool setTimestamp(int timestamp = 0, int* response_msec = NULL,
                      int* force_delay_msec = NULL);

    long recentTimestamp(void) const;


    bool setLaserOutput(bool on);

    double index2rad(const int index) const;
    int rad2index(const double radian) const;

    void setParameter(const RangeSensorParameter &parameter);
    RangeSensorParameter parameter(void) const;
    RangeSensorInformation information(void) const;
    RangeSensorInternalInformation internalInformation(void) const;

    RangeSensorParameter parameterNow(void) const;
    RangeSensorInformation informationNow(void) const;
    RangeSensorInternalInformation internalInformationNow(void) const;
    QVector<string> supportedCommands(void) const;
    QStringList supportedModes(bool force = false) const;
    bool isSupportedMode(RangeCaptureMode mode);

    bool loadParameter(void);

    bool loadInformation(void);

    bool versionLines(QVector<string> &lines);
    bool commandLines(string cmd);
    bool commandLines(string cmd, int &status, QVector<string> &lines, bool all = false);
    bool commandLines(string cmd, QVector<string> &lines, bool all = false);
    bool commandLines(int &status, QVector<string> &lines, bool all = false);
    bool commandLines(QVector<string> &lines, bool all = false);

    /*!
      \brief Reboot URG

      \attention Only Top-URG (2010-02-04)
    */
    bool reboot(void);


    bool loadParameters();

    bool isWorking();

    long decode(string word, int size = 0);

    bool setConnectionDebug(bool mode, const QString &locationPart = QCoreApplication::applicationFilePath(),
                            const QString &sendFilePart = "SendFile",
                            const QString &receiveFilePart = "ReceiveFile");

    bool changeBaurate(long baud);

    Converter getConverter();

protected:
    virtual void captureReceived(void);


private:
    UrgDevice(const UrgDevice &rhs);
    UrgDevice &operator = (const UrgDevice &rhs);

    struct pImpl;
    auto_ptr<pImpl> pimpl;
    bool m_debugMode;
};
}

#endif /* !QRK_URG_DEVICE_H */

