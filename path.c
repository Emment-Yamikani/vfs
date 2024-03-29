#include <generic.h>
#include <errno.h>
#include <stdint.h>
#include <stddef.h>
#include <fs.h>

int verify_path(const char *path)
{
    int err = -ENOTNAM;

    if (!path)
        goto error;

    if (!*path)
        goto error;

    for (; *path; ++path)
    {
        if ((*path < (char)' ') || (*path == (char)'\\') ||
            (*path == (char)0x7f) || (*path == (char)0x81) ||
            (*path == (char)0x8D) || (*path == (char)0x9D) ||
            (*path == (char)0xA0) || (*path == (char)0xAD) ||
            (*path == '\n') || (*path == '\t') || (*path == '\r')) // NOT NECESSARY. REALLY!
            goto error;
    }

    return 0;
error:
    return err;
}

/**
 * @brief Parse the pathname of a file and return an absolute path.
 * __abspath must be a valid address while __abspath_tokens and __last_token may be NULL.
 * The returned __abspath, __abspath_tokens and __last_token should be free-ed after use.
 * 
 * @param path is the path to be parsed.
 * @param __cwd is the working directory into which path resides.
 * @param __abspath is the returned absolute path.
 * @param __abspath_tokens is the returned absolute path in a canonical form.
 * @param __last_token is the last token in the resolved path.
 * 
 * @returns (int)0 on success and non-zero on failure.
 * 
*/
int parse_path(const char *path, const char *__cwd, char **__abspath, char ***__abspath_tokens, char **__last_token, int *pisdir) {
    int err = -ENOTNAM;
    int is_dir = 0;
    size_t tmp_cwdlen = 0, tmp_pathlen = 0;
    size_t tmp_abslen = 0, ntoken = 0, tok_i = 0;
    char *tmp_path = NULL, *last_token = NULL, **token_buffer = NULL;
    char *cwd = NULL, **tokens = NULL, *abspath = NULL, *tmp_cwd = NULL;

    if (!__abspath)
    {
        err = -EINVAL;
        goto error;
    }

    if (!path || !*path)
        goto error;

    if (!__cwd || (*path == '/'))
        cwd = "/";
    else if (!*__cwd)
        goto error;
    else if (*__cwd != '/')
    {
        err = -ENOMEM;
        tmp_cwdlen = strlen(__cwd);
        if (!(tmp_cwd = cwd = malloc((tmp_cwdlen + 2))))
            goto error;
        cwd[0] = '/';
        strncpy((cwd + 1), __cwd, tmp_cwdlen);
        cwd[tmp_cwdlen + 1] = '\0';
    }
    else
        cwd = (char *)__cwd;

    if ((err = verify_path(cwd)))
        goto error;

    if ((err = verify_path(path)))
        goto error;

    tmp_cwdlen = strlen(cwd);
    tmp_pathlen = strlen(path);
    tmp_abslen = tmp_cwdlen + tmp_pathlen + 2;

    if (path[tmp_pathlen - 1] == '/')
        is_dir = 1;

    err = -ENOMEM;
    if (!(tmp_path = malloc(tmp_abslen)))
        goto error;

    strncpy(tmp_path, cwd, tmp_cwdlen);
    tmp_path[tmp_cwdlen] = '/';
    strncpy((tmp_path + tmp_cwdlen + 1), path, tmp_pathlen);
    tmp_path[tmp_abslen - 1] = '\0';

    if (!(tokens = canonicalize_path(tmp_path, &ntoken, NULL)))
        goto error;

    if (!(token_buffer = calloc((ntoken + 1), sizeof(char *))))
        goto error;

    foreach (token, tokens)
    {
        if (!compare_strings(".", token))
            continue;
        if (!compare_strings("..", token))
        {
            if (tok_i > 0)
                tok_i--;
            continue;
        }
        token_buffer[tok_i++] = last_token = token;
    }

    token_buffer[tok_i] = NULL;

    err = -ENOMEM;

    if (!(abspath = malloc(2)))
        goto error;

    *abspath = '/';
    abspath[1] = '\0';
    tmp_abslen = 1;
    tmp_pathlen = 0;

    foreach (token, token_buffer)
    {
        tmp_abslen += (tmp_pathlen = strlen(token)) + 1;

        err = -ENAMETOOLONG;
        if (tmp_pathlen > MAXFNAME)
            goto error;

        err = -ENOMEM;

        if (!(abspath = realloc(abspath, tmp_abslen)))
            goto error;

        strncpy(abspath + (tmp_abslen - (tmp_pathlen + 1)), token, tmp_pathlen);

        if (token != last_token)
            abspath[(tmp_abslen - 1)] = '/';
        else
            abspath[(tmp_abslen - 1)] = '\0';
    }

    err = -ENOMEM;
    if (__last_token) {
        if (last_token) {
            if (!(*__last_token = strdup(last_token)))
                goto error;
        } else if (!(*__last_token = strdup("/")))
            goto error;
    }

    free(tokens);
    free(tmp_path);

    if (tmp_cwd)
        free(tmp_cwd);

    *__abspath = abspath;

    if (__abspath_tokens)
        *__abspath_tokens = token_buffer;
    else
        tokens_free(token_buffer); /// @FIXME: may cause malloc to fail.

    if (pisdir)
        *pisdir = is_dir;
    return 0;
error:
    if (tokens)
        free(tokens);
    if (tmp_cwd)
        free(tmp_cwd);
    if (tmp_path)
        free(tmp_path);
    if (abspath)
        free(abspath);
    if (token_buffer)
        tokens_free(token_buffer);
    return err;
}