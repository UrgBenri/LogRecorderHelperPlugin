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

#include "log_printf.h"
#include <string>
#include <cstdio>
#include <cstdarg>
#include <iostream>
#include <QDebug>

using namespace std;


namespace
{
string log_file_ = "error_log.txt";
}


int qrk::log_printf(const char* format, ...)
{
    static FILE* fd = NULL;
    if (fd == NULL) {
        fd = fopen(log_file_.c_str(), "w");
        if (fd == NULL) {
            return -1;
        }
    }

    va_list ap;

    // 標準エラー出力
//    va_start(ap, format);
//    vfprintf(stderr, format, ap);
//    va_end(ap);

    // ファイル出力
    va_start(ap, format);
    int ret = vfprintf(fd, format, ap);
    va_end(ap);

    // fclose() する機会がないので、内容を書き出しておく
    fflush(fd);

    return ret;
}


void qrk::log_setName(const char* file_name)
{
    log_file_ = file_name;
}

