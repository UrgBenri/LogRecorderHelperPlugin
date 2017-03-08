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

#ifndef QRK_THREAD_H
#define QRK_THREAD_H

/*!
  \file
  \brief スレッド処理のラッパー

  \author Satofumi KAMIMURA

  $Id: Thread.h 130 2012-09-07 07:42:08Z kristou $
*/

#include <memory>
#include <QObject>

namespace qrk
{
class Thread: public QObject
{
    Q_OBJECT
public:

    bool exitThread;
    enum {
        Infinity = -1,
    };

    /*!
      \brief コンストラクタ

      \param[in,out] fn 関数ポインタ
      \param[in,out] args スレッド関数への引数
    */
    explicit Thread(int (*fn)(void*), void* args);
    virtual ~Thread(void);


    /*!
      \brief 処理回数を指定してスレッドを起動

      \param[in] times 処理回数
    */
    void run(/*int times = 1*/);


    //! スレッドを停止
    void stop(void);


    /*!
      \brief スレッドの終了を待つ

      \return スレッドの戻り値
    */
    int wait(void);
    void msleep(unsigned long msecs);


    /*!
      \brief スレッドが動作中かを返す

      \retval true 動作中
      \retval false 停止中
    */
    bool isRunning(void) const;
signals:
    void finished();

private:
    Thread(void);
    Thread(const Thread &rhs);
    Thread &operator = (const Thread &rhs);

    struct pImpl;
    const std::auto_ptr<pImpl> pimpl;
};
}

#endif /* !QRK_THREAD_H */

