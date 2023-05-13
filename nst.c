#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdint.h>

typedef enum {
	TOK_NUM,
	TOK_IDENT,
	TOK_LBRACE,
	TOK_RBRACE,
	TOK_ASSEMBLY,
} token_kind_t;

typedef struct {
	token_kind_t type;
	char *val;
} token_t;

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
	 || c == '.' \
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
			++i;
			size_t k = 1;
			
			while (i < len && is_ident(input[i])) {
				++i;
				++k;
			}
			
			char *val = malloc(sizeof(char) * (k+1));
			strncpy(val, &input[i-k], k);
			val[k] = 0;
			tokens[j].type = TOK_IDENT;
			tokens[j].val = val;
			++j;
			
			continue;
		}
		
		if (c == '(') {
			size_t k = 0;
			++i;
			while (i < len && input[i] != ')') {
				++i;
				++k;
			}

			char *val = malloc(sizeof(char) * (k+1));
			strncpy(val, &input[i-k], k);
			val[k] = 0;
			tokens[j].val = val;
			tokens[j].type = TOK_ASSEMBLY;
			++i;
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
			tokens[j].val = malloc(sizeof(char)*2);
			tokens[j].val[0] = '{';
			tokens[j].val[1] = 0;
			j++;
			i++;
			continue;
		}
		
		if (c == '}') {
			tokens[j].type = TOK_RBRACE;
			tokens[j].val = malloc(sizeof(char)*2);
			tokens[j].val[0] = '}';
			tokens[j].val[1] = 0;
			++j;
			++i;
			continue;
		}
		
		fprintf(stderr, "TokenizingError: invalid char - %c\n", input[i]);
	}
	
	*num_tokens = j;
	return tokens;
}

#undef is_ident

void
tokens_free(t, n)
	token_t *t;
	size_t n;
{
	while (n) {
		--n;
		if (t[n].val != NULL)
			free(t[n].val);
	}

	free(t);
}

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

char
char_uppercase(c)
	char c;
{
	if (c <= 'z' && c >= 'a')
		return c + ('A'-'a');
	return c;
}

char
*str_uppercase(s)
	char *s;
{
	size_t sl = strlen(s);

	for (size_t i = 0; i < sl; ++i)
		s[i] = char_uppercase(s[i]);

	return s;
}

char
char_downcase(c)
	char c;
{
	if (c <= 'Z' && c >= 'A')
		return c - ('A'-'a');
	return c;
}

char
*str_downcase(s)
	char *s;
{
	size_t sl = strlen(s);

	for (size_t i = 0; i < sl; ++i)
		s[i] = char_downcase(s[i]);

	return s;
}

typedef struct {
	char **p;
	size_t l;
} strvec_t;

strvec_t
strvec_new(void)
	/* VOID */
{
	strvec_t r;
	r.p = NULL;
	r.l = 0;
	return r;
}

void
strvec_add(v, s)
	strvec_t *v;
	char *s;
{
	++v->l;
	v->p = realloc(v->p, sizeof(char*)*v->l);
	v->p[v->l-1] = s;
}

void
strvec_push(v, s)
	strvec_t *v;
	char *s;
{
	strvec_add(v, strdup(s));
}

uint8_t
strvec_contains(v, s)
	strvec_t *v;
	char *s;
{
	for (size_t i = 0; i < v->l; ++i)
		if (!strcmp(v->p[i], s))
			return 1;

	return 0;
}

void
strvec_free(v)
	strvec_t *v;
{
	while (v->l) {
		--v->l;
		free(v->p[v->l]);
	}

	free(v->p);
}

void
translate_nst(fn, out, fns)
	char *fn;
	FILE *out;
	strvec_t *fns;
{
	char *input = str_uppercase(read_file(fn));
	int num_tokens;
	token_t *tokens = tokenize(input, &num_tokens);
	free(input);
	unsigned short get_fn_name = 0;
	unsigned short conditional_jump = 0;
	unsigned short include = 0;
	unsigned short include_sma = 0;
	char *next_jmp = NULL;
	size_t conds = 0;
	
	for (int i = 0; i < num_tokens; ++i) {
		switch (tokens[i].type) {
			case TOK_NUM:
				fprintf(out,"PSH %s\n", tokens[i].val);
				break;
			case TOK_ASSEMBLY:
				fprintf(out, "%s\n", tokens[i].val);
				break;
			case TOK_IDENT:
				if (get_fn_name) {
					if (strvec_contains(fns, tokens[i].val)) {
						fprintf(stderr, "ParsingError: function %s is already defined\n", tokens[i].val);
						exit(1);
					}
					
					if (!strcmp(tokens[i].val, "ENTRY"))
						fprintf(out, "_START:\n");
					else
						fprintf(out, "%s:\n", tokens[i].val);

					strvec_push(fns, tokens[i].val);
				
					get_fn_name = 0;
				} else if (include) {
					include = 0;
					translate_nst(str_downcase(tokens[i].val), out, fns);
				} else if (include_sma) {
					include_sma = 0;
					char *smac = read_file(str_downcase(tokens[i].val));
					fprintf(out, smac);
					free(smac);
				} else if (conditional_jump) {
					conditional_jump = 0;
					fprintf(out,
						"%s C%lu\n"
						"JA CE%lu\n"
						"C%lu:\n"
						"JB %s\n"
						"CE%lu:\n"
					,next_jmp, conds, conds, conds, tokens[i].val, conds);
					++conds;
				} else if (!strcmp(tokens[i].val, "LOAD")) {
					include = 1;
				} else if (!strcmp(tokens[i].val, "LOAD-SMA")) {
					include_sma = 1;
				} else if (!strcmp(tokens[i].val, "<?")) {
					fprintf(out, "JB SCMP\n");
					next_jmp = "JL";
					conditional_jump = 1;
				} else if (!strcmp(tokens[i].val, ">?")) {
					fprintf(out, "JB SCMP\n");
					next_jmp = "JG";
					conditional_jump = 1;
				} else if (!strcmp(tokens[i].val, "<=?")) {
					fprintf(out, "JB SCMP\n");
					next_jmp = "JLE";
					conditional_jump = 1;
				} else if (!strcmp(tokens[i].val, ">=?")) {
					fprintf(out, "JB SCMP\n");
					next_jmp = "JGE";
					conditional_jump = 1;
				} else if (!strcmp(tokens[i].val, "=?")) {
					fprintf(out,"JB SCMP\n");
					next_jmp = "JZ";
					conditional_jump = 1;
				} else if (!strcmp(tokens[i].val, "~=?")) {
					fprintf(out,"JB SCMP\n");
					next_jmp = "JNZ";
					conditional_jump = 1;
				} else {
					if (!strvec_contains(fns, tokens[i].val)) {
						fprintf(stderr, "ParsingError: function %s undefined\n", tokens[i].val);
						exit(1);
					}
					fprintf(out, "JB %s\n", tokens[i].val);
				}
				break;
			case TOK_LBRACE:
				get_fn_name = 1;
				break;
			case TOK_RBRACE:
				fprintf(out, "GB\n");
				break;
		}
	}

	tokens_free(tokens, num_tokens);
}

int
main(argc, argv)
	int argc;
	char *argv[argc];
{
	strvec_t fns = strvec_new();
	FILE *out = fopen(argv[2], "w");
	translate_nst(argv[1], out, &fns);
	fclose(out);
	strvec_free(&fns);
	return 0;
}
