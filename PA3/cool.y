/* Declare types for the grammar's non-terminals. */
    %type <program> program
    %type <classes> class_list
    %type <class_> class
    
    /* You will want to change the following line. */
    %type <features> feature_list
    %type <feature> feature
    %type <formals> formal_list
    %type <formal> formal
    %type <expressions> expr_list
    %type <expressions> expr_list_
    %type <expression> expr
    %type <expression> let_
    %type <cases> case_list
    %type <case_> case
    
    /* Precedence declarations go here. */
    %right ASSIGN
    %left NOT
    %nonassoc LE '<' '='
    %left '+' '-'
    %left '*' '/'
    %left ISVOID
    %left '~'
    %left '@'
    %left '.'
    
    %%
    /* 
    Save the root of the abstract syntax tree in a global variable.
    */
    program	
      : class_list	
      { @$ = @1; 
        ast_root = program($1); };
    
    class_list
      : class			/* single class */
      { $$ = single_Classes($1);
        parse_results = $$; }
    | class_list class	/* several classes */
      { $$ = append_Classes($1,single_Classes($2)); 
        parse_results = $$; };
    
    /* If no parent is specified, the class inherits from the Object class. */
    class	
      : CLASS TYPEID '{' feature_list '}' ';'
      { $$ = class_($2,idtable.add_string("Object"),$4,
      stringtable.add_string(curr_filename)); }
    | CLASS TYPEID INHERITS TYPEID '{' feature_list '}' ';'
      { $$ = class_($2,$4,$6,stringtable.add_string(curr_filename)); }
    | CLASS error ';' class
      {$$ = $4;}
    
    /* Feature list may be empty, but no empty features in list. */
    feature_list
      :		/* empty */
      {$$ = nil_Features(); }
    | feature_list feature
      {$$ = append_Features($1, single_Features($2)); };
    | feature_list error ';'
      {$$ = $1;}

    feature
      : OBJECTID '(' ')' ':' TYPEID '{' expr '}' ';'
      {$$ = method($1, nil_Formals(), $5, $7);}
    | OBJECTID '(' formal_list ')' ':' TYPEID '{' expr '}' ';'
      {$$ = method($1, $3, $6, $8);}
    | OBJECTID  ':' TYPEID
      {$$ = attr($1, $3, no_expr());}
    | OBJECTID  ':' TYPEID ASSIGN expr ';'
      {$$ = attr($1, $3, $5);};
    
    formal_list
      : formal
      {$$ = single_Formals($1);}
    | formal_list ',' formal
      {$$ = append_Formals($1, single_Formals($3));};
    
    formal
      : OBJECTID ':' TYPEID
      {$$ = formal($1, $3);};
    
    expr_list
      : expr
      {$$ = single_Expressions($1);}
    | expr_list ',' expr
      {$$ = append_Expressions($1, single_Expressions($3));};

    expr_list_
      : expr ';'
      {$$ = single_Expressions($1);}
    | expr_list_ expr ';' 
      {$$ = append_Expressions($1, single_Expressions($2));}
    | expr_list error ';'
      {$$ = $1;};
    
    let_
      : OBJECTID ':' TYPEID IN expr
      {$$ = let($1, $3, no_expr(), $5);}
    | OBJECTID ':' TYPEID ASSIGN expr IN expr
      {$$ = let($1, $3, $5, $7);}
    | OBJECTID ':' TYPEID ',' let_
      {$$ = let($1, $3, no_expr(), $5);}
    | OBJECTID ':' TYPEID ASSIGN expr ',' let_
      {$$ = let($1, $3, $5, $7);};
    | error ',' let_
      {$$ = $3;}

    case
      : OBJECTID ':' TYPEID DARROW expr ';'
      {$$ = branch($1, $3, $5);};
    
    case_list
      : case
      {$$ = single_Cases($1);}
    | case_list case
      {$$ = append_Cases($1, single_Cases($2));};

    expr
      : OBJECTID ASSIGN expr
      {$$ = assign($1, $3);}
    | expr '.' OBJECTID '(' ')'
      {$$ = dispatch($1, $3, nil_Expressions());}
    | expr '.' OBJECTID '(' expr_list ')'
      {$$ = dispatch($1, $3, $5);}
    | expr '@' TYPEID '.' OBJECTID '(' ')'
      {$$ = static_dispatch($1, $3, $5, nil_Expressions());}
    | expr '@' TYPEID '.' OBJECTID '(' expr_list ')'
      {$$ = static_dispatch($1, $3, $5, $7);}
    | OBJECTID '(' ')'
      {$$ = dispatch(object(idtable.add_string("self")), $1, nil_Expressions());}
    | OBJECTID '(' expr_list ')'
      {$$ = dispatch(object(idtable.add_string("self")), $1, $3);}
    | IF expr THEN expr ELSE expr FI
      {$$ = cond($2, $4, $6);}
    | WHILE expr LOOP expr POOL
      {$$ = loop($2, $4);}
    | '{' expr_list_ '}'
      {$$ = block($2);}
    | LET let_
      {$$ = $2;}
    | CASE expr OF case_list ESAC
      {$$ = typcase($2, $4);}
    | NEW TYPEID
      {$$ = new_($2);}
    | ISVOID expr
      {$$ = isvoid($2);}
    | expr '+' expr
      {$$ = plus($1, $3);}
    | expr '-' expr
      {$$ = sub($1, $3);}
    | expr '*' expr
      {$$ = mul($1, $3);}
    | expr '/' expr
      {$$ = divide($1, $3);}
    | '~' expr
      {$$ = neg($2);}
    | expr '<' expr
      {$$ = lt($1, $3);}
    | expr LE expr
      {$$ = leq($1, $3);}
    | expr '=' expr
      {$$ = eq($1, $3);}
    | NOT expr
      {$$ = comp($2);}
    | '(' expr ')'
      {$$ = $2;}
    | OBJECTID
      {$$ = object($1);}
    | INT_CONST
      {$$ = int_const($1);}
    | STR_CONST
      {$$ = string_const($1);}
    | BOOL_CONST
      {$$ = bool_const($1);};
