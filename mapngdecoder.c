#include <assert.h>
#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#if defined(_WIN32)
#include <io.h>
#endif
#include <openssl/aes.h>

const unsigned char RESOURCE_KEY[] = "A1dPUcrvur2CRQyl";

static int load_resource(const char * fname, void ** blk, size_t * size)
{
    size_t n;
    FILE * s = 0;
    struct stat sb;
    if (-1 == stat(fname, &sb))
    {
        fprintf(stderr, "stat(%s) errno=%d, info=%s\n", fname, errno,
                strerror(errno));
        return -1;
    }

    if (!S_ISREG(sb.st_mode))
    {
        fprintf(stderr, "%s is not regular file\n", fname);
        return -2;
    }

    *size = sb.st_size;
    if (*size % AES_BLOCK_SIZE != 0)
    {
        fprintf(stderr, "%s 's size if invalid (size = %u)\n", fname, *size);
        return -3;
    }

    if ((*blk = calloc(1, *size)) == 0)
    {
        fprintf(stderr, "no enough memory for size = %u\n", *size);
        return -1;
    }

    if ((s = fopen(fname, "rb")) == 0)
    {
        fprintf(stderr, "fopen(%s) errno=%d, info=%s\n", fname, errno,
                strerror(errno));
        free(*blk);
        return -1;
    }

    n = 0;
    while (!feof(s) && n < *size)
    {
        n += fread((unsigned char *) *blk + n, 1, *size - n, s);
        if (ferror(s))
        {
            fprintf(stderr, "fread(%s) errno=%d, info=%s\n", fname, errno,
                    strerror(errno));
            free(*blk);
            fclose(s);
            return -1;
        }
    }
    fclose(s);

    assert(*size == n);

    return 0;
}

static int decrypt_resource(void * begin, size_t * len)
{
    AES_KEY k;
    void * b = 0;
    unsigned m = 0, n = 0;
    assert(len && *len && *len % AES_BLOCK_SIZE == 0);

    if ((b = calloc(1, *len)) == 0)
    {
        fprintf(stderr, "no enough memory for size = %u\n", *len);
        return -1;
    }

    AES_set_decrypt_key(RESOURCE_KEY, 128, &k);
    for (n = 0; n < *len; n += AES_BLOCK_SIZE)
    {
        AES_ecb_encrypt(n + (unsigned char *) begin, n + (unsigned char *) b,
                &k, AES_DECRYPT);
    }

    m = *(*len + (unsigned char *) b - 1);
    assert(m > 0 && m <= AES_BLOCK_SIZE);
    for (n = 1; n < m; ++n)
    {
        if (m != *(*len + (unsigned char *) b - n))
        {
            fprintf(stderr, "invalid padding! m = %u, n = %u, * = %u\n", m, n,
                    *(*len + (unsigned char *) b - n));
            free(b);
            return -1;
        }
    }
    *len -= m;
    memcpy(begin, b, *len);

    free(b);
    return 0;
}

static int save_resource(const char * fname, void * blk, size_t size)
{
    FILE * s = 0;
    size_t n;

    if (0 == access(fname, W_OK) || errno != ENOENT)
    {
        if (!errno)
            errno = EEXIST;
        fprintf(stderr, "access(%s) errno=%d, info=%s\n", fname, errno,
                strerror(errno));
        free(blk);
        return -1;
    }

    if ((s = fopen(fname, "wb")) == 0)
    {
        fprintf(stderr, "fopen(%s) errno=%d, info=%s\n", fname, errno,
                strerror(errno));
        free(blk);
        return -1;
    }

    n = 0;
    while (n < size)
    {
        n += fwrite(blk + n, 1, size + n, s);
        if (ferror(s))
        {
            fprintf(stderr, "fwrite(%s) errno=%d, info=%s\n", fname, errno,
                    strerror(errno));
            free(blk);
            fclose(s);
            return -1;
        }
    }
    free(blk);
    fclose(s);
    return 0;
}

int picker(const char * fname)
{
    void * blk;
    size_t size;
    char oname[256];
    strcpy(oname, fname);
    strcat(oname, ".png");

    if (0 != load_resource(fname, &blk, &size))
    {
        return -1;
    }

    if (0 != decrypt_resource(blk, &size))
    {
        return -1;
    }

    if (0 != save_resource(oname, blk, size))
    {
        return -1;
    }

    printf("decode> %s\n", oname);

    return 0;
}

int main(int c, char *v[])
{
    while (*(++v))
    {
        picker(*v);
    }
    return 0;
}
