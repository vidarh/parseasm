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

class Assembler {
    VM p;
public:
    Assembler()
    {
	p.set_trigger(boost::shared_ptr<TriggerBase>(new Trigger<Assembler>(this)));
	p.set_brk_trigger(boost::shared_ptr<TriggerBase>(new Trigger<Assembler>(this)));
	p.read_memory("assembler.mem");
	p.write_memory("assembler2.mem");
//	p.set_debug(true);
    }
    
    bool put(int ch)
    {
	return p.put(ch);
    }

    
    std::map<std::string, int> labels;
    std::map<int, std::string> reloc;
    std::vector<int> memory;

    int convert(std::string ch)
    {
	if (ch.size() && ch[0] == '#') ch = ch.substr(1);
	
	if (isdigit(ch[0])) return boost::lexical_cast<int>(ch);

	if (ch == "'\\r'") return 13;
	if (ch == "'\\n'") return 10;
	if (ch == "'\\t'") return 9;
	
	if (ch[1] == '\\') return (int)((unsigned char)ch[2]);
	return (int)((unsigned char)ch[1]);
    }

    bool write_labels(const std::string & fname)
    {
	std::ofstream out(fname.c_str(),std::fstream::binary | std::fstream::out | std::fstream::trunc);
	if (!out) return false;
	
	std::map<std::string, int>::const_iterator i = labels.begin();
	while (i != labels.end()) {
	    out << i->first << std::endl;
	    out << i->second << std::endl;
	    ++i;
	}
	
	out.close();
	return true;
    }
    
    std::string unescape_str(const std::string & str)
    {
	std::string tmp;
	std::string::const_iterator i = str.begin();
	while (i != str.end()) {
	    if (*i == '\\') {
		++i;
		if (i != str.end()) {
		    if (*i == 'n') tmp += '\n';
		    else if (*i == 'r') tmp += '\r';
		    else if (*i == 't') tmp += '\t';
		    else if (*i == '0') tmp += '\0';
		    else tmp += *i;
		}
	    } else tmp += (char)*i;
	    ++i;
	}
	return tmp;
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
	
	if (vm.get_var(1) != "") {
	    labels.insert(std::make_pair(vm.get_var(1),memory.size()));
	}

	std::string op = vm.get_var(2);
	if (op == "") {
	} else if (op == "ret") {
	    memory.push_back(OP_RET);
	    memory.push_back(0);
	} else if (op == "clr") {
	    memory.push_back(OP_CLR);
	    memory.push_back(0);
	} else if (op == "nop") {
	    memory.push_back(OP_NOP);
	    memory.push_back(0);
	} else if (op == "eat") {
	    memory.push_back(OP_EAT);
	    memory.push_back(0);
	} else if (op == "err") {
	    memory.push_back(OP_ERR);
	    memory.push_back(0);
	} else if (op == "eof") {
	    memory.push_back(OP_TRY);
	    memory.push_back(-1);
	    int pos = memory.size();
	    memory.push_back(OP_BEQ);
	    memory.push_back(pos +4);
	    memory.push_back(OP_ERR);
	    memory.push_back(0);
	} else if (op == "beq") {
	    memory.push_back(OP_BEQ);
	    reloc.insert(std::make_pair(memory.size(),vm.get_var(3)));
	    memory.push_back(0);
	} else if (op == "bne") {
	    memory.push_back(OP_BNE);
	    reloc.insert(std::make_pair(memory.size(),vm.get_var(3)));
	    memory.push_back(0);
	} else if (op == "bgt") {
	    memory.push_back(OP_BGT);
	    reloc.insert(std::make_pair(memory.size(),vm.get_var(3)));
	    memory.push_back(0);
	} else if (op == "blt") {
	    memory.push_back(OP_BLT);
	    reloc.insert(std::make_pair(memory.size(),vm.get_var(3)));
	    memory.push_back(0);
	} else if (op == "jsr") {
	    memory.push_back(OP_JSR);
	    reloc.insert(std::make_pair(memory.size(),vm.get_var(3)));
	    memory.push_back(0);
	} else if (op == "kln") {
	    int pos = memory.size();
	    memory.push_back(OP_JSR);
	    reloc.insert(std::make_pair(memory.size(),vm.get_var(3)));
	    memory.push_back(0);
	    memory.push_back(OP_BEQ);
	    memory.push_back(pos);
	} else if (op == "cut") {
	    int pos = memory.size();
	    std::string tmp = vm.get_var(6);
	    memory.push_back(OP_JSR);  //0
	    memory.push_back(pos + 8 + tmp.size()); // 1
	    memory.push_back(OP_BNE);  // 2
	    memory.push_back(pos + 5); // 3
	    memory.push_back(OP_RET);  // 4
	    memory.push_back(OP_BRK);  // 5
	    memory.push_back(pos+7);   // 6
	    
	    // 7
	    std::string::const_iterator i = tmp.begin();
	    while (i != tmp.end()) {
		memory.push_back(*i);
		++i;
	    }
	    memory.push_back(0); // 7 + tmp.size()
	    // 8 + tmp.size()
	} else if (op == "jmp") {
	    memory.push_back(OP_JMP);
	    reloc.insert(std::make_pair(memory.size(),vm.get_var(3)));
	    memory.push_back(0);
	} else if (op == "sto") {
	    memory.push_back(OP_STO);
	    memory.push_back(convert(vm.get_var(4)));
	} else if (op == "cmp") {
	    memory.push_back(OP_CMP);
	    memory.push_back(convert(vm.get_var(4)));
	} else if (op == "cmp.r") {
	    memory.push_back(OP_CMP_R);
	    memory.push_back(convert(vm.get_var(4)));
	    memory.push_back(convert(vm.get_var(5)));
	} else if (op == "str") {
	    std::string tmp = unescape_str(vm.get_var(6));
	    std::string::const_iterator i = tmp.begin();
	    while (i != tmp.end()) {
		memory.push_back(*i);
		++i;
	    }
	    memory.push_back(0);
	} else if (op == "brk") {
	    int pos = memory.size();
	    memory.push_back(OP_BRK);
	    if (vm.get_var(3) != "") {
		reloc.insert(std::make_pair(memory.size(),vm.get_var(3)));
		memory.push_back(0);
	    } else if (vm.get_var(6) != "") {
		memory.push_back(pos+2);
		std::string tmp = unescape_str(vm.get_var(6));
		std::string::const_iterator i = tmp.begin();
		while (i != tmp.end()) {
		    memory.push_back(*i);
		    ++i;
		}
		memory.push_back(0);
	    } else throw "Error with BRK";
	} else if (op == "req") {
	    // req $prod
	    if (vm.get_var(3) != "") {
		memory.push_back(OP_JSR);
		reloc.insert(std::make_pair(memory.size(),vm.get_var(3)));
		memory.push_back(0);
		int pos = memory.size();
		memory.push_back(OP_BEQ);
		memory.push_back(pos + 4);
		memory.push_back(OP_ERR);
		memory.push_back(0);
	    } else if (vm.get_var(6) != "") { // req ""
		std::string tmp = unescape_str(vm.get_var(6));
		std::string::const_iterator i = tmp.begin();
		while (i != tmp.end()) {
		    memory.push_back(OP_REQ);
		    memory.push_back((unsigned char)*i);
		    ++i;
		}
	    } else if (vm.get_var(3) == "") { // req #
		memory.push_back(OP_REQ);
		memory.push_back(convert(vm.get_var(4)));
	    } else throw "Unknown req syntax";
	} else if (op == "try") {
	    if (vm.get_var(3) != "") {
		memory.push_back(OP_JSR);
		reloc.insert(std::make_pair(memory.size(),vm.get_var(3)));
		memory.push_back(0);
	    } else if (vm.get_var(6) != "") { // req ""
		std::string tmp = unescape_str(vm.get_var(6));
		int pos = memory.size();
		memory.push_back(OP_JSR);
		memory.push_back(pos + 4);
		memory.push_back(OP_JMP);
		memory.push_back(pos + 5 + (tmp.size()*2));
		
		std::string::const_iterator i = tmp.begin();
		while (i != tmp.end()) {
		    memory.push_back(OP_REQ);
		    memory.push_back((unsigned char)*i);
		    ++i;
		}
		memory.push_back(OP_RET);
	    } else {
		memory.push_back(OP_TRY);
		memory.push_back(convert(vm.get_var(4)));
	    }
	} else if (op == "trg") {
	    memory.push_back(OP_TRG);
	    memory.push_back(convert(vm.get_var(4)));
	} else throw "Unexpected command: " + op;
	return true;
    }
    
    void pass2() {
	std::cerr << "memory.size() = " << memory.size() << std::endl;
	std::cerr << "labels.size() = " << labels.size() << std::endl;
	std::cerr << "reloc.size()  = " << reloc.size() << std::endl;
	
	std::map<int, std::string>::const_iterator i = reloc.begin();
	while (i != reloc.end()) {
	    std::map<std::string, int>::const_iterator j = labels.find(i->second);
	    if (j != labels.end()) {
		std::cerr << "relocating '" << j->first << "' at " << i->first << " to " << j->second << std::endl;
		memory[i->first] = j->second;
	    } else {
		throw "Unknown label: " + i->second;
	    }
	    ++i;
	}
	
	VM p(memory);
	p.write_memory("result.mem");
	write_labels("result.dbg");
    };
};



int main()
{
    Assembler a;
    
    try {
	while (!std::cin.eof() && a.put(std::cin.get()));
	a.pass2();
    } catch(const char * str) {
	std::cerr << "EXCEPTION: " << str << std::endl;
    } catch(const std::string & str) {
	std::cerr << "EXCEPTION: " << str << std::endl;
    }
}
