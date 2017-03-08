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

#ifndef QRK_RANGE_SENSOR_INFORMATION_H
#define QRK_RANGE_SENSOR_INFORMATION_H

/*!
  \file
  \brief Manage range sensor's parameter

  \author Satofumi KAMIMURA

  $Id: RangeSensorInformation.h 57 2012-06-12 04:43:55Z kristou $
*/

#include <string>


namespace qrk
{
//! class to manage range sensor's parameter
class RangeSensorInformation
{
public:
    enum {
        VEND = 0,             //!< Vendor Information
        PROD,                 //!< Product Information
        FIRM,                 //!< Firmware Version
        PROT,                 //!< Firmware Version
        SERI,                 //!< Sensor Serial Number
    };

    std::string vendor;             //!< VEND
    std::string product;            //!< PROD
    std::string firmware;           //!< FIRM
    std::string protocol;           //!< PROT
    std::string serial_number;      //!< SERI

    RangeSensorInformation(void)
        : vendor(""), product(""), firmware(""), protocol(""), serial_number("") {
    }
};
}

#endif /* !QRK_RANGE_SENSOR_PARAMETER_H */

