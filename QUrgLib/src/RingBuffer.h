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

#ifndef QRK_RING_BUFFER_H
#define QRK_RING_BUFFER_H

/*!
  \file
  \brief リングバッファ

  \author Satofumi KAMIMURA

  $Id: RingBuffer.h 131 2012-09-11 06:23:04Z kristou $
*/

#include <deque>
#include <QMutex>

namespace qrk
{
//! リングバッファ
template <class T>
class RingBuffer
{
public:
    explicit RingBuffer(void) {
    }


    /*!
      \brief バッファサイズの取得
    */
    size_t size(void) const {
        return ring_buffer_.size();
    }


    /*!
      \brief バッファが空か

      \retval true データなし
      \retval false データあり
    */
    bool empty(void) {
        return ring_buffer_.empty();
    }


    /*!
      \brief データの格納

      \param[in] data データ
      \param[in] size データ個数
    */
    void put(const T* data, size_t size) {
        QMutexLocker locker(&mutex);
        const T* last_p = data + size;
        ring_buffer_.insert(ring_buffer_.end(), data, last_p);
    }


    /*!
      \brief データの取り出し

      \param[out] data データ取り出し用バッファ
      \param[in] size 取り出すデータの最大個数

      \return 取り出したデータ個数
    */
    size_t get(T* data, size_t size) {
        QMutexLocker locker(&mutex);
        size_t n = qMin(size, ring_buffer_.size());
        std::copy(ring_buffer_.begin(), ring_buffer_.begin() + n, data);
        ring_buffer_.erase(ring_buffer_.begin(), ring_buffer_.begin() + n);
        return n;
    }


    /*!
      \brief データの書き戻し

      \param[in] ch 書き戻すデータ
    */
    void ungetc(const T ch) {
        ring_buffer_.push_front(ch);
    }


    /*!
      \brief 格納データのクリア
    */
    void clear(void) {
        ring_buffer_.clear();
    }


private:
    RingBuffer(const RingBuffer &rhs);
    RingBuffer &operator = (const RingBuffer &rhs);

    std::deque<T> ring_buffer_;
    QMutex mutex;
};
}

#endif /* !QRK_RING_BUFFER_H */

