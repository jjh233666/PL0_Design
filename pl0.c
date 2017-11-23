// pl0 compNTler source code

#pragma warning(disable:4996)


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "pl0.h"
#include "set.c"

//////////////////////////////////////////////////////////////////////
// print error message.
void error(int n)
{
	int i;

	printf("      ");
	for (i = 1; i <= cc - 1; i++)
		printf(" ");
	printf("^\n");
	printf("Error %3d: %s\n", n, err_msg[n]);
	err++;
} // error

//////////////////////////////////////////////////////////////////////
void getch(void)
{
	if (cc == ll)
	{
		if (feof(infile))
		{
			printf("\nPROGRAM INCOMPLETE\n");
			exit(1);
		}
		ll = cc = 0;
		printf("%5d  ", cx);
		while ( (!feof(infile)) // added & modified by alex 01-02-09
			    && ((ch = getc(infile)) != '\n'))
		{
			printf("%c", ch);
			line[++ll] = ch;
		} // while
		printf("\n");
		line[++ll] = ' ';
	}
	ch = line[++cc];
} // getch

//////////////////////////////////////////////////////////////////////
// gets a symbol from input stream.
void getsym(void)
{
	int i, k;
	char a[MAXIDLEN + 1];

	while (ch == ' '||ch == '\t')
		getch();

	if (isalpha(ch))
	{ // symbol is a reserved word or an identifier.
		k = 0;
		do
		{
			if (k < MAXIDLEN)
				a[k++] = ch;
			getch();
		}
		while (isalpha(ch) || isdigit(ch));
		a[k] = 0;
		strcpy(id, a);//
		word[0] = id;
		i = NRW;
        
		while (strcmp(id, word[i--]));
		if (++i)
			sym = wsym[i]; // symbol is a reserved word
		else
			sym = SYM_IDENTIFIER;   // symbol is an identifier
	}
	else if (isdigit(ch))
	{ // symbol is a number.
		k = num = 0;
		sym = SYM_NUMBER;
		do
		{
			num = num * 10 + ch - '0';
			k++;
			getch();
		}
		while (isdigit(ch));

		if (k > MAXNUMLEN)
			error(25);     // The number is too great.
	}
	else if (ch == '=')
	{
		getch();
		if (ch == '=')
		{
			sym = SYM_EQU; // :=
			getch();
		}
		else
		{
			sym = SYM_BECOMES;       // illegal?
		}
	}
	else if (ch == '>')
	{
		getch();
		if (ch == '=')
		{
			sym = SYM_GEQ;     // >=
			getch();
		}
		else
		{
			sym = SYM_GTR;     // >
		}
	}
	else if (ch == '<')
	{
		getch();
		if (ch == '=')
		{
			sym = SYM_LEQ;     // <=
			getch();
		}
		else if (ch == '>')
		{
			sym = SYM_NEQ;     // <>
			getch();
		}
		else
		{
			sym = SYM_LES;     // <
		}
	}
	else if (ch=='|')
	{
		getch();
		if(ch=='|')
		{
			sym =SYM_OR;
		    getch();
		}
		else if(ch=='=')
		{
			 sym=SYM_OREQ;
			 getch();
		}
		else sym=SYM_BOR;
	}
	else if(ch=='&')
	{
        getch();
		if(ch=='&') {sym=SYM_AND;getch();}
		else if(ch=='='){sym=SYM_ANDEQ;getch();}
        else sym=SYM_BAND;	
	}
	else if(ch=='^')
	{
		getch();
		if(ch=='='){sym=SYM_NOREQ;getch();}
		else sym=SYM_BNOR;
	}
	/*else if(ch=='%')
	{
		getch();
		if(ch=='='){sym=SYM_MODEQ;getch();}
		else sym=SYM_MOD;
	}*/
	else if (ch == '/')
	{
		getch();
		if (ch == '/') { cc = ll; getch(); getsym(); } // line comment
		else if (ch == '*') { // block-comment
			getch(); //skip '*'
			while (1) {
				if (ch == '*') {
					getch();
					if (ch == '/') break;
				}
				getch();
			}
			getch();
			getsym();
		}
		else if(ch=='=')
		{
			sym=SYM_DIVEQ;
            getch();
		}
		else sym = SYM_SLASH;
	}
	else if(ch=='!')
	{
        sym=SYM_NOT;
		getch();
	}
	else if(ch=='+')
	{
       getch();
	   if(ch=='+')
	   {
		   sym=SYM_DPLUS;
		   getch();
	   }
	   else if(ch=='=')
	   {
		   sym=SYM_ADDEQ;
		   getch();
	   }
	   else sym=SYM_PLUS;
	}
	else if(ch=='-'){
		getch();
		if(ch=='-'){
			sym=SYM_DMINUS;
			getch();
		}
		else if(ch=='='){
			sym=SYM_MINUSEQ;
			getch();
		}
		else sym=SYM_MINUS;
	}
	else if(ch=='*'){
		getch();
		if(ch=='='){
			sym=SYM_MULTEQ;
			getch();
		}
		else sym=SYM_TIMES;
	}
	/*else if(ch=='%'){
		getch();
		if(ch=='='){
			sym=SYM_MODEQ;
			getch();
		}
		else sym=SYM_MOD;
	}*/
	else
	{ // other tokens
		i = NSYM;
		csym[0] = ch;
		while (csym[i--] != ch);
		if (++i)
		{
			sym = ssym[i];
			getch();
		}
		else
		{
			printf("Fatal Error: Unknown character.\n");
			exit(1);
		}
	}
} // getsym

//////////////////////////////////////////////////////////////////////
// generates (assembles) an instruction.
void gen(int x, int y, int z)  //生成code中的
{
	if (cx > CXMAX)
	{
		printf("Fatal Error: Program too long.\n");
		exit(1);
	}
	code[cx].f = x;
	code[cx].l = y;
	code[cx++].a = z;
} // gen

//////////////////////////////////////////////////////////////////////
// tests if error occurs and skips all symbols that do not belongs to s1 or s2.
void test(symset s1, symset s2, int n)
{
	symset s;

	if (! inset(sym, s1))
	{
		error(n);
		s = uniteset(s1, s2);
		while(! inset(sym, s))
			getsym();
		destroyset(s);
	}
} // test

//////////////////////////////////////////////////////////////////////
int dx;  // data allocation index
int id_redundancy;
// enter object(constant, variable or procedre) into table.
void enter(int kind)
{
	mask* mk;

	tx++;
	strcpy(table[tx].name, id);
	table[tx].kind = kind;
	switch (kind)
	{
	case ID_CONSTANT:
		if (num > MAXADDRESS)
		{
			error(25); // The number is too great.
			num = 0;
		}
		table[tx].value = num;
		break;
	case ID_VARIABLE:
		mk = (mask*) &table[tx];
		mk->level = level;
		mk->address = dx++;
		break;
	case ID_PROCEDURE:
		mk = (mask*) &table[tx];
		mk->level = level;
		p_index++;
		mk->index=p_index;
		break;
	case ID_ARRAY :
	    mk=(mask*) &table[tx];
        mk->level=level;
		ax++;
		mk->index=ax;
		mk->address=dx;
		break;
	    
	} // switch
} // enter

//////////////////////////////////////////////////////////////////////
// locates identifier in symbol table.
int position(char* id)
{
	int i;
	strcpy(table[0].name, id);
	i = tx + 1;
	while (strcmp(table[--i].name, id) != 0);
	return i;
} // position

//////////////////////////////////////////////////////////////////////
void constdeclaration()
{
	if (sym == SYM_IDENTIFIER)
	{
		getsym();
		if (sym == SYM_EQU || sym == SYM_BECOMES)
		{
			if (sym == SYM_EQU)
				error(1); // Found ':=' when expecting '='.
			getsym();
			if (sym == SYM_NUMBER)
			{
				enter(ID_CONSTANT);
				getsym();
			}
			else
			{
				error(2); // There must be a number to follow '='.
			}
		}
		else
		{
			error(3); // There must be an '=' to follow the identifier.
		}
	} else	error(4);
	 // There must be an identifier to follow 'const', 'var', or 'procedure'.
} // constdeclaration

//////////////////////////////////////////////////////////////////////
void vardeclaration(void)
{   int dim;
    int i;
	if (sym == SYM_IDENTIFIER)
	{
		getsym();
		if(sym!=SYM_LBRACKET)
		{
			enter(ID_VARIABLE);
		}
		else 
		{
			enter(ID_ARRAY);
			dim=0;
			{
				while(sym==SYM_LBRACKET)
				{
					getsym();
					if(sym==SYM_NUMBER)
					{
						arrtable[ax].range[dim]=num-1;
						++dim;
					}
					else 
					{
						error(26);
					}
					getsym();
					if(sym==SYM_RBRACKET)
					{
						getsym();
					}
					else error(27);

				}//end while
				arrtable[ax].dimension=dim;
				arrtable[ax].count[dim-1]=1;
				for(i=dim-2;i>=0;i--)
				{
					arrtable[ax].count[i]=arrtable[ax].count[i+1]*(arrtable[ax].range[i+1]+1);
					dx+=arrtable[ax].count[0]*(arrtable[ax].count[0]+1);
				}
			}
		}
	}
	else
	{
		error(4); // There must be an identifier to follow 'const', 'var', or 'procedure'.
	}
} // vardeclaration

int arg_get_type(void)     //get the variables' type 
{
	if(sym==SYM_VAR)
	{
		getsym();
		vardeclaration();
	}
	else 
	{
        error(33);
	}
	return(table[tx].kind);
}
void arglist_declar(symset fsys)
{
    int count=0;
	int proc_tx=tx;
	int arg_max=-1;
	int index;
	mask *mk;
	dx=0;
	if(sym!=SYM_RPAREN)
	{
		Protable[p_index].type[count]=arg_get_type();
        count++;
		while(sym==SYM_COMMA)
		{
			getsym();
			arg_get_type();
			Protable[p_index].type[count]=table[tx].kind;
			count++;
		}// end while 
	}//end if
    Protable[p_index].argc_num=count;
	for(index=count;index>0;index--)
	{
		mk=(mask*) &table[proc_tx+index];
		mk->address=mk->address-dx;
	}
	dx=0;
}
int real_arg_list(symset fsys)
{
	void assign_expr(symset fsys);
	int arg_num=0;
	symset set;
	if(sym==SYM_LPAREN)
	{   
		getsym();
		if(sym!=SYM_RPAREN)
		{
            set=uniteset(fsys,createset(SYM_COMMA,SYM_LPAREN,SYM_RPAREN,SYM_NULL));
		    assign_expr(set);
			arg_num++;
			while(sym==SYM_COMMA)
			{
				getsym();
				set=uniteset(fsys,createset(SYM_COMMA,SYM_RPAREN,SYM_NULL));
		        assign_expr(set);
			    arg_num++;
			}//end while
			destroyset(set);
		}// end if
		if(sym==SYM_RPAREN)
		{
		    getsym();	
		}//end if
		else
		{
            error(22);
		}// end else
		
	}// end if
	return arg_num;
}//end real_arg_expr
void pro_call(symset fsys,int i)
{
     symset set;
	 mask* mk=(mask*)&table[i];
     int count;
	 gen(INT,0,1);
	 getsym();
	 set=uniteset(fsys,createset(SYM_LPAREN,SYM_RPAREN,SYM_NULL));
	 count=real_arg_list(set);
	 destroyset(set);
	 gen(CAL,level-mk->level,mk->address);
	 gen(INT,0,-count);
	 if(count!=Protable[mk->index].argc_num)
	 {
		 error(35);
	 }
}

void arr_address_compute(symset fsys, int i)
{
	void assign_expr(symset  fsys);
	int dim=0;
	int index=((mask*)&table[i])->index;
	symset set;
	set=uniteset(fsys,createset(SYM_LBRACKET,SYM_RBRACKET,NULL));
    if(sym==SYM_LBRACKET)
	{
		getsym();
		assign_expr(set);     //num 
		gen(LIT,0,arrtable[index].count[dim]); //index into the stack
		gen(OPR,0,OPR_MUL);  //compute address
        dim++;
		if(sym==SYM_RBRACKET)
		{
        getsym();
		}
		else error(27);
		while(sym==SYM_LBRACKET&&dim<arrtable[index].dimension)
		{
			getsym();
			assign_expr(set);
			gen(LIT,0,arrtable[index].count[dim]); //index into the stack
		    gen(OPR,0,OPR_MUL);  //compute address
			gen(OPR,0,OPR_ADD);  //add to the final address
			dim++;
	        if(sym==SYM_RBRACKET)
			{
				getsym();       //keep read
			}
			else error(27);
		}
		if (dim!=arrtable[index].dimension) error(29);
	}
	else error(28);
    destroyset(set);

}


//////////////////////////////////////////////////////////////////////
void listcode(int from, int to)
{
	int i;
	
	printf("\n");
	for (i = from; i < to; i++)
	{
		printf("%5d %s\t%d\t%d\n", i, mnemonic[code[i].f], code[i].l, code[i].a);
	}
	printf("\n");
} // listcode

//////////////////////////////////////////////////////////////////////
void factor(symset fsys)
{
	void or_express(symset fsys);
	void assign_expr(symset fsys);
	void expression(symset fsys);
	int i,arg_num;
	symset set;
	if(id_redundancy==1)            //check the id's redundancy
	{
        id_redundancy=0;
	}
	else{test(facbegsys, fsys, 24); // The symbol can not be as the beginning of an expression.
    
	while (inset(sym, facbegsys))
	{
		if (sym == SYM_IDENTIFIER)
		{
			if ((i = position(id)) == 0)
			{
				error(11); // Undeclared identifier.
			}
			else
			{
				switch (table[i].kind)
				{
					mask* mk;
				case ID_CONSTANT:
					gen(LIT, 0, table[i].value);
					getsym();
					break;
				case ID_VARIABLE:
					mk = (mask*) &table[i];
					gen(LOD, level - mk->level, mk->address);
					getsym();
					break;
				case ID_ARRAY:
				    mk = (mask*) &table[i];
					getsym();
					arr_address_compute(fsys,i);// compute the adddress of our array and put it into the stack[top]
					gen(ARR_LOD,level-mk->level,mk->address);
					break;
				case ID_PROCEDURE:
				    pro_call(fsys,i);
					break;
				} // switch
			}
			
		}
		else if (sym == SYM_NUMBER)
		{
			if (num > MAXADDRESS)
			{
				error(25); // The number is too great.
				num = 0;
			}
			gen(LIT, 0, num);
			getsym();
		}
		else if (sym == SYM_LPAREN)
		{
			getsym();
			set = uniteset(createset(SYM_RPAREN, SYM_NULL), fsys);
			assign_expr(set);
			destroyset(set);
			if (sym == SYM_RPAREN)
			{
				getsym();
			}
			else
			{
				error(22); // Missing ')'.
			}
		}
		else if(sym == SYM_MINUS) // UMINUS,  Expr -> '-' Expr
		{  
			 getsym();
			 factor(fsys);//expression(fsys);
			 gen(OPR, 0, OPR_NEG);
		}
		else if(sym == SYM_NOT)
		{
			getsym();
			factor(fsys);
			gen(OPR,0,OPR_NOT);
		}
		/*else if(sym == SYM_ODD)
		{
			getsym();
			expression(fsys);
            gen(OPR,0,6);
		}*/
		test(fsys, createset(SYM_LPAREN, SYM_NULL), 23);
	} // while
	}
} // factor

//////////////////////////////////////////////////////////////////////
void term(symset fsys)
{
	int mulop;
	symset set;
	
	set = uniteset(fsys, createset(SYM_TIMES, SYM_SLASH, SYM_MOD,SYM_NULL));
	factor(set);
	while (sym == SYM_TIMES || sym == SYM_SLASH||sym == SYM_MOD)
	{
		mulop = sym;
		getsym();
		factor(set);
		if (mulop == SYM_TIMES)
		{
			gen(OPR, 0, OPR_MUL);
		}
		else if(mulop == SYM_SLASH)
		{
			gen(OPR, 0, OPR_DIV);
		}
		else 
		{
			gen(OPR,0,OPR_MOD);
		}
	} // while
	destroyset(set);
} // term

//////////////////////////////////////////////////////////////////////
void expression(symset fsys)
{
	int addop;
	symset set;
	set = uniteset(fsys, createset(SYM_PLUS, SYM_MINUS, SYM_NULL));
	
	term(set);
	while (sym == SYM_PLUS || sym == SYM_MINUS)
	{
		addop = sym;
		getsym();
		term(set);
		if (addop == SYM_PLUS)
		{
			gen(OPR, 0, OPR_ADD);
		}
		else
		{
			gen(OPR, 0, OPR_MIN);
		}
	} // while

	destroyset(set);
} // expression

//////////////////////////////////////////////////////////////////////
void rel_expr(symset fsys)//condition 10_25
{
	int relop;
	symset set;

	if (sym == SYM_ODD)
	{
		getsym();
		expression(fsys);
		gen(OPR, 0, 6);
	}
	else
	{
		set = uniteset(relset, fsys);
		expression(set);
		//destroyset(set);
		if (! inset(sym, relset))
		{
			//error(20);//10_25
		}
		else
		{
			relop = sym;
			getsym();
			expression(fsys);
			switch (relop)
			{
			case SYM_EQU:
				gen(OPR, 0, OPR_EQU);
				break;
			case SYM_NEQ:
				gen(OPR, 0, OPR_NEQ);
				break;
			case SYM_LES:
				gen(OPR, 0, OPR_LES);
				break;
			case SYM_GEQ:
				gen(OPR, 0, OPR_GEQ);
				break;
			case SYM_GTR:
				gen(OPR, 0, OPR_GTR);
				break;
			case SYM_LEQ:
				gen(OPR, 0, OPR_LEQ);
				break;
			} // switch
		} // else
		destroyset(set);
	} // else
} // rel_expr condition
void BAND_express(symset fsys)
{
	symset set;
	set =uniteset(fsys,createset(SYM_BAND,SYM_NULL));
    rel_expr(set);
	while(sym==SYM_BAND)
	{
        getsym();
		rel_expr(set);
		gen(OPR,0,OPR_BAND);
	}
	destroyset(set);
}
void BNOR_express(symset fsys)
{
	symset set;
	set =uniteset(fsys,createset(SYM_BNOR,SYM_NULL));
    BAND_express(set);
	while(sym==SYM_BNOR)
	{
        getsym();
		BAND_express(set);
		gen(OPR,0,OPR_BNOR);
	}
	destroyset(set);
}
void BOR_express(symset fsys)
{
	symset set;
	set =uniteset(fsys,createset(SYM_BOR,SYM_NULL));
	BNOR_express(set);
	while(sym==SYM_BOR)
	{
		getsym();
		BNOR_express(set);
		gen(OPR,0,OPR_BOR);
	}
	destroyset(set);
}
void and_express(symset fsys)
{
	symset set;
	set =uniteset(fsys,createset(SYM_AND,SYM_NULL));
	BOR_express(set);
	while(sym==SYM_AND){
		getsym();
		BOR_express(set);
		gen(OPR,0,OPR_AND);
	}//while
	destroyset(set);
}

void or_express(symset fsys)
{
	symset set;
	set =uniteset(fsys,createset(SYM_OR,SYM_NULL));
    and_express(set);
	while(sym==SYM_OR)
	{
        getsym();
		and_express(set);
		gen(OPR,0,OPR_OR);
	}
	destroyset(set);
}


void assign_expr(symset fsys)
{
	int i;
	symset set;
	if (sym==SYM_IDENTIFIER)
	{
		mask *mk;
		if(!(i=position(id)))
		{
			error(11);
			getsym();
			set=uniteset(fsys,createset(SYM_SEMICOLON,SYM_NULL));
			test(statbegsys,set,24);
			destroyset(set);
		}
		else if(table[i].kind==ID_ARRAY||table[i].kind==ID_VARIABLE)
		{
			getsym();
			if(table[i].kind==ID_ARRAY)
			{
			    arr_address_compute(fsys,i);
			}
			mk=(mask*) &table[i];
			if(sym==SYM_BECOMES)
			{
				getsym();
				set=uniteset(fsys,createset(SYM_BECOMES,SYM_NULL));
				assign_expr(set);   //left
				destroyset(set);
				if(i)
				{
					if(table[i].kind==ID_VARIABLE) gen(STO,level-mk->level,mk->address);   //if the text you read is id  (ass->id=ass|id|or_express)
					else gen(ARR_STO,level-mk->level,mk->address);  //if it is an item of array use the ARR_STO
				
				}
			}
			else 
			{
				if(i)
				{
					if(table[i].kind==ID_VARIABLE) gen(LOD,level-mk->level,mk->address);
					else gen(ARR_LOD,level-mk->level,mk->address);
				}// end if
				id_redundancy=1;
				or_express(fsys);

			}// end else

		}// end else if
		else 
		{
			or_express(fsys);
		}// end else
	}//end if
    else or_express(fsys);
}
//////////////////////////////////////////////////////////////////////
void statement(symset fsys)
{
	int i, cx1, cx2;
	symset set1, set;

	if (sym == SYM_IDENTIFIER)
	{ // variable assignment
		mask* mk;
		if (! (i = position(id)))
		{
			error(11); // Undeclared identifier.
		}
		
		/*else if (table[i].kind != ID_VARIABLE)
		{
			error(12); // Illegal assignment.
			i = 0;
		}*/
		
		assign_expr(fsys);gen(INT,0,-1);

		
		/*if (sym == SYM_BECOMES)
		{
			getsym();
		}
		else
		{
			error(13); // ':=' expected.
		}
		else if(table[i].kind==ID_PROCEDURE)
		{
			    mask* mk;
				mk = (mask*) &table[i];
				gen(CAL, level - mk->level, mk->address);
				getsym();
		}*/

	}
	else if (sym == SYM_CALL)
	{ // procedure call
		getsym();
		if (sym != SYM_IDENTIFIER)
		{
			error(14); // There must be an identifier to follow the 'call'.
		}
	}
    else if(sym==SYM_RETURN)
	{
		getsym();
		if(sym!=SYM_SEMICOLON) assign_expr(fsys);
		else gen(LIT,0,0); // return 0 if there is nothing to return;
		mask *mk;
		int return_address;
		if(curr_proc)
		{
			mk=(mask *)&table[curr_proc];
			return_address=-Protable[mk->index].argc_num-1;
		}
		else return_address=-1;  //  like main return 0
		gen(STO,0,return_address);
		gen(RETURN,0,0);
		if(sym==SYM_SEMICOLON) getsym();
		else error(10);
	}
	else if (sym == SYM_EXIT)
	{
		gen(JMP,0,-cx);
		getsym();
		/*
		if(sym == SYM_SEMICOLON){
			getsym();
		}
		else
		{
			error(10); //';'expected
		}*/
	}
	else if (sym == SYM_IF)
	{ // if statement
		getsym();
		set1 = createset(SYM_THEN, SYM_DO,SYM_RPAREN,SYM_ELSE,SYM_NULL);
		set = uniteset(set1, fsys);
		assign_expr(set);
		//destroyset(set1);
		//destroyset(set);
		if (sym == SYM_THEN)
		{
			getsym();
		}
		else
		{
			error(16); // 'then' expected.
		}
		
		cx1 = cx;	//保存当前指令位置
		gen(JPC, 0, 0);	//生成条件跳转指令，跳转地址暂写0
		//set1 = createset(SYM_ELSE,SYM_NULL);
		//set = uniteset(set1,fsys);
		statement(set);	//处理then后语句
		if(sym == SYM_ELSE){
			getsym();
			cx2 = cx;
			code[cx1].a = cx+1;
			gen(JPC,0,0); 
			statement(fsys);
			code[cx2].a = cx;
		}
		else{
			code[cx1].a = cx;
		}
		destroyset(set1);
		destroyset(set);
	}
	else if (sym == SYM_BEGIN)
	{ // block
		getsym();
		set1 = createset(SYM_SEMICOLON, SYM_END, SYM_NULL);
		set = uniteset(set1, fsys);
		statement(set);
		while (sym == SYM_SEMICOLON || inset(sym, statbegsys))
		{
			if (sym == SYM_SEMICOLON)
			{
				getsym();
			}
			else
			{
				error(10);
			}
			statement(set);
		} // while
		destroyset(set1);
		destroyset(set);
		if (sym == SYM_END)
		{
			getsym();
		}
		else
		{
			error(17); // ';' or 'end' expected.
		}
	}
	else if (sym == SYM_WHILE)
	{ // while statement
		cx1 = cx;
		getsym();
		set1 = createset(SYM_DO, SYM_NULL);
		set = uniteset(set1, fsys);
		or_express(set);
		destroyset(set1);
		destroyset(set);
		cx2 = cx;
		gen(JPC, 0, 0);
		if (sym == SYM_DO)
		{
			getsym();
		}
		else
		{
			error(18); // 'do' expected.s
		}
		statement(fsys);
		gen(JMP, 0, cx1);
		code[cx2].a = cx;
	}
	test(fsys, phi, 19);
} // statement
			
//////////////////////////////////////////////////////////////////////
void block(symset fsys)
{
	int cx0; // initial code index
	mask* mk;
	int block_dx;
	int savedTx;
	int savedAx;
	int savedp_index;
	int savedProc;
	int return_address;
	symset set1, set;

	dx = 3;
	block_dx = dx;
	mk = (mask*) &table[curr_proc];
	mk->address = cx;
	gen(JMP, 0, 0);
	if (level > MAXLEVEL)
	{
		error(32); // There are too many levels.
	}
	do
	{
		if (sym == SYM_CONST)
		{ // constant declarations
			getsym();
			do
			{
				constdeclaration();
				while (sym == SYM_COMMA)
				{
					getsym();
					constdeclaration();
				}
				if (sym == SYM_SEMICOLON)
				{
					getsym();
				}
				else
				{
					error(5); // Missing ',' or ';'.
				}
			}
			while (sym == SYM_IDENTIFIER);
		} // if

		else if (sym == SYM_VAR)
		
		{ // variable declarations
			getsym();
			do
			{
				vardeclaration();
			
				while (sym == SYM_COMMA)
				{
					getsym();
					vardeclaration();
				}
				if (sym == SYM_SEMICOLON)
				{
					getsym();
				}
				else
				{
					error(5); // Missing ',' or ';'.
				}
			}
			while (sym == SYM_IDENTIFIER);
		} // if
		while(sym == SYM_PROCEDURE)
        { // procedure declarations
			block_dx = dx; //save dx before handling procedure call!
			getsym();
			if (sym == SYM_IDENTIFIER)
			{
				enter(ID_PROCEDURE);
				getsym();
			}
			else
			{
				error(4); // There must be an identifier to follow 'const', 'var', or 'procedure'.
			}
			level++;
			savedTx = tx;  //save the last number of table
			savedAx = ax;  //save the last number of arraytable
			savedp_index = p_index;  //save the last number of procedure
			savedProc=curr_proc;
			curr_proc=tx;
            if(sym==SYM_LPAREN)
			{
                getsym();
                arglist_declar(fsys);
			}
            else error(34);   // '('

			if(sym==SYM_RPAREN){getsym();}
			else error(22);
			if (sym == SYM_LBrace)
			{
				getsym();
			}
			else
			{
				error(30); // Missing '{'.
			}

			
			set1 = createset(SYM_LBrace,SYM_RBrace, SYM_NULL);
			set = uniteset(set1, fsys);
			block(set);
			destroyset(set1);
			destroyset(set);

			if (sym == SYM_RBrace)
			{
				getsym();
				set1 = createset(SYM_RBrace, SYM_PROCEDURE, SYM_NULL);
				set = uniteset(statbegsys, set1);
				test(set, fsys, 6);
				destroyset(set1);
				destroyset(set);
			}
			else
			{
				error(31); // Missing '}'.
			}
				tx = savedTx;
			    ax = savedAx;
			    p_index = savedp_index;
			    level--;
				dx=block_dx;
				curr_proc=savedProc;
		} // while
		//dx = block_dx; //restore dx after handling procedure call!
		set1 = createset(SYM_IDENTIFIER, SYM_NULL);
		set = uniteset(statbegsys, set1);
		test(set, declbegsys, 7);
		destroyset(set1);
		destroyset(set);
	}
	while (inset(sym, declbegsys));	

	code[mk->address].a = cx;
	mk->address = cx;
	cx0 = cx;
	gen(INT, 0, block_dx);
	set1 = createset(SYM_SEMICOLON, SYM_END, SYM_NULL);
	set = uniteset(set1, fsys);
	statement(set);
	destroyset(set1);
	destroyset(set);
	gen(LIT, 0, 0); // return 0
	if(curr_proc)
	{
		return_address=-Protable[mk->index].argc_num-1;       
	}
	else return_address=-1;
	gen(STO,0,return_address);
    gen(RETURN,0,0);
	test(fsys, phi, 8); // test for error: Follow the statement is an incorrect symbol.
	listcode(cx0, cx);
} // block

//////////////////////////////////////////////////////////////////////
int base(int stack[], int currentLevel, int levelDiff)
{
	int b = currentLevel;
	
	while (levelDiff--)
		b = stack[b];
	return b;
} // base

//////////////////////////////////////////////////////////////////////
// interprets and executes codes.
void interpret()
{
	int pc;        // program counter
	int stack[STACKSIZE];
	int top;       // top of stack
	int b;         // program, base, and top-stack register
	instruction i; // instruction register
    int offset;
	printf("Begin executing PL/0 program.\n");

	pc = 0;
	b = 1;
	top = 3;
	stack[1] = stack[2] = stack[3] = 0;
	do
	{
		i = code[pc++];
		switch (i.f)
		{
		case LIT:
			stack[++top] = i.a;
			break;
		case OPR:
			switch (i.a) // operator
			{
			case OPR_RET:
				top = b - 1;
				pc = stack[top + 3];
				b = stack[top + 2];
				break;
			case OPR_NEG:
				stack[top] = -stack[top];
				break;
			case OPR_ADD:
				top--;
				stack[top] += stack[top + 1];
				break;
			case OPR_MIN:
				top--;
				stack[top] -= stack[top + 1];
				break;
			case OPR_MUL:
				top--;
				stack[top] *= stack[top + 1];
				printf("%d\n",stack[top]);
				break;
			case OPR_DIV:	
				top--;
				if (stack[top + 1] == 0)
				{
					fprintf(stderr, "Runtime Error: Divided by zero.\n");
					fprintf(stderr, "Program terminated.\n");
					continue;
				}
				stack[top] /= stack[top + 1];
				break;
			case OPR_ODD:
				stack[top] %= 2;
				break;
			case OPR_EQU:
				top--;
				stack[top] = stack[top] == stack[top + 1];
				break;
			case OPR_NEQ:
				top--;
				stack[top] = stack[top] != stack[top + 1];
			case OPR_LES:
				top--;
				stack[top] = stack[top] < stack[top + 1];
				break;
			case OPR_GEQ:
				top--;
				stack[top] = stack[top] >= stack[top + 1];
			case OPR_GTR:
				top--;
				stack[top] = stack[top] > stack[top + 1];
				break;
			case OPR_LEQ:
				top--;
				stack[top] = stack[top] <= stack[top + 1];
			} // switch
			break;
		case LOD:
			stack[++top] = stack[base(stack, b, i.l) + i.a];
			break;
		case STO:
			stack[base(stack, b, i.l) + i.a] = stack[top];
			printf("%d\n", stack[top]);
			//top--;
			break;
		case CAL:
			stack[top + 1] = base(stack, b, i.l);
			// generate new block mark
			stack[top + 2] = b;
			stack[top + 3] = pc;
			b = top + 1;
			pc = i.a;
			break;
		case INT:
			top += i.a;
			break;
		case JMP:	//条件跳转
			pc = i.a;
			break;
		case JPC:	//无条件跳转
			if (stack[top] == 0)
				pc = i.a;
			top--;
			break;
		case ARR_LOD:      //exchange the address with the item which the address points to
		    offset=stack[top];
			stack[top]=stack[base(stack,b,i.l)+i.a+offset];
			break;
		case ARR_STO:
		    offset=stack[top-1];//stack[top-1] is offset and the stack[top] is the value after assign_express's recursion
            stack[base(stack,b,i.l)+offset+i.a]=stack[top];
			stack[top-1]=stack[top];// pop the stack top
			top-=1;
			printf("%d\n",stack[top]);
		    break;
		case RETURN:
			//printf("%d\n",stack[top]);
			top=b-1;
			pc=stack[b+2];
			b =stack[b+1];
			break;
		} // switch
	}
	while (pc);

	printf("End executing PL/0 program.\n");
} // interpret

//////////////////////////////////////////////////////////////////////
void main ()
{
	FILE* hbin;
	char s[80];
	int i;
	symset set, set1, set2;

	printf("Please input source file name: "); // get file name to be compiled
	scanf("%s", s);
	if ((infile = fopen(s,"r"))==NULL)
	{
		printf("File %s can't be opened.\n", s);
		exit(1);
	}

	phi = createset(SYM_NULL);
	relset = createset(SYM_EQU, SYM_NEQ, SYM_LES, SYM_LEQ, SYM_GTR, SYM_GEQ, SYM_NULL);
	
	// create begin symbol sets
	declbegsys = createset(SYM_CONST, SYM_VAR, SYM_PROCEDURE, SYM_NULL);
	statbegsys = createset(SYM_BEGIN, SYM_CALL, SYM_IF, SYM_WHILE,SYM_RETURN,SYM_NULL);
	facbegsys = createset(SYM_IDENTIFIER, SYM_NUMBER, SYM_LPAREN, SYM_MINUS,SYM_NOT,SYM_NULL);

	err = cc = cx = ll = 0; // initialize global variables
	ch = ' ';
	p_index=0;
	kk = MAXIDLEN;

	getsym();

	set1 = createset(SYM_PERIOD, SYM_NULL);
	set2 = uniteset(declbegsys, statbegsys);
	set = uniteset(set1, set2);
	block(set);
	destroyset(set1);
	destroyset(set2);
	destroyset(set);
	destroyset(phi);
	destroyset(relset);
	destroyset(declbegsys);
	destroyset(statbegsys);
	destroyset(facbegsys);

	if (sym != SYM_PERIOD)
		error(9); // '.' expected.
	if (err == 0)
	{
		hbin = fopen("hbin.txt", "w");
		for (i = 0; i < cx; i++)
			fwrite(&code[i], sizeof(instruction), 1, hbin);
		fclose(hbin);
	}
	if (err == 0)
		interpret();
	else
		printf("There are %d error(s) in PL/0 program.\n", err);
	listcode(0, cx);
} // main

//////////////////////////////////////////////////////////////////////
// eof pl0.c
