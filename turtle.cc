
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

class RDFModel {
    typedef std::multimap<std::string, std::pair<std::string, std::string> > TripleMap;
    typedef std::map<std::string, std::string> TripleIndex;
    
    TripleMap triples;
    TripleIndex verbIndex;
    TripleIndex objectIndex;
    int genid;

    bool debug;
public:
    RDFModel(bool d) : debug(d)
    {
	genid = 0;
    }

    std::string gen_id()
    {
	int id = ++genid;
	return "_:genid" + boost::lexical_cast<std::string>(genid);
    }
    
    std::string add(const std::string & subject, const std::string & verb, const std::string & object)
    {
	std::string mysubject = subject;
	if (mysubject == "") mysubject = gen_id();
	std::string myobject = object;
	if (myobject == "") myobject = gen_id();
	
	if (debug) std::cout << "Adding triple: " << mysubject << " " << verb << " " << myobject << std::endl;
	// FIXME: Prevent duplicate triples from being added
	triples.insert(std::make_pair(mysubject, std::make_pair(verb, myobject)));
	verbIndex.insert(std::make_pair(verb,mysubject));
	objectIndex.insert(std::make_pair(myobject,mysubject));
	return mysubject;
    }
    
    void dump() {
	std::cout << "DUMP -----------------------" << std::endl;
	TripleMap::const_iterator i = triples.begin();
	while (i != triples.end()) {
	    std::cout << i->first << " " << i->second.first << " " << i->second.second << " ." << std::endl;
	    ++i;
	}
    }
};


class Turtle {
    VM p;
    RDFModel model;
    
    struct StackFrame {
	std::string curSubject;
	std::string curVerb;
	std::string curObject;
    };
    
    std::stack<StackFrame> s;

    typedef std::map<std::string, std::string> PrefixMap;
    PrefixMap prefixes;
    std::string tmp;
    std::string coll_start;
    std::string coll_end;
    bool error;
    
    bool debug;
public:
    Turtle(const std::string & parser, bool d = false) : model(d),debug(d)
    {
	p.set_trigger(boost::shared_ptr<TriggerBase>(new Trigger<Turtle>(this)));
	p.set_brk_trigger(boost::shared_ptr<TriggerBase>(new Trigger<Turtle>(this)));
	p.read_memory(parser);
	p.set_debug(debug);
	s.push(StackFrame());
	error = false;
    }
    
    bool put(int ch)
    {
	return p.put(ch);
    }

    void dump()
    {
	error = !p.get_s();
	if (!error) model.dump();
	else std::cout << "ERROR - Triples will be discarded" << std::endl;
    }

    void add_prefix(const std::string & prefix, std::string uri)
    {
	uri = uri.substr(1,uri.size()-2);
	prefixes.insert(std::make_pair(prefix,uri));
    }
    
    std::string get_uri(VM & vm)
    {
	if (vm.get_var(2) != "") return vm.get_var(2);
	if (vm.get_var(3) == "a") return "<http://www.w3.org/1999/02/22-rdf-syntax-ns#type>";
	if (vm.get_var(4) != "") return "\"" + boost::lexical_cast<std::string>(boost::lexical_cast<unsigned int>(vm.get_var(4))) + "\"^^<http://www.w3.org/2001/XMLSchema#integer>";
	if (vm.get_var(5) != "") return "\"" + vm.get_var(5) + "\"";
	if (vm.get_var(7) != "") {
	    if (vm.get_var(1) == "_") return "_:" + vm.get_var(7);
	    PrefixMap::const_iterator i = prefixes.find(vm.get_var(1));
	    if (i == prefixes.end()) return vm.get_var(7);
	    return "<" + i->second + vm.get_var(7) + ">";
	}
	return "";
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

	if (debug) {
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
	}

	switch(arg) {
	case 1:
	    add_prefix(vm.get_var(1),vm.get_var(2));
	    break;
	case 3:
	    if (tmp == "") s.top().curSubject = get_uri(vm);
	    else {
		s.top().curSubject = tmp;
		tmp = "";
	    }
	    break;
	case 4:
	    s.top().curVerb = get_uri(vm);
	    break;
	case 5:
	    if (tmp == "") s.top().curObject = get_uri(vm);
	    else {
		s.top().curObject = tmp;
		tmp = "";
	    }
	    s.top().curSubject = model.add(s.top().curSubject,s.top().curVerb,s.top().curObject);
	    break;
	case 6:
	    tmp = "";
	    s.push(StackFrame());
	    break;
	case 7:
	    tmp = s.top().curSubject;
	    std::cout << "SET TMP TO '" << tmp << "'" << std::endl;
	    s.pop();
	    break;
	case 8:
	    // Push stack frame AND set subject/verb
	    s.push(StackFrame());
	    s.top().curSubject = model.gen_id();
	    s.top().curVerb = "<http://www.w3.org/1999/02/22-rdf-syntax-ns#first>";
	    coll_end = s.top().curSubject;

	    if (tmp != "") {
		model.add(tmp,"<http://www.w3.org/1999/02/22-rdf-syntax-ns#rest>", s.top().curSubject);
		tmp = "";
	    } 
	    
	    if (coll_start == "") coll_start = s.top().curSubject;
	    break;
	case 9:
	    tmp = s.top().curSubject;
	    s.pop();
	    break;
	case 10:
	    // Close collection.
	    if (coll_end != "") {
		model.add(coll_end,"<http://www.w3.org/1999/02/22-rdf-syntax-ns#rest>",
			  "<http://www.w3.org/1999/02/22-rdf-syntax-ns#nil>");
	    }
	    if (coll_start != "") tmp = coll_start;
	    else tmp = "<http://www.w3.org/1999/02/22-rdf-syntax-ns#nil>";
	    coll_start = "";
	    coll_end = "";
	    break;
	}
	return true;
    }
    
};

int main(int argc, const char ** argv)
{
    Turtle a(argv[1], argc > 2 && argv[2] == std::string("debug"));
    
    try {
	while (!std::cin.eof() && a.put(std::cin.get()));
	a.dump();
    } catch(const char * str) {
	std::cout << "EXCEPTION: " << str << std::endl;
    } catch(const std::string & str) {
	std::cout << "EXCEPTION: " << str << std::endl;
    }
}
