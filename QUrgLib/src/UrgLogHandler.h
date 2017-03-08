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

#ifndef URG_LOG_HANDLER_H
#define URG_LOG_HANDLER_H

/*!
  \file
  \author $author$

  $Id: UrgLogHandler.h 146 2012-09-27 23:30:56Z kristou $
*/
#include "RangeSensor.h"
#include "RangeCaptureMode.h"
#include "RangeSensorParameter.h"
#include <QVector>
#include "BasicExcel.hpp"
using namespace YExcel;

#include <QFile>

#include <QObject>
#include <QPoint>
#include <QVector>
#include <QMutex>
#include <QStringList>

using namespace std;
using namespace qrk;


class UrgLogHandler: public QObject
{
    Q_OBJECT
public:
    UrgLogHandler(const UrgLogHandler &rhs);
    UrgLogHandler &operator = (const UrgLogHandler &rhs);
    UrgLogHandler(void);
    virtual ~UrgLogHandler(void);

    void setRangeMode(bool state) { m_isRange = state; }


    bool create(const QString &file_name);

    bool addData(size_t timestamp, const QVector<QPoint> &points);

    bool close();

    bool load(const QString &file_name);

    int getLastTimestamp(void);

    bool getData(size_t &timestamp, QVector<QPoint> &points,
                 size_t current_timestamp);

    //! Setting the capture mode to define the log format
    long addCaptureMode(RangeCaptureMode mode);

//    long addData(QVector<long> &ranges, QVector<long> &levels, long timestamp);

    //! setting separator used in log file
    void setSeparators(const QString &block, const QString &ranges, const QString &levels);

    long setDataPos(long pos);
    QStringList headerRecords() const { return m_header; }


    long add(const QString &key, int value);
    long add(const QString &key, const QString &value);
    long add(const QString &key, long value);

    long addModel(const QString &value);
    long addSerialNumber(const QString &value);
    long addSensorVersion(const QString &value);
    long addStartStep(int value);
    long addEndStep(int value);
    long addGrouping(int value);
    long addFrontStep(int value);
    long addTotalSteps(int value);
    long addMinDistance(long value);
    long addMaxDistance(long value);
    long addMotorSpeed(int value);
    long addScanMsec(int value);

    void setMaxEchoNumber(int number);

    RangeCaptureMode readCaptureMode(const QString &key);

//    long getData(QVector<long> &ranges, QVector<long> &levels, long &timestamp);
    long getData(SensorDataArray &ranges, SensorDataArray &levels, long &timestamp);
    long getTimestamp(long &timestamp);

    QString getAppName() {return appName;}
    QString getAppVersion() {return appVersion;}
    QString getModel() {return model;}
    QString getSerialNumber() {return serialNumber;}
    QString getFirmwareVersion() {return firmwareVersion;}
    int getStartStep() {return startStep;}
    void setReadStartStep(int step);
    int getReadStartStep() {return m_startStepRead;}
    int getEndStep() {return endStep;}
    int getGrouping() {return grouping;}
    void setReadEndStep(int step);
    int getReadEndStep() {return m_endStepRead;}
    long getReadPosition() {return m_readPosition;}
    long getWritePosition() {return m_writePosition;}
    int getFrontStep() {return frontStep;}
    int getTotalSteps() {return totalSteps;}
    long getMinDistance() {return minDistance;}
    long getMaxDistance() {return maxDistance;}
    int getMotorSpeed() {return motorSpeed;}
    int getScanMsec() {return scanMsec;}

    Converter getConverter();

    RangeCaptureMode getCaptureMode();

    QString what();
    void useFlush(bool state);

    int headerCheck();
    bool getDataInit();

    QString getFileName() {return m_filename;}

    Q_DECL_DEPRECATED bool timestampSequential(long &totalTimestamp, long &skipTimestamp);
    bool scanCoherent(bool noFreeze = true);

    long addRangeSensorParameter(RangeSensorParameter parameter);


    int readIntFromHeader(const QString &key);
    long readLongFromHeader(const QString &key);
    QString readStringFromHeader(const QString &key);

    QStringList supportedModes();
    bool isOpen();

    RangeSensorParameter getRangeSensorParameter();
//    void eraseVectorLast(QVector<long> &ranges, int limit, int startS);
//    void eraseVectorFront(QVector<long> &ranges, int limit, int startS);
    int guessStartStep(const QString &model);
    int guessEndStep(const QString &model);
    int guessGrouping(const QString &model);
    int guessFrontStep(const QString &model);
    int guessTotalSteps(const QString &model);
    int guessMaxDistance(const QString &model);
    int guessMinDistance(const QString &model);
    int guessScanMsec(const QString &model);
    void skipDataForward(long pos);
//    void getAllData(QVector<QVector<long> > &vdata, QVector<QVector<long> > &vlevels, QVector<long> &vtimestamp);
    void getAllData(QVector<SensorDataArray> &vdata, QVector<SensorDataArray> &vlevels, QVector<long> &vtimestamp);
    void skipDataBackward(long pos);
    long getTotalTimestamps();

    void setCaptureMode(RangeCaptureMode mode);
//    bool getDataFromCSV(const QString &source,
//                        QVector<QVector<long> > &ranges,
//                        QVector<QVector<long> > &levels,
//                        int distanceNumber,
//                        int intensityNumber);
    bool getDataFromCSV(const QString &source,
                        QVector<SensorDataArray> &ranges,
                        QVector<SensorDataArray> &levels,
                        int distanceNumber,
                        int intensityNumber);
//    void trimVector(QVector<QVector<long> > &ldata);
//    static QVector<long> getEcho(int echo, QVector<long> &ranges, int length);
    bool init(bool noFreeze = false);
    long getSkippedTimeStamps();

    QString getLogTime() {return m_logTime;}
    long getTimeStamp() {return m_timeStamp;}

    void stopInit() {m_shouldStopInit = true;}

//    long getNextData(QVector<long> &ranges, QVector<long> &levels, long &timestamp);
    long getNextData(SensorDataArray &ranges, SensorDataArray &levels, long &timestamp);
//    void convertData(QVector<QVector<long> > &output, QVector<long> &input);
    long addData(const SensorDataArray &ranges, const SensorDataArray &levels, long timestamp);

    bool fileExists();
    long timestampAt(qint64 pos);

signals:
    void initProgress(int progress);

private:
    enum LogMode {
        UnknownMode = 0,
        WriteMode,
        ReadMode,
    };

    LogMode m_currentMode;
    QString m_filename;


//    ifstream fin;
//    ofstream fout;

    QFile m_sin;
    QFile m_sout;

    int m_lastTimestamp;
    int m_cached_timestamp;
    QVector<QPoint> m_cached_points;

    RangeCaptureMode m_captureMode;
    RangeCaptureMode m_captureModeRead;

    QString blockSeparator;
    QString dataSeparator;
    QString intensitySeparator;

    QString m_errorMessage;

    bool m_useFlush;

    QString appName;
    QString appVersion;
    QString model;
    QString serialNumber;
    QString firmwareVersion;
    int startStep;
    int endStep;
    int grouping;
    int frontStep;
    int totalSteps;
    long minDistance;
    long maxDistance;
    int motorSpeed;
    int scanMsec;

    QString applicationVersionKey;
    QString applicationNameKey;
    QString captureModeKey; //"[captureMode]"
    QString modelKey;
    QString serialNumberKey;
    QString firmwareVersionKey;
    QString startStepKey;
    QString endStepKey;
    QString groupingKey;
    QString frontStepKey;
    QString totalStepsKey;
    QString minDistanceKey;
    QString maxDistanceKey;
    QString motorSpeedKey;
    QString scanMsecKey;
    QString timestampKey; //"[timestamp]"
    QString scanKey; //"[scan]"
    QString logtimeKey; //"[logtime]"


    int m_startStepRead;
    int m_endStepRead;

    long m_readPosition;
    long m_writePosition;

    long m_totalTimestamps;

    RangeSensorParameter m_urgParameter;

    QVector<qint64> m_markPoints;

    QString m_logTime;
    long m_timeStamp;
    bool m_shouldStopInit;

    QMutex m_mutex;

    QString m_logFormat;
    BasicExcel m_excel;
    int m_maxEchoNumber;
    QStringList m_header;
    bool m_firstTime;
    bool m_isRange;

    bool m_isClosed;

    //    QVector<long> getFirstEcho(QVector<long> &ranges);
    void getFirstEcho(SensorDataArray &ranges);
    long addDataUbh(const SensorDataArray &ranges, const SensorDataArray &levels, long timestamp);
    long addDataXls(const SensorDataArray &ranges, const SensorDataArray &levels, long timestamp);
    long addDataCsv(const SensorDataArray &ranges, const SensorDataArray &levels, long timestamp);
    long addDataXy(const SensorDataArray &ranges, const SensorDataArray &levels, long timestamp);
    void initHeaderRecords();
};

#endif /* !URG_LOG_HANDLER_H */

