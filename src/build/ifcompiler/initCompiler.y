/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* fips730 src/engd/initfiles/ifcompiler/initCompiler.y 1.1               */
/*                                                                        */
/* IBM CONFIDENTIAL                                                       */
/*                                                                        */
/* OBJECT CODE ONLY SOURCE MATERIALS                                      */
/*                                                                        */
/* COPYRIGHT International Business Machines Corp. 2010                   */
/* All Rights Reserved                                                    */
/*                                                                        */
/* The source code for this program is not published or otherwise         */
/* divested of its trade secrets, irrespective of what has been           */
/* deposited with the U.S. Copyright Office.                              */
/*                                                                        */
/* IBM_PROLOG_END_TAG                                                     */
// Change Log *************************************************************************************
//                                                                      
//  Flag Track    Userid   Date       Description                
//  ---- -------- -------- --------   -------------------------------------------------------------
//        D754106  dgilbert 06/14/10  Create
//        D774126  dgilbert 09/30/10  Add ERROR: to yyerror message
//                 andrewg  09/19/11  Updates based on review
// End Change Log *********************************************************************************
/**
 * @file initCompiler.y
 * @brief Contains the yacc/bison code for parsing an initfile.
 * 
 * This code runs as part of the build process to generate a
 * byte-coded representation of an initfile
 */
%{
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <initCompiler.H>
#include <initSymbols.H>


init::Scom * current_scom = NULL;

extern int yylex();
void yyerror(const char * s);

int scom;

%}

/* Union for the yylval variable in lex or $$ variables in bsion code.
 * Used to store the data associated with a parsed token.
 */
%union{
    uint32_t integer;
    uint64_t uint64;
    std::string * str_ptr;
    init::Rpn * rpn_ptr;
}

    /* indicates the name for the start symbol */
%start input

    /* Define terminal symbols and the union type
     * associated with each. */

%token <integer> INIT_INTEGER
%token <uint64>  INIT_INT64
%token <str_ptr>  INIT_INT64_STR
%token <str_ptr>  INIT_SCOM_ADDR
%token <str_ptr>  INIT_SCOM_SUFFIX
%token <uint64>  INIT_SCOM_DATA
%token <str_ptr> INIT_ID
%token <str_ptr> INIT_VERSIONS
%token <str_ptr> ATTRIBUTE_INDEX


    /* Define terminal symbols that don't have any associated data */

%token INIT_VERSION
%token INIT_ENDINITFILE
%token INIT_BITS
%token INIT_EXPR
%token INIT_TARG
%token INIT_DEFINE
%token INIT_EQ
%token INIT_NE
%token INIT_LE
%token INIT_GE
%token INIT_SCANINIT
%token INIT_SCOMINIT
%token INIT_SCOM
%token INIT_SCOMD


    /* non-terminal tokens and the union data-type associated with them */

%type <str_ptr> bitsrows
%type <rpn_ptr> expr id_col num_list





/* top is lowest precedent - done last */
%left INIT_LOGIC_OR
%left INIT_LOGIC_AND
%left '|'     /* bitwise OR */
%left '^'     /* bitwise XOR */
%left '&'     /* bitwise AND */
%left INIT_EQ INIT_NE
%left INIT_LE INIT_GE '<' '>'
%left INIT_SHIFT_RIGHT INIT_SHIFT_LEFT
%left '-' '+'
%left '*' '/' '%'
%right '!' '~'   /* logic negation  bitwise complement*/
/* bottom is highest precedent - done first */




%%
/* Grammars */
    /* the 'input' is simply all the lines */
input:
        | input line
;

line:   scom
        | cvs_versions
        | syntax_version
        | define 
        | INIT_ENDINITFILE   { yyscomlist->clear_defines(); }
;


cvs_versions:   INIT_VERSIONS
            {
                yyscomlist->set_cvs_versions($1); delete $1;
            }
;

syntax_version: INIT_VERSION '=' INIT_INTEGER
            { 
                yyscomlist->set_syntax_version($3);
            }
;

scom:       INIT_SCOM {current_scom = new init::Scom(yyscomlist->get_symbols(),yyline);}
            | scom scomaddr '{' scombody '}'
		{
                     /* printf("Found an INIT_SCOM!\n"); */
                    /* current_scom = new init::Scom(yyscomlist->get_symbols(),yyline); */
                }
;	

scomaddr: 
      | INIT_SCOM_ADDR {
		           /*printf("Found an INIT_SCOM_ADDR 0x%X!\n",$1);*/
                           current_scom->set_scom_address(*($1)); delete $1;
                           yyscomlist->insert(current_scom);
                       }
      | scomaddr '(' scom_list ')' { } 
      | scomaddr '(' scom_list ')' INIT_SCOM_SUFFIX  { current_scom->set_scom_suffix(*($5)); delete $5; } 
;


scom_list:       INIT_INT64_STR                { current_scom->dup_scom_address(*($1));delete $1;}
                | scom_list ',' INIT_INT64_STR  { current_scom->dup_scom_address(*($3));delete $3;}
;


    /* The scombody was reformatted by the scanner
     * colname1 , row1 , row 2, ... , row n ;
     * colname2 , row1 , row 2, ... , row n ;
     */

scombody:        scombodyline ';' {}
                | scombody scombodyline ';' {}
;

scombodyline:   INIT_SCOMD  ',' scomdrows  {}
                | INIT_BITS ',' bitsrows  {}
                | INIT_EXPR ',' exprrows  { init::dbg << "Add col EXPR" << endl; current_scom->add_col("EXPR"); }
                | INIT_ID   ',' idrows  {   
                                            current_scom->add_col(*($1));
                                            init::dbg << "Add col " << *($1) << endl;
                                            delete $1;
                                        }

;


scomdrows:      expr  {
                          /*printf("scomdrows - RPN Address:0x%X\n",$1);*/
                          init::dbg << $1->listing("Length scom RPN");
                          current_scom->add_scom_rpn($1);
                       }
                | scomdrows ',' expr {  init::dbg << $3->listing("Length scom RPN"); current_scom->add_scom_rpn($3); }
;
    

/*
scomdrows:      id_col { printf("scomdrows\n"); }
;
*/

bitsrows:       bitrange {}
                | bitsrows ',' bitrange {}
;

bitrange:       INIT_INTEGER { current_scom->add_bit_range($1,$1);  }
                | INIT_INTEGER ':' INIT_INTEGER 
                    { current_scom->add_bit_range($1,$3); }
;

exprrows:       expr { init::dbg << $1->listing(NULL); current_scom->add_row_rpn($1); }
                | exprrows ',' expr
                        { init::dbg << $3->listing(NULL); current_scom->add_row_rpn($3); }
;

idrows:         id_col  { init::dbg << $1->listing(NULL); current_scom->add_row_rpn($1); }
                | idrows ',' id_col { init::dbg << $3->listing(NULL); current_scom->add_row_rpn($3); }
;


        // TODO num_list could be VARs,LITs, or even ranges eg {1,2..5,7}

id_col:         INIT_ID { $$ = new init::Rpn(*($1),yyscomlist->get_symbols()); $$->push_op(init::Rpn::EQ); delete $1; }  
                | INIT_INTEGER { $$ = new init::Rpn($1,yyscomlist->get_symbols()); $$->push_op(init::Rpn::EQ); }
                | '{' num_list '}' { $$ = $2; $2->push_op(init::Rpn::LIST); $2->push_op(init::Rpn::EQ); }
;



num_list:       INIT_INTEGER { $$ = new init::Rpn($1,yyscomlist->get_symbols()); }
                | INIT_ID    { $$ = new init::Rpn(*($1),yyscomlist->get_symbols()); }
                | num_list ',' INIT_INTEGER { $$ = $1; $1->merge(new init::Rpn($3,yyscomlist->get_symbols())); }
                | num_list ',' INIT_ID { $$ = $1; $1->merge(new init::Rpn(*($3),yyscomlist->get_symbols())); }
;


define: INIT_DEFINE INIT_ID '=' expr  ';'
            {
                init::dbg << $2 << ':' << endl << $4->listing("Length of rpn for Define");
                yyscomlist->add_define($2,$4);
                delete $2;
            }
;

 /* expr should return an RPN string of some kind */
expr:   INIT_INTEGER                    { $$= new init::Rpn($1,yyscomlist->get_symbols()); }
        | INIT_ID                       { $$= new init::Rpn(*($1),yyscomlist->get_symbols()); delete $1; }
        | INIT_INT64                    { $$=new init::Rpn($1,yyscomlist->get_symbols()); }
        | expr INIT_LOGIC_OR expr       { $$ = $1->push_merge($3,init::Rpn::OR); }
        | expr INIT_LOGIC_AND expr      { $$ = $1->push_merge($3,init::Rpn::AND); }
        | expr ATTRIBUTE_INDEX          { $1->push_array_index(*($2));}
        | expr INIT_EQ expr             { $$ = $1->push_merge($3,init::Rpn::EQ); }
        | expr INIT_NE expr             { $$ = $1->push_merge($3,init::Rpn::NE); }
        | expr INIT_LE expr             { $$ = $1->push_merge($3,init::Rpn::LE); }
        | expr INIT_GE expr             { $$ = $1->push_merge($3,init::Rpn::GE); }
        | expr '<' expr                 { $$ = $1->push_merge($3,init::Rpn::LT); }
        | expr '>' expr                 { $$ = $1->push_merge($3,init::Rpn::GT); }
        | expr INIT_SHIFT_RIGHT expr    { $$ = $1->push_merge($3,init::Rpn::SHIFTRIGHT); }
        | expr INIT_SHIFT_LEFT expr     { $$ = $1->push_merge($3,init::Rpn::SHIFTLEFT); }
        | expr '+' expr                 { $$ = $1->push_merge($3,init::Rpn::PLUS); }
        | expr '-' expr                 { $$ = $1->push_merge($3,init::Rpn::MINUS); }
        | expr '*' expr                 { $$ = $1->push_merge($3,init::Rpn::MULT); }
        | expr '/' expr                 { $$ = $1->push_merge($3,init::Rpn::DIVIDE); }
        | expr '%' expr                 { $$ = $1->push_merge($3,init::Rpn::MOD); }
        | '!' expr                      { $$ = $2->push_op(init::Rpn::NOT); }
        /* | '~' expr                      { $$ = $2->push_op("~"); } */
        | '(' expr ')'                  { $$ = $2; }
;


%%

void yyerror(const char * s)
{
    init::erros << setfill('-') << setw(80) << '-' << endl;
    init::erros << setfill('0');
    init::erros << "Parse Error line " << dec << setw(4) << yyline << ": yychar = " 
                << dec << (uint32_t) yychar << " [0x" << hex << (uint32_t) yychar << "] '";
    if(isprint(yychar)) init::erros << (char)yychar;
    else  init::erros << ' ';
    init::erros << "'  yylval = " << hex << "0x" <<  setw(8) << yylval.integer << endl;
    init::erros << "ERROR: " << s << endl;
    init::erros << setfill('-') << setw(80) << '-' << endl << endl;
}
