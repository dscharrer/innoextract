
/* Allow this code to be skipped by declaring NOFDZIP */
#ifndef NOFDZIP

#ifndef FDZIPSTREAM_H
#define FDZIPSTREAM_H

#include <stdint.h>
#include <time.h>
#include <sys/types.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifndef DEF_MEM_LEVEL
#  if MAX_MEM_LEVEL >= 8
#    define DEF_MEM_LEVEL 8
#  else
#    define DEF_MEM_LEVEL MAX_MEM_LEVEL
#  endif
#endif

/* ZIP record type signatures */
#define LOCALHEADERSIG      (0x04034b50)
#define DATADESCRIPTIONSIG  (0x08074b50)
#define CENTRALHEADERSIG    (0x02014b50)
#define ZIP64ENDRECORDSIG   (0x06064b50)
#define ZIP64ENDLOCATORSIG  (0x07064b50)
#define ENDHEADERSIG        (0x06054b50)

/* Compression methods, match ZIP specification */
#define ZS_STORE      0
#define ZS_DEFLATE    8

/* Maximum single size to write(), 1 MiB */
#define ZS_WRITE_SIZE 1048576

/* Multi-use stream buffer, 256 KiB */
#define ZS_BUFFER_SIZE 262144

/* Maximum length of file/entry name including NULL terminator */
#define ZENTRY_NAME_LENGTH 256

/* ZIP archive entry */
typedef struct zipentry_s
{
  uint16_t ZipVersion;
  uint16_t GeneralFlag;
  uint16_t CompressionMethod;
  uint16_t DOSDate;
  uint16_t DOSTime;
  uint32_t CRC32;
  uint64_t CompressedSize;
  uint64_t UncompressedSize;
  uint64_t LocalHeaderOffset;
  uint16_t NameLength;
  char Name[ZENTRY_NAME_LENGTH];
  struct zipmethod_s *method;    /* Pointer to compression method entry */
  void *methoddata;              /* A private pointer for method data */
  struct zipentry_s *next;
} ZIPentry;

/* ZIP output stream managment */
typedef struct zipstream_s
{
  int64_t WriteOffset;
  int64_t CentralDirectoryOffset;
  int32_t EntryCount;
  struct zipentry_s *FirstEntry;
  struct zipentry_s *LastEntry;
  struct zipmethod_s *firstMethod;
  uint8_t buffer[ZS_BUFFER_SIZE];
} ZIPstream;


/* List of ZIP method (compression) implementations */
typedef struct zipmethod_s
{
  int32_t ID;
  int32_t (*init)( ZIPstream *zstream, ZIPentry *zentry );
  int32_t (*process)( ZIPstream *zstream, ZIPentry *zentry,
                      uint8_t *entry, int64_t entrySize, int64_t *entryConsumed,
                      uint8_t* writeBuffer, int64_t writeBufferSize );
  int32_t (*finish)( ZIPstream *zstream, ZIPentry *zentry );
  struct zipmethod_s* next;
} ZIPmethod;


extern  ZIPmethod * zs_registermethod ( ZIPstream *zs, int32_t methodID,
                                        int32_t (*init)( ZIPstream*, ZIPentry* ),
                                        int32_t (*process)( ZIPstream*, ZIPentry*,
                                                            uint8_t*, int64_t, int64_t*,
                                                            uint8_t*, int64_t ),
                                        int32_t (*finish)( ZIPstream*, ZIPentry* )
                                        );

extern ZIPstream * zs_init ( ZIPstream *zs );

extern void zs_free ( ZIPstream *zs );

extern ZIPentry * zs_writeentry ( ZIPstream *zstream, uint8_t *entry, int64_t entrySize,
                                  const char *name, time_t modtime, int methodID, int64_t *writestatus );

extern ZIPentry * zs_entrybegin ( ZIPstream *zstream, const char *name,
                                  time_t modtime, int methodID,
                                  int64_t *writestatus );

extern ZIPentry * zs_entrydata ( ZIPstream *zstream, ZIPentry *zentry,
                                 uint8_t *entry, int64_t entrySize,
                                 int64_t *writestatus );

extern ZIPentry * zs_entryend ( ZIPstream *zstream, ZIPentry *zentry,
                                int64_t *writestatus);

ZIPentry *zs_entrydatetime ( ZIPentry *zentry, time_t modtime);

extern int zs_finish ( ZIPstream *zstream, int64_t *writestatus );


#ifdef __cplusplus
}
#endif

#endif /* FDZIPSTREAM_H */

#endif /* NOFDZIP */

