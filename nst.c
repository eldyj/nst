#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

typedef enum {
	TOK_NUM,
	TOK_IDENT,
	TOK_LBRACE,
	TOK_RBRACE,
} token_kind_t;

typedef struct {
	token_kind_t type;
	char *val;
} token_t;

void
error(msg)
	char *msg;
{
	fprintf(stderr, "Error: %s\n", msg);
	exit(1);
}

#define is_ident(c) \
	(isalpha(c)\
	 || c == '+' \
	 || c == '-' \
	 || c == '/' \
	 || c == '*' \
	 || c == '_' \
	 || c == '%' \
	 || c == '^' \
	 || c == '|' \
	 || c == '&' \
	 || c == '~' \
	 || c == '!' \
	 || c == '<' \
	 || c == '>' \
	 || c == '=' \
	 || c == '?')

token_t
*tokenize(input, num_tokens)
	char *input;
	int *num_tokens;
{
	int len = strlen(input);
	int i = 0;
	int j = 0;
	token_t *tokens = malloc(sizeof(token_t) * len);
	
	while (i < len) {
		char c = input[i];

		if (c == '#') {
			++i;
			while (i < len && input[i] != '\n')
				++i;
			++i;
			continue;
		}

		if (c == '"') {
			++i;
			while (i < len && input[i] != '"')
				++i;
			++i;
			continue;
		}
		
		if (isspace(c)) {
			++i;
			continue;
		}
		
		if (is_ident(c)) {
			char *val = malloc(sizeof(char) * 21);
			int k = 0;
			
			while (i < len && is_ident(input[i]))
				val[k++] = input[i++];
			
			val[k] = '\0';
			tokens[j].type = TOK_IDENT;
			tokens[j].val = val;
			++j;
			
			continue;
		}

		if (isdigit(c)) {
			char* val = malloc(sizeof(char) * 20);
			int k = 0;
			
			while (i < len && isdigit(input[i])) {
				val[k++] = input[i++];
			}
			
			val[k] = '\0';
			
			tokens[j].type = TOK_NUM;
			tokens[j].val = val;
			++j;
			
			continue;
		}
		
		if (c == '{') {
			tokens[j].type = TOK_LBRACE;
			tokens[j].val = "{";
			j++;
			i++;
			continue;
		}
		
		if (c == '}') {
			tokens[j].type = TOK_RBRACE;
			tokens[j].val = "}";
			++j;
			++i;
			continue;
		}
		
		error("Invalid token");
	}
	
	*num_tokens = j;
	return tokens;
}

#undef is_ident

char
*read_file(fn)
	char *fn;
{
	FILE *f = fopen(fn, "r");
	
	if (!f)
		return NULL;

	fseek(f, 0, SEEK_END);
	long fs = ftell(f);
	fseek(f, 0, SEEK_SET);

	char* buffer = malloc(sizeof(char)*(fs+1));
	if (!buffer) {
		fclose(f);
		return NULL;
	}

	fread(buffer, fs, 1, f);
	fclose(f);

	buffer[fs] = '\0';
	return buffer;
}

int
main(argc, argv)
	int argc;
	char *argv[argc];
{
	char* input = read_file(argv[1]);
	int num_tokens;
	token_t* tokens = tokenize(input, &num_tokens);
	free(input);
	unsigned short get_fn_name = 0;
	unsigned short conditional_jump = 0;
	char *next_jmp = NULL;
	size_t conds = 0;
	
	for (int i = 0; i < num_tokens; ++i) {
		switch (tokens[i].type) {
			case TOK_NUM:
				printf("psh %s\n", tokens[i].val);
				break;
			case TOK_IDENT:
				if (get_fn_name) {
					if (!strcmp(tokens[i].val, "entry"))
						printf("_start:\n");
					else
						printf("%s:\n", tokens[i].val);
				
					get_fn_name = 0;
				} else if (conditional_jump) {
					conditional_jump = 0;
					printf(
						"%s c%lu\n"
						"ja ce%lu\n"
						"c%lu:\n"
						"jb %s\n"
						"ce%lu:\n"
					,next_jmp, conds, conds, conds, tokens[i].val, conds);
					++conds;
				} else if (!strcmp(tokens[i].val, "<?")) {
					printf("jb scmp\n");
					next_jmp = "jl";
					conditional_jump = 1;
				} else if (!strcmp(tokens[i].val, ">?")) {
					printf("jb scmp\n");
					next_jmp = "jg";
					conditional_jump = 1;
				} else if (!strcmp(tokens[i].val, "<=?")) {
					printf("jb scmp\n");
					next_jmp = "jle";
					conditional_jump = 1;
				} else if (!strcmp(tokens[i].val, ">=?")) {
					printf("jb scmp\n");
					next_jmp = "jge";
					conditional_jump = 1;
				} else if (!strcmp(tokens[i].val, "=?")) {
					printf("jb scmp\n");
					next_jmp = "jz";
					conditional_jump = 1;
				} else if (!strcmp(tokens[i].val, "~=?")) {
					printf("jb scmp\n");
					next_jmp = "jnz";
					conditional_jump = 1;
				} else if (!strcmp(tokens[i].val, "inc")
				|| !strcmp(tokens[i].val, "dec")
				|| !strcmp(tokens[i].val, "dup")
				|| !strcmp(tokens[i].val, "swp")
				|| !strcmp(tokens[i].val, "prn")
				|| !strcmp(tokens[i].val, "hlt")
				|| !strcmp(tokens[i].val, "add")
				|| !strcmp(tokens[i].val, "sub")
				|| !strcmp(tokens[i].val, "mul")
				|| !strcmp(tokens[i].val, "div")
				|| !strcmp(tokens[i].val, "mod")
				|| !strcmp(tokens[i].val, "xor")
				|| !strcmp(tokens[i].val, "or")
				|| !strcmp(tokens[i].val, "and")
				|| !strcmp(tokens[i].val, "shl")
				|| !strcmp(tokens[i].val, "shr")
				|| !strcmp(tokens[i].val, "rm")
				|| !strcmp(tokens[i].val, "nrm")
				|| !strcmp(tokens[i].val, "rot")
				|| !strcmp(tokens[i].val, "not")
				) {
					printf("jb s%s\n", tokens[i].val);
				} else {
					printf("jb %s\n", tokens[i].val);
				}
				break;
			case TOK_LBRACE:
				get_fn_name = 1;
				break;
			case TOK_RBRACE:
				printf("gb\n");
				break;
		}
	}
	
	return 0;
}
