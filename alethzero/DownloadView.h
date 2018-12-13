/*
	This file is part of cpp-ethereum.

	cpp-ethereum is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	cpp-ethereum is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with cpp-ethereum.  If not, see <http://www.gnu.org/licenses/>.
*/
/** @file DownloadView.h
 * @author Gav Wood <i@gavwood.com>
 * @date 2014
 */

#pragma once

#ifdef Q_MOC_RUN
#define BOOST_MPL_IF_HPP_INCLUDED
#endif

#include <list>
#include <QtWidgets/QWidget>
#ifndef Q_MOC_RUN
#include <libethereum/Client.h>
#endif

namespace dev { namespace eth {
struct MineInfo;
class DownloadMan;
}}

class DownloadView: public QWidget
{
	Q_OBJECT

public:
	DownloadView(QWidget* _p = nullptr);

	void setDownloadMan(dev::eth::DownloadMan const* _man) { m_man = _man; }

protected:
	virtual void paintEvent(QPaintEvent*);

private:
	dev::eth::DownloadMan const* m_man = nullptr;
};
