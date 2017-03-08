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

#ifndef QRK_RANGE_SENSOR_NTERNAL_INFORMATION_H
#define QRK_RANGE_SENSOR_NTERNAL_INFORMATION_H

#include <string>

namespace qrk
{
//! class to manage range sensor's parameter
class RangeSensorInternalInformation
{
public:
    enum {
        MODL = 0,             //!< Product model
        LASR,                 //!< Laser status On OR Off
        SCSP,                 //!< Motor desired speed [rpm]
        MESM,                 //!< Sensor state ID with title
        SBPS,                 //!< Communication type
        TIME,                 //!< Sensor time 6 bits Hex
        STAT,                 //!< Sensor stuation "stable/unstable"
    };

    std::string model;             //!< MODL
    std::string laserStatus;       //!< LASR
    std::string motorDesiredSpeed; //!< SCSP
    std::string stateID;           //!< MESM
    std::string communicationType; //!< SBPS
    std::string internalTime;      //!< TIME
    std::string sensorSituation;   //!< STAT

    RangeSensorInternalInformation(void)
        : model(""), laserStatus(""), motorDesiredSpeed(""), stateID(""),
          communicationType(""), internalTime(""), sensorSituation("") {
    }
};
}

#endif /* !QRK_RANGE_SENSOR_NTERNAL_INFORMATION_H */

