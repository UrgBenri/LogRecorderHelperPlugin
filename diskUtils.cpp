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

#include "diskUtils.h"
#include <QDir>

#if defined(Q_OS_WIN)
#include <windows.h>
#endif
#if defined(Q_OS_LINUX)
#include <sys/vfs.h>
#include <sys/stat.h>
#endif

#if defined(Q_OS_MACX)
#include <sys/param.h>
#include <sys/mount.h>
#endif



bool getFreeTotalSpace(const QString &sDirPath, double &fTotal, double &fFree)
{
    double fKB = 1024.0;
    fTotal = 0;
    fFree = 0;

#if defined(Q_OS_WIN)

    QString sCurDir = QDir::current().absolutePath();
    QDir::setCurrent(sDirPath);

    ULARGE_INTEGER free, total;
    bool bRes = ::GetDiskFreeSpaceExA(0 , &free , &total , NULL);
    if (!bRes) {
        return false;
    }

    QDir::setCurrent(sCurDir);

    fFree = static_cast<double>(static_cast<__int64>(free.QuadPart)) / fKB;
    fTotal = static_cast<double>(static_cast<__int64>(total.QuadPart)) / fKB;
    return true;
#endif

#if defined(Q_OS_LINUX)

    struct stat stst;
    struct statfs stfs;

    if (::stat(sDirPath.toLocal8Bit(), &stst) == -1) {
        return false;
    }
    if (::statfs(sDirPath.toLocal8Bit(), &stfs) == -1) {
        return false;
    }

    fFree = stfs.f_bavail * (stst.st_blksize / fKB);
    fTotal = stfs.f_blocks * (stst.st_blksize / fKB);
    return false;
#endif

#if defined(Q_OS_MACX)
    struct statfs stfs;

    if (::statfs(sDirPath.toLocal8Bit(), &stfs) == -1) {
        return false;
    }

    fFree = stfs.f_bavail * (stfs.f_bsize / fKB);
    fTotal = stfs.f_blocks * (stfs.f_bsize / fKB);
    return true;
#endif

}

