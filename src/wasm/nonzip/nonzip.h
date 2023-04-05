#ifndef NONZIP_H
#define NONZIP_H

#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <time.h>

#define NONZIP_LF_SIGNATURE   (0x04034b50)
#define NONZIP_CF_SIGNATURE   (0x02014b50)
#define NONZIP_END_SIGNATURE  (0x06054b50)
#define NONZIP_VERMADE_DEF    (63U) // Version 6.3 of document
#define NONZIP_VERNEED_DEF    (41U) // Version 4.1 required for UTF

// Enable UTF filenames and comments, lengths and CRC only correct in central header
#define NONZIP_FLAGS_DEF      ((1<<11) | (1<<3))
#define NONZIP_COMP_DEF       (0U) // No compression
#define NONZIP_DISK_DEF       (0U) // only "disk 0" used
#define NONZIP_ATTRS_DEF      (0U) // attributes might be set to 0

#define NONZIP_STATUS_IDLE    (0)
#define NONZIP_STATUS_READY   (1)
#define NONZIP_ERR_NOTREADY   (-1)
const char nonzip_comment[] = "Created with nonzip by Michał Stoń <michal.ston@mobica.com>";

struct dostime{
    uint16_t time;
    uint16_t date;
};

// Struct for internal use to keep all fire parameters used by CFH and LFH
struct nonzip_file {
    uint16_t zf_verm;   // version made by
    uint16_t zf_vere;   // version needed to extract
    uint16_t zf_flags;  // general purpose bit flag
    uint16_t zf_comp;   // compression method
    uint16_t zf_modt;   // last mod file time
    uint16_t zf_modd;   // last mod file date
    uint32_t zf_crc;    // crc-32
    uint32_t zf_csize;  // compressed size
    uint32_t zf_usize;  // uncompressed size
    uint16_t zf_fnlen;  // file name length
    uint16_t zf_eflen;  // extra field length
    uint16_t zf_fclen;  // file comment length
    uint16_t zf_dns;    // disk number start
    uint16_t zf_attri;  // internal file attributes
    uint32_t zf_attre;  // external file attributes
    uint32_t zf_offset;  // relative offset of local header
    char     *zf_name;
    char     *zf_data;
};

#pragma pack(push, 1)
// Local file header
struct nonzip_lf {
    uint32_t lf_sig;    // local file header signature
    uint16_t lf_vere;   // version needed to extract
    uint16_t lf_flags;  // general purpose bit flag
    uint16_t lf_comp;   // compression method
    uint16_t lf_modt;   // time in MS-DOS format
    uint16_t lf_modd;   // date, ditto
    uint32_t lf_crc;    // CRC32, magic=0xdebb20e3
    uint32_t lf_csize;  // compressed size
    uint32_t lf_usize;  // uncompressed size
    uint16_t lf_fnlen;  // file name length
    uint16_t lf_eflen;  // extra field length
    // filename
    // extra field
    // actual data
};

// Central file header
struct nonzip_cf {
    uint32_t cf_sig;    // central file header signature
    uint16_t cf_verm;   // version made by
    uint16_t cf_vere;   // version needed to extract
    uint16_t cf_flags;  // general purpose bit flag
    uint16_t cf_comp;   // compression method
    uint16_t cf_modt;   // last mod file time
    uint16_t cf_modd;   // last mod file date
    uint32_t cf_crc;    // crc-32
    uint32_t cf_csize;  // compressed size
    uint32_t cf_usize;  // uncompressed size
    uint16_t cf_fnlen;  // file name length
    uint16_t cf_eflen;  // extra field length
    uint16_t cf_fclen;  // file comment length
    uint16_t cf_dns;    // disk number start
    uint16_t cf_attri;  // internal file attributes
    uint32_t cf_attre;  // external file attributes
    uint32_t cf_offset;  // relative offset of local header
    // filename
    // extra field
    // file comment
};

struct nonzip_end {
    uint32_t en_sig;    // end of central dir signature
    uint16_t en_dnum;   // number of this disk
    uint16_t en_cdnum;  // number of the disk with the
                        // start of the central directory
    uint16_t en_ennum;  // total number of entries in the
                        // central directory on this disk
    uint16_t en_cdennum;// total number of entries in
                        // the central directory
    uint32_t en_cdsize; // size of the central directory
    uint32_t en_cdoff;  // offset of start of central
                        // directory with respect to
                        // the starting disk number
    uint16_t en_clen;   // .ZIP file comment length = NONZIP_ZIPCOMMENT_LEN
    char en_comment[sizeof(nonzip_comment)-1]; // comment
};

#pragma pack(pop)
typedef struct nonzip_zip {
    int status;
    uint64_t numfiles;
    uint64_t numfalloc;
    uint32_t offset;
    struct nonzip_end *end;
    struct nonzip_file **files;
    struct dostime dt;
} nonzip_t;




class Nonzip {
private:
    int status;
    uint64_t numfiles;
    uint64_t numfalloc;
    uint32_t offset;
    struct nonzip_file **files;
    struct dostime dt;
public:
    Nonzip(const char *path);
    int addFile(const char *name, const void *data, uint32_t dlen, uint32_t *zipindex);
    int appendFile(const void *data, uint32_t dlen);
    int close();
    int getStatus();
    void setTime(uint32_t index, time_t t);
};

// nonzip_t nonzip_open(const char *path);
// int nonzip_addfile(nonzip_t *z, const char *name, const void *data, uint32_t dlen, uint32_t *zipindex);
// int nonzip_appendfile(nonzip_t *z, const void *data, uint32_t dlen);
// void nonzip_settime(nonzip_t *z, uint32_t index, time_t t);
// int nonzip_close(nonzip_t *z);

#endif /* NONZIP_H */
