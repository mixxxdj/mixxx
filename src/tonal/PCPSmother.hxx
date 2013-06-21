/*
 * Copyright (c) 2001-2006 MUSIC TECHNOLOGY GROUP (MTG)
 *                         UNIVERSITAT POMPEU FABRA
 *
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

#ifndef PCPSmother_hxx
#define PCPSmother_hxx
#include <list>
#include <vector>
#include <cmath>
#include <sstream>

namespace Simac
{

class PCPSmother
{
public:
	typedef std::vector<double> PCP;
public:
	PCPSmother(double inertia)
		: _inertia(inertia)
	{
		_output.resize(12);
	}
	~PCPSmother()
	{
	}
	void doIt(const PCP & pcp)
	{
		for (unsigned int i=0; i<pcp.size(); i++)
		{
			_output[i]*=_inertia;
			_output[i]+= (1-_inertia) * pcp[i];
		}
	}
	void inertia(double inertia)
	{
		_inertia = inertia;
	}
	const PCP & output() const
	{
		return _output;
	}
private:
	PCP _output;
	double _inertia;
};

} // namespace Simac

#endif// PCPSmother_hxx

