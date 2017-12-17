#include <stdio.h>

#define NRW        21     // number of reserved words
#define TXMAX      500    // length of identifier table
#define MAXNUMLEN  14     // maximum number of digits in numbers
#define NSYM       18     // maximum number of symbols in array ssym and csym
#define MAXIDLEN   10     // length of identifiers

#define MAXADDRESS 32767  // maximum address
#define MAXLEVEL   32     // maximum depth of nesting block
#define CXMAX      5000    // size of code array

#define MAXSYM     30     // maximum number of symbols  
#define MAXDIM     10     // maximum dimension of the array
#define MAXPROC    30     // maximum number of proc
#define MAXARGC    10     // maximun number of procedure argument
#define MAXARRAY   30
#define STACKSIZE  1000   // maximum storage

enum symtype
{
	SYM_NULL,
	SYM_IDENTIFIER,
	SYM_NUMBER,
	SYM_PLUS,
	SYM_MINUS,
	SYM_TIMES,
	SYM_SLASH,
	SYM_ODD,
	SYM_EQU,
	SYM_NEQ,
	SYM_LES,
	SYM_LEQ,
	SYM_GTR,
	SYM_GEQ,
	SYM_LPAREN,
	SYM_RPAREN,
	SYM_COMMA,
	SYM_SEMICOLON,//;
	SYM_PERIOD,
	SYM_BECOMES,
    SYM_BEGIN,
	SYM_END,
	SYM_IF,
	SYM_THEN,
	SYM_WHILE,
	SYM_DO,
	SYM_CALL,
	SYM_CONST,
	SYM_VAR,
	SYM_ANNO,
	SYM_PROCEDURE,
	SYM_NOT,
	SYM_AND,
	SYM_OR,
	SYM_BAND,//10.25
	SYM_BOR,
	SYM_BNOR,
	SYM_MOD,
	SYM_DPLUS,
	SYM_DMINUS,
	SYM_LBRACKET, //[
	SYM_RBRACKET, //]
    SYM_CONLON,   //:
	SYM_QUESMARK, //?
    SYM_RETURN,   //return
	SYM_ELSE,     //else
	SYM_ELIF,     //elif
	SYM_GOTO,	  //goto
	SYM_BREAK,	  //break
	SYM_CONTINUE, //continue
	SYM_EXIT,
	SYM_FOR ,     //for
	SYM_OREQ,
	SYM_ANDEQ,
	SYM_NOREQ,
	SYM_ADDEQ,
	SYM_MINUSEQ,
	SYM_MULTEQ,
	SYM_DIVEQ,
	SYM_MODEQ,
	SYM_LBrace,   //{
	SYM_RBrace,    //}
	SYM_RANDOM,
	SYM_PRINT,
	SYM_SLL, //<<
	SYM_SRL  //>>
};

enum idtype
{
	ID_CONSTANT, ID_VARIABLE, ID_PROCEDURE,ID_ARRAY,ID_ADDRESS
};

enum opcode
{
	LIT, OPR, LOD, STO, CAL, INT, JMP, JZ,ARR_STO,ARR_LOD,RETURN,RANDOM,PRINT
	,LEA,STO_A,LOD_A,JNZ,JG,JGE,JL,JLE,JE,JNE
};

enum oprcode
{
	OPR_RET, OPR_NEG, OPR_ADD, OPR_MIN,
	OPR_MUL, OPR_DIV, OPR_ODD, OPR_EQU,
	OPR_NEQ, OPR_LES, OPR_LEQ, OPR_GTR,
	OPR_GEQ, OPR_NOT, OPR_AND, OPR_OR,
	OPR_BAND,OPR_BOR, OPR_BNOR,OPR_MOD, //10.25
	OPR_SLL,OPR_SRL,
};


typedef struct
{
	int f; // function code
	int l; // level
	int a; // displacement address
} instruction;

//////////////////////////////////////////////////////////////////////
char* err_msg[] =
{
/*  0 */    "",
/*  1 */    "Found '==' when expecting '='.",
/*  2 */    "There must be a number to follow '='.",
/*  3 */    "There must be an '=' to follow the identifier.",
/*  4 */    "There must be an identifier to follow 'const', 'var', or 'procedure'.",
/*  5 */    "Missing ',' or ';'.",
/*  6 */    "Incorrect procedure name.",
/*  7 */    "Statement expected.",
/*  8 */    "Follow the statement is an incorrect symbol.",
/*  9 */    "'.' expected.",
/* 10 */    "';' expected.",
/* 11 */    "Undeclared identifier.",
/* 12 */    "Illegal assignment.",
/* 13 */    "':=' expected.",
/* 14 */    "There must be an identifier to follow the 'call'.",
/* 15 */    "A constant or variable can not be called.",
/* 16 */    "'then' expected.",
/* 17 */    "';' or 'end' expected.",
/* 18 */    "'do' expected.",
/* 19 */    "Incorrect symbol.",
/* 20 */    "Relative operators expected.",
/* 21 */    "Procedure identifier can not be in an expression.",
/* 22 */    "Missing ')'.",
/* 23 */    "The symbol can not be followed by a factor.",
/* 24 */    "The symbol can not be as the beginning of an expression.",
/* 25 */    "The number is too great.",
/* 26 */    "There should be a number after it",
/* 27 */    "']' expected",
/* 28 */    "'[' expected",
/* 29 */    "It is not a final array",
/* 30 */    "Missing '{' ",
/* 31 */    "Missing '}' ",
/* 32 */    "There are too many levels.",
/* 33 */    "It can not be a argument",
/* 34 */    " '(' expected",
/* 35 */    "the real arg and the formal arg is not matched"
};

//////////////////////////////////////////////////////////////////////
char ch;         // last character read
int  sym;        // last symbol read
char id[MAXIDLEN + 1]; // last identifier read
int  num;        // last number read
int  cc;         // character count
int  ll;         // line length
int  kk;
int  err;
int  cx;         // index of current instruction to be generated.
int  level = 0;
int  tx = 0;
int  ax=0;
int  p_index=0;
char line[80];
int curr_proc=0;
instruction code[CXMAX];
int truelist[5][30] = { 0 };
int falselist[5][30] = { 0 };
int tsize[5]={0};
int fsize[5]={0};
int counter=0;
int cmax=0;

char* labelNameList[MAXADDRESS];	//store the label name
int labelIndex=0;	//store the next useful index
int labelAddressList[MAXADDRESS];	//store the label address
char* gotoBackFillNameList[MAXADDRESS];	//store the goto name
int gotoBackFillAdressList[MAXADDRESS];	//store the backfill code address
int gotoBackFillNum = 0;

char* word[NRW + 1] =
{
	"", /* place holder */
	"begin", "call", "const", "do", "end","if",
	"odd", "procedure", "then", "var", "while",
	"else","elif","return","exit","for","random","print","goto","break","continue"
};

int wsym[NRW + 1] =
{
	SYM_NULL, SYM_BEGIN, SYM_CALL, SYM_CONST, SYM_DO, SYM_END,
	SYM_IF, SYM_ODD, SYM_PROCEDURE, SYM_THEN, SYM_VAR, SYM_WHILE,
	SYM_ELSE,SYM_ELIF,SYM_RETURN,SYM_EXIT,SYM_FOR,SYM_RANDOM,SYM_PRINT,
	SYM_GOTO,SYM_BREAK,SYM_CONTINUE
};
#define MAXINS   23
char* mnemonic[MAXINS] =
{
	"LIT", "OPR", "LOD", "STO", "CAL", "INT", "JMP", "JZ","ARR_STO","ARR_LOD","RETURN","RANDOM","PRINT"
	,"LEA","STO_A","LOD_A","JNZ","JG","JGE","JL","JLE","JE","JNE"
};
int ssym[NSYM + 1] =
{
	SYM_NULL, SYM_LBRACKET, SYM_RBRACKET, SYM_TIMES, SYM_SLASH,
	SYM_LPAREN, SYM_RPAREN, SYM_EQU, SYM_COMMA, SYM_PERIOD, SYM_SEMICOLON,SYM_NOT,
	SYM_BAND ,SYM_BOR,SYM_BNOR,SYM_MOD,SYM_LBrace,SYM_RBrace,SYM_QUESMARK
};
char csym[NSYM + 1] =
{
	' ', '[', ']', '*', '/', '(', ')', '=', ',', '.', ';', '!','&','|','^','%','{','}','?'
	
};



typedef struct
{
	char name[MAXIDLEN+1];
	int kind;
	int value;
	int index;
}	comtab;
comtab table[TXMAX];
typedef struct
{
	char name[MAXIDLEN+1];
	int kind;
	short level;
	short address;
	int index;
}mask;
typedef struct
{
	int dimension;
	int count[MAXDIM];
	int range[MAXDIM];
}array_info;
array_info arrtable[MAXARRAY];

typedef struct{
	int argc_num;
	int type[MAXARGC];
}Pro_info;
Pro_info Protable[MAXPROC];
FILE* infile;

// EOF PL0.h
