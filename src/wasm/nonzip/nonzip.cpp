#include "nonzip.h"
#include "crc32.h"

#define NONZIP_PLATFORM

#ifdef NONZIP_PLATFORM
// FILE* nz_fopen(const char *__restrict name, const char *__restrict modes);
// size_t nz_emjs::write(const void *__restrict ptr, size_t size, size_t n, FILE *__restrict stream);
// int nz_fclose(FILE *stream);
#include "wasm/emjs.h"
// #define fopen emjs::open
// #define emjs::write emjs::write
// #define fclose emjs::close
#endif

static inline void filetolf(struct nonzip_lf *lf, struct nonzip_file *f) {
    lf->lf_sig   = NONZIP_LF_SIGNATURE;
    lf->lf_vere  = f->zf_vere;
    lf->lf_flags = f->zf_flags;
    lf->lf_comp  = f->zf_comp;
    lf->lf_modt  = f->zf_modt;
    lf->lf_modd  = f->zf_modd;
    // CRC, csize and usize are ignored in local header
    lf->lf_fnlen = f->zf_fnlen;
    lf->lf_eflen = f->zf_eflen;
}

static inline void filetocf(struct nonzip_cf *cf, struct nonzip_file *f) {
    cf->cf_sig    = NONZIP_CF_SIGNATURE;
    cf->cf_verm   = f->zf_verm;
    cf->cf_vere   = f->zf_vere;
    cf->cf_flags  = f->zf_flags;
    cf->cf_comp   = f->zf_comp;
    cf->cf_modt   = f->zf_modt;
    cf->cf_modd   = f->zf_modd;
    cf->cf_crc    = f->zf_crc;
    cf->cf_csize  = f->zf_csize;
    cf->cf_usize  = f->zf_usize;
    cf->cf_fnlen  = f->zf_fnlen;
    cf->cf_offset = f->zf_offset;
}

static inline struct dostime timetodos(time_t tt) {
    struct tm l = *localtime(&tt);
    return (struct dostime){l.tm_hour << 11 | l.tm_min << 5 | l.tm_sec >> 1, (l.tm_year - 80) << 9 | (l.tm_mon + 1) << 5 | l.tm_mday};
}

Nonzip::Nonzip(const char *path) {
    emjs::open(path, "wb");
    dt = timetodos(time(0));
    status = NONZIP_STATUS_READY;
}

int Nonzip::addFile(const char *name, const void *data, uint32_t dlen, uint32_t *zipindex) {
    if (status != NONZIP_STATUS_READY) return NONZIP_ERR_NOTREADY;

    // Alloc space for lfhs
    if (numfalloc < numfiles + 1) {
        numfalloc = numfalloc ? numfalloc * 3 / 2 : 8;
        files = (struct nonzip_file **) realloc(files, numfalloc * sizeof(struct nonzip_file *));
    }
    int i = (numfiles++);
    int nlen = strlen(name);
    struct dostime dt = dt;
    files[i] = (struct nonzip_file *)calloc(1, sizeof(struct nonzip_file));

    files[i]->zf_verm = NONZIP_VERMADE_DEF;
    files[i]->zf_vere = NONZIP_VERNEED_DEF;
    files[i]->zf_flags = NONZIP_FLAGS_DEF;
    files[i]->zf_comp = NONZIP_COMP_DEF;
    files[i]->zf_crc = crc32(~0, (uint8_t*) data, dlen);
    files[i]->zf_csize = dlen;
    files[i]->zf_usize = dlen;
    files[i]->zf_fnlen = nlen;
    files[i]->zf_offset = offset;

    files[i]->zf_name = (char *)malloc(nlen);
    memcpy(files[i]->zf_name, name, nlen);

    struct nonzip_lf lf = {0, };
    filetolf(&lf, files[i]);
    emjs::write(&lf, sizeof(lf), 1);   // Write header
    emjs::write(name, 1, nlen);        // Write filename
    dlen = emjs::write(data, 1, dlen); // Write file data
    offset += sizeof(lf) + nlen + dlen;

    printf("writing file %s: len=%u, offset=%u\n", name, dlen, files[i]->zf_offset);
    if(zipindex != NULL)
        *zipindex = i;

    return dlen;
}

int Nonzip::appendFile(const void *data, uint32_t dlen) {
    if (status != NONZIP_STATUS_READY)
        return NONZIP_ERR_NOTREADY;

    long i = numfiles - 1; // Append to most recently added file
    if (i<0)
        return NONZIP_ERR_NOTREADY;

    files[i]->zf_crc = crc32(~files[i]->zf_crc, (uint8_t*) data, dlen);
    files[i]->zf_csize += dlen;
    files[i]->zf_usize += dlen;
    // local file header will not be not updated.
    // valid crc, csize and usize will be only written to the central header for easier appending
    dlen = emjs::write(data, 1, dlen); // Write file data
    offset += dlen;

    printf("Appending file %s: len=%u(+%u), offset=%u\n", files[i]->zf_name, files[i]->zf_csize , dlen, files[i]->zf_offset);

    return dlen;
}

int Nonzip::close() {
    if (status != NONZIP_STATUS_READY) return NONZIP_ERR_NOTREADY;

    int i;
    struct nonzip_cf cf = {0, };
    uint16_t cdsize = 0;
    uint32_t cdoffset = offset;
    for (i = 0; i < numfiles; i++) {
        filetocf(&cf, files[i]);
        cdsize += sizeof(cf) * emjs::write(&cf, sizeof(cf), 1);  // write central file header
        cdsize += emjs::write(files[i]->zf_name, 1, files[i]->zf_fnlen);  // write filename
        free(files[i]->zf_name);
        free(files[i]);
    }
    free(files);

    struct nonzip_end end = {0, };
    memcpy(end.en_comment, nonzip_comment, sizeof(nonzip_comment) - 1);
    end.en_sig = NONZIP_END_SIGNATURE;
    end.en_ennum = numfiles;
    end.en_cdennum = numfiles;
    end.en_cdoff = cdoffset;
    end.en_cdsize = cdsize;
    end.en_clen = sizeof(nonzip_comment) - 1;
    emjs::write(&end, sizeof(end), 1);
    emjs::close();

    status = NONZIP_STATUS_IDLE;
    return numfiles;
}

void Nonzip::setTime(uint32_t index, time_t t)
{
    struct dostime dt = timetodos(t);
    files[index]->zf_modd = dt.date;
    files[index]->zf_modt = dt.time;
}

int Nonzip::getStatus()
{
    return status;
}

#ifdef TEST
// Usecase test
int main(void) {
    nonzip_t z;
    z = nonzip_open("test.zip");
    char fname[64];
    int i;
    srand(time(0));
    for (i = 0; i < 10; i++) {
        uint32_t fsize = abs(rand()) % (1 * 1024 * 1024);
        char *file = malloc(fsize);
        sprintf(fname, "file%d", i);
        nonzip_addfile(&z, fname, file, fsize, time(0));
        free(file);
    }
    nonzip_close(&z);
    return 0;
}
#endif

#ifdef TEST2
// Usecase test for file appending
int main(void) {
    nonzip_t z;
    z = nonzip_open("test.zip");
    char fname[64];
    int j,i;
    const char t[] = "test 123\n";
    char text[32];

    for(j=0; j<10; j++){
        sprintf(fname, "file%d", j);
        nonzip_addfile(&z, fname, t, sizeof(t)-1, time(0));
        for (i = 0; i < 10; i++) {
            snprintf(text, sizeof(text), "test write %4d\n", i);
            nonzip_appendfile(&z, text, 16);
        }
    }
    nonzip_close(&z);
    return 0;
}
#endif
