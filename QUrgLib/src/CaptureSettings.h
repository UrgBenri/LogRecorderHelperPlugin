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

#ifndef QRK_CAPTURE_SETTINGS_H
#define QRK_CAPTURE_SETTINGS_H

/*!
  \file
  \brief Data acquisition setting

  \author Satofumi KAMIMURA

  $Id: CaptureSettings.h 57 2012-06-12 04:43:55Z kristou $
*/

/*!
  \brief Quick Robot Development Kit.
*/
namespace qrk
{
/*!
  \brief Receive data type
*/
typedef enum {
    TypeUnknown,                //!< Unknown
    QT,                         //!< Quit to receive
    GD,                         //!< Get Data (each data size: 3 byte)
    GS,                         //!< Get Short data (each data size: 2 byte)
    GE,                         //!< Get Data + Intensity data (each data size: 3 byte + 3 byte)
    Gx_Reply,                   //!< The first response message of Mx command
    HD,                         //!< Get Data (each data size: 3 byte)
    HS,                         //!< Get Short data (each data size: 2 byte)
    HE,                         //!< Get Data + Intensity data (each data size: 3 byte + 3 byte)
    Hx_Reply,                   //!< The first response message of Mx command
    MD,                         //!< Continous Measure Data (each data size: 3 byte)
    MS,                         //!< Continous Measure Data (each data size: 2 byte)
    ME,                         //!< Continous Measure Data + Intensity data (each data size: 3 byte + 3 byte)
    Mx_Reply,                   //!< The first response message of Mx command
    ND,                         //!< Get multiEcho Data (each data size: 3 byte separated by '&')
    NS,                         //!< Get multiEcho Short data (each data size: 2 byte separated by '&')
    NE,                         //!< Get multiEcho Data + Intensity data (each data size: 3 byte + 3 byte separated by '&')
    Nx_Reply,                   //!< The first response message of Nx command
    InvalidData,                //!< Error response
} CaptureType;


/*!
  \brief Data acquisition setting
*/
class CaptureSettings
{
public:
    CaptureType type;           //!< Type of receive data
    int error_code;             //!< Error code
    long timestamp;             //!< [msec]
    int capture_first;          //!< Acquisition beginning index
    int capture_last;           //!< Acquisition end index
    int group_steps;             //!< Number of lines to skip
    int skip_frames;            //!< Data acquisition interval
    int remain_times;           //!< Remaining number of  scans
    int data_byte;              //!< Number of data bytes


    CaptureSettings(void)
        : type(TypeUnknown), error_code(-1), timestamp(-1),
          capture_first(-1), capture_last(-1),
          group_steps(-1), skip_frames(-1), remain_times(-1), data_byte(-1) {
    }
};
}

#endif /* !QRK_CAPTURE_SETTINGS_H */

