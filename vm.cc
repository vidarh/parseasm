
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
#include <vector>
#include <iomanip>
#include <boost/shared_ptr.hpp>
#include <boost/lexical_cast.hpp>
#include <fstream>
#include <iterator>

#include "vm.h"
#include "packed_int.h"

std::string VM::get_var(int var) const
{
    std::map<int,std::string>::const_iterator i = v.find(var);
    if (i == v.end()) return "";
    return i->second;
}

void VM::write_memory(std::ostream & out)
{
    std::copy(memory.begin(),memory.end(),std::ostream_iterator<packed_int,char>(out));
}

bool VM::write_memory(const std::string & f)
{
    std::ofstream out(f.c_str(),std::fstream::binary | std::fstream::out | std::fstream::trunc);
    if (!out) return false;
    write_memory(out);
    out.close();
    return true;
}

void VM::read_memory(std::istream & inf)
{
    memory.resize(0);
    std::copy(std::istream_iterator<packed_int,char>(inf), 
	      std::istream_iterator<packed_int>(), std::back_inserter(memory));
}

bool VM::read_memory(const std::string & f)
{
    std::ifstream inf(f.c_str(),std::fstream::in | std::fstream::binary);
    if (!inf) return false;
    read_memory(inf);
    inf.close();
    return true;
}


bool VM::trigger(int i)
{
    bool ret;
    if (trigger_) ret = trigger_->trigger(*this,OP_TRG,i);
    else ret = true;
    v.erase(v.begin(),v.end());
    return ret;
}

void VM::opcode_brk(int i)
{
    if (brktrigger_) {
	if (brktrigger_->trigger(*this,OP_BRK,i)) pc = -1;
    } else pc = -1;
}

void VM::intop_psh()
{
    StackFrame f;
    f.r = pc;
    f.u = u;
    f.t = t;
    f.row = row;
    f.col = col;
    stack.push(f);
    u = "";
    
}

void VM::opcode_ret()
{
    s = true;
    if (!stack.size()) {
	pc = -1;
	return;
    }
    StackFrame f = stack.top();
    stack.pop();
    pc = f.r;
    u = f.u + u;
}

void VM::opcode_err()
{
    std::string tmp = u;
    if (c != -1) {
	tmp += (char)c;
	c = -1;
    }

    s = false;

    if (!stack.size()) {
	pc = -1;
	return;
    }
    
    StackFrame f = stack.top();
    stack.pop();
    pc = f.r;
    u = f.u;
    t = f.t;
    row = f.row;
    col = f.col;

    put(tmp);
}


// --- I/O

void VM::opcode_eat()
{
    s = true;
    u += c;
    t += c;
    if (c == '\n') {
	col = 1;
	++row;
    } else ++col;
    c = -2;
}

bool VM::opcode_cmp(int arg)
{
    gt = (c > arg);
    lt = (c < arg);
    return s = (!gt && !lt);
}

bool VM::opcode_cmp_r(int arg,int arg2)
{
    gt = (c > arg2);
    lt = (c < arg);
    return s = (!gt && !lt);
}



// ----- Pure compound ops (i.e. could be removed and implemented in the assembler) -------------

bool VM::opcode_try(int arg)
{
    if (opcode_cmp(arg)) opcode_eat();
    return s;
}

bool VM::opcode_req(int arg)
{
    if (opcode_cmp(arg)) opcode_eat();
    else opcode_err();
    return s;
}


// --------------------------------------


void out(int pc, const std::string & op, int arg = -1)
{
    std::cerr << std::setw(4) << pc << std::setw(-1) << " " << op;
    if (arg != -1) std::cerr << " " << arg;
    std::cerr << std::endl;
}

#ifndef NDEBUG
#define OUT(op,arg) do { if (debug) out(pc-2,op,arg); } while(0)
#else
#define OUT(op,arg) do { } while(0)
#endif

bool VM::put(int ch)
{
    c = ch;
    
    if (debug) {
	std::cerr << "PUT " << c << " '" << (char)c << "'" << std::endl;
	std::cerr << "    t: '" << t << "'" << std::endl;
    }
    
    while (1) {
	if (pc == -1) return false;
	int op = memory[pc];
	int arg = memory[pc + 1];
	int arg2;

	if (debug) {
	    std::string label = get_label();
	    if (label != "") std::cerr << ":" << label << std::endl;
	}
	
	if ((op & RANGE_FLAG)) arg2 = memory[pc+2];
	
	// Ops with YIELD_FLAG set yields if there is no waiting character
	if ((op & YIELD_FLAG) && (c == -2)) return true;

	// Skip to next op now, before any of the opcode's change PC.
	pc += 2;
	
	if (op & RANGE_FLAG) pc +=1; // Additional arg
	
	switch(op) {
	case OP_RET:
	    OUT("ret",-1);
	    opcode_ret();
	    break;
	case OP_BEQ:
	    OUT("beq",arg);
	    if (s) pc = arg;
	    break;
	case OP_BNE:
	    OUT("bne",arg);
	    if (!s) pc = arg;
	    break;
	case OP_BLT:
	    OUT("blt",arg);
	    if (lt) pc = arg;
	    break;
	case OP_BGT:
	    OUT("bgt",arg);
	    if (gt) pc = arg;
	    break;
	case OP_STO:
	    OUT("sto",arg);
	    v[arg] = t;
	    t = "";
	    break;
	case OP_TRG:
	    OUT("trg",arg);
	    s = trigger(arg);
	    break;
	case OP_ERR:
	    OUT("err",-1);
	    opcode_err();
	    break;
	case OP_CLR:
	    OUT("clr",-1);
	    t = "";
	    break;
	case OP_JSR:
	    OUT("jsr",arg);
	    intop_psh();
	    pc = arg;
	    break;
	case OP_JMP:
	    OUT("jmp",arg);
	    pc = arg;
	    break;
	case OP_CMP:
	    OUT("cmp",arg);
	    opcode_cmp(arg);
	    break;
	case OP_CMP_R:
	    OUT("cmp.r",arg);
	    OUT("-----",arg2);
	    opcode_cmp_r(arg,arg2);
	    break;	    
	case OP_TRY:
	    OUT("try",arg);
	    opcode_try(arg);
	    break;
	case OP_REQ:
	    OUT("req",arg);
	    opcode_req(arg);
	    break;
	case OP_EAT:
	    OUT("eat",-1);
	    opcode_eat();
	    break;
	case OP_NOP:
	    break;
	case OP_BRK:
	    OUT("brk",arg);
	    opcode_brk(arg);
	    break;
	default:
	    throw "Unknown opcode: " + boost::lexical_cast<std::string>(op);
	};
    }
}
#undef OUT

int VM::add(int op, int v)
{
    int m = memory.size();
    memory.push_back(op);
    memory.push_back(v);
    return m;
}

void VM::put(const std::string & str)
{
    std::string::const_iterator i = str.begin();
    while (i != str.end() && put(*i)) ++i;
}
