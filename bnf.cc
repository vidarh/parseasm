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
#include <cassert>

#include "vm.h"


class Output {
    std::stack<std::string> o;

    std::string prod;
    int num;
public:
    Output()
    {
	num = 0;
    }
    
    void out(const std::string & str)
    {
	if (o.size()) o.top() += str;
	else std::cout << str;
    }
    
    void set_prod_name(const std::string & p)
    {
	prod = p;
    }
    
    std::string get_prod_name()
    {
	return prod;
    }
    
    std::string get_label(const std::string & base = "")
    {
	std::string tmp = base;
	if (tmp == "") tmp = prod;
	return tmp + "_" + boost::lexical_cast<std::string>(++num);
    }
    
    void out(int l)
    {
	out(boost::lexical_cast<std::string>(l));
    }

    void open()
    {
	o.push("");
    }
    
    void close()
    {
	if (o.size()) {
	    std::cout << o.top() << std::endl;
	    o.pop();
	}
    }
};


class Rule {
    std::string name;
    boost::shared_ptr<Rule> next;
    boost::shared_ptr<Rule> parent;

    std::string or_name;
    boost::shared_ptr<Rule> or_expr;
    
    std::string cut;
    std::string trigger;
    std::string store;
    std::string keyword;
    std::string production;
    std::string str;
    std::string op;
    std::string number;
    std::string set_;
public:
    Rule()
    {
    }
    
    explicit Rule(boost::shared_ptr<Rule> p)
    {
	parent = p;
    }
    
    boost::shared_ptr<Rule> get_parent()
    {
	return parent;
    }

    void set_name(const std::string & n)
    {
	name = n;
    }
    
    void set_set(const std::string & n)
    {
	set_ = n;
    }

    std::string get_name()
    {
	return name;
    }

    void set_or_name(const std::string & n)
    {
	or_name = n;
    }
    
    std::string get_or_name()
    {
	return or_name;
    }
    
    
    void set_parent(boost::shared_ptr<Rule> r)
    {
	parent = r;
    }
    
    void set_next(boost::shared_ptr<Rule> r)
    {
//	std::cout << "  this = " << this << " next = " << r.get() << std::endl;
	next = r;
    }
    
    boost::shared_ptr<Rule> get_next()
    {
	return next;
    }

    void set_cut(const std::string & s)
    {
	cut = s;
    }
    
    void set_trigger(const std::string & s)
    {
	trigger = s;
    }
    
    void set_number(const std::string & s)
    {
	number = s;
    }

    void set_or_expr(boost::shared_ptr<Rule> o)
    {
	or_expr = o;
    }
    
    void set_store(const std::string & s)
    {
	store = s;
    }
    
    void set_keyword(const std::string & s)
    {
	keyword = s;
    }
    
    void set_production(const std::string & s)
    {
	production = s;
    }
    
    void set_string(const std::string & s)
    {
	str = s;
    }
    
    void set_op(const std::string & s)
    {
	op = s;
    }
    
    boost::shared_ptr<Rule> get_or_expr()
    {
	return or_expr;
    }
    
    void output(Output &,const std::string & endlabel = "");
    void output_all(Output &, const std::string & endlabel = "");
};

std::string arg_from_char(unsigned int ch)
{
    
    if ((unsigned int)ch<32 || ch == '\\' || ch == '\'' || (unsigned int) ch >=128) return "#" + boost::lexical_cast<std::string>((int)ch);
    std::string tmp;
    tmp = "#'";
    tmp += (char)ch;
    tmp += "'";
    return tmp;
}


void output_set(Output & out, const std::string & label, const std::string & set)
{
    const char * c = set.c_str();
    bool n = *c == '~';
    char firstChar, secondChar;

    out.open();
    if (n) ++c;
    
    out.out(":" + label + " ; [" + set.c_str() + "]\n");
    
    while (*c) {
	firstChar = *c;
	if (*c == '\\') {
	    ++c;
	    if (*c == 'n') firstChar = '\n';
	    else if (*c == 'r') firstChar = '\r';
	    else if (*c == 't') firstChar = '\t';
	    else firstChar = *c;
	}
	if (*c == 0) break;
	
	++c;
	if (*c == '-') {
	    ++c;
	    if (*c == 0) break;
	    secondChar = *c;
	    if (*c == '\\') {
		++c;
		if (*c == 0) break;
		if (*c == 'n') secondChar = '\n';
		else if (*c == 'r') secondChar = '\r';
		else if (*c == 't') secondChar = '\t';
		else secondChar = *c;
	    }
	    ++c;
	} else secondChar = firstChar;
	
	if (firstChar == secondChar) {
	    out.out("   cmp " + arg_from_char(firstChar) + "\n");
	} else {
	    out.out("   cmp.r " + arg_from_char(firstChar) + "," + arg_from_char(secondChar) + "\n");
	}
	
	// FIXME
	std::string next = out.get_label();
	
	out.out("   bne $" + next + "\n");
	if (n) out.out("   err\n");
	else out.out("   eat\n   ret\n");
	out.out(":" + next + "\n");
    }
    if (n) {
	out.out("   eat\n");
	out.out("   ret\n");
    } else out.out("   err\n");
    
    // FIXME: Could sort ranges, and fail earlier based on sort order, and start by verifying if
    // inside outside range boundaries or similar.

    out.close();
}


void Rule::output(Output & out, const std::string & endlabel)
{
    std::string arg;
    std::string elabel;
    
    if (endlabel != "") elabel = endlabel;
    else if (or_expr) elabel = get_or_name();

    if (set_ != "") {
	std::string tmp = out.get_label();
	output_set(out,tmp,set_);
	arg = "$";
	arg += tmp;
    }
    
    if (op == "!") out.out("   cut \"" + cut + "\"\n");
    else if (op == "/") out.out("   trg #" + trigger +"\n");
    else if (keyword == "EOF") out.out("   eof\n");
    else if (keyword == "ANY") out.out("   eat\n");
    else {
	if (store != "") out.out("   clr\n");
	
//	std::cout << "   output - production = '" << production << "'" << std::endl;
	if (arg == "") {
	    if (production != "") arg = "$" + production;
	    else if (number != "") arg = "#" + number;
	    else if (str != "") {
		if (str != "" && str.size() == 1) arg = arg_from_char(str[0]);
		else arg = "\"" + str + "\"";
	    }
	}
	
	if (arg != "") {
	    // Don't output the first "next" label
	    if (or_expr && endlabel != "") out.out(":" + get_or_name() + "\n");
	    
	    if (op == "*" && !or_expr) {
		out.out("   kln " + arg + "\n");
	    } else if (op == "+" || (op == "*" && or_expr)) {
		if (or_expr) {
		    out.out("   try " + arg + "\n");
		    if (or_expr) out.out("   bne $" + get_or_name() + "\n");
		    else {
			out.out("   beq $" + get_or_name() + "_cont\n");
			out.out("   err\n");
			out.out(":" + get_or_name() + "_cont\n");
		    }
		    out.out("   kln " + arg + "\n");
		    out.out("   jmp $" + elabel + "\n");
		} else {
		    out.out("   req " + arg + "\n");
		    out.out("   kln " + arg + "\n");
		}
	    } else if (op == "?") {
		out.out("   try " + arg + "\n");
	    } else {
		if (or_expr) {
		    out.out("   try " + arg + "\n");
		    out.out("   beq $" + elabel + "\n");
		} else out.out("   req " + arg + "\n");
	    }
	}
	
	if (store != "") out.out("   sto #" + store + "\n");

	if (or_expr) {
	    or_expr->output(out,elabel);
	} else if (elabel != "") {
	    out.out(":" + elabel + "\n");
	}
    }
}

void Rule::output_all(Output & out, const std::string & endlabel)
{
    out.open();
    out.out(":" + name + "\n");
    out.set_prod_name(name);
    output(out);
    boost::shared_ptr<Rule> cur = next;
    while (cur) {
	cur->output(out,endlabel);
	cur = cur->get_next();
    }
    out.out("   ret\n");
    out.close();
}


class BNF {
    VM p;
    
    typedef std::map<std::string, std::string> TriggerMap;
    TriggerMap triggermap;

    Output o;
    
    boost::shared_ptr<Rule> rules;
    std::stack<boost::shared_ptr<Rule> > rulestack;
    
    boost::shared_ptr<Rule> cur_rule;
    std::string prod;
    bool debug;
public:
    BNF(const std::string & parser, bool d = false)
    {
	debug = d;
	
	p.set_trigger(boost::shared_ptr<TriggerBase>(new Trigger<BNF>(this)));
	p.set_brk_trigger(boost::shared_ptr<TriggerBase>(new Trigger<BNF>(this)));
	p.read_memory(parser);
    }
    
    bool put(int ch)
    {
	return p.put(ch);
    }

    void out(const std::string & str)
    {
	o.out(str);
    }
    
    void out(int i)
    {
	o.out(i);
    }

    std::string get_name()
    {
	return o.get_label(prod);
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

	std::string argstr;
	TriggerMap::const_iterator ti;
	std::string ch;
	std::string tmpstr;
	boost::shared_ptr<Rule> tmprule;

	switch (arg) {
	case 1:
	    tmpstr = get_name();
	    cur_rule->set_production(tmpstr);
	    tmprule = cur_rule;
	    cur_rule = boost::shared_ptr<Rule>(new Rule(cur_rule));
	    cur_rule->set_name(tmpstr);
	    rulestack.push(cur_rule);
	    cur_rule->set_parent(tmprule);
	    break;
	    
	case 2:
	    assert(cur_rule);
	    rulestack.top()->output_all(o);
	    cur_rule = cur_rule->get_parent();
	    rulestack.pop();
	    assert(cur_rule);
	    break;

	case 3:
	    assert(cur_rule);
	    cur_rule->set_cut(vm.get_var(6));
	    cur_rule->set_op("!");
	    break;
	    
	case 4:
	    ti = triggermap.find(vm.get_var(2));
	    if (ti == triggermap.end()) throw "Unknown trigger";
	    argstr = ti->second;
	    assert(cur_rule);
	    cur_rule->set_trigger(argstr);
	    cur_rule->set_op("/");
	    break;

	case 6:
	    triggermap.insert(std::make_pair(vm.get_var(2), vm.get_var(3)));
	    break;
	    
	case 7: // start of production
	    prod = vm.get_var(2);
	    rules = boost::shared_ptr<Rule>(new Rule());
	    rules->set_name(prod);
	    cur_rule = rules;
	    assert(cur_rule);
	    break;
	    
	case 8: // end of production
	    rules->output_all(o);
	    rules = cur_rule = boost::shared_ptr<Rule>();
	    break;
	    
	case 13:
	    if (cur_rule) cur_rule = cur_rule->get_parent();
	    break;

	case 14: // STORE
	    assert(cur_rule);
	    cur_rule->set_store(vm.get_var(3)); // FIXME: Hex
	    break;
	case 15:
	    if (vm.get_var(1) != "") cur_rule->set_keyword(vm.get_var(1));
	    if (vm.get_var(2) != "") cur_rule->set_production(vm.get_var(2));
	    if (vm.get_var(3) != "") cur_rule->set_number(vm.get_var(3));
	    if (vm.get_var(4) != "") cur_rule->set_number(vm.get_var(4)); // FIXME: HEX
	    if (vm.get_var(5) != "") cur_rule->set_set(vm.get_var(5));
	    if (vm.get_var(6) != "") cur_rule->set_string(vm.get_var(6));
	    if (vm.get_var(7) != "") cur_rule->set_op(vm.get_var(7));
	    break;
	case 16:
	    assert(cur_rule);
	    cur_rule->set_or_name(o.get_label(prod));
	    tmprule = cur_rule;
	    cur_rule = boost::shared_ptr<Rule>(new Rule());
	    cur_rule->set_parent(tmprule);
	    tmprule->set_or_expr(cur_rule);
	    break;
	case 17: // start of rule
	    assert(cur_rule);
	    tmprule = cur_rule;
	    cur_rule  = boost::shared_ptr<Rule>(new Rule());
	    tmprule->set_next(cur_rule);
	    cur_rule->set_parent(tmprule->get_parent());
	    break;

	}

	if (debug) {
	    std::string msg;
	    msg += "TRIGGER at ";
	    msg += boost::lexical_cast<std::string>(vm.get_lineno());
	    msg += ",";
	    msg += boost::lexical_cast<std::string>(vm.get_column());
	    msg += ":";
	    msg += boost::lexical_cast<std::string>(arg);
	    std::cout << msg << "\n"; //std::cout << msg << std::endl;
	    int i = 1;
	    while (i < 10) {
		std::string tmp = vm.get_var(i);
		if (tmp != "") std::cout << "   " << i << " -> " << tmp << std::endl;
		++i;
	    }
	    std::cout << std::flush;
	}
	
	return true;
    }
    
};

int main(int argc, const char ** argv)
{
    BNF a(argv[1],argc > 2 && argv[2] == std::string("debug"));
    
    try {
	while (!std::cin.eof() && a.put(std::cin.get()));
    } catch(const char * str) {
	std::cerr << "EXCEPTION: " << str << std::endl;
    } catch(const std::string & str) {
	std::cerr << "EXCEPTION: " << str << std::endl;
    } catch(const std::exception & e)
    {
	std::cerr << "EXCEPTION: " << e.what() << std::endl;
    }
}
