/*Working on union of types for simplecase*/

#include <stdarg.h>
#include "semant.h"
#include "utilities.h"
#include <map>

extern int semant_debug;
extern char *curr_filename;

typedef SymbolTable <Symbol, Symbol>* SymTab;
typedef std::map<Symbol, Class_> MAP;

//////////////////////////////////////////////////////////////////////
//
// Symbols
//
// For convenience, a large number of symbols are predefined here.
// These symbols include the primitive type and method names, as well
// as fixed names used by the runtime system.
//
//////////////////////////////////////////////////////////////////////
static Symbol
    arg,
    arg2,
    Bool,
    concat,
    cool_abort,
    copy,
    Int,
    in_int,
    in_string,
    IO,
    length,
    Main,
    main_meth,
    No_class,
    No_type,
    Object,
    out_int,
    out_string,
    prim_slot,
    self,
    SELF_TYPE,
    Str,
    str_field,
    substr,
    type_name,
    val
;

//
// Initializing the predefined symbols.
//
static void initialize_constants(void)
{
    arg         = idtable.add_string("arg");
    arg2        = idtable.add_string("arg2");
    Bool        = idtable.add_string("Bool");
    concat      = idtable.add_string("concat");
    cool_abort  = idtable.add_string("abort");
    copy        = idtable.add_string("copy");
    Int         = idtable.add_string("Int");
    in_int      = idtable.add_string("in_int");
    in_string   = idtable.add_string("in_string");
    IO          = idtable.add_string("IO");
    length      = idtable.add_string("length");
    Main        = idtable.add_string("Main");
    main_meth   = idtable.add_string("main");
    //   _no_class is a symbol that can't be the name of any 
    //   user-defined class.
    No_class    = idtable.add_string("_no_class");
    No_type     = idtable.add_string("_no_type");
    Object      = idtable.add_string("Object");
    out_int     = idtable.add_string("out_int");
    out_string  = idtable.add_string("out_string");
    prim_slot   = idtable.add_string("_prim_slot");
    self        = idtable.add_string("self");
    SELF_TYPE   = idtable.add_string("SELF_TYPE");
    Str         = idtable.add_string("String");
    str_field   = idtable.add_string("_str_field");
    substr      = idtable.add_string("substr");
    type_name   = idtable.add_string("type_name");
    val         = idtable.add_string("_val");
}

std::map<Symbol, Class_> classMap;
Class_ Object_class;
ClassTable::ClassTable(Classes classes) : semant_errors(0) , error_stream(cerr) {

    install_basic_classes();
    int cMain = 0;  /* Check if main class exists */
    for(int i = classes->first(); classes->more(i); i = classes->next(i))
	{
		if(classes->nth(i)->getName() == Main) cMain = 1;
	}    
	if(!cMain)	semant_error()<<"Class Main is not defined.\n";

    /* Traverse all the classes filling in the table that maps class names to class declarations.*/
    for(int i = classes->first(); classes->more(i); i = classes->next(i))
    {
        Symbol s = classes->nth(i)->getName();
        Symbol p = classes->nth(i)->getParent();
        if(s == Object)
        {
            semant_error(classes->nth(i))<<"Object can't be redefined\n";
        }
        else if(s == SELF_TYPE)
        {
            semant_error(classes->nth(i))<<"Class name can not be SELF TYPE.\n";
        }
        else if(classMap.find(s)->first==s)
        {
            semant_error(classes->nth(i))<<"Class "<<s<<" is redefined\n"; 
        }
        else if(p == Bool ||  p == Str || p == Int || p == SELF_TYPE)
        {
            semant_error(classes->nth(i))<<"Can't inherit from "<<p<<endl;
        }
        else
            classMap[s] = classes->nth(i);
    }

    /* Checking the inherit graph if there's undefined class or inherit cycle */
    int flag=0; //if there's undefined class
    // Check undefined classes in the classMap
    for(int i = classes->first(); classes->more(i); i = classes->next(i))
    {
        if(classMap.count(classes->nth(i)->getParent())==0)
        {
            semant_error(classes->nth(i))<<"Class "<<classes->nth(i)->getParent()<<" is not defined\n";
            flag=0;
        }
    }
    // Check inherit cycle
    std::map<Symbol, Symbol> cycle;
    for(int i = classes->first(); classes->more(i) && flag; i = classes->next(i))
    {
        if(classes->nth(i)->getParent() == Object)
        {
            cycle.clear();
        }
        else
        {
            Class_ x = classes->nth(i);
            while(1)
            {
                Symbol s = x->getParent();
                if( !classMap.empty() && (cycle.find(s)->first == s) && (x->getName() == cycle[cycle.find(s)->first])) 
                {
                    semant_error(x)<<"Error\n"; break;
                }
                if( x->getParent() == Object ) {break;}
                cycle[x->getParent()] = x->getName();
                x = classMap.find(x->getParent())->second;
            }
        } 
    } 
}

void ClassTable::install_basic_classes() {

    // The tree package uses these globals to annotate the classes built below.
    // curr_lineno  = 0;
    Symbol filename = stringtable.add_string("<basic class>");
    
    // The following demonstrates how to create dummy parse trees to
    // refer to basic Cool classes.  There's no need for method
    // bodies -- these are already built into the runtime system.
    
    // IMPORTANT: The results of the following expressions are
    // stored in local variables.  You will want to do something
    // with those variables at the end of this method to make this
    // code meaningful.

    // 
    // The Object class has no parent class. Its methods are
    //        abort() : Object    aborts the program
    //        type_name() : Str   returns a string representation of class name
    //        copy() : SELF_TYPE  returns a copy of the object
    //
    // There is no need for method bodies in the basic classes---these
    // are already built in to the runtime system.

    Object_class =
	class_(Object, 
	       No_class,
	       append_Features(
			       append_Features(
					       single_Features(method(cool_abort, nil_Formals(), Object, no_expr())),
					       single_Features(method(type_name, nil_Formals(), Str, no_expr()))),
			       single_Features(method(copy, nil_Formals(), SELF_TYPE, no_expr()))),
	       filename);

    // 
    // The IO class inherits from Object. Its methods are
    //        out_string(Str) : SELF_TYPE       writes a string to the output
    //        out_int(Int) : SELF_TYPE            "    an int    "  "     "
    //        in_string() : Str                 reads a string from the input
    //        in_int() : Int                      "   an int     "  "     "
    //
    Class_ IO_class = 
	class_(IO, 
	       Object,
	       append_Features(
			       append_Features(
					       append_Features(
							       single_Features(method(out_string, single_Formals(formal(arg, Str)),
										      SELF_TYPE, no_expr())),
							       single_Features(method(out_int, single_Formals(formal(arg, Int)),
										      SELF_TYPE, no_expr()))),
					       single_Features(method(in_string, nil_Formals(), Str, no_expr()))),
			       single_Features(method(in_int, nil_Formals(), Int, no_expr()))),
	       filename);  

    //
    // The Int class has no methods and only a single attribute, the
    // "val" for the integer. 
    //
    Class_ Int_class =
	class_(Int, 
	       Object,
	       single_Features(attr(val, prim_slot, no_expr())),
	       filename);

    //
    // Bool also has only the "val" slot.
    //
    Class_ Bool_class =
	class_(Bool, Object, single_Features(attr(val, prim_slot, no_expr())),filename);

    //
    // The class Str has a number of slots and operations:
    //       val                                  the length of the string
    //       str_field                            the string itself
    //       length() : Int                       returns length of the string
    //       concat(arg: Str) : Str               performs string concatenation
    //       substr(arg: Int, arg2: Int): Str     substring selection
    //       
    Class_ Str_class =
	class_(Str, 
	       Object,
	       append_Features(
			       append_Features(
					       append_Features(
							       append_Features(
									       single_Features(attr(val, Int, no_expr())),
									       single_Features(attr(str_field, prim_slot, no_expr()))),
							       single_Features(method(length, nil_Formals(), Int, no_expr()))),
					       single_Features(method(concat, 
								      single_Formals(formal(arg, Str)),
								      Str, 
								      no_expr()))),
			       single_Features(method(substr, 
						      append_Formals(single_Formals(formal(arg, Int)), 
								     single_Formals(formal(arg2, Int))),
						      Str, 
						      no_expr()))),
	       filename);

    //initial the classMap with defined Class
    classMap[Object] = Object_class;
    classMap[IO]     = IO_class;
    classMap[Int]    = Int_class;
    classMap[Bool]   = Bool_class;
    classMap[Str]    = Str_class;
}

////////////////////////////////////////////////////////////////////
//
// semant_error is an overloaded function for reporting errors
// during semantic analysis.  There are three versions:
//
//    ostream& ClassTable::semant_error()                
//
//    ostream& ClassTable::semant_error(Class_ c)
//       print line number and filename for `c'
//
//    ostream& ClassTable::semant_error(Symbol filename, tree_node *t)  
//       print a line number and filename
//
///////////////////////////////////////////////////////////////////

ostream& ClassTable::semant_error(Class_ c)
{                                                             
    return semant_error(c->get_filename(),c);
}    

ostream& ClassTable::semant_error(Symbol filename, tree_node *t)
{
    error_stream << filename << ":" << t->get_line_number() << ": ";
    return semant_error();
}

ostream& ClassTable::semant_error()                  
{                                                 
    semant_errors++;                            
    return error_stream;
}


SymbolTable <Symbol, Symbol> *attrTable;
SymbolTable <Symbol, Symbol> *methodTable;

void attr_class::store()
{
    attrTable->addid(this->getName(), new Symbol(this->getReturnType()));
    return;
}

void method_class::store()
{
    methodTable->addid(this->getName(), new Symbol(this->getReturnType()));
    return;
}

ClassTable *classtable;
bool lub(Symbol sym1, Symbol sym2);

//type checking
Symbol method_class::typeChecker(SymTab attrTable, MAP classMap, Class_ curClasss) 
{
	Symbol type;
	if(return_type == SELF_TYPE)
	{
		type = curClasss->getName();
	}
	else type = return_type;
	Symbol return_expr = expr->typeChecker(attrTable, classMap, curClasss);
	return type;
}

Symbol int_const_class::typeChecker(SymTab attrTable, MAP classMap, Class_ curClasss) 
{
    type = Int;
    return type;
}

Symbol object_class::typeChecker(SymTab attrTable, MAP classMap, Class_ curClasss) 
{
	if(this->name == self) {type = SELF_TYPE; }
	else
	{
		type = *(attrTable->lookup(name));
		if(type == NULL) type = Object;
	}
    return type;
}

Symbol no_expr_class::typeChecker(SymTab attrTable, MAP classMap, Class_ curClasss) 
{
    type = No_type;
    return type;
}

Symbol isvoid_class::typeChecker(SymTab attrTable, MAP classMap, Class_ curClasss) 
{
    e1->typeChecker(attrTable, classMap, curClasss) ;
    type = Bool;
    return type;
}

Symbol new__class::typeChecker(SymTab attrTable, MAP classMap, Class_ curClasss) 
{
    if(type_name == SELF_TYPE) {type = SELF_TYPE; return type;}
    Symbol s = type_name;
    Symbol *a;
    Symbol q = classMap.find(s)->first;
    if(q != NULL) {
    	type = q;
    }
    else {
		type = Object;
	}
    return type;
}

Symbol string_const_class::typeChecker(SymTab attrTable, MAP classMap, Class_ curClasss) 
{
    type = Str;
    return type;
}

Symbol bool_const_class::typeChecker(SymTab attrTable, MAP classMap, Class_ curClasss) 
{
    type = Bool;
    return type;
}

Symbol comp_class::typeChecker(SymTab attrTable, MAP classMap, Class_ curClasss) 
{
    type = e1->typeChecker(attrTable, classMap, curClasss) ;
    return type;
}

Symbol leq_class::typeChecker(SymTab attrTable, MAP classMap, Class_ curClasss) 
{
    Symbol type1 = e1->typeChecker(attrTable, classMap, curClasss) ;
    Symbol type2 = e2->typeChecker(attrTable, classMap, curClasss) ;
    if(type1 != Int || type2 != Int) 
    {
        classtable->semant_error(curClasss)<<"Both leq expresssions are not Int type.\n";
    }
    else type = Bool;
    return type;
}

Symbol eq_class::typeChecker(SymTab attrTable, MAP classMap, Class_ curClasss) 
{
    Symbol type1 = e1->typeChecker(attrTable, classMap, curClasss) ;
    Symbol type2 = e2->typeChecker(attrTable, classMap, curClasss) ;
	if((type1 == Int || type1 == Str || type1 == Bool) && type1 != type2)
	{
		classtable->semant_error(curClasss)<<"Both eq expressions are not Int type.\n";
	}
    else type = Bool;
    return type;
}

Symbol lt_class::typeChecker(SymTab attrTable, MAP classMap, Class_ curClasss) 
{
    Symbol type1 = e1->typeChecker(attrTable, classMap, curClasss) ;
    Symbol type2 = e2->typeChecker(attrTable, classMap, curClasss) ;
    if(type1 != Int || type2 != Int) 
    {
        classtable->semant_error(curClasss)<<"Both lt expressions are not Int type.\n";
    }
    else type = Bool;
    return type;
}

Symbol neg_class::typeChecker(SymTab attrTable, MAP classMap, Class_ curClasss) 
{
    Symbol type1 = e1->typeChecker(attrTable, classMap, curClasss);
    if(type1 != Int)
    {
        classtable->semant_error(curClasss)<<"Both neg expressions are not Int type.\n";
    }
    type = Int;
    return type;
}

Symbol divide_class::typeChecker(SymTab attrTable, MAP classMap, Class_ curClasss) 
{
    Symbol type1 = e1->typeChecker(attrTable, classMap, curClasss) ;
    Symbol type2 = e2->typeChecker(attrTable, classMap, curClasss) ;
    if(type1 != Int || type2 != Int) 
    {
        classtable->semant_error(curClasss)<<"Both divide expressions are not Int type.\n";
    }
    else type = Int;
    return type;
}

Symbol mul_class::typeChecker(SymTab attrTable, MAP classMap, Class_ curClasss) 
{
    Symbol type1 = e1->typeChecker(attrTable, classMap, curClasss) ;
    Symbol type2 = e2->typeChecker(attrTable, classMap, curClasss) ;
    if(type1 != Int || type2 != Int) 
    {
        classtable->semant_error(curClasss)<<"Both mul expressions are not Int type.\n";
    }
    else type = Int;
    return type;
}

Symbol sub_class::typeChecker(SymTab attrTable, MAP classMap, Class_ curClasss) 
{
    Symbol type1 = e1->typeChecker(attrTable, classMap, curClasss) ;
    Symbol type2 = e2->typeChecker(attrTable, classMap, curClasss) ;
    if(type1 != Int || type2 != Int) 
    {
        classtable->semant_error(curClasss)<<"Both sub expressions are not Int type.\n";
    }
    else type = Int;
    return type;
}

Symbol plus_class::typeChecker(SymTab attrTable, MAP classMap, Class_ curClasss) 
{
    Symbol type1 = e1->typeChecker(attrTable, classMap, curClasss) ;
    Symbol type2 = e2->typeChecker(attrTable, classMap, curClasss) ;
    if(type1 != Int || type2 != Int) 
    {
        classtable->semant_error(curClasss)<<"Both plus expressions are not Int type.\n";
    }
    else type = Int;
    return type;
}

Symbol let_class::typeChecker(SymTab attrTable, MAP classMap, Class_ curClasss) 
{
	if(identifier == self)
	{
		classtable->semant_error(curClasss)<<"No self binding in LET allowed.\n";
	}
	else 
	{
		Symbol type1;
		if(this->init)
		{
			if(type_decl == SELF_TYPE)
				type1 = curClasss->getName();
			else type1 = type_decl;
			Symbol typeExpr = init->typeChecker(attrTable, classMap, curClasss);
			if(!lub(type1, typeExpr))
			{
				classtable->semant_error(curClasss)<<"Wrong let expression.";
			}
			attrTable->enterscope();
			attrTable->addid(identifier, new Symbol(type_decl));
		}
		else 
		{
			if(type_decl == SELF_TYPE)
				type1 = curClasss->getName();
			else type1 = type_decl;
			attrTable->enterscope();
			attrTable->addid(identifier, new Symbol(type_decl));

		}
		type = body->typeChecker(attrTable, classMap, curClasss);
	}
	return type;	
}

Symbol block_class::typeChecker(SymTab attrTable, MAP classMap, Class_ curClasss) 
{
    for(int i = body->first(); body->more(i); i = body->next(i))
    type = body->nth(i)->typeChecker(attrTable, classMap, curClasss) ;
    return type;
}

Symbol typcase_class::typeChecker(SymTab attrTable, MAP classMap, Class_ curClasss) 
{
    expr->typeChecker(attrTable, classMap, curClasss);
    for(int i = cases->first(); cases->more(i); i = cases->next(i))
    {
    	Case c = cases->nth(i);
    	c->typeChecker(attrTable, classMap, curClasss);
    }
    return Int;
}

Symbol branch_class::typeChecker(SymTab attrTable, MAP classMap, Class_ curClasss) 
{
	attrTable->enterscope();
    attrTable->addid(this->name, new Symbol(type_decl));
    Symbol t = this->expr->typeChecker(attrTable, classMap, curClasss);
    attrTable->exitscope();
    return t;
}

Symbol loop_class::typeChecker(SymTab attrTable, MAP classMap, Class_ curClasss) 
{
    type = Int;
    return type;
}

Symbol cond_class::typeChecker(SymTab attrTable, MAP classMap, Class_ curClasss) 
{
    type = Int;
    return type;
}

Symbol dispatch_class::typeChecker(SymTab attrTable, MAP classMap, Class_ curClasss) 
{
    if(name == ::copy || name == out_int || name == out_string)
	{
		type = expr->typeChecker(attrTable, classMap, curClasss);
		for(int i = actual->first(); actual->more(i); i = actual->next(i))
	    {
	    	actual->nth(i)->typeChecker(attrTable, classMap, curClasss); 
	    }
		return type;
	}

    Symbol type1;
    type1 = expr->typeChecker(attrTable, classMap, curClasss); 
    if(type1 == SELF_TYPE) type1 = curClasss->getName();
    int i;
    for(i = actual->first(); actual->more(i); i = actual->next(i))
    {
    	actual->nth(i)->typeChecker(attrTable, classMap, curClasss); 
    }
    Symbol *type_name = methodTable->lookup(this->name);
    if(*type_name == SELF_TYPE) *type_name = type1;
    type = *type_name;
    return type;
}

Symbol static_dispatch_class::typeChecker(SymTab attrTable, MAP classMap, Class_ curClasss) 
{
	Symbol type0 = expr->typeChecker(attrTable, classMap, curClasss); 
	if(!lub(type_name, type0)) 
	{
		classtable->semant_error(curClasss)<<"Not a valid type in static dispacth.\n";
	}
    

    Symbol type1;
    type1 = expr->typeChecker(attrTable, classMap, curClasss); 
    if(type1 == SELF_TYPE) type1 = curClasss->getName();
    int i;
    for(i = actual->first(); actual->more(i); i = actual->next(i))
    {
    	actual->nth(i)->typeChecker(attrTable, classMap, curClasss); 
    }
    Symbol *type_name = methodTable->lookup(this->name);
    if(*type_name == SELF_TYPE) *type_name = type1;
    type = *type_name;
    return type;
    return type;
}

Symbol assign_class::typeChecker(SymTab attrTable, MAP classMap, Class_ curClasss) 
{
    Symbol *varType;
    varType = new Symbol();
    varType = attrTable->lookup(name);
    Symbol exprType = expr->typeChecker(attrTable, classMap, curClasss);
	if(name == self) classtable->semant_error(curClasss)<<"Self type.\n";
	else if(*varType == exprType) 
	{
		type = exprType;
	}
    else if(lub(*varType, exprType))
    {
        type = *varType;
    }
    else 
    {
    	classtable->semant_error(curClasss)<<"Error1.\n";
    }
    return type;
}

Symbol attr_class::typeChecker(SymTab attrTable, MAP classMap, Class_ curClasss)
{
	Symbol initType = init->typeChecker(attrTable, classMap, curClasss);
	if(initType == SELF_TYPE) ;
	else if(initType != No_type && lub(type_decl, initType)) {}
	else if(!classMap.find(initType)->second) classtable->semant_error(curClasss)<<"Undefined.\n";
	return type_decl;
}

void method_class::checkFormals(Formals formals, SymTab attrTable, Class_ curClasss)
{
    attrTable->enterscope();
    for(int i = formals->first(); formals->more(i); i = formals->next(i))
    {
		if(attrTable->lookup(formals->nth(i)->getName()) && formals->nth(i)->getReturnType())
		{
			classtable->semant_error(curClasss)<<"Duplicate formal parameter.\n";
		}
		 if(formals->nth(i)->getName() == self)
		{
			classtable->semant_error(curClasss)<<"Formal parameter can't include self.\n";
		}
		else if(formals->nth(i)->getReturnType() == SELF_TYPE)
		{         
			classtable->semant_error(curClasss)<<"Formal parameter "<<formals->nth(i)->getName()<<" can't be SELF_TYPE.\n";		
		}
		attrTable->addid(formals->nth(i)->getName(), new Symbol(formals->nth(i)->getReturnType()));
    }
}

void TypeCheck(Classes classes)
{
	methodTable->enterscope();
	for(int i = classes->first(); classes->more(i); i = classes->next(i)) {
        int flag=0;
        attrTable->enterscope();
        Class_ curClass = classes->nth(i);
        while(curClass!=Object_class) {
        	Features classFeatures = curClass->getFeatures();
            for(int j = classFeatures->first(); classFeatures->more(j); j = classFeatures->next(j)) {
                if(classFeatures->nth(j)->isMethod()) {
                    Feature curMethod = classFeatures->nth(j);                    
                    curMethod->store();
                }
                else if(!classFeatures->nth(j)->isMethod() && !attrTable->probe(classFeatures->nth(j)->getName())) {
                	Feature curAttribute = classFeatures->nth(j);
                    if(curAttribute->getName() == self) {
                        classtable->semant_error(curClass)<<"Attribute can't be self\n";
                        return;
                    }
                    else {
                        curAttribute->store();
                    }
                }
                else classtable->semant_error(curClass)<<"Error\n";
            }
            if(curClass->getParent()==Object) {
            	flag=1;
            	break;
            }
            curClass = (classMap.find(curClass->getParent()))->second;
        }
        
    }
    methodTable->addid(in_string, new Symbol(Str));
    methodTable->addid(in_int, new Symbol(Int));
    methodTable->addid(length, new Symbol(Int));
    methodTable->addid(cool_abort, new Symbol(Object));
    for(int i = classes->first(); classes->more(i); i = classes->next(i)) {
    	Class_ curClass = classes->nth(i);
    	Features classFeatures = curClass->getFeatures();
    	for(int j = classFeatures->first(); classFeatures->more(j); j = classFeatures->next(j)) {
    		Feature currFeature = classFeatures->nth(j);
    		if(currFeature->isMethod()) {currFeature->checkFormals(currFeature->getFormals(), attrTable, curClass); }
    		currFeature->typeChecker(attrTable,classMap,curClass);
    		if(currFeature->isMethod()){attrTable->exitscope();}
    	}
    }
    attrTable->exitscope();
    methodTable->exitscope();
}

//sym1 is bigger type and sym2 is smaller or equal
bool lub(Symbol sym1, Symbol sym2) {
    if(sym1 == sym2) return true;
    while(sym2 != Object) {
	    if(sym1 == (classMap.find(sym2)->second)->getParent()) {
	        return true;
	    }
	    sym2 = (classMap.find(sym2)->second)->getParent();
	}
    return false;
}

/*   This is the entry point to the semantic checker.

     Your checker should do the following two things:

     1) Check that the program is semantically correct
     2) Decorate the abstract syntax tree with type information
        by setting the `type' field in each Expression node.
        (see `tree.h')

     You are free to first do 1), make sure you catch all semantic
     errors. Part 2) can be done in a second stage, when you want
     to build mycoolc.
 */

void program_class::semant()
{
    initialize_constants();
    classtable = new ClassTable(classes);
    attrTable   = new SymbolTable<Symbol, Symbol>();
    methodTable = new SymbolTable<Symbol, Symbol>();
    if (classtable->errors())
    {
        cerr << "Compilation halted due to static semantic errors." << endl;
        exit(1);
    }
    TypeCheck(classes);
    if (classtable->errors())
    {
    	cerr << "Compilation halted due to static semantic errors." << endl;
    	exit(1);
    }
}