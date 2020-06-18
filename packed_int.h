#ifndef __packed_int_H
#define __packed_int_H

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


#include <iostream>

// The purpose of packed_int() is to allow easy reading and writing of int's that are
// stored as sequences of 7 bit values with the MSB indicating whether or not the next
// byte is part of the int or not. That allows arbitrary length values, and in an
// environment where most values used are small it is likely to result in a net "compression".
// The idea is taken from Oberon.

class packed_int
{
private:
    int i_;
public:
    packed_int(int i = 0) : i_(i)
    {
    }
    
    operator int() const
    {
	return i_;
    }
    
    packed_int & operator=(int i)
    {
	i_ = i;
	return *this;
    }
};

std::istream & operator>>(std::istream & inf, packed_int & p);
std::ostream & operator<<(std::ostream & o, const packed_int & p);

#endif
