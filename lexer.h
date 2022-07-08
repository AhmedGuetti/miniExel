

/**
 * https://github.com/VSH2-Devs/Vs-Saxton-Hale-2/blob/develop/addons/sourcemod/scripting/modules/ff2/formula_parser.sp
 */

enum
{
	TokenInvalid,
	TokenNum,
	TokenLParen, TokenRParen,
	TokenLBrack, TokenRBrack,
	TokenPlus, TokenSub,
	TokenMul, TokenDiv,
	TokenPow,
	TokenVar
};

enum
{
	LEXEME_SIZE = 64,
	dot_flag = 1,
};

typedef struct
{
	char lexeme[LEXEME_SIZE];
	int size;
	int tag;
	float val;
} Token;

typedef struct
{
	Token tok;
	int i;
} LexState;


static void GetToken(LexState* ls, const char* formula);
static float ParseAddExpr(LexState* ls, const char* formula);
static float ParseMulExpr(LexState* ls, const char* formula);
static float ParsePowExpr(LexState* ls, const char* formula);
static float ParseFactor(LexState* ls, const char* formula);
static int LexOctal(LexState* ls, const char* formula);
static int LexHex(LexState* ls, const char* formula);
static int LexDec(LexState* ls, const char* formula);


/**
 * formula grammar (hint PEMDAS):
 * expr      = <add_expr> ;
 * add_expr  = <mult_expr> [ ('+' | '-') <add_expr> ] ;
 * mult_expr = <pow_expr> [ ('*' | '/') <mult_expr> ] ;
 * pow_expr  = <factor> [ '^' <pow_expr> ] ;
 * factor    = <number> | <var> | '(' <expr> ')' | '[' <expr> ']' ;
 */

float ParseFormula(const char* formula)
{
	LexState ls = {0};
	GetToken(&ls, formula);
	return ParseAddExpr(&ls, formula);
}

float ParseAddExpr(LexState* ls, const char* formula)
{
	float val = ParseMulExpr(ls, formula);
	if (ls->tok.tag == TokenPlus)
	{
		GetToken(ls, formula);
		float a = ParseAddExpr(ls, formula);
		return val + a;
	}
	else if (ls->tok.tag == TokenSub)
	{
		GetToken(ls, formula);
		float a = ParseAddExpr(ls, formula);
		return val - a;
	}
	return val;
}

float ParseMulExpr(LexState* ls, const char* formula)
{
	float val = ParsePowExpr(ls, formula);
	if (ls->tok.tag == TokenMul)
	{
		GetToken(ls, formula);
		float m = ParseMulExpr(ls, formula);
		return val * m;
	}
	else if (ls->tok.tag == TokenDiv)
	{
		GetToken(ls, formula);
		float m = ParseMulExpr(ls, formula);
		return val / m;
	}
	return val;
}

float ParsePowExpr(LexState* ls, const char* formula)
{
	float val = ParseFactor(ls, formula);
	if (ls->tok.tag == TokenPow)
	{
		GetToken(ls, formula);
		float e = ParsePowExpr(ls, formula);
		float p = powf(val, e);
		return p;
	}
	return val;
}

float ParseFactor(LexState* ls, const char* formula)
{
	switch (ls->tok.tag)
	{
	case TokenNum:
	{
		float f = ls->tok.val;
		GetToken(ls, formula);
		return f;
	}
	case TokenLParen:
	{
		GetToken(ls, formula);
		float f = ParseAddExpr(ls, formula);
		if (ls->tok.tag != TokenRParen)
		{
			printf("expected ')' bracket but got '%s'", ls->tok.lexeme);
			return 0.0;
		}
		GetToken(ls, formula);
		return f;
	}
	case TokenLBrack:
	{
		GetToken(ls, formula);
		float f = ParseAddExpr(ls, formula);
		if (ls->tok.tag != TokenRBrack)
		{
			printf("expected ']' bracket but got '%s'", ls->tok.lexeme);
			return 0.0;
		}
		GetToken(ls, formula);
		return f;
	}
	}
	return 0.0;
}

int LexOctal(LexState* ls, const char* formula)
{
	
	while (formula[ls->i] != 0 && (isdigit(formula[ls->i])))
	{
		switch (formula[ls->i])
		{
		case '0': case '1':
		case '2': case '3':
		case '4': case '5':
		case '6': case '7':
		case '8': case '9':
		{
			ls->tok.lexeme[ls->tok.size++] = formula[ls->i++];
			break;
		}
		default:
		{
			ls->tok.lexeme[ls->tok.size++] = formula[ls->i++];
			printf("invalid octal literal: '%s'", ls->tok.lexeme);
			return 0;
		}
		}
	}
	return 1;
}

int LexHex(LexState* ls, const char* formula)
{
	while (formula[ls->i] != 0 && (isdigit(formula[ls->i]) || isalpha(formula[ls->i])))
	{
		switch (formula[ls->i])
		{
		case '0': case '1':
		case '2': case '3':
		case '4': case '5':
		case '6': case '7':
		case '8': case '9':
		case 'a': case 'A':
		case 'b': case 'B':
		case 'c': case 'C':
		case 'd': case 'D':
		case 'e': case 'E':
		case 'f': case 'F':
		{
			ls->tok.lexeme[ls->tok.size++] = formula[ls->i++];
			break;
		}
		default:
		{
			ls->tok.lexeme[ls->tok.size++] = formula[ls->i++];
			printf("invalid hex literal: '%s'", ls->tok.lexeme);
			return 0;
		}
		}
	}
	return 1;
}

int LexDec(LexState* ls, const char* formula)
{
	int lit_flags = 0;
	while (formula[ls->i] != 0 && (isdigit(formula[ls->i]) || formula[ls->i] == '.'))
	{
		switch (formula[ls->i])
		{
		case '0': case '1':
		case '2': case '3':
		case '4': case '5':
		case '6': case '7':
		case '8': case '9':
		{
			ls->tok.lexeme[ls->tok.size++] = formula[ls->i++];
			break;
		}
		case '.':
		{
			if (lit_flags & dot_flag)
			{
				printf("extra dot in decimal literal");
				return 0;
			}
			ls->tok.lexeme[ls->tok.size++] = formula[ls->i++];
			lit_flags |= dot_flag;
			break;
		}
		default:
		{
			ls->tok.lexeme[ls->tok.size++] = formula[ls->i++];
			printf("invalid decimal literal: '%s'", ls->tok.lexeme);
			return 0;
		}
		}
	}
	return 1;
}

void GetToken(LexState* ls, const char* formula)
{
	int len = (int)strlen(formula);
	Token empty = { 0 };
	ls->tok = empty;
	while (ls->i < len)
	{
		switch (formula[ls->i])
		{
		case ' ': case '\t': case '\n':
		{
			ls->i++;
			break;
		}
		case '0':
		{
			/// possible hex, octal, binary, or float.
			ls->tok.tag = TokenNum;
			ls->i++;
			switch (formula[ls->i])
			{
			case 'o': case 'O':
			{
				/// Octal.
				ls->i++;
				if (LexOctal(ls, formula))
				{
					ls->tok.val = (float)strtol(ls->tok.lexeme, NULL, 8);
				}
				return;
			}
			case 'x': case 'X':
			{
				/// Hex.
				ls->i++;
				if (LexHex(ls, formula))
				{
					ls->tok.val = (float)strtol(ls->tok.lexeme, NULL, 16);
				}
				return;
			}
			case '.': case '1':
			case '2': case '3':
			case '4': case '5':
			case '6': case '7':
			case '8': case '9':
			{
				/// Decimal/Float.
				if (LexDec(ls, formula))
				{
					ls->tok.val = (float)atof(ls->tok.lexeme);
				}
				return;
			}
			}
			break;
		}
		case '.': case '1':
		case '2': case '3':
		case '4': case '5':
		case '6': case '7':
		case '8': case '9':
		{
			ls->tok.tag = TokenNum;
			/// Decimal/Float.
			if (LexDec(ls, formula))
			{
				ls->tok.val = (float)atof(ls->tok.lexeme);
			}
			return;
		}
		case '(':
		{
			ls->tok.lexeme[ls->tok.size++] = formula[ls->i++];
			ls->tok.tag = TokenLParen;
			return;
		}
		case ')':
		{
			ls->tok.lexeme[ls->tok.size++] = formula[ls->i++];
			ls->tok.tag = TokenRParen;
			return;
		}
		case '[':
		{
			ls->tok.lexeme[ls->tok.size++] = formula[ls->i++];
			ls->tok.tag = TokenLBrack;
			return;
		}
		case ']':
		{
			ls->tok.lexeme[ls->tok.size++] = formula[ls->i++];
			ls->tok.tag = TokenRBrack;
			return;
		}
		case '+':
		{
			ls->tok.lexeme[ls->tok.size++] = formula[ls->i++];
			ls->tok.tag = TokenPlus;
			return;
		}
		case '-':
		{
			ls->tok.lexeme[ls->tok.size++] = formula[ls->i++];
			ls->tok.tag = TokenSub;
			return;
		}
		case '*':
		{
			ls->tok.lexeme[ls->tok.size++] = formula[ls->i++];
			ls->tok.tag = TokenMul;
			return;
		}
		case '/':
		{
			ls->tok.lexeme[ls->tok.size++] = formula[ls->i++];
			ls->tok.tag = TokenDiv;
			return;
		}
		case '^':
		{
			ls->tok.lexeme[ls->tok.size++] = formula[ls->i++];
			ls->tok.tag = TokenPow;
			return;
		}
		case 'x':
		case 'n':
		case 'X':
		case 'N':
		{
			ls->tok.lexeme[ls->tok.size++] = formula[ls->i++];
			ls->tok.tag = TokenVar;
			return;
		}
		default:
		{
			ls->tok.lexeme[ls->tok.size++] = formula[ls->i++];
			printf("invalid formula token '%s'.", ls->tok.lexeme);
			return;
		}
		}
	}
}