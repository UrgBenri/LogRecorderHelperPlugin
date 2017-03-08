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

#include "ScipHandler.h"
#include "RangeSensorParameter.h"
#include "RangeSensorInformation.h"
#include "RangeSensorInternalInformation.h"
#include "Connection.h"
#include "ConnectionUtils.h"
#include "ticks.h"
#include "delay.h"
#include "log_printf.h"
#include <cstring>
#include <cstdio>
#include <iostream>

#include <QVector>
#include <QDebug>
#include <QTime>

#ifdef Q_CC_MSVC
#define snprintf _snprintf
#endif

using namespace qrk;
using namespace std;


namespace
{
typedef enum {
    ProcessContinue,
    ProcessBreak,
    ProcessNormal,
} LoopProcess;

void clearReceived(QVector<long> &ranges, CaptureType &type,
                   int &line_count, int &timeout,
                   string &remain_string,
                   string &left_packet_data)
{
    ranges.clear();
    type = TypeUnknown;
    line_count = 0;
    timeout = 0;
    remain_string.clear();
    left_packet_data.clear();
}
}


struct ScipHandler::pImpl {
    enum {
        TotalTimeout = 1000,        // [msec]
        ContinuousTimeout = 1000,    // [msec] <<- changed
        FirstTimeout = 1000,        // [msec]

        BufferSize = 4096 + 1, //64 + 1 + 1,    // データ長 + チェックサム + 改行 4096 + 1, //

        ResponseTimeout = -1,
        MismatchResponse = -2,
        SendFail = -3,
        ChecksumFail = -4,
        Scip11Response = -14,

        InvalidRange = -1,
    };

    typedef enum {
        LaserUnknown = 0,
        LaserOn,
        LaserOff,
    } LaserState;

    string error_message_;
    Connection* con_;
    LaserState laser_state_;
    bool mx_capturing_;
    bool nx_capturing_;

    bool isPreCommand_QT_;


    pImpl(void)
        : error_message_("no error."), con_(NULL), laser_state_(LaserUnknown),
          mx_capturing_(false), nx_capturing_(false), isPreCommand_QT_(false) {
    }


    bool connect(const char* device, long baudrate) {
        if (! con_->connect(device, baudrate)) {
            error_message_ = con_->what();
            return false;
        }

        long try_baudrates[] = { 115200, 19200, 38400, 57600, 250000,
                                 500000, 750000, 781250, 230400, 460800, 614400,
                                 921600, 1000000, 1500000};
        size_t try_size = sizeof(try_baudrates) / sizeof(try_baudrates[0]);


        for (size_t i = 1; i < try_size; ++i) {
            if (baudrate == try_baudrates[i]) {
                qSwap(try_baudrates[0], try_baudrates[i]);
                break;
            }
        }

        for (size_t i = 0; i < try_size; ++i) {

            if (con_->connectionType() == Connection::SERIAL_TYPE &&
                    ! con_->setBaudrate(try_baudrates[i])) {
                error_message_ = con_->what();
                return false;
            }

            con_->clear();

            int return_code = -1;
            char qt_expected_response[] = { 0, 19, 0x10, -1 };
            if (response(return_code, qt_expected_response, "QT\n")) {
                laser_state_ = LaserOff;
                return true;
            }
            else if (return_code == ResponseTimeout) {
                error_message_ = "Connection time out.";
                continue;
            }
            else if (return_code == MismatchResponse) {
//                con_->clear();
//                skip(con_, ContinuousTimeout);
                continue;
            }
            else if (return_code == Scip11Response) {
                char scip20_expected_response[] = { 0, -1 };
                if (! response(return_code, scip20_expected_response, "SCIP2.0\n")) {
                    error_message_ =
                            "SCIP1.1 protocol is not supported. Please update URG firmware, or reconnect after a few seconds because sensor is booting.";
                    return false;
                }
                laser_state_ = LaserOff;
                return true;
            }
            else if (return_code == 0xE) {
                char tm2_expected_response[] = { 0, -1 };
                if (response(return_code, tm2_expected_response, "TM2\n")) {
                    laser_state_ = LaserOff;
                    return true;
                }
            }
        }
        con_->disconnect();

        error_message_ = "Could not connect to sensor with supported speeds.";
        return false;
    }


    bool changeBothBaudrate(long baudrate) {
        if (con_->connectionType() == Connection::SERIAL_TYPE) {
            // 既に目標対象のボーレート値ならば、成功とみなす
            // この関数は、ScipHandler::connect() か、それ以後でないと呼ばれないため
            if (con_->baudrate() == baudrate) {
                return true;
            }

            // URG 側のボーレートを変更
            int pre_ticks = ticks();
            if (! changeBaudrate(baudrate)) {
                return false;
            }

            // シリアル通信の場合、ボーレート変更後、１周分だけ待つ必要がある
            int reply_msec = ticks() - pre_ticks;
            delay((reply_msec * 4 / 3) + 10);

            // ホスト側のボーレートを変更
            return con_->setBaudrate(baudrate);
        }

        return true;
    }


    bool changeBaudrate(long baudrate) {
#if 0
        // Tcpip 接続に対応するため、コメントアウト
        if (!((baudrate == 19200) || (baudrate == 38400) ||
              (baudrate == 57600) || (baudrate == 115200))) {
            error_message_ = "Invalid baudrate value.";
            return false;
        }
#endif

        // SS を送信し、URG 側のボーレートを変更する
        char send_buffer[] = "SSxxxxxx\n";
        snprintf(send_buffer, 10, "SS%06ld\n", baudrate);
        int return_code = -1;
        // !!! 既に設定対象のボーレート、の場合の戻り値を ss_expected... に追加する
        char ss_expected_response[] = { 0, 0x3, 0x4, 0xf, -1 };
        if (! response(return_code, ss_expected_response, send_buffer)) {
            error_message_ = "Baudrate change fail.";
            return false;
        }

        return true;
    }


    bool loadParameter(RangeSensorParameter &parameters) {
        // PP の送信とデータの受信
        int return_code = -1;
        char pp_expected_response[] = { 0, -1 };
        QVector<string> lines;
        if (! response(return_code, pp_expected_response, "PP\n", &lines)) {
            error_message_ = "Getting sensor information failed.";
            return false;
        }

        // PP 応答内容の格納
        if (lines.size() != 8) {
            error_message_ = "Invalid PP response.";
            return false;
        }

        // !!! チェックサムの評価を行うべき

        int modl_length =
                static_cast<int>(lines[RangeSensorParameter::MODL].size());
        // 最初のタグと、チェックサムを除いた文字列を返す
        if (modl_length > (5 + 2)) {
            modl_length -= (5 + 2);
        }
        parameters.model = lines[RangeSensorParameter::MODL].substr(5, modl_length);

        parameters.distance_min = substr2int(lines[RangeSensorParameter::DMIN], 5);
        parameters.distance_max = substr2int(lines[RangeSensorParameter::DMAX], 5);
        parameters.area_total = substr2int(lines[RangeSensorParameter::ARES], 5);
        parameters.area_min = substr2int(lines[RangeSensorParameter::AMIN], 5);
        parameters.area_max = substr2int(lines[RangeSensorParameter::AMAX], 5);
        parameters.area_front = substr2int(lines[RangeSensorParameter::AFRT], 5);
        parameters.scan_rpm = substr2int(lines[RangeSensorParameter::SCAN], 5);

        return true;
    }


    bool loadInformation(RangeSensorInformation &informations) {
        // PP の送信とデータの受信
        int return_code = -1;
        char vv_expected_response[] = { 0, -1 };
        QVector<string> lines;
        if (! response(return_code, vv_expected_response, "VV\n", &lines)) {
            error_message_ = "VV fail.";
            return false;
        }

        // PP 応答内容の格納
        if (lines.size() != 5) {
            error_message_ = "Invalid VV response.";
            return false;
        }

        // !!! チェックサムの評価を行うべき

        informations.vendor = lines[RangeSensorInformation::VEND].substr(5,
                                                                         lines[RangeSensorInformation::VEND].size() - 7);
        informations.product = lines[RangeSensorInformation::PROD].substr(5,
                                                                          lines[RangeSensorInformation::PROD].size() - 7);
        informations.firmware = lines[RangeSensorInformation::FIRM].substr(5,
                                                                           lines[RangeSensorInformation::FIRM].size() - 7);
        informations.protocol = lines[RangeSensorInformation::PROT].substr(5,
                                                                           lines[RangeSensorInformation::PROT].size() - 7);
        informations.serial_number = lines[RangeSensorInformation::SERI].substr(5,
                                                                                lines[RangeSensorInformation::SERI].size() - 7);
        return true;
    }

    bool loadInternalInformation(RangeSensorInternalInformation &informations) {
        int return_code = -1;
        char ii_expected_response[] = { 0, -1 };
        QVector<string> lines;
        if (! response(return_code, ii_expected_response, "II\n", &lines)) {
            error_message_ = "II fail.";
            return false;
        }

        if (lines.size() == 7) {

            informations.model = lines[RangeSensorInternalInformation::MODL].substr(5,
                                                                                    lines[RangeSensorInternalInformation::MODL].size() - 7);
            informations.laserStatus = lines[RangeSensorInternalInformation::LASR].substr(5,
                                                                                          lines[RangeSensorInternalInformation::LASR].size() - 7);
            informations.motorDesiredSpeed = lines[RangeSensorInternalInformation::SCSP].substr(5,
                                                                                                lines[RangeSensorInternalInformation::SCSP].size() - 7);
            informations.stateID = lines[RangeSensorInternalInformation::MESM].substr(5,
                                                                                      lines[RangeSensorInternalInformation::MESM].size() - 7);
            informations.communicationType = lines[RangeSensorInternalInformation::SBPS].substr(5,
                                                                                                lines[RangeSensorInternalInformation::SBPS].size() - 7);
            informations.internalTime = lines[RangeSensorInternalInformation::TIME].substr(5,
                                                                                           lines[RangeSensorInternalInformation::TIME].size() - 7);
            informations.sensorSituation = lines[RangeSensorInternalInformation::STAT].substr(5,
                                                                                              lines[RangeSensorInternalInformation::STAT].size() - 7);
        }
        else if (lines.size() == 2) {
            informations.model = lines[0].substr(5, lines[0].size() - 7);
            informations.sensorSituation = lines[1].substr(5, lines[1].size() - 7);
            error_message_ = "Invalid II Command response.";
            return false;
        }
        else {
            error_message_ = "Invalid II Command response.";
            return false;
        }
        return true;
    }

    QVector<string> supportedCommands() {
        string firstChar = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
        string secondChar = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";

        QVector<string> result;

        int return_code = -1;
        char expected_response[] = {0x0E, 0x0F, -1 }; // -1 is a list terminator

        for (unsigned int i = 0; i < firstChar.length(); ++i) {
            for (unsigned int j = 0; j < secondChar.length(); ++j) {
                string command = string(1, firstChar[i]) + string(1, secondChar[j]);

                string commandToSend = command + "\n";
                QVector<string> lines;
                if (response(return_code, expected_response, commandToSend.c_str(), &lines)) {
                    result.push_back(command);
                    //                    printf("Command [%s]\n", command.c_str());
                }

                //                _sleep(1000);

                commandToSend = "%" + command + "\n";
                if (response(return_code, expected_response, commandToSend.c_str(), &lines)) {
                    result.push_back("%" + command);
                    //                    printf("Command [%%%s]\n", command.c_str());
                }

                //                _sleep(1000);

                commandToSend = "$" + command + "\n";
                if (response(return_code, expected_response, commandToSend.c_str(), &lines)) {
                    result.push_back("$" + command);
                    //                    printf("Command [%%%s]\n", command.c_str());
                }

                //                _sleep(1000);

                commandToSend = "#" + command + "\n";
                if (response(return_code, expected_response, commandToSend.c_str(), &lines)) {
                    result.push_back("#" + command);
                    //                    printf("Command [%%%s]\n", command.c_str());
                }
                //                //_sleep(1000);
            }
        }

        return result;
    }

    QStringList supportedModes() {
        QStringList result;
        QVector<string> commandsList;
        commandsList.push_back("GD\n");
        commandsList.push_back("MD\n");
        commandsList.push_back("GE\n");
        commandsList.push_back("ME\n");
        commandsList.push_back("HD\n");
        commandsList.push_back("ND\n");
        commandsList.push_back("HE\n");
        commandsList.push_back("NE\n");

        int return_code = -1;
        char expected_response[] = {0x0E, 0X0F, -1 }; // -1 is a list terminator
        for (int i = 0; i < commandsList.size(); ++i) {
            if (!response(return_code, expected_response, commandsList[i].c_str())) {
                result << QString::fromStdString(commandsList[i].substr(0, 2));
            }
        }
        return result;
    }


    int substr2int(const string &line, int from_n, int length = string::npos) {
        return atoi(line.substr(from_n, length).c_str());
    }


    bool response(int &return_code,
                          char expected_response[],
                          const char *send_command = NULL,
                          QVector<string>* lines = NULL,
                          bool return_all = false) {
        return_code = -1;
        if (! con_) {
            error_message_ = "No connection defined.";
            return false;
        }

        if (! con_->isConnected()) {
            error_message_ = "Sensor not connected.";
            return false;
        }

        size_t send_size;// = strlen(send_command);
        if(send_command){
            send_size = strlen(send_command);
            int actual_send_size = con_->send(send_command, send_size);
            if (!strncmp(send_command, "QT\n", send_size)) {
                isPreCommand_QT_ = false;
                mx_capturing_ = false;
                nx_capturing_ = false;
            }
            if (actual_send_size != static_cast<int>(send_size)) {
                return_code = SendFail;
                error_message_ = "Sending command failed.";
                return false;
            }
        }

        char buffer[BufferSize +1];
        int recv_size = readline(con_, buffer, BufferSize, FirstTimeout);
        if (recv_size < 0) {
            error_message_ = "Sesponse timeout.";
            return_code = ResponseTimeout;
            return false;
        }

        buffer[recv_size] = '\0';
        if(return_all) lines->push_back(buffer);

        if(send_command){
            send_size = strlen(send_command);
            if (!((recv_size == 1) && (buffer[0] == 0x00))) {
                if ((recv_size != static_cast<int>(send_size - 1)) ||
                        (strncmp(buffer, send_command, recv_size))) {
                    error_message_ = "mismatch response: " + string(buffer);
                    return_code = MismatchResponse;
                    std::cerr << "Error: " <<  error_message_.c_str() << " command: " << send_command << endl;
                    con_->clear();
                    skip(con_, ContinuousTimeout);
                    skip(con_, ContinuousTimeout);
                    return false;
                }
            }
        }

        recv_size = readline(con_, buffer, BufferSize, ContinuousTimeout);
        if (recv_size < 0) {
            error_message_ = "Response timeout.";
            return_code = ResponseTimeout;
            return false;
        }

        buffer[recv_size] = '\0';
        if(return_all) lines->push_back(buffer);

        if (recv_size == 3) {
            if (! checkSum(buffer, recv_size - 1, buffer[recv_size - 1])) {
                return_code = ChecksumFail;
                error_message_ = "Checksum failed.";
                return false;
            }
            buffer[2] = '\0';
            return_code = strtol(buffer, NULL, 16);
            if (!strcmp(buffer, "0G")) {
                return_code = 16;
            }
            if (!strcmp(buffer, "0H")) {
                return_code = 17;
            }
            if (!strcmp(buffer, "0I")) {
                return_code = 18;
            }
            if (!strcmp(buffer, "0L")) {
                return_code = 19;
            }

        }
        else if (recv_size == 1) {
            return_code = Scip11Response;
            error_message_ = "SCIP 1 response";
        }

        do {
            recv_size = readline(con_, buffer, BufferSize, ContinuousTimeout);
            if (lines && (recv_size > 0)) {
                lines->push_back(buffer);
            }
        }
        while (recv_size > 0);

        if (lines) {
            lines->squeeze();
        }

        for (int i = 0; expected_response[i] != -1; ++i) {
            if (return_code == (int)expected_response[i]) {
                return true;
            }
        }
        return false;
    }

    bool setLaserOutput(bool on, bool force) {
        if (((on == true) && (laser_state_ == LaserOn)) ||
                ((on == false) && (laser_state_ == LaserOff))) {
            if (! force) {
                return true;
            }
        }
        if ((!on) && isPreCommand_QT_) {
            return false;
        }

        if (on) {
            int return_code = -1;
            char expected_response[] = { 0, 2, -1 };
            if (!response(return_code, expected_response, "BM\n")) {
                if(return_code == 1) laser_state_ = LaserOff;
                error_message_ = "BM fail.";
                return false;
            }
            laser_state_ = LaserOn;
            return true;

        }
        else {
            // "QT"

            int return_code = -1;
            char qt_expected_response[] = { 0, -1 };
            if (! response(return_code, qt_expected_response, "QT\n") &&
                    ! response(return_code, qt_expected_response, "QT\n")) {
                return false;
            }
            laser_state_ = LaserOff;
            isPreCommand_QT_ = true;
            return true;
        }
    }


    bool testChecksum(const char* buffer, int line_size) {
        if (! checkSum(buffer, line_size - 1, buffer[line_size - 1])) {
            //log_printf("checksum error: %s\n", buffer);
            // return InvalidData;
            // !!! URG のパケットエラーがなくなったら、この実装に戻す

            // !!! 存在するだけのパケットを読み飛ばす
            error_message_ = "invalid packet.";
            return false;
        }

        return true;
    }


    LoopProcess handleEchoback(const char* buffer, CaptureSettings &settings,
                               CaptureType &type) {
        string line = buffer;
        if ((! line.compare(0, 2, "GD")) ||
                (! line.compare(0, 2, "GS"))) {
            if (! parseGdEchoback(settings, line)) {
                return ProcessBreak;
            }
            type = (line[1] == 'D') ? GD : GS;

        }
        else if ((! line.compare(0, 2, "HD")) ||
                 (! line.compare(0, 2, "HS"))) {
            if (! parseHdEchoback(settings, line)) {
                return ProcessBreak;
            }
            type = (line[1] == 'D') ? HD : HS;

        }
        else if ((! line.compare(0, 2, "MD")) ||
                 (! line.compare(0, 2, "MS"))) {
            if (! parseMdEchoback(settings, line)) {
                return ProcessBreak;
            }
            type = (line[1] == 'D') ? MD : MS;
            laser_state_ = LaserOn;

        }
        else if ((! line.compare(0, 2, "ND")) ||
                 (! line.compare(0, 2, "NS"))) {
            if (! parseNdEchoback(settings, line)) {
                return ProcessBreak;
            }
            type = (line[1] == 'D') ? ND : NS;
            laser_state_ = LaserOn;

        }
        else if (! line.compare(0, 2, "GE")) {
            if (! parseGeEchoback(settings, line)) {
                return ProcessBreak;
            }
            type = GE;

        }
        else if (! line.compare(0, 2, "HE")) {
            if (! parseHeEchoback(settings, line)) {
                return ProcessBreak;
            }
            type = HE;

        }
        else if (! line.compare(0, 2, "ME")) {
            if (! parseMeEchoback(settings, line)) {
                return ProcessBreak;
            }
            type = ME;
            laser_state_ = LaserOn;

        }
        else if (! line.compare(0, 2, "NE")) {
            if (! parseNeEchoback(settings, line)) {
                return ProcessBreak;
            }
            type = NE;
            laser_state_ = LaserOn;

        }
        else if (! line.compare(0, 2, "QT")) {
            settings.remain_times = 0;
            laser_state_ = LaserOff;
            mx_capturing_ = false;
            nx_capturing_ = false;
            return ProcessNormal;

        }
        else {
            //return InvalidData;
            // !!! URG が正常なパケットを返すようになったら、この実装に戻す
            return ProcessContinue;
        }


        return ProcessNormal;
    }


    void handleReturnCode(char* buffer, CaptureSettings &settings, int timeout,
                          CaptureType &type, int* total_times) {
        // !!! 長さが 2 + 1 かのチェックをすべき
        buffer[2] = '\0';
        settings.error_code = atoi(buffer);

        if (settings.error_code == 10) {
            // レーザ消灯を検出
            laser_state_ = pImpl::LaserOff;
            mx_capturing_ = false;
            nx_capturing_ = false;
        }

        // "0B" が返された場合、センサとホストの応答がずれている可能性があるので
        // 続く応答を読み捨てる
        if (! strncmp(buffer, "0B", 2)) {
            skip(con_, TotalTimeout, timeout);
        }

        // !!! "00P" との比較をすべき
        if ((settings.error_code == 0) &&
                ((type == MD) || (type == MS) || (type == ME))) {
            if (total_times) {
                *total_times = settings.remain_times;
            }
            type = Mx_Reply;
        }

        if ((settings.error_code == 0) &&
                ((type == ND) || (type == NS) || (type == NE))) {
            if (total_times) {
                *total_times = settings.remain_times;
            }
            type = Nx_Reply;
        }
    }


    CaptureType receiveCaptureData(QVector<QVector<long> > &ranges
                                   , QVector<QVector<long> > &levels
                                   , CaptureSettings &settings
                                   , long &timestamp
                                   , int *remain_times
                                   , int *total_times) {
        int line_count = 0;
        string left_packet_data;
        char buffer[BufferSize];

        ranges.clear();
        levels.clear();

        error_message_ = "no response.";

        CaptureType type = TypeUnknown;
        int timeout = FirstTimeout;
        int line_size = 0;

//        QTime timer;
//        timer.start();
        while ((line_size = readline(con_, buffer, BufferSize, timeout)) > 0) {
            //            fprintf(stderr, "%d: %3d: %s\n", ticks(), line_count, buffer);

            //log_printf("%d: %3d: %s\n",  ticks(), line_count, buffer);

            if (line_count != 0) {
                // エコーバックにはチェックサム文字列がないので、無視
                if (! testChecksum(buffer, line_size)) {
                    error_message_ = "Checksum error";
                    skip(con_, 25);
                    ranges.clear();
                    levels.clear();
                    break;
                }
            }

            if (line_count == 0) {
                // エコーバック
                LoopProcess loop_process =
                        handleEchoback(buffer, settings, type);

                if (loop_process == ProcessContinue) {
                    error_message_ = "Not range command";
                    break;

                }
                else if (loop_process == ProcessBreak) {
                    error_message_ = "Echo back error.";
                    break;
                }

            }
            else if (line_count == 1) {
                handleReturnCode(buffer, settings, timeout, type, total_times);
            }
            else if (line_count == 2) {
                timestamp = decode(buffer, 4);
            }
            else {
                left_packet_data =
                        addLengthData(ranges, levels, string(buffer), left_packet_data, type,
                                      settings.data_byte);
            }
            ++line_count;
            timeout = ContinuousTimeout;
        } // Loop end

//        std::cerr << "receiveCaptureData time: " << timer.elapsed() << "ms" << endl;

        if(line_count == 0){
            settings.error_code = -1;
        }

        //log_printf("receiveCaptureData_size[%d]\n", data.size());

        // !!! type が距離データ取得のときは、正常に受信が完了したか、を確認すべき

        // ME で "まとめる数" 設定以上のデータが返されるバグに対処

        /*
        size_t expected_n = settings.capture_last * ((type == ME) ? 2 : 1);
        if (expected_n < data.size()) {
            data.erase(data.begin() + expected_n, data.end());
            //log_printf("expected and size is different %d %d \n", expected_n, data.size());
        }
        */
        if (remain_times) {
            *remain_times = settings.remain_times;
        }

        return type;
    }


    bool parseGdEchoback(CaptureSettings &settings, const string &line) {
        if (line.size() != 12) {
            error_message_ = "Invalid GD packet has arrived.";
            return false;
        }

        settings.capture_first = substr2int(line, 2, 4);
        settings.capture_last = substr2int(line, 6, 4) + 1;
        int skip_lines = substr2int(line, 10, 2);
        settings.group_steps = (skip_lines == 0) ? 1 : skip_lines;
        settings.data_byte = (line[1] == 'D') ? 3 : 2;

        return true;
    }

    bool parseGeEchoback(CaptureSettings &settings, const string &line) {
        if (line.size() != 12) {
            error_message_ = "Invalid GE packet has arrived.";
            return false;
        }

        settings.capture_first = substr2int(line, 2, 4);
        settings.capture_last = substr2int(line, 6, 4) + 1;
        int skip_lines = substr2int(line, 10, 2);
        settings.group_steps = (skip_lines == 0) ? 1 : skip_lines;
        settings.data_byte = 3;

        return true;
    }


    bool parseHdEchoback(CaptureSettings &settings, const string &line) {
        if (line.size() != 12) {
            error_message_ = "Invalid HD packet has arrived.";
            return false;
        }

        settings.capture_first = substr2int(line, 2, 4);
        settings.capture_last = substr2int(line, 6, 4) + 1;
        int skip_lines = substr2int(line, 10, 2);
        settings.group_steps = (skip_lines == 0) ? 1 : skip_lines;
        settings.data_byte = (line[1] == 'D') ? 3 : 2;

        return true;
    }

    bool parseHeEchoback(CaptureSettings &settings, const string &line) {
        if (line.size() != 12) {
            error_message_ = "Invalid HE packet has arrived.";
            return false;
        }

        settings.capture_first = substr2int(line, 2, 4);
        settings.capture_last = substr2int(line, 6, 4) + 1;
        int skip_lines = substr2int(line, 10, 2);
        settings.group_steps = (skip_lines == 0) ? 1 : skip_lines;
        settings.data_byte = 3;

        return true;
    }



    bool parseMdEchoback(CaptureSettings &settings, const string &line) {
        if (line.size() != 15) {
            error_message_ = "Invalid MD packet has arrived.";
            return false;
        }

        settings.capture_first = substr2int(line, 2, 4);
        settings.capture_last = substr2int(line, 6, 4) + 1;
        int skip_lines = substr2int(line, 10, 2);
        settings.group_steps = (skip_lines == 0) ? 1 : skip_lines;
        settings.skip_frames = substr2int(line, 12, 1);
        settings.remain_times = substr2int(line, 13, 2);
        settings.data_byte = (line[1] == 'D') ? 3 : 2;

        if (settings.remain_times == 1) {
            // 最後のデータ取得で、レーザを消灯扱いにする
            // 本当は、次のデータ取得後にレーザは消灯されている
            // 1 で判定すると、取得回数が 1 のときにも正常に動作するため
            mx_capturing_ = false;

        }
        else {
            if (settings.remain_times > 0) {
                mx_capturing_ = true;
            }
            else if (settings.remain_times == 0) {
                settings.remain_times = 100;
            }
        }

        return true;
    }

    bool parseNdEchoback(CaptureSettings &settings, const string &line) {
        if (line.size() != 15) {
            error_message_ = "Invalid ND packet has arrived.";
            return false;
        }

        settings.capture_first = substr2int(line, 2, 4);
        settings.capture_last = substr2int(line, 6, 4) + 1;
        int skip_lines = substr2int(line, 10, 2);
        settings.group_steps = (skip_lines == 0) ? 1 : skip_lines;
        settings.skip_frames = substr2int(line, 12, 1);
        settings.remain_times = substr2int(line, 13, 2);
        settings.data_byte = (line[1] == 'D') ? 3 : 2;

        if (settings.remain_times == 1) {
            // 最後のデータ取得で、レーザを消灯扱いにする
            // 本当は、次のデータ取得後にレーザは消灯されている
            // 1 で判定すると、取得回数が 1 のときにも正常に動作するため
            nx_capturing_ = false;

        }
        else {
            if (settings.remain_times > 0) {
                nx_capturing_ = true;
            }
            else if (settings.remain_times == 0) {
                settings.remain_times = 100;
            }
        }

        return true;
    }


    bool parseMeEchoback(CaptureSettings &settings, const string &line) {
        if (line.size() != 15) {
            error_message_ = "Invalid ME packet has arrived.";
            return false;
        }

        settings.capture_first = substr2int(line, 2, 4);
        settings.capture_last = substr2int(line, 6, 4) + 1;
        int skip_lines = substr2int(line, 10, 2);
        settings.group_steps = (skip_lines == 0) ? 1 : skip_lines;
        settings.skip_frames = substr2int(line, 12, 1);
        settings.remain_times = substr2int(line, 13, 2);
        settings.data_byte = 3;

        if (settings.remain_times == 1) {
            mx_capturing_ = false;

        }
        else {
            mx_capturing_ = true;
        }

        return true;
    }


    bool parseNeEchoback(CaptureSettings &settings, const string &line) {
        if (line.size() != 15) {
            error_message_ = "Invalid NE packet has arrived.";
            return false;
        }

        settings.capture_first = substr2int(line, 2, 4);
        settings.capture_last = substr2int(line, 6, 4) + 1;
        int skip_lines = substr2int(line, 10, 2);
        settings.group_steps = (skip_lines == 0) ? 1 : skip_lines;
        settings.skip_frames = substr2int(line, 12, 1);
        settings.remain_times = substr2int(line, 13, 2);
        settings.data_byte = 3;

        if (settings.remain_times == 1) {
            nx_capturing_ = false;

        }
        else {
            nx_capturing_ = true;
        }

        return true;
    }

    void printData(QVector<long> &ranges) {
        for (int i = 0; i < ranges.size(); ++i) {
            printf("%ld | ", ranges[i]);
        }
    }

    string addLengthData(QVector<QVector<long> > &ranges
                          , QVector<QVector<long> > &levels
                          , const string &line
                          , const string &left_packet_data
                          , CaptureType type
                          , const size_t data_byte) {
        if (line.empty()) {
            return left_packet_data;
        }

        string left_byte = left_packet_data;

        //size_t data_size = (left_byte.size() + (line.size() - 1)) / data_byte;
        size_t double_data_byte = data_byte * 2;
        //size_t n = data_size * data_byte - left_byte.size();
        size_t n = line.size() - 1;
        for (size_t i = 0; i < n; ++i) {
            left_byte.push_back(line[i]);

            if (type == GE || type == HE || type == ME || type == NE) {
                if (left_byte.size() >= double_data_byte) {
                    size_t found = left_byte.find('&');
                    if (found == string::npos) {
                        ranges.push_back(QVector<long>());
                        levels.push_back(QVector<long>());
                    }
                    else {
                        if (left_byte.size() == double_data_byte) {
                            continue;
                        }

                        if (found == 0) {
                            left_byte.erase(0, 1);
                        }
                    }

                    if(ranges.isEmpty()){
                        ranges.push_back(QVector<long>());
                    }
                    long length = decode(&left_byte[0], data_byte);
                    ranges.last().push_back(length);

                    if(levels.isEmpty()){
                        levels.push_back(QVector<long>());
                    }
                    length = decode(&left_byte[data_byte], data_byte);
                    levels.last().push_back(length);

                    left_byte.erase(0, double_data_byte);
                }
            }
            else {
                if (left_byte.size() >= data_byte) {
                    size_t found = left_byte.find('&');
                    if (found == string::npos) {
                        ranges.push_back(QVector<long>());
                    }
                    else {
                        if (left_byte.size() == data_byte) {
                            continue;
                        }

                        if (found == 0) {
                            left_byte.erase(0, 1);
                        }
                    }

                    if(ranges.isEmpty()){
                        ranges.push_back(QVector<long>());
                    }
                    long length = decode(&left_byte[0], data_byte);
                    ranges.last().push_back(length);
                    left_byte.erase(0, data_byte);
                }
            }

        }
        return left_byte;
    }
};


ScipHandler::ScipHandler(void) : pimpl(new pImpl)
{
}


ScipHandler::~ScipHandler(void)
{
}


const char* ScipHandler::what(void) const
{
    return pimpl->error_message_.c_str();
}


unsigned long ScipHandler::decode(const char* data, size_t size)
{
    const char* p = data;
    const char* last_p = p + size;

    unsigned long value = 0;
    while (p < last_p) {
        value <<= 6;
        value &= ~0x3f;
        value |= *p++ - 0x30;
    }
    return value;
}


bool ScipHandler::checkSum(const char* buffer, int size, char actual_sum)
{
    if (size < 0) {
        return false;
    }

    const char* p = buffer;
    const char* last_p = p + size;

    char expected_sum = 0x00;
    while (p < last_p) {
        expected_sum += *p++;
    }

    expected_sum = (expected_sum & 0x3f) + 0x30;

    return (expected_sum == actual_sum) ? true : false;
}


void ScipHandler::setConnection(Connection* con)
{
    pimpl->con_ = con;
}


Connection* ScipHandler::connection(void)
{
    return pimpl->con_;
}


bool ScipHandler::connect(const char* device, long baudrate)
{
    return pimpl->connect(device, baudrate);
}


int ScipHandler::send(const char data[], int size)
{
    if (size >= 2) {
        if ((! strncmp("MD", data, 2)) || (! strncmp("MS", data, 2)) || (! strncmp("ME", data, 2))) {
            pimpl->laser_state_ = pImpl::LaserOn;
            pimpl->mx_capturing_ = true;
            pimpl->isPreCommand_QT_ = false;
        }

        if ((! strncmp("ND", data, 2)) || (! strncmp("NS", data, 2)) || (! strncmp("NE", data, 2))) {
            pimpl->laser_state_ = pImpl::LaserOn;
            pimpl->nx_capturing_ = true;
            pimpl->isPreCommand_QT_ = false;
        }
    }
    return pimpl->con_->send(data, size);
}


int ScipHandler::recv(char data[], int size, int timeout)
{
    return pimpl->con_->receive(data, size, timeout);
}


bool ScipHandler::loadParameter(RangeSensorParameter &parameters)
{
    return pimpl->loadParameter(parameters);
}

bool ScipHandler::loadInformation(RangeSensorInformation &informations)
{
    return pimpl->loadInformation(informations);
}

bool ScipHandler::loadInternalInformation(RangeSensorInternalInformation &internalInformations)
{
    return pimpl->loadInternalInformation(internalInformations);
}

QVector<string> ScipHandler::supportedCommands()
{
    return pimpl->supportedCommands();
}

QStringList ScipHandler::supportedModes()
{
    return pimpl->supportedModes();
}

bool ScipHandler::commandLines(string &cmd, QVector<string >* lines, bool all)
{
    int return_code = -1;
    char expected_response[] = { 0, pImpl::Scip11Response, -1 };
    if (cmd[cmd.length() - 1] != '\n') {
        cmd += "\n";
    }
    if (! pimpl->response(return_code, expected_response, cmd.c_str(), lines, all)) {
        return false;
    }
    return true;
}

bool ScipHandler::commandLines(string &cmd, int &status, QVector<string >* lines, bool all)
{
    int return_code = -1;
    char expected_response[] = { 0, -1 };
    if (cmd[cmd.length() - 1] != '\n') {
        cmd += "\n";
    }
    pimpl->response(return_code, expected_response, cmd.c_str(), lines, all);
    status = return_code;
    if (return_code < 0) {
        return false;
    }
    return true;
}

bool ScipHandler::commandLines(QVector<string >* lines, bool all)
{
    int return_code = -1;
    char expected_response[] = { 0, -1 };

    if (! pimpl->response(return_code, expected_response, NULL, lines, all)) {
        return false;
    }
    return true;
}

bool ScipHandler::commandLines(int &status, QVector<string >* lines, bool all)
{
    int return_code = -1;
    char expected_response[] = { 0, -1 };

    pimpl->response(return_code, expected_response, NULL, lines, all);
    status = return_code;
    if (return_code < 0) {
        return false;
    }
    return true;
}

bool ScipHandler::versionLines(QVector<string> &lines)
{
    int return_code = -1;
    char expected_response[] = { 0, -1 };
    if (! pimpl->response(return_code, expected_response, "VV\n", &lines)) {
        return false;
    }
    return true;
}

bool ScipHandler::parameterLines(QVector<string> &lines)
{
    int return_code = -1;
    char expected_response[] = { 0, -1 };
    if (! pimpl->response(return_code, expected_response, "PP\n", &lines)) {
        return false;
    }
    return true;
}

bool ScipHandler::informationLines(QVector<string> &lines)
{
    int return_code = -1;
    char expected_response[] = { 0, -1 };
    if (! pimpl->response(return_code, expected_response, "II\n", &lines)) {
        return false;
    }
    return true;
}


bool ScipHandler::setRawTimestampMode(bool on)
{
    char send_command[] = "TMx\n";
    send_command[2] = (on) ? '0' : '2';

    // TM0 or TM2 の送信
    int return_code = -1;
    char expected_response[] = { 0, -1 };
    if (! pimpl->response(return_code, expected_response, send_command)) {
        pimpl->error_message_ = (on) ? "TM0 fail." : "TM2 fail.";
        return false;
    }

    // TM1, TM2 の応答が正常ならば、レーザは消灯しているはず
    pimpl->laser_state_ = pImpl::LaserOff;

    return true;
}


bool ScipHandler::rawTimestamp(long &timestamp)
{
    // TM1 の値を返す
    int return_code = -1;
    char expected_response[] = { 0, -1 };
    QVector<string> lines;
    if (! pimpl->response(return_code, expected_response, "TM1\n", &lines)) {
        pimpl->error_message_ = "TM1 fail.";
        return false;
    }

    if ((lines.size() != 1) || (lines[0].size() != 5)) {
        pimpl->error_message_ = "response mismatch.";
        return false;
    }

    timestamp = decode(lines[0].c_str(), 4);
    return true;
}


bool ScipHandler::setLaserOutput(bool on, bool force)
{
    return pimpl->setLaserOutput(on, force);
}


CaptureType ScipHandler::receiveCaptureData(QVector<QVector<long> > &ranges
                                            , QVector<QVector<long> > &levels
                                            , CaptureSettings &settings
                                            , long &timestamp
                                            , int* remain_times
                                            , int* total_times)
{
    CaptureType result = pimpl->receiveCaptureData(ranges, levels, settings,
                                                   timestamp, remain_times, total_times);
    return result;
}

bool ScipHandler::isContiniousMode()
{
    return pimpl->mx_capturing_ || pimpl->nx_capturing_;
}

