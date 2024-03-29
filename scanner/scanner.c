#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

#define NO_KEYWORDS 7
#define ID_LENGTH 12

const char *keyword[NO_KEYWORDS] = {
	"const","else","if","int","return","void","while"
};

enum tsymbol {
	tnull = -1,
	tnot, tnotequ, tmod, tmodAssign, tident, tnumber,
	tand, tlparen, trparen, tmul, tmulAssign, tplus,
	tinc, taddAssign, tcomma, tminus, tdec, tsubAssign,
	tdiv, tdivAssign, tsemicolon, tless, tlesse, tassign,
	tequal, tgreat, tgreate, tlbracket, trbracket, teof,
	//단어 심볼들
	tconst, telse, tif, tint, treturn, tvoid,
	twhile, tlbrace, tor, trbrace
};

enum tsymbol tnum[NO_KEYWORDS] = {
	tconst,telse,tif,tint,treturn,tvoid,twhile
};

struct tokenType
{
	int number;
	union {
		char id[ID_LENGTH];
		int num;
	}value;
};
struct tokenType scaner();
void lexicalError(int n);
int superLetter(char ch);
int superLetterOrDigit(char ch);
int getIntNum(char firstCharacter);
int hexValue(char ch);
FILE *file_in, *output;

int main() {
	struct tokenType token;
	char name[100];
	printf("토큰구조를 분석할 파일명을 써주세요 : ");
	scanf("%s", name);
	file_in = fopen(name, "r");
	output = fopen("output.txt", "w");
	if (!(file_in&&output)) {
		printf("파일을 열수 없습니다.\n");
		return 1;
	}
	token = scaner();
	while (token.number != teof) {
		if (token.number == tnumber)
			fprintf(output, "Token --> number       : (%d, %d)\n", token.number, token.value.num);
		else if (token.number == tident)
			fprintf(output, "Token --> %-12s : (%d, %s)\n", token.value.id, token.number, token.value.id);
		else if (token.number == tsemicolon)
			fprintf(output, "Token --> semicolon    : (%d, 0)\n", token.number);
		else if (token.number < tconst || token.number >= tlbrace)
			fprintf(output, "Token --> operator     : (%d, 0)\n", token.number);
		else
			fprintf(output, "Token --> %-12s : (%d, 0)\n", keyword[token.number - 30], token.number);
		token = scaner();
	}
	printf("토큰구조 분석완료!!!\noutput.txt파일을 확인해주세요.\n");
	fcloseall();
	return 0;
}
void lexicalError(int n) {
	printf(" *** Lexical Error : ");
	switch (n) {
	case 1:printf("an identifier length must be less than 12.\n");
		break;
	case 2:printf("next character must be &.\n");
		break;
	case 3:printf("next character must be |.\n");
		break;
	case 4:printf("invalid character!!!\n");
		break;
	}
}
int superLetter(char ch) {
	return isalpha(ch) || ch == '_';
}
int superLetterOrDigit(char ch) {
	return isalnum(ch) || ch == '_';
}
int getIntNum(char firstCharacter) {
	int num = 0;
	int value;
	char ch;
	if (firstCharacter != '0') {
		ch = firstCharacter;
		do {
			num = 10 * num + (int)(ch - '0');
			ch = fgetc(file_in);
		} while (isdigit(ch));
	}
	else {
		ch = fgetc(file_in);
		if ((ch >= '0') && (ch <= '7'))
			do {
				num = 8 * num + (int)(ch - '0');
				ch = fgetc(file_in);
			} while ((ch >= '0') && (ch <= '7'));
		else if ((ch >= 'X') || (ch == 'x')) {
			while ((value = hexValue(ch = fgetc(file_in))) != -1)
				num = 16 * num + value;
		}
	}
	return num;
}
int hexValue(char ch) {
	switch (ch) {
	case '0':case '1':case '2':case '3':case '4':
	case '5':case '6':case '7':case '8':case '9':
		return ch - '0';
	case'A':case'B':case'C':case'D':case'E':case'F':
		return ch - 'A' + 10;
	case'a':case'b':case'c':case'd':case'e':case'f':
		return ch - 'a' + 10;
	default:
		return -1;
	}
}
struct tokenType scaner()
{
	struct tokenType token;
	int i, index;
	char ch, id[ID_LENGTH];

	token.number = tnull;

	do {
		while (isspace(ch = fgetc(file_in)));
		if (superLetter(ch))
		{//identifier or keyword
			i = 0;
			do {
				if (i < ID_LENGTH) id[i++] = ch;
				ch = fgetc(file_in);
			} while (superLetterOrDigit(ch));

			if (i >= ID_LENGTH)   lexicalError(1);

			id[i] = '\0';
			ungetc(ch, file_in);

			for (index = 0; index < NO_KEYWORDS; index++)
				if (!strcmp(id, keyword[index])) break;
			if (index < NO_KEYWORDS)
				token.number = tnum[index];
			else {
				token.number = tident;
				strcpy(token.value.id, id);
			}
		}
		else if (isdigit(ch)) {
			token.number = tnumber;
			token.value.num = getIntNum(ch);
		}
		else switch (ch) {
		case '/':
			ch = fgetc(file_in);
			if (ch == '*')
				do {
					while (ch != '*')   ch = fgetc(file_in);
					ch = fgetc(file_in);
				} while (ch != '/');
			else if (ch == '/')
				while (fgetc(file_in) != '\n');
			else if (ch == '=')token.number = tdivAssign;
			else
			{
				token.number = tdiv;
				ungetc(ch, file_in);
			}
			break;

		case '!':
			ch = fgetc(file_in);
			if (ch == '=')token.number = tnotequ;
			else
			{
				token.number = tnot;
				ungetc(ch, file_in);
			}
			break;

		case '%':
			ch = fgetc(file_in);
			if (ch == '=')
				token.number = tmodAssign;
			else
			{
				token.number = tmod;
				ungetc(ch, file_in);
			}
			break;

		case '&':
			ch = fgetc(file_in);
			if (ch == '&')
				token.number = tand;
			else
			{
				lexicalError(2);
				ungetc(ch, file_in);
			}
			break;

		case '*':
			ch = fgetc(file_in);
			if (ch == '=')
				token.number = tmulAssign;
			else
			{
				token.number = tmul;
				ungetc(ch, file_in);
			}
			break;

		case '+':
			ch = fgetc(file_in);
			if (ch == '+')
				token.number = tinc;
			else if (ch == '=')
				token.number = taddAssign;
			else
			{
				token.number = tplus;
				ungetc(ch, file_in);
			}
			break;

		case '-':
			ch = fgetc(file_in);
			if (ch == '-')
				token.number = tdec;
			else if (ch == '=')token.number = tsubAssign;
			else
			{
				token.number = tminus;
				ungetc(ch, file_in);
			}
			break;

		case '<':
			ch = fgetc(file_in);
			if (ch == '=')
				token.number = tlesse;
			else
			{
				token.number = tless;
				ungetc(ch, file_in);
			}
			break;

		case '=':
			ch = fgetc(file_in);
			if (ch == '=')
				token.number = tequal;
			else {
				token.number = tassign;
				ungetc(ch, file_in);
			}
			break;

		case '>':
			ch = fgetc(file_in);
			if (ch == '=')
				token.number = tgreate;
			else
			{
				token.number = tgreat;
				ungetc(ch, file_in);
			}
			break;

		case '|':
			ch = fgetc(file_in);
			if (ch == '|')
				token.number = tor;
			else
			{
				lexicalError(3);
				ungetc(ch, file_in);
			}
			break;

		case '(': token.number = tlparen; break;
		case ')': token.number = trparen; break;
		case ',': token.number = tcomma; break;
		case ';': token.number = tsemicolon; break;
		case '[': token.number = tlbracket; break;
		case ']': token.number = trbracket; break;
		case '{': token.number = tlbrace; break;
		case '}': token.number = trbrace; break;
		case EOF: token.number = teof; break;
		default:
		{
			printf("Current character : %c", ch);
			lexicalError(4);
			break;
		}
		}
	} while (token.number == tnull);
	return token;
}