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

#ifndef QRK_FIND_COM_PORTS_H
#define QRK_FIND_COM_PORTS_H

/*!
  \file
  \brief COM ポート一覧の取得

  \author Satofumi KAMIMURA

  $Id: FindComPorts.h 57 2012-06-12 04:43:55Z kristou $

  \attention 動作確認は Windows XP のみ。他では未検証
*/

#include <QVector>
#include <QString>
#include <memory>


namespace qrk
{
/*!
  \brief シリアルポートの探索を行う

  Windows では、レジストリ情報を元に COM 一覧を取得して返す。\n
  Linux, MacOS では、与えられたパターンに一致するファイル名を取得して返す。
*/
class FindComPorts
{
public:
    FindComPorts(void);
    virtual ~FindComPorts(void);


    /*!
      \brief 登録済みのベース名のクリア
    */
    void clearBaseNames(void);


    /*!
      \brief 探索で用いるポートのベース名を追加する

      \param[in] base_name ポートのベース名
    */
    void addBaseName(const QString &base_name);


    /*!
      \brief 登録済みのポートのベース名を返す

      Linux 実装で用いる

      \return ポートのベース名
    */
    QStringList baseNames(void);


    void addDriverName(const QString &driver_name);


    /*!
      \brief COM ポート一覧の取得

      \param[out] ports 検出したポート
      \param[in] all_ports addDriverName() で指定したポートのみ取得するとき false

      \retval COM 一覧が格納された string 配列
    */
    size_t find(QStringList &ports, bool all_ports = true);

    size_t find(QStringList &ports,
                QStringList &driver_names, bool all_ports = true);

private:
    FindComPorts(const FindComPorts &rhs);
    FindComPorts &operator = (const FindComPorts &rhs);

    struct pImpl;
    std::auto_ptr<pImpl> pimpl;
};
}

#endif /* !QRK_FIND_COM_PORTS_H */

