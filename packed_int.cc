/*
    Parser Assembler v0.1
    Copyright (C) 2005 Vidar Hokstad

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/


#include "packed_int.h"

std::istream & operator>>(std::istream & inf, packed_int & p)
{
    int tmp = 0;
    int ch;
    int rot = 0;
    ch= inf.get();
    if (ch == -1) return inf;
    do {
	tmp = tmp | ((ch & 0x7f) << rot);
	rot += 7;
	if (ch < 0x80) break;
	ch= inf.get();
    } while (ch != -1);
    p = tmp;
    return inf;
}

std::ostream & operator<<(std::ostream & o, const packed_int & p)
{
    unsigned int i = (int)p;
    do {
	if ((unsigned int)i < 0x80) {
	    o << (unsigned char)i;
	    break;
	} else o << (unsigned char)((i & 0x7f)|0x80);
	i = i >> 7;
    } while(i);
    return o;
}
