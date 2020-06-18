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


#include <stack>
#include <map>
#include <string>
#include <iostream>
#include <vector>
#include <iomanip>
#include <boost/shared_ptr.hpp>
#include <boost/lexical_cast.hpp>
#include <fstream>

#include "vm.h"

class Run {
    VM p;
public:
    Run(const std::string & parser,bool debug = false)
    {
	p.set_trigger(boost::shared_ptr<TriggerBase>(new Trigger<Run>(this)));
	p.set_brk_trigger(boost::shared_ptr<TriggerBase>(new Trigger<Run>(this)));
	p.read_memory(parser);
	p.set_debug(debug);
    }
    
    bool put(int ch)
    {
	return p.put(ch);
    }

    bool trigger(VM & vm, int opcode, int arg)
    {
	if (opcode == OP_BRK) {
	    std::string msg;
	    msg += "ERROR at ";
	    msg += boost::lexical_cast<std::string>(vm.get_lineno());
	    msg += ",";
	    msg += boost::lexical_cast<std::string>(vm.get_column());
	    msg += ":";
	    msg += vm.get_string(arg);
	    throw msg;
	}
	
	std::string msg;
	msg += "TRIGGER at ";
	msg += boost::lexical_cast<std::string>(vm.get_lineno());
	msg += ",";
	msg += boost::lexical_cast<std::string>(vm.get_column());
	msg += ":";
	msg += boost::lexical_cast<std::string>(arg);
	std::cout << msg << std::endl;
	int i = 1;
	while (i < 10) {
	    std::string tmp = vm.get_var(i);
	    if (tmp != "") std::cout << "   " << i << " -> " << tmp << std::endl;
	    ++i;
	}
	return true;
    }
    
};

int main(int argc, const char ** argv)
{
    Run a(argv[1], argc > 2 && argv[2] == std::string("debug"));
    
    try {
	while (!std::cin.eof() && a.put(std::cin.get()));
    } catch(const char * str) {
	std::cout << "EXCEPTION: " << str << std::endl;
    } catch(const std::string & str) {
	std::cout << "EXCEPTION: " << str << std::endl;
    }
}
