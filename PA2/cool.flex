/*
 *  The scanner definition for COOL.
 */
%option noyywrap

/*
 *  Stuff enclosed in %{ %} in the first section is copied verbatim to the
 *  output, so headers and global definitions are placed here to be visible
 * to the code in the file.  Don't remove anything that was here initially
 */
%{
#include <cool-parse.h>
#include <stringtab.h>
#include <utilities.h>

/* The compiler assumes these identifiers. */
#define yylval cool_yylval
#define yylex  cool_yylex

/* Max size of string constants */
#define MAX_STR_CONST 1025
#define YY_NO_UNPUT   /* keep g++ happy */

extern FILE *fin; /* we read from this file */

/* define YY_INPUT so we read from the FILE fin:
 * This change makes it possible to use this scanner in
 * the Cool compiler.
 */
#undef YY_INPUT
#define YY_INPUT(buf,result,max_size) \
	if ( (result = fread( (char*)buf, sizeof(char), max_size, fin)) < 0) \
		YY_FATAL_ERROR( "read() in flex scanner failed");

char string_buf[MAX_STR_CONST]; /* to assemble string constants */
char *string_buf_ptr;

extern int curr_lineno;
extern int verbose_flag;

extern YYSTYPE cool_yylval;

/*
 *  Add Your own definitions here
 */

static std::string currentStr; /* For determining if a string is too long */

int commentLayer; /* For counting layers of nested comment */

%}

/*
 * Define names for regular expressions here.
 */

INTEGER	[0-9]+
NEWLINE	"\n"
OneLineComm --[^\n]*
TYPEID	[a-z][a-z0-9]*
OBJECTID  [a-z][A-Z0-9]*
WHITESPACE  [\v \f \r \t]
SINGLECHAR [+*/@$\><,.:-]
DARROW	=>
ASSIGN <-
TRUE t(?i:rue)
FALSE f(?i:alse)

%x NCOMMENT
%x STRING

%%

 /*
  * One-line comment
  */

{OneLineComm}.* ;

 /*
  * Nested comments
  */

"(*" {
	BEGIN (NCOMMENT);
}

"*)" {
	cool_yylval.error_msg = "Unmatched *)";
	return (ERROR);
}

<NCOMMENT>"(*" {
  commentLayer++;
}

<NCOMMENT>"*)" {
  if(commentLayer == 1) {
    BEGIN(INITIAL);
  }
  commentLayer--;
}

<NCOMMENT>\n {
	++curr_lineno;
}

<NCOMMENT><<EOF>> {
	BEGIN (INITIAL);
	cool_yylval.error_msg = "EOF in comment.";
	return (ERROR); 
}

<NCOMMENT>. ;

 /*
  *  The multiple-character operators.
  */

{DARROW} {
	return (DARROW);
}

{ASSIGN} {
  return (ASSIGN);
}

 /*
  *  The single-character operators.
  */
 
{SINGLECHAR} {
	return (yytext[0]);
}

 /*
  *  The integers.
  */

{INTEGER} {
    cool_yylval.symbol = inttable.add_string(yytext);
    return INT_CONST;
} 

 /*
  *  The type identifiers.
  */

{TYPEID} {
    cool_yylval.symbol = idtable.add_string(yytext);
    return TYPEID;
}

 /*
  *  The object identifiers.
  */

{OBJECTID} {
    cool_yylval.symbol = idtable.add_string(yytext);
    return OBJECTID;
}

 /*
  *  New line
  */

{NEWLINE} {
    curr_lineno++;
}

 /*
  *  White space
  */

{WHITESPACE} ;


 /*
  * Keywords are case-insensitive except for the values true and false,
  * which must begin with a lower-case letter.
  */

(?i:class) return CLASS;
(?i:else) return ELSE;
(?i:fi) return FI;
(?i:if) return IF;
(?i:in) return IN;
(?i:isvoid) return ISVOID;
(?i:inherits) return INHERITS;
(?i:let) return LET;
(?i:loop) return LOOP;
(?i:pool) return POOL;
(?i:then) return THEN;
(?i:while) return WHILE;
(?i:case) return CASE;
(?i:esac) return ESAC;
(?i:new) return NEW;
(?i:of) return OF;
(?i:not) return NOT;

 /*
  *  True condition
  */

{TRUE} {
  cool_yylval.boolean = 1;
  return BOOL_CONST;
}

 /*
  *  False condition
  */

{FALSE} {
  cool_yylval.boolean = 0;
  return BOOL_CONST;
}

 /*
  *  String constants (C syntax)
  *  Escape sequence \c is accepted for all characters c. Except for 
  *  \n \t \b \f, the result is c.
  *
  */

\" {
  currentStr = "";
	BEGIN(STRING);
}

<STRING>\" {
	BEGIN(INITIAL);
  if(currentStr.size() >= MAX_STR_CONST) {
    cool_yylval.error.msg = "String constant too long";
    return (ERROR);
  }
}

<STRING>\n {
	++curr_lineno;
  cool_yylval.error_msg = "Unterminated string constant.";
	return (ERROR);
}

<STRING>\0 {
	BEGIN(INITIAL);
	cool_yylval.error_msg = "Null character in string.";
	return(ERROR);
}

<STRING><<EOF>> {
	BEGIN(INITIAL);
	cool_yylval.error_msg = "EOF in string constant.";
	return (ERROR); 
}

<STRING>\\n {
  currentStr++ = '\n';
}

<STRING>\\t {
	currentStr++ = '\t';
}

<STRING>\\r {
	currentStr++ = '\r';
}

<STRING>\\b {
	currentStr++ = '\b';
}

<STRING>\\f {
	currentStr++ = '\f';
}

<STRING>\\(.|\n) {
  currentStr++ = yytext[1];
}

<STRING>. {
  currentStr++ = yytext;  
}

. {
  cool_yylval.error_msg = yytext;
  return (ERROR);
}

%%