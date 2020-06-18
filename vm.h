#ifndef __VM_H
#define __VM_H

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


#include <string>
#include <map>
#include <stack>

enum {
    YIELD_FLAG = 0x40,
    RANGE_FLAG = 0x20
};

enum {
    OP_RET = 0x00,
    OP_BEQ = 0x01,
    OP_BNE = 0x02,
    OP_STO = 0x03,
    OP_TRG = 0x04,
    OP_ERR = 0x05,
    OP_CLR = 0x06,
    OP_JSR = 0x07,
    OP_JMP = 0x08,
    OP_NOP = 0x09,
    OP_BLT = 0x0a,
    OP_BGT = 0x0b,
    OP_BRK = 0x0c,
    OP_CMP = 0x00 | YIELD_FLAG,  
    OP_EAT = 0x01 | YIELD_FLAG,  
    OP_TRY = 0x02 | YIELD_FLAG,  
    OP_REQ = 0x03 | YIELD_FLAG,
    OP_CMP_R = 0x00 | RANGE_FLAG | YIELD_FLAG,  	
    
};


class VM;

class TriggerBase {
public:
    virtual bool trigger(VM & vm, int op, int arg) = 0;
};

template<typename T>
    class Trigger : public TriggerBase {
	T * ptr;
    public:
	Trigger(T * p)
	{
	    ptr = p;
	}
	
	bool trigger(VM & vm, int op, int arg)
	{
	    return ptr->trigger(vm,op,arg);
	}
    };


int read_int(std::istream &);
void write_int(std::ostream &, int);

class VM {
    struct StackFrame {
	std::string t;
	std::string u;
	int r;
	int row;
	int col;
    };
    
    std::stack<StackFrame> stack;
    std::vector<int> memory;

    // FIXME: Complete stackframe, so why not make it one?
    std::string t;
    std::string u;
    int pc;
    int row;
    int col;

    bool gt;
    bool lt;
    bool s;
    int c;

    // Trigger handling
    std::map<int, std::string> v;
    boost::shared_ptr<TriggerBase> trigger_;
    boost::shared_ptr<TriggerBase> brktrigger_;

    // Debug data
    std::map<int, std::string> labels;
    void add_label(int pos, const std::string & l)
    {
	labels.insert(std::make_pair(pos,l));
    }
    
    std::string get_label(int pos = -1) const
    {
	int p = (pos == -1) ? pc : pos;
	std::map<int, std::string>::const_iterator i = labels.find(p);
	if (i == labels.end()) return "";
	return i->second;
    }
	
    
    bool debug;
public:
    
    void set_debug(bool d)
    {
	debug = d;
    }
    
    VM(const std::vector<int> & mem) : memory(mem)
    {
	s = false;
	pc = 0;
	row = 1;
	col = 1;
	debug = false;
    }
    
    VM()
    {
	s  = false;
	pc = 0;
	row = 1;
	col = 1;
	debug = false;
    }

    void set_trigger(boost::shared_ptr<TriggerBase> t)
    {
	trigger_ = t;
    }
    
    void set_brk_trigger(boost::shared_ptr<TriggerBase> t)
    {
	brktrigger_ = t;
    }

    bool get_s() const
    {
	return s;
    }
    
    std::string get_var(int i) const;
    
    int get_lineno() const
    {
	return row;
    }

    int get_column() const
    {
	return col;
    }
    
    std::string get_string(int pos)
    {
	std::string str;
	int ch;
	while ((ch = memory.at(pos))) {
	    str += (char)ch;
	    ++pos;
	}
	return str;
    }

    
    bool put(int ch);
    void put(const std::string & str);
    
    bool trigger(int t);
    int add(int op, int v);

private:
    void opcode_ret();
    void opcode_err();
    void opcode_eat();
    bool opcode_cmp(int);
    bool opcode_cmp_r(int,int);
    bool opcode_try(int);
    bool opcode_req(int);
    void opcode_brk(int);
    void intop_psh();
public:   
    void write_memory(std::ostream &);
    void read_memory(std::istream &);
    bool write_memory(const std::string & f);
    bool read_memory(const std::string & f);
};


#endif
