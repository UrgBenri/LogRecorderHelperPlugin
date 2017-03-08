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

#include "Thread.h"
#include <QThread>
#include <limits>
#include <QDebug>

using namespace qrk;
using namespace std;


namespace
{
class ThreadWrapper : public QThread
{
    int (*function_)(void*);
    void* args_;
    int times_;
    int return_value_;


public:

    ThreadWrapper(int (*function)(void*), void* args)
        : function_(function), args_(args), times_(1), return_value_(-1) {
    }

    void run(void) {
        return_value_ = function_(args_);

    }

    void msleep(unsigned long msecs) {
        QThread::msleep(msecs);
    }


    int returnValue(void) {
        return return_value_;
    }
};
}


struct Thread::pImpl {
    ThreadWrapper thread_;


    pImpl(int (*fn)(void*), void* args) : thread_(fn, args) {
    }
};


Thread::Thread(int (*fn)(void*), void* args) : exitThread(false), pimpl(new pImpl(fn, args))
{
    connect(&pimpl->thread_, &QThread::finished,
            this, &Thread::finished);
}


Thread::~Thread(void)
{
    stop();
//    exitThread = true;
//    pimpl->thread_.wait();
}


void Thread::run(/*int times*/)
{
//    pimpl->thread_.setTimes((times == Infinity)
//                            ? numeric_limits<int>::max() : times);
    exitThread = false;
    pimpl->thread_.start();
}


void Thread::stop(void)
{
    exitThread = true;
    wait();
//    pimpl->thread_.terminate();
}


int Thread::wait(void)
{
    if(isRunning()){
        pimpl->thread_.wait();
        return pimpl->thread_.returnValue();
    }else{
        return 0;
    }
}

void Thread::msleep(unsigned long msecs)
{
    pimpl->thread_.msleep(msecs);
}


bool Thread::isRunning(void) const
{
    return pimpl->thread_.isRunning();
}

