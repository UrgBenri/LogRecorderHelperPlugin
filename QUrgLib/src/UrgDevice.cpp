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

#include <QMutex>
#include <QDebug>
#include <QTime>
#include <QApplication>
#include <QFileInfo>
#include <QStringList>

#include "ticks.h"

#include "UrgDevice.h"
#include "ConnectionUtils.h"
#include "SerialDevice.h"
#include "ScipHandler.h"
#include "RangeSensorParameter.h"
#include "RangeSensorInformation.h"
#include "RangeSensorInternalInformation.h"


//#include "LockGuard.h"
//#include "Lock.h"
#include <limits>
#include <cstring>
#include <cstdio>
#include <stdlib.h>
#include "log_printf.h"
#include <iostream>
#include <algorithm>

//#define DEBUG

#ifdef DEBUG
#include <QTime>
#endif

#ifdef Q_CC_MSVC
#define snprintf _snprintf
#endif

using namespace qrk;
using namespace std;

namespace
{
enum {
    MdScansMax = 100,           // [times]
    NdScansMax = 100,           // [times]
};
}

struct UrgDevice::pImpl {
    class Capture
    {
    public:
        virtual ~Capture(void) {
        }

        virtual string createCaptureCommand(void) = 0;
        virtual int capture(QVector<QVector<long> > &ranges, QVector<QVector<long> > &levels, long &timestamp) = 0;
        virtual void setCapturesSize(size_t size) = 0;
        virtual size_t capturesSize(void) = 0;
        virtual size_t remainCaptureTimes(void) = 0;
    };

    class GD_Capture : public Capture
    {
        pImpl* pimpl_;

    public:
        explicit GD_Capture(pImpl* pimpl) : pimpl_(pimpl) {
        }


        ~GD_Capture(void) {
        }

        string createCaptureCommand(void) {
            char buffer[] = "GDbbbbeeeegg\n";
            snprintf(buffer, strlen(buffer) + 1, "GD%04d%04d%02u\n",
                     pimpl_->capture_begin_, pimpl_->capture_end_,
                     (unsigned int)pimpl_->capture_group_steps_);

            return buffer;
        }

        int capture(QVector<QVector<long> > &ranges, QVector<QVector<long> > &levels, long &timestamp) {
            pimpl_->scip_.setLaserOutput(ScipHandler::On);

            string command = createCaptureCommand();
            int n = pimpl_->scip_.send(command.c_str(),
                                       static_cast<int>(command.size()));
            if (n != static_cast<int>(command.size())) {
                pimpl_->error_message_ = "Send command:" + command + " fail.";
                return -1;
            }

            CaptureSettings settings;
            pimpl_->scip_.receiveCaptureData(ranges, levels, settings, timestamp);

            if(settings.error_code < 0){
                pimpl_->error_message_ = "Connection Timeout reached" ;
                return -2;
            }

            if(settings.error_code != 0){
                pimpl_->error_message_ = "Non valid status received";
                return -1;
            }

            if (ranges.isEmpty()) {
                pimpl_->error_message_ = string("Device capture: ") + pimpl_->scip_.what();
            }
            return static_cast<int>(ranges.size());
        }

        void setCapturesSize(size_t size) {
            static_cast<void>(size);
        }

        size_t capturesSize(void) {
            return 1;
        }

        size_t remainCaptureTimes(void) {
            return 0;
        }
    };


    class GE_Capture : public Capture
    {
        pImpl* pimpl_;

    public:
        explicit GE_Capture(pImpl* pimpl) : pimpl_(pimpl) {
        }


        ~GE_Capture(void) {
        }


        string createCaptureCommand(void) {

            char buffer[] = "GEbbbbeeeegg\n";
            snprintf(buffer, strlen(buffer) + 1, "GE%04d%04d%02u\n",
                     pimpl_->capture_begin_, pimpl_->capture_end_,
                     (unsigned int)pimpl_->capture_group_steps_);

            return buffer;
        }


        int capture(QVector<QVector<long> > &ranges, QVector<QVector<long> > &levels, long &timestamp) {
            pimpl_->scip_.setLaserOutput(ScipHandler::On);

            string command = createCaptureCommand();
            int res = pimpl_->scip_.send(command.c_str(),
                                         static_cast<int>(command.size()));
            if (res != static_cast<int>(command.size())) {
                pimpl_->error_message_ = "Send command:" + command + " fail.";
                return -1;
            }

            CaptureSettings settings;
            pimpl_->scip_.receiveCaptureData(ranges, levels, settings, timestamp);

            if(settings.error_code < 0){
                pimpl_->error_message_ = "Connection Timeout reached" ;
                return -2;
            }

            if(settings.error_code != 0){
                pimpl_->error_message_ = "Non valid status received";
                return -1;
            }

            if (ranges.isEmpty()) {
                pimpl_->error_message_ = string("Device capture: ") + pimpl_->scip_.what();
            }

            return static_cast<int>(ranges.size());
        }


        void setCapturesSize(size_t size) {
            static_cast<void>(size);
        }


        size_t capturesSize(void) {
            return 1;
        }


        size_t remainCaptureTimes(void) {
            return 0;
        }
    };



    class HD_Capture : public Capture
    {
        pImpl* pimpl_;

    public:
        explicit HD_Capture(pImpl* pimpl) : pimpl_(pimpl) {
        }


        ~HD_Capture(void) {
        }


        string createCaptureCommand(void) {
            char buffer[] = "HDbbbbeeeegg\n";
            snprintf(buffer, strlen(buffer) + 1, "HD%04d%04d%02u\n",
                     pimpl_->capture_begin_, pimpl_->capture_end_,
                     (unsigned int)pimpl_->capture_group_steps_);

            return buffer;
        }


        int capture(QVector<QVector<long> > &ranges, QVector<QVector<long> > &levels, long &timestamp) {
            pimpl_->scip_.setLaserOutput(ScipHandler::On);


            string command = createCaptureCommand();
            int n = pimpl_->scip_.send(command.c_str(),
                                       static_cast<int>(command.size()));
            if (n != static_cast<int>(command.size())) {
                pimpl_->error_message_ = "Send command:" + command + " fail.";
                return -1;
            }

            CaptureSettings settings;
            pimpl_->scip_.receiveCaptureData(ranges, levels, settings, timestamp);

            if(settings.error_code < 0){
                pimpl_->error_message_ = "Connection Timeout reached" ;
                return -2;
            }

            if(settings.error_code != 0){
                pimpl_->error_message_ = "Non valid status received";
                return -1;
            }

            if (ranges.isEmpty()) {
                pimpl_->error_message_ = string("Device capture: ") + pimpl_->scip_.what();
            }
            return static_cast<int>(ranges.size());
        }


        void setCapturesSize(size_t size) {
            Q_UNUSED(size);
        }


        size_t capturesSize(void) {
            return 1;
        }


        size_t remainCaptureTimes(void) {
            return 0;
        }
    };


    class HE_Capture : public Capture
    {
        pImpl* pimpl_;

    public:
        explicit HE_Capture(pImpl* pimpl) : pimpl_(pimpl) {
        }


        ~HE_Capture(void) {
        }


        string createCaptureCommand(void) {

            char buffer[] = "HEbbbbeeeegg\n";
            snprintf(buffer, strlen(buffer) + 1, "HE%04d%04d%02u\n",
                     pimpl_->capture_begin_, pimpl_->capture_end_,
                     (unsigned int)pimpl_->capture_group_steps_);

            return buffer;
        }


        int capture(QVector<QVector<long> > &ranges, QVector<QVector<long> > &levels, long &timestamp) {
            pimpl_->scip_.setLaserOutput(ScipHandler::On);

            string command = createCaptureCommand();
            int res = pimpl_->scip_.send(command.c_str(),
                                         static_cast<int>(command.size()));
            if (res != static_cast<int>(command.size())) {
                pimpl_->error_message_ = "Send command:" + command + " fail.";
                return -1;
            }

            CaptureSettings settings;
            pimpl_->scip_.receiveCaptureData(ranges, levels, settings, timestamp);

            if(settings.error_code < 0){
                pimpl_->error_message_ = "Connection Timeout reached" ;
                return -2;
            }

            if(settings.error_code != 0){
                pimpl_->error_message_ = "Non valid status received";
                return -1;
            }

            if (ranges.isEmpty()) {
                pimpl_->error_message_ = string("Device capture: ") + pimpl_->scip_.what();
            }
            return static_cast<int>(ranges.size());
        }


        void setCapturesSize(size_t size) {
            Q_UNUSED(size);
        }


        size_t capturesSize(void) {
            return 1;
        }


        size_t remainCaptureTimes(void) {
            return 0;
        }
    };


    class MD_Capture : public Capture
    {
        pImpl* pimpl_;
        size_t captures_size_;


    public:
        explicit MD_Capture(pImpl* pimpl) : pimpl_(pimpl), captures_size_(1) {
        }


        ~MD_Capture(void) {
        }


        string createCaptureCommand(void) {
            char buffer[] = "MDbbbbeeeeggstt\n";
            snprintf(buffer, strlen(buffer) + 1, "MD%04d%04d%02u%01u%02u\n",
                     pimpl_->capture_begin_, pimpl_->capture_end_,
                     pimpl_->capture_group_steps_,
                     pimpl_->capture_frame_interval_,
                     (pimpl_->capture_times_ > 99) ? 0 : (unsigned int)pimpl_->capture_times_);

            pimpl_->remain_times_ = pimpl_->capture_times_;

            return buffer;
        }


        int capture(QVector<QVector<long> > &ranges, QVector<QVector<long> > &levels, long &timestamp) {

            // Thread syncronization
            QMutexLocker locker(&pimpl_->mutex_);
            if(!pimpl_->scip_.isContiniousMode()){
                string command = createCaptureCommand();
                int res = pimpl_->scip_.send(command.c_str(),
                                             static_cast<int>(command.size()));
                if (res != static_cast<int>(command.size())) {
                    pimpl_->error_message_ = "Send command:" + command + " fail.";
                    return -1;
                }
                //                pimpl_->base_timestamp_ = 0;
            }

            CaptureSettings settings;
            CaptureType type = pimpl_->scip_.receiveCaptureData(ranges, levels, settings, timestamp);

            if(settings.error_code < 0){
                pimpl_->error_message_ = "Connection Timeout reached" ;
                return -2;
            }

            if(settings.error_code != 0 &&
                    settings.error_code != 99){
                pimpl_->error_message_ = "Non valid status received";
                return -1;
            }

            if (!((type == MD) || (type == MS))) {

                pimpl_->error_message_ = "Received data is not of a continious type";
                return -1;
            }

            if (ranges.isEmpty()) {
                pimpl_->error_message_ = string("Device capture: ") + pimpl_->scip_.what();
            }
            return static_cast<int>(ranges.size());
        }


        void setCapturesSize(size_t size) {
            captures_size_ = size;
        }


        size_t capturesSize(void) {
            return captures_size_;
        }


        size_t remainCaptureTimes(void) {
            if (pimpl_->capture_times_ == 0) {
                return numeric_limits<size_t>::max();
            }

            QMutexLocker locker(&pimpl_->mutex_);
            return pimpl_->remain_times_;
        }
    };

    class ME_Capture : public Capture
    {
        pImpl* pimpl_;
        size_t captures_size_;

    public:
        explicit ME_Capture(pImpl* pimpl) : pimpl_(pimpl), captures_size_(1) {
        }

        ~ME_Capture(void) {
        }

        string createCaptureCommand(void) {
            char buffer[] = "MEbbbbeeeeggstt\n";
            snprintf(buffer, strlen(buffer) + 1, "ME%04d%04d%02u%01u%02u\n",
                     pimpl_->capture_begin_, pimpl_->capture_end_,
                     pimpl_->capture_group_steps_,
                     pimpl_->capture_frame_interval_,
                     (pimpl_->capture_times_ > 99) ? 0 : (unsigned int)pimpl_->capture_times_);

            pimpl_->remain_times_ = pimpl_->capture_times_;

            return buffer;
        }


        int capture(QVector<QVector<long> > &ranges, QVector<QVector<long> > &levels, long &timestamp) {

            QMutexLocker locker(&pimpl_->mutex_);
            if(!pimpl_->scip_.isContiniousMode()){
                string command = createCaptureCommand();
                int res = pimpl_->scip_.send(command.c_str(),
                                             static_cast<int>(command.size()));
                if (res != static_cast<int>(command.size())) {
                    pimpl_->error_message_ = "Send command:" + command + " fail.";
                    return -1;
                }
                //                pimpl_->base_timestamp_ = 0;
            }

            CaptureSettings settings;
            CaptureType type = pimpl_->scip_.receiveCaptureData(ranges, levels, settings,timestamp);

            if(settings.error_code < 0){
                pimpl_->error_message_ = "Connection Timeout reached" ;
                return -2;
            }

            if(settings.error_code != 0 &&
                    settings.error_code != 99){
                pimpl_->error_message_ = "Non valid status received";
                return -1;
            }

            if (type != ME) {

                pimpl_->error_message_ = "Received data is not of a continious type";
                return -1;
            }

            if (ranges.isEmpty()) {
                pimpl_->error_message_ = string("Device capture: ") + pimpl_->scip_.what();
            }

            return static_cast<int>(ranges.size());
        }


        void setCapturesSize(size_t size) {
            captures_size_ = size;
        }


        size_t capturesSize(void) {
            return captures_size_;
        }

        size_t remainCaptureTimes(void) {
            if (pimpl_->capture_times_ == 0) {
                return numeric_limits<size_t>::max();
            }

            QMutexLocker locker(&pimpl_->mutex_);
            return pimpl_->remain_times_;
        }
    };


    class ND_Capture : public Capture
    {
        pImpl* pimpl_;
        size_t captures_size_;


    public:
        explicit ND_Capture(pImpl* pimpl) : pimpl_(pimpl), captures_size_(1) {
        }


        ~ND_Capture(void) {
        }


        string createCaptureCommand(void) {
            char buffer[] = "NDbbbbeeeeggstt\n";
            snprintf(buffer, strlen(buffer) + 1, "ND%04d%04d%02u%01u%02u\n",
                     pimpl_->capture_begin_, pimpl_->capture_end_,
                     pimpl_->capture_group_steps_,
                     pimpl_->capture_frame_interval_,
                     (pimpl_->capture_times_ > 99) ? 0 : (unsigned int)pimpl_->capture_times_);

            pimpl_->remain_times_ = pimpl_->capture_times_;

            return buffer;
        }


        int capture(QVector<QVector<long> > &ranges, QVector<QVector<long> > &levels, long &timestamp) {
            QMutexLocker locker(&pimpl_->mutex_);
            if(!pimpl_->scip_.isContiniousMode()){
                string command = createCaptureCommand();
                int res = pimpl_->scip_.send(command.c_str(),
                                             static_cast<int>(command.size()));
                if (res != static_cast<int>(command.size())) {
                    pimpl_->error_message_ = "Send command:" + command + " fail.";
                    return -1;
                }
                //                pimpl_->base_timestamp_ = 0;
            }

            CaptureSettings settings;
            CaptureType type = pimpl_->scip_.receiveCaptureData(ranges, levels, settings, timestamp);

            if(settings.error_code < 0){
                pimpl_->error_message_ = "Connection Timeout reached" ;
                return -2;
            }

            if(settings.error_code != 0 &&
                    settings.error_code != 99){
                pimpl_->error_message_ = "Non valid status received";
                return -1;
            }

            if (type != ND) {
                pimpl_->error_message_ = "Received data is not of a continious type";
                return -1;
            }

            if (ranges.isEmpty()) {
                pimpl_->error_message_ = string("Device capture: ") + pimpl_->scip_.what();
            }

            return static_cast<int>(ranges.size());
        }


        void setCapturesSize(size_t size) {
            captures_size_ = size;
        }


        size_t capturesSize(void) {
            return captures_size_;
        }


        size_t remainCaptureTimes(void) {
            if (pimpl_->capture_times_ == 0) {
                return numeric_limits<size_t>::max();
            }

            QMutexLocker locker(&pimpl_->mutex_);
            return pimpl_->remain_times_;
        }
    };


    class NE_Capture : public Capture
    {
        pImpl* pimpl_;
        size_t captures_size_;


    public:
        explicit NE_Capture(pImpl* pimpl) : pimpl_(pimpl), captures_size_(1) {
        }


        ~NE_Capture(void) {
        }


        string createCaptureCommand(void) {
            char buffer[] = "NEbbbbeeeeggstt\n";
            snprintf(buffer, strlen(buffer) + 1, "NE%04d%04d%02u%01u%02u\n",
                     pimpl_->capture_begin_, pimpl_->capture_end_,
                     pimpl_->capture_group_steps_,
                     pimpl_->capture_frame_interval_,
                     (pimpl_->capture_times_ > 99) ? 0 : (unsigned int)pimpl_->capture_times_);
            pimpl_->remain_times_ = pimpl_->capture_times_;

            return buffer;
        }


        int capture(QVector<QVector<long> > &ranges, QVector<QVector<long> > &levels, long &timestamp) {
            QMutexLocker locker(&pimpl_->mutex_);
            if(!pimpl_->scip_.isContiniousMode()){
                string command = createCaptureCommand();
                int res = pimpl_->scip_.send(command.c_str(),
                                             static_cast<int>(command.size()));
                if (res != static_cast<int>(command.size())) {
                    pimpl_->error_message_ = "Send command:" + command + " fail.";
                    return -1;
                }
                //                pimpl_->base_timestamp_ = 0;
            }

            CaptureSettings settings;
            CaptureType type = pimpl_->scip_.receiveCaptureData(ranges, levels, settings, timestamp);

            if(settings.error_code < 0){
                pimpl_->error_message_ = "Connection Timeout reached" ;
                return -2;
            }

            if(settings.error_code != 0 &&
                    settings.error_code != 99){
                pimpl_->error_message_ = "Non valid status received";
                return -1;
            }

            if (type != NE) {

                pimpl_->error_message_ = "Received data is not of a continious type";
                return -1;
            }

            if (ranges.isEmpty()) {
                pimpl_->error_message_ = string("Device capture: ") + pimpl_->scip_.what();
            }

            return static_cast<int>(ranges.size());
        }


        void setCapturesSize(size_t size) {
            captures_size_ = size;
        }


        size_t capturesSize(void) {
            return captures_size_;
        }

        size_t remainCaptureTimes(void) {
            if (pimpl_->capture_times_ == 0) {
                return numeric_limits<size_t>::max();
            }

            QMutexLocker locker(&pimpl_->mutex_);
            return pimpl_->remain_times_;
        }
    };




    string error_message_;
    UrgDevice* parent_;
    Connection* con_;
    SerialDevice* serial_;
    ScipHandler scip_;
    RangeSensorParameter parameters_;
    RangeSensorInformation informations_;
    RangeSensorInternalInformation internalInformations_;
    QStringList supportedModes;
    string urg_type_;
    long recent_timestamp_;
    int timestamp_offset_;

    RangeCaptureMode capture_mode_;
    GD_Capture gd_capture_;
    GE_Capture ge_capture_;
    HD_Capture hd_capture_;
    HE_Capture he_capture_;
    MD_Capture md_capture_;
    ME_Capture me_capture_;
    ND_Capture nd_capture_;
    NE_Capture ne_capture_;
    Capture* capture_;
    QMutex mutex_;

    int capture_begin_;
    int capture_end_;
    size_t capture_group_steps_;
    int capture_skip_frames_;

    size_t capture_frame_interval_;
    size_t capture_times_;
    size_t remain_times_;
    bool invalid_packet_;

    //    long base_timestamp_;
    //    long pre_timestamp_;

    explicit pImpl(UrgDevice* parent)
        : error_message_("no error."), parent_(parent),
          con_(NULL), serial_(NULL), urg_type_(""),
          recent_timestamp_(0), timestamp_offset_(0),
          capture_mode_(GD_Capture_mode),
          gd_capture_(this), ge_capture_(this),
          hd_capture_(this), he_capture_(this),
          md_capture_(this), me_capture_(this),
          nd_capture_(this), ne_capture_(this),
          capture_(&gd_capture_),
          capture_begin_(0), capture_end_(0),
          capture_group_steps_(1), capture_skip_frames_(0),
          capture_frame_interval_(0), capture_times_(0),
          remain_times_(0), invalid_packet_(false)
        //          base_timestamp_(0), pre_timestamp_(0)
    {
    }


    ~pImpl(void) {
        disconnect();
        if(serial_) delete serial_;
    }


    void initializeSerial(void) {
        if (! serial_) {
            serial_ = new SerialDevice;
        }
        con_ = serial_;
    }


    bool connect(const char* device, long baudrate) {
        error_message_ = "";

        if (! con_) {
            initializeSerial();
        }

        if (con_->isConnected()) {
            disconnect();
        }

        scip_.setConnection(con_);

        if (! scip_.connect(device, baudrate)) {
            error_message_ = scip_.what();
            return false;
        }

        return true;
    }

    bool loadParameters() {
        bool result = true;

        if (loadParameter()){
            updateCaptureParameters();
        }else{
            error_message_ = "Error loading parameters.";
            result = false;
        }

        if (! loadInformation()) {
            error_message_ = "Error loading information.";
            result = false;
        }

        if (! loadInternalInformation()) {
            error_message_ = "Error loading internal information.";
            result = false;
        }

        return result;
    }

    void updateSupportedModes() {
        supportedModes = scip_.supportedModes();

        if((urg_type_ == "URG-04LX") ||
                (urg_type_ == "UBG-04LX-F01") ||
                (urg_type_ == "URG-04LX-UG01")){
            supportedModes.removeAll("HD");
            supportedModes.removeAll("HE");
            supportedModes.removeAll("ND");
            supportedModes.removeAll("NE");
        }
    }

    bool loadParameter(void) {
        RangeSensorParameter parameters;
        if (! scip_.loadParameter(parameters)) {
            error_message_ = scip_.what();
            return false;
        }
        qSwap(parameters_, parameters);

        size_t type_length = qMin(parameters_.model.find('('),
                                  parameters_.model.find('['));
        urg_type_ = parameters_.model.substr(0, type_length);

        return true;
    }

    bool loadInformation(void) {
        bool result = true;
        RangeSensorInformation informations;
        if (! scip_.loadInformation(informations)) {
            error_message_ = scip_.what();
            result = false;
        }
        qSwap(informations_, informations);
        return result;
    }

    bool loadInternalInformation(void) {
        bool result = true;
        RangeSensorInternalInformation informations;
        if (! scip_.loadInternalInformation(informations)) {
            error_message_ = scip_.what();
            result = false;
        }
        qSwap(internalInformations_, informations);
        return result;
    }

    void disconnect(void) {
        if (con_) {
            stop();
            con_->disconnect();
        }
    }

    void updateCaptureParameters(void) {
        capture_begin_ = parameters_.area_min;
        capture_end_ = parameters_.area_max;
    }

    int capture(QVector<QVector<long> > &ranges, QVector<QVector<long> > &levels, long &timestamp) {
        long raw_timestamp = 0;
        int n = capture_->capture(ranges, levels, raw_timestamp);
        if (n < 0) {
            error_message_ = scip_.what();
            return n;
        }

        recent_timestamp_ = raw_timestamp - timestamp_offset_;
        timestamp = recent_timestamp_;

        return n;
    }


    bool setTimestamp(int timestamp, int* response_msec, int* force_delay_msec) {
        long prepare_ticks = ticks();

        if (! scip_.setRawTimestampMode(true)) {
            error_message_ = scip_.what();
            return false;
        }

        long urg_timestamp = 0;
        int first_ticks = ticks();
        if (scip_.rawTimestamp(urg_timestamp)) {

            int delay = ticks() - first_ticks;
            if (force_delay_msec) {
                delay = *force_delay_msec;
            }
            if (response_msec) {
                *response_msec = delay;
            }
            timestamp_offset_ =
                    (urg_timestamp + (delay / 2))
                    - (timestamp + (first_ticks - prepare_ticks));
        }

        if (! scip_.setRawTimestampMode(false)) {
            error_message_ = scip_.what();
            return false;
        }

        return true;
    }


    int rad2index(const double radian) const {
        int index =
                qFloor(((radian * parameters_.area_total) / (2.0 * M_PI)) + 0.5)
                + parameters_.area_front;

        return qBound(parameters_.area_min, index, parameters_.area_max);
    }


    bool isConnected(void) const {
        return (con_ == NULL) ? false : con_->isConnected();
    }


    void stop(void) {
        if (! isConnected()) {
            return;
        }

        scip_.setLaserOutput(ScipHandler::Off, ScipHandler::Force);
        //        skip(con_, 500);
    }
};


UrgDevice::UrgDevice(void)
    : m_debugMode(false)
  , pimpl(new pImpl(this))
{
    qRegisterMetaType<SensorDataArray>("SensorDataArray");
    qRegisterMetaType<Converter>("Converter");
}


UrgDevice::~UrgDevice(void)
{
}


void UrgDevice::captureReceived(void)
{
}


const char* UrgDevice::what(void) const
{
    return pimpl->error_message_.c_str();
}


bool UrgDevice::connect(const char* device, long baudrate)
{
    bool result = pimpl->connect(device, baudrate);
    return result;
}


void UrgDevice::setConnection(Connection* con)
{
    pimpl->serial_ = NULL;
    pimpl->con_ = con;
    pimpl->scip_.setConnection(con);
}


Connection* UrgDevice::connection(void)
{
    return pimpl->con_;
}


void UrgDevice::disconnect(void)
{
    pimpl->disconnect();
    pimpl->supportedModes.clear();
    setConnectionDebug(false);
}


bool UrgDevice::isConnected(void) const
{
    return pimpl->isConnected();
}


long UrgDevice::minDistance(void) const
{
    return pimpl->parameters_.distance_min;
}


long UrgDevice::maxDistance(void) const
{
    return pimpl->parameters_.distance_max;
}

int UrgDevice::minArea(void) const
{
    return pimpl->parameters_.area_min;
}


int UrgDevice::maxArea(void) const
{
    return pimpl->parameters_.area_max;
}


int UrgDevice::maxScanLines(void) const
{
    return pimpl->parameters_.area_max + 1;
}

int UrgDevice::startStep(void) const
{
    return pimpl->capture_begin_;
}

int UrgDevice::endStep(void) const
{
    return pimpl->capture_end_;
}

void UrgDevice::setCapturesSize(size_t size)
{
    pimpl->capture_->setCapturesSize(size);
}


size_t UrgDevice::remainCaptureTimes(void)
{
    return pimpl->capture_->remainCaptureTimes();
}

bool UrgDevice::loadParameters()
{
    return pimpl->loadParameters();
}

bool UrgDevice::isWorking()
{
    return pimpl->scip_.isContiniousMode();
}

long UrgDevice::decode(string word, int size)
{
    if (!size) {
        size = word.length();
    }
    return pimpl->scip_.decode(word.c_str(), size);
}

bool UrgDevice::setConnectionDebug(bool mode
                                   , const QString &locationPart
                                   , const QString &sendFilePart
                                   , const QString &receiveFilePart)
{
    if (pimpl->con_) {
        m_debugMode = mode;
        if(mode){
            pimpl->con_->startRecording(locationPart, sendFilePart, receiveFilePart);
        }else{
            pimpl->con_->stopRecording();
        }
        return true;
    }
    else {
        return false;
    }
}

int UrgDevice::scanMsec(void) const
{
    int scan_rpm = pimpl->parameters_.scan_rpm;
    return (scan_rpm <= 0) ? 1 : (1000 * 60 / scan_rpm);
}


void UrgDevice::setCaptureMode(RangeCaptureMode mode)
{
    stop();

    if (mode == GD_Capture_mode) {
        pimpl->capture_ = &pimpl->gd_capture_;

    }
    else if (mode == GE_Capture_mode) {
        pimpl->capture_ = &pimpl->ge_capture_;

    }
    else if (mode == HD_Capture_mode) {
        pimpl->capture_ = &pimpl->hd_capture_;

    }
    else if (mode == HE_Capture_mode) {
        pimpl->capture_ = &pimpl->he_capture_;

    }
    else if (mode == MD_Capture_mode) {
        pimpl->capture_ = &pimpl->md_capture_;

    }
    else if (mode == ME_Capture_mode) {
        pimpl->capture_ = &pimpl->me_capture_;

    }
    else if (mode == ND_Capture_mode) {
        pimpl->capture_ = &pimpl->nd_capture_;

    }
    else if (mode == NE_Capture_mode) {
        pimpl->capture_ = &pimpl->ne_capture_;

    }

    pimpl->capture_mode_ = mode;
}


RangeCaptureMode UrgDevice::captureMode(void)
{
    return pimpl->capture_mode_;
}


void UrgDevice::setCaptureRange(int begin_index, int end_index)
{
    stop();

    pimpl->capture_begin_ = begin_index;
    pimpl->capture_end_ = end_index;
}


void UrgDevice::setCaptureFrameInterval(size_t interval)
{
    stop();

    pimpl->capture_frame_interval_ = interval;
}

void UrgDevice::setCaptureTimes(size_t times)
{
    stop();

    pimpl->capture_times_ = times;
}

void UrgDevice::setCaptureGroupSteps(size_t group_steps)
{
    stop();

    pimpl->capture_group_steps_ = group_steps > 0 ? group_steps : 1;
}

size_t UrgDevice::captureGroupSteps() const
{
    return pimpl->capture_group_steps_;
}

int UrgDevice::capture(SensorDataArray &ranges, SensorDataArray &levels, long &timestamp)
{
    int result = pimpl->capture(ranges.steps, levels.steps, timestamp);
    if(result < 0){
        return result;
    }
    ranges.converter = getConverter();
    ranges.timestamp = timestamp;
    levels.converter = getConverter();
    levels.timestamp = timestamp;
    return ranges.steps.size();
}


void UrgDevice::stop(void)
{
    pimpl->stop();
}


bool UrgDevice::setTimestamp(int ticks, int* response_msec,
                             int* force_delay_msec)
{
    if (! isConnected()) {
        pimpl->error_message_ = "Sensor not connected.";
        return false;
    }

    return pimpl->setTimestamp(ticks, response_msec, force_delay_msec);
}


long UrgDevice::recentTimestamp(void) const
{
    return pimpl->recent_timestamp_;
}


bool UrgDevice::setLaserOutput(bool on)
{
    if (! isConnected()) {
        pimpl->error_message_ = "Sensor not connected.";
        return false;
    }
    return pimpl->scip_.setLaserOutput(on, ScipHandler::Force);
}


double UrgDevice::index2rad(const int index) const
{
    int index_from_front = index - pimpl->parameters_.area_front;
    return index_from_front * (2.0 * M_PI) / pimpl->parameters_.area_total;
}


int UrgDevice::rad2index(const double radian) const
{
    return pimpl->rad2index(radian);
}


void UrgDevice::setParameter(const RangeSensorParameter &parameter)
{
    pimpl->parameters_ = parameter;
    pimpl->updateCaptureParameters();
}

const char* UrgDevice::getType(void)
{
    return pimpl->urg_type_.c_str();
}

const char* UrgDevice::getSerialNumber(void)
{
    return pimpl->informations_.serial_number.c_str();
}

const char* UrgDevice::getFirmwareVersion(void)
{
    return pimpl->informations_.firmware.c_str();
}

RangeSensorParameter UrgDevice::parameter(void) const
{
    return pimpl->parameters_;
}

RangeSensorInformation UrgDevice::information(void) const
{
    return pimpl->informations_;
}

RangeSensorInternalInformation UrgDevice::internalInformation(void) const
{
    return pimpl->internalInformations_;
}

RangeSensorParameter UrgDevice::parameterNow(void) const
{
    pimpl->loadParameter();
    return pimpl->parameters_;
}

RangeSensorInformation UrgDevice::informationNow(void) const
{
    pimpl->loadInformation();
    return pimpl->informations_;
}

RangeSensorInternalInformation UrgDevice::internalInformationNow(void) const
{
    pimpl->loadInternalInformation();
    return pimpl->internalInformations_;
}

bool UrgDevice::loadParameter(void)
{
    bool res = pimpl->loadParameter();
    if (!res) {
        pimpl->error_message_ = pimpl->scip_.what();
    }
    return res;
}

bool UrgDevice::loadInformation(void)
{
    bool res = pimpl->loadInformation();
    if (!res) {
        pimpl->error_message_ = pimpl->scip_.what();
    }
    return res;
}


bool UrgDevice::versionLines(QVector<string> &lines)
{
    if (! isConnected()) {
        pimpl->error_message_ = "Sensor not connected.";
        return false;
    }

    bool res = pimpl->scip_.versionLines(lines);
    if (!res) {
        pimpl->error_message_ = pimpl->scip_.what();
    }
    return res;
}

bool UrgDevice::commandLines(string cmd, QVector<string> &lines, bool all)
{
    if (! isConnected()) {
        pimpl->error_message_ = "Sensor not connected.";
        return false;
    }
    bool res = pimpl->scip_.commandLines(cmd, &lines, all);
    if (!res) {
        pimpl->error_message_ = pimpl->scip_.what();
    }
    return res;
}

bool UrgDevice::commandLines(string cmd)
{
    if (! isConnected()) {
        pimpl->error_message_ = "Sensor not connected.";
        return false;
    }
    bool res = pimpl->scip_.commandLines(cmd);
    if (!res) {
        pimpl->error_message_ = pimpl->scip_.what();
    }
    return res;
}

bool UrgDevice::commandLines(string cmd, int &status, QVector<string> &lines, bool all)
{
    if (! isConnected()) {
        pimpl->error_message_ = "Sensor not connected.";
        return false;
    }
    bool res = pimpl->scip_.commandLines(cmd, status, &lines, all);
    if (!res) {
        pimpl->error_message_ = pimpl->scip_.what();
    }
    return res;
}

bool UrgDevice::commandLines(QVector<string> &lines, bool all)
{
    if (! isConnected()) {
        pimpl->error_message_ = "Sensor not connected.";
        return false;
    }
    bool res = pimpl->scip_.commandLines(&lines, all);
    if (!res) {
        pimpl->error_message_ = pimpl->scip_.what();
    }
    return res;
}

bool UrgDevice::commandLines(int &status, QVector<string> &lines, bool all)
{
    if (! isConnected()) {
        pimpl->error_message_ = "Sensor not connected.";
        return false;
    }
    bool res = pimpl->scip_.commandLines(status, &lines, all);
    if (!res) {
        pimpl->error_message_ = pimpl->scip_.what();
    }
    return res;
}

QVector<string> UrgDevice::supportedCommands(void) const
{
    return pimpl->scip_.supportedCommands();
}

bool UrgDevice::isSupportedMode(RangeCaptureMode mode)
{
    QStringList supportedModes = pimpl->supportedModes;
    if(supportedModes.isEmpty()) return true;
    switch (mode) {
    case GD_Capture_mode:
        return supportedModes.contains("GD");
        break;
    case MD_Capture_mode:
        return supportedModes.contains("MD");
        break;
    case GE_Capture_mode:
        return supportedModes.contains("GE");
        break;
    case ME_Capture_mode:
        return supportedModes.contains("ME");
        break;
    case HD_Capture_mode:
        return supportedModes.contains("HD");
        break;
    case HE_Capture_mode:
        return supportedModes.contains("HE");
        break;
    case ND_Capture_mode:
        return supportedModes.contains("ND");
        break;
    case NE_Capture_mode:
        return supportedModes.contains("NE");
        break;
    default:
        return false;
        break;
    }
    return false;
}

QStringList UrgDevice::supportedModes(bool force) const
{
    if(pimpl->supportedModes.isEmpty() || force){
        pimpl->updateSupportedModes();
    }
    return pimpl->supportedModes;
}


bool UrgDevice::reboot(void)
{
    UrgDevice::setLaserOutput(ScipHandler::Off);

    // send "RB" twice.
    for (int i = 0; i < 2; ++i) {
        pimpl->scip_.send("RB\n", 3);

        enum {
            RB_Timeout = 1000,        // [msec]
        };
        char recv_buffer[10];
        int answerLength = 2 + 1 + 3 + 1 + 1;
        int send_n = pimpl->scip_.recv(recv_buffer, answerLength, RB_Timeout);
        if (send_n != answerLength) {
            pimpl->error_message_ = "Wrong answer size for RB command";
            return false;
        }
        if (strncmp(recv_buffer, "RB\n", 3)) {
            pimpl->error_message_ = "Wrong answer echoback for RB command";
            return false;
        }
    }

    UrgDevice::disconnect();

    return true;
}

bool UrgDevice::changeBaurate(long baud)
{
    QString ss_command = "SS";
    ss_command += QString("%1").arg(baud, 6, 10, QLatin1Char('0'));
    ss_command += "\n";

    string cmd = ss_command.toStdString();
    return pimpl->scip_.commandLines(cmd);
}

Converter UrgDevice::getConverter()
{
    return Converter(pimpl->parameters_.area_front, pimpl->parameters_.area_total,
                     pimpl->capture_begin_, pimpl->capture_end_,
                     pimpl->capture_group_steps_);
}

