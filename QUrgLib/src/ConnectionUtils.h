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

#ifndef QRK_CONNECTION_UTILS_H
#define QRK_CONNECTION_UTILS_H

/*!
  \file
  \brief Connection を用いた補助関数

  \author Satofumi KAMIMURA

  $Id: ConnectionUtils.h 57 2012-06-12 04:43:55Z kristou $
*/

#include <cstddef>
#include <algorithm>


//! Quick Robot Develoment Kit.
namespace qrk
{
class Connection;


/*!
  \brief 改行コードかを返す

  \retval true LF, CR のとき
  \retval false 上記以外のとき
*/
extern bool isLF(const char ch);


/*!
  \brief 受信データを読み飛ばす

  Connection::clear() とは、タイムアウト時間を指定して読み飛ばせる点が異なる

  \param[in,out] con 通信リソース
  \param[in] total_timeout タイムアウト時間の上限 [msec]
  \param[in] each_timeout 受信データ間におけるタイムアウト時間の上限 [msec]
*/
extern void skip(Connection* con, int total_timeout, int each_timeout = 0);


/*!
  \brief 改行までのデータ読み出し

  文字列終端に \\0 を付加して返す

  \param[in,out] con 通信リソース
  \param[out] buf 受信バッファ
  \param[in] count 受信バッファの最大サイズ
  \param[in] timeout タイムアウト [msec]

  \return 受信文字数 (受信がなくてタイムアウトした場合は -1)
*/
extern int readline(Connection* con, char* buf, const size_t count,
                    int timeout);


/*!
  \brief 接続オブジェクトの交換

  a と b の接続オブジェクトを交換する。
*/
template <class T>
void swapConnection(T &a, T &b)
{
    Connection* t = a.connection();
    a.setConnection(b.connection());
    b.setConnection(t);
}
}

#endif /* !QRK_CONNECTION_UTILS_H */

