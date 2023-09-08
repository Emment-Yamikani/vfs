#include <generic.h>
#include <errno.h>
#include <stdint.h>
#include <stddef.h>

int compare_strings(const char *s0, const char *s1)
{
    if (!s0 || !s1)
        return -EINVAL;
    else if ((strlen(s0) != strlen(s1)))
        return -EINVAL;
    else
        return strcmp(s0, s1);
}

static char *
grabtok(char *s, int *rlen, int c)
{
    int len = 0;
    char *tok = NULL, *buf = NULL;
    if (!s)
        return NULL;
    buf = s;
    while (*s && (*s++ != c))
        len++;
    s = buf;
    tok = malloc(len + 1);
    memset(tok, 0, len + 1);
    strncpy(tok, s, len);
    if (rlen)
        *rlen = len;
    return tok;
}

char **tokenize(char *s, int c, size_t *ptoks, char **plast_tok)
{
    int len = 0;
    size_t i = 0;
    char *buf = NULL;
    char *tmp = NULL, *tmp2 = NULL;
    char **tokens = NULL, *last_tok = NULL;

    if (!s || !c)
        return NULL;

    len = strlen(s);
    buf = malloc(len + 1);

    memset(buf, 0, len + 1);
    strncpy(buf, s, len);
    tmp = buf;
    tmp2 = &buf[strlen(buf) - 1];

    while (*tmp2 && (*tmp2 == c))
        *tmp2-- = '\0';

    for (int len = 0; *buf; ++i)
    {
        tokens = realloc(tokens, (sizeof(char *) * (i + 1)));
        while (*buf && (*buf == c))
            buf++;
        char *tok = grabtok(buf, &len, c);
        if (!tok)
            break;
        last_tok = tokens[i] = tok;
        buf += len;
    }

    tokens = realloc(tokens, (sizeof(char *) * (i + 1)));
    tokens[i] = NULL;
    free(tmp);

    if (ptoks)
        *ptoks = i;
    if (plast_tok)
        *plast_tok = last_tok;
    return tokens;
}

void tokens_free(char **tokens)
{
    if (!tokens)
        return;
    foreach (token, tokens)
        free(token);
    free(tokens);
}

char **canonicalize_path(const char *path, size_t *ptoks, char **plast)
{
    /* Tokenize slash seperated words in path into tokens */
    char **tokens = tokenize((char *)path, '/', ptoks, plast);
    return tokens;
}

char *combine_strings(const char *s0, const char *s1)
{
    if (!s0 || !s1)
        return NULL;

    size_t string_len = strlen(s0) + strlen(s1) + 1;
    char *string = (typeof(string))malloc(string_len);

    if (!string)
        return NULL;

    memset(string, 0, string_len);
    strcat(string, (char *)s0);
    strcat(string, (char *)s1);
    return string;
}