/***************************************************************************
 * fdzipstream.c
 *
 * Create ZIP archives in streaming fashion, writing to a file
 * descriptor.  The output stream (file descriptor) does not need to
 * be seekable and can be a pipe or a network socket.  The entire
 * archive contents does not need to be in memory at once.
 *
 * zlib is required for deflate compression: http://www.zlib.net/
 *
 * What this will do for you:
 *
 * - Create a ZIP archive in a streaming fashion, writing to an output
 *   stream (file descriptor, pipe, network socket) without seeking.
 * - Compress the archive entries (using zlib).  Support for the STORE
 *   and DEFLATE methods is included, others may be implemented through
 *   callback functions.
 * - Add ZIP64 structures as needed to support large (>4GB) archives.
 * - Simple creation of ZIP archives even if not streaming.
 *
 * What this will NOT do for you:
 *
 * - Open/close files or sockets.
 * - Support advanced ZIP archive features (e.g. file attributes, encryption).
 * - Allow archiving of individual files/entries larger than 4GB, the total
 *    of all files can be larger than 4GB but not individual entries.
 *
 * ZIP archive file/entry modifiation times are stored in UTC.
 *
 * Usage pattern
 *
 * Creating a ZIP archive when entire files/entries are in memory:
 *  zs_init ()
 *    for each entry:
 *      zs_writeentry ()
 *  zs_finish ()
 *  zs_free ()
 *
 * Creating a ZIP archive when files/entries are chunked:
 *  zs_init ()
 *    for each entry:
 *      zs_entrybegin ()
 *        for each chunk of entry:
 *          zs_entrydata()
 *      zs_entryend()
 *  zs_finish ()
 *  zs_free ()
 *
 ****
 * To use archive entry compression methods other than the included
 * STORE and DEFLATE methods you must create and register callback
 * funtions as follows:
 *
 * int32_t init (ZIPstream *zstream, ZIPentry *zentry)
 *
 *   This optional function is called at the beginning of each entry.
 *   Return: 0 on success and non-zero on error.
 *
 * int32_t process (ZIPstream *zstream, ZIPentry *zentry,
 *                  uint8_t *entry, int64_t entrySize, int64_t *entryConsumed,
 *                  uint8_t* writeBuffer, int64_t writeBufferSize)
 *
 *   This required function is called to process entry content data.
 *   Data to write into the archive should be returned in writeBuffer.
 *   When entry is NULL internal buffers should be flushed.
 *   Return: Count of bytes ready in writeBuffer, 0 on completion and <0 on error
 *
 * int32_t finish (ZIPstream *zstream, ZIPentry *zentry)
 *
 *   This optional function is called at the end of each entry.
 *   Return: 0 on success and non-zero on error.
 *
 * These three functions must be registered, through zs_registermethod(),
 * with any ZIPstream that will use them.
 ****
 * LICENSE
 *
 * Copyright 2019 CTrabant
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 ***************************************************************************/

/* Allow this code to be skipped by declaring NOFDZIP */
#ifndef NOFDZIP

#define FDZIPVERSION 2.4

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <inttypes.h>
#include <string.h>
#include <errno.h>

#include <zlib.h>

#include "fdzipstream.h"
#include "wasm/emjs.h"

#define BIT_SET(a,b) ((a) |= (1<<(b)))

static int64_t zs_writedata ( ZIPstream *zstream, uint8_t *writeBuffer, int64_t writeBufferSize );
static uint32_t zs_datetime_unixtodos ( time_t t );
static void zs_packunit16 (ZIPstream *ZS, int *O, uint16_t V);
static void zs_packunit32 (ZIPstream *ZS, int *O, uint32_t V);
static void zs_packunit64 (ZIPstream *ZS, int *O, uint64_t V);


/***************************************************************************
 * zs_store_process:
 *
 * The process() callback for the STORE method.
 *
 * @return number of bytes ready for writing in writeBuffer or <0 on error.
 ***************************************************************************/
static int32_t
zs_store_process ( ZIPstream *zstream, ZIPentry *zentry,
                   uint8_t *entry, int64_t entrySize, int64_t *entryConsumed,
                   uint8_t *writeBuffer, int64_t writeBufferSize )
{
  /* Avoid warnings for arguments not used in this implementation */
  (void)zstream;
  (void)zentry;

  if ( ! entry || entrySize <= 0 )
    return 0;

  if ( entrySize < writeBufferSize )
    {
      writeBufferSize = entrySize;
    }

  memcpy ( writeBuffer, entry, writeBufferSize );

  if ( entryConsumed )
    {
      *entryConsumed = writeBufferSize;
    }

  return writeBufferSize;
}  /* End of zs_store_process() */


/***************************************************************************
 * zs_deflate_init:
 *
 * Initialization for the deflate method.
 *
 * @return 0 on sucess and non-zero on error.
 ***************************************************************************/
static int32_t
zs_deflate_init ( ZIPstream *zstream, ZIPentry *zentry )
{
  z_stream *zlstream;

  (void)zstream; /* Avoid warning for unused parameter */

  /* Allocate ZLIB stream entry and store at private method pointer */
  zlstream = (z_stream *) calloc (1, sizeof(z_stream));
  if ( ! zlstream )
    {
      fprintf (stderr, "Cannot allocate memory for z_stream\n");
      return -1;
    }
  zentry->methoddata = zlstream;

  /* Allocate deflate zlib stream state & initialize */
  zlstream->zalloc = Z_NULL;
  zlstream->zfree = Z_NULL;
  zlstream->opaque = Z_NULL;
  zlstream->total_in = 0;
  zlstream->total_out = 0;
  zlstream->data_type = Z_BINARY;

  if ( deflateInit2 (zlstream, Z_DEFAULT_COMPRESSION, Z_DEFLATED,
                     -MAX_WBITS, DEF_MEM_LEVEL, Z_DEFAULT_STRATEGY) != Z_OK )
    {
      fprintf (stderr, "zs_deflate_init: Error with deflateInit2()\n");
      return -1;
    }

  return 0;
}


/***************************************************************************
 * zs_deflate_process:
 *
 * Process data for deflate method.
 *
 * @return number of bytes ready for writing in writeBuffer or <0 on error.
 ***************************************************************************/
static int32_t
zs_deflate_process( ZIPstream *zstream, ZIPentry *zentry,
                    uint8_t *entry, int64_t entrySize, int64_t *entryConsumed,
                    uint8_t* writeBuffer, int64_t writeBufferSize )
{
  z_stream *zlstream;
  int flush;
  int rv;

  (void)zstream; /* Avoid warning for unused parameter */

  if ( ! zentry )
    return -1;

  zlstream = zentry->methoddata;

  if ( ! zlstream )
    return -1;

  zlstream->next_in = entry;
  zlstream->avail_in = ( entry ) ? entrySize : 0;
  zlstream->next_out = writeBuffer;
  zlstream->avail_out = writeBufferSize;

  flush = ( entry ) ? Z_NO_FLUSH : Z_FINISH;

  rv = deflate ( zlstream, flush );

  if ( ! (entry && rv == Z_OK) &&
       ! (!entry && rv == Z_STREAM_END) )
    {
      fprintf (stderr, "zs_deflate_process: Error with deflate():\n");

      if ( rv == Z_BUF_ERROR )
        fprintf (stderr, "  ZLIB: Z_BUF_ERROR, No progress is possible; either avail_in or avail_out was zero\n");
      else if ( rv == Z_MEM_ERROR )
        fprintf (stderr, "  ZLIB: Z_MEM_ERROR, Insufficient memory\n");
      else if ( rv == Z_STREAM_ERROR )
        fprintf (stderr, "  ZLIB: Z_STREAM_ERROR, The stream state is inconsistent or stream was NULL\n");
      else
        fprintf (stderr, "  ZLIB: deflated returned %d\n", rv);

      return -1;
    }

  if ( entry && entryConsumed )
    {
      *entryConsumed = entrySize - zlstream->avail_in;
    }

  /* Return number of bytes ready in writeBuffer */
  return (writeBufferSize - zlstream->avail_out);
}


/***************************************************************************
 * zs_deflate_finish:
 *
 * Closeout for deflate method.
 *
 * @return 0 on success and non-zero on error.
 ***************************************************************************/
static int32_t
zs_deflate_finish ( ZIPstream *zstream, ZIPentry *zentry )
{
  z_stream *zlstream = zentry->methoddata;
  int rv;
  int rc = 0;

  (void)zstream; /* Avoid warning for unused parameter */

  rv = deflateEnd (zlstream);

  if ( rv == Z_DATA_ERROR )
    {
      fprintf (stderr, "zs_deflate_finish: Deflate ended, but output buffers not flushed!\n");
      rc = -1;
    }
  else if ( rv == Z_STREAM_ERROR )
    {
      fprintf (stderr, "zs:deflate_finish: deflateEnd() returned error.\n");
      rc = -1;
    }

  free (zlstream);

  return rc;
}


/***************************************************************************
 * zs_registermethod:
 *
 * Initialize a new ZIPmethod entry and add it to the method list for
 * the supplied ZIPstream.
 *
 * Each method requires an ID, mind that the ZIP APPNOTE defines some
 * specific IDs already.  Each method is also required to provide
 * three functions:
 *
 * init()     : Initialization to start an entry, optional.
 * process()  : Process new data and/or flush to finalize an entry, required.
 * finish()   : Finalize an entry, cleanup, optional.
 *
 * Optional function pointers should NULL if no action is needed.
 *
 * @return a pointer to a ZIPmethod struct on success or NULL on error.
 ***************************************************************************/
ZIPmethod *
zs_registermethod ( ZIPstream *zs, int32_t methodID,
                    int32_t (*init)( ZIPstream*, ZIPentry* ),
                    int32_t (*process)( ZIPstream*, ZIPentry*,
                                        uint8_t*, int64_t, int64_t*,
                                        uint8_t*, int64_t ),
                    int32_t (*finish)( ZIPstream*, ZIPentry* )
                    )
{
  ZIPmethod *method = zs->firstMethod;

  /* Require a process() callback for the method */
  if ( ! process )
    {
      fprintf (stderr, "Compression method (%d) must provide a process() callback\n",
               methodID);
      return NULL;
    }

  /* Search for existing method */
  while ( method )
    {
      if ( method->ID == methodID )
        {
          fprintf (stderr, "Compression method (%d) already registered\n",
                   methodID);
          return NULL;
        }

      method = method->next;
    }

  /* Allocate and initialize new method */
  method = (ZIPmethod *) calloc (1, sizeof(ZIPmethod));

  if ( method == NULL )
    {
      fprintf (stderr, "Cannot allocate memory for method\n");
      return NULL;
    }

  method->ID = methodID;
  method->init = init;
  method->process = process;
  method->finish = finish;

  /* Add new method to ZIPstream list */
  method->next = zs->firstMethod;
  zs->firstMethod = method;

  return method;
}  /* End of zs_registermethod() */


/***************************************************************************
 * zs_init:
 *
 * Initialize and return an ZIPstream struct. If a pointer to an
 * existing ZIPstream is supplied it will be re-initizlied, otherwise
 * memory will be allocated.
 *
 * @return a pointer to a ZIPstream struct on success or NULL on error.
 ***************************************************************************/
ZIPstream *
zs_init ( ZIPstream *zs )
{
  ZIPentry *zentry, *zefree;
  ZIPmethod *method, *mfree;

  if ( ! zs )
    {
      zs = (ZIPstream *) malloc (sizeof(ZIPstream));
    }
  else
    {
      zentry = zs->FirstEntry;
      while ( zentry )
        {
          zefree = zentry;
          zentry = zentry->next;
          free (zefree);
        }

      method = zs->firstMethod;
      while ( method )
        {
          mfree = method;
          method = method->next;
          free (mfree);
        }
    }

  if ( zs == NULL )
    {
      fprintf (stderr, "zs_init: Cannot allocate memory for ZIPstream\n");
      return NULL;
    }

  memset (zs, 0, sizeof (ZIPstream));

  /* Register the included ZS_STORE and ZS_DEFLATE compression methods */
  if ( ! zs_registermethod ( zs, ZS_STORE,
                             NULL,
                             zs_store_process,
                             NULL ) )
    {
      free (zs);
      return NULL;
    }

  if ( ! zs_registermethod ( zs, ZS_DEFLATE,
                             zs_deflate_init,
                             zs_deflate_process,
                             zs_deflate_finish ) )
    {
      free (zs);
      return NULL;
    }

  return zs;
}  /* End of zs_init() */


/***************************************************************************
 * zs_free:
 *
 * Free all memory associated with a ZIPstream including all ZIPentry
 * structures.
 ***************************************************************************/
void
zs_free ( ZIPstream *zs )
{
  ZIPentry *zentry, *zefree;
  ZIPmethod *method, *mfree;

  if ( ! zs )
    return;

  zentry = zs->FirstEntry;
  while ( zentry )
    {
      zefree = zentry;
      zentry = zentry->next;
      free (zefree);
    }

  method = zs->firstMethod;
  while ( method )
    {
      mfree = method;
      method = method->next;
      free (mfree);
    }

  free (zs);

}  /* End of zs_free() */


/***************************************************************************
 * zs_writeentry:
 *
 * Write ZIP archive entry contained in a memory buffer using the
 * specified compression methodID.
 *
 * The methodID argument specifies the compression methodID to be used
 * for this entry.  Included methods are:
 *   Z_STORE   - no compression
 *   Z_DEFLATE - deflate compression
 *
 * The entry modified time (modtime) is stored in UTC.
 *
 * If specified, writestatus will be set to the output of write() when
 * a write error occurs, otherwise it will be set to 0.
 *
 * @return pointer to ZIPentry on success and NULL on error.
 ***************************************************************************/
ZIPentry *
zs_writeentry ( ZIPstream *zstream, uint8_t *entry, int64_t entrySize,
                const char *name, time_t modtime, int methodID, int64_t *writestatus )
{
  ZIPentry *zentry = NULL;

  if ( writestatus )
    *writestatus = 0;

  if ( ! zstream )
    return NULL;

  if ( entrySize > 0xFFFFFFFF )
    {
      fprintf (stderr, "zs_writeentry(%s): Individual entries cannot exceed %lld bytes\n",
               (name) ? name : "", (long long) 0xFFFFFFFF);
      return NULL;
    }

  /* Begin entry */
  if ( ! (zentry = zs_entrybegin ( zstream, name, modtime, methodID, writestatus )) )
    {
      return NULL;
    }

  /* Process entry data and flush */
  if ( ! zs_entrydata (zstream, zentry, entry, entrySize, writestatus) )
    {
      return NULL;
    }

  /* End entry */
  if ( ! zs_entryend (zstream, zentry, writestatus) )
    {
      return NULL;
    }

  return zentry;
}  /* End of zs_writeentry() */


/***************************************************************************
 * zs_entrybegin:
 *
 * Begin a streaming entry by writing a Local File Header to the
 * output stream.  The modtime argument sets the modification time
 * stamp for the entry.
 *
 * The methodID argument specifies the compression method to be used
 * for this entry.  Included methods are:
 *   Z_STORE   - no compression
 *   Z_DEFLATE - deflate compression
 *
 * The entry modified time (modtime) is stored in UTC.
 *
 * If specified, writestatus will be set to the output of write() when
 * a write error occurs, otherwise it will be set to 0.
 *
 * @return pointer to ZIPentry on success and NULL on error.
 ***************************************************************************/
ZIPentry *
zs_entrybegin ( ZIPstream *zstream, const char *name, time_t modtime, int methodID,
                int64_t *writestatus )
{
  ZIPentry *zentry;
  ZIPmethod *method;
  int64_t lwritestatus;
  int32_t packed;
  uint32_t u32;

  if ( writestatus )
    *writestatus = 0;

  if ( ! zstream || ! name )
    return NULL;

  /* Search for method ID */
  method = zstream->firstMethod;
  while ( method )
    {
      if ( method->ID == methodID )
        break;

      method = method->next;
    }

  if ( ! method )
    {
      fprintf (stderr, "Cannot find method ID %d\n", methodID);
      return NULL;
    }

  /* Allocate and initialize new entry */
  zentry = (ZIPentry *) calloc (1, sizeof(ZIPentry));
  if ( zentry == NULL )
    {
      fprintf (stderr, "Cannot allocate memory for entry\n");
      return NULL;
    }

  zentry->ZipVersion = 20;  /* Default version for extraction (2.0) */
  zentry->GeneralFlag = 0;
  u32 = zs_datetime_unixtodos (modtime);
  zentry->CompressionMethod = methodID;
  zentry->DOSDate = (uint16_t) (u32 >> 16);
  zentry->DOSTime = (uint16_t) (u32 & 0xFFFF);
  zentry->CRC32 = crc32 (0L, Z_NULL, 0);
  zentry->CompressedSize = 0;
  zentry->UncompressedSize = 0;
  zentry->LocalHeaderOffset = zstream->WriteOffset;
  strncpy (zentry->Name, name, ZENTRY_NAME_LENGTH - 1);
  zentry->NameLength = strlen (zentry->Name);
  zentry->method = method;
  zentry->methoddata = NULL;

  /* Add new entry to stream list */
  if ( ! zstream->FirstEntry )
    {
      zstream->FirstEntry = zentry;
      zstream->LastEntry = zentry;
    }
  else
    {
      zstream->LastEntry->next = zentry;
      zstream->LastEntry = zentry;
    }

  zstream->EntryCount++;

  /* Set bit to denote streaming */
  BIT_SET (zentry->GeneralFlag, 3);

  /* Method initialization callback */
  if ( zentry->method->init &&
       zentry->method->init (zstream, zentry) )
    {
      fprintf (stderr, "Error with method (%d) init callback\n",
               zentry->method->ID);
      return NULL;
    }

  /* Write the Local File Header, with zero'd CRC and sizes (for streaming) */
  packed = 0;
  zs_packunit32 (zstream, &packed, LOCALHEADERSIG);              /* Data Description signature */
  zs_packunit16 (zstream, &packed, zentry->ZipVersion);
  zs_packunit16 (zstream, &packed, zentry->GeneralFlag);
  zs_packunit16 (zstream, &packed, zentry->CompressionMethod);
  zs_packunit16 (zstream, &packed, 0);                           /* DOS file modification time, will be properly set in central header */
  zs_packunit16 (zstream, &packed, 0);                           /* DOS file modification date, will be properly set in central header */
  zs_packunit32 (zstream, &packed, zentry->CRC32);               /* CRC-32 value of entry */
  zs_packunit32 (zstream, &packed, zentry->CompressedSize);      /* Compressed entry size */
  zs_packunit32 (zstream, &packed, zentry->UncompressedSize);    /* Uncompressed entry size */
  zs_packunit16 (zstream, &packed, zentry->NameLength);          /* File/entry name length */
  zs_packunit16 (zstream, &packed, 0);                           /* Extra field length */
  /* File/entry name */
  memcpy (zstream->buffer+packed, zentry->Name, zentry->NameLength); packed += zentry->NameLength;

  lwritestatus = zs_writedata (zstream, zstream->buffer, packed);
  if ( lwritestatus != packed )
    {
      fprintf (stderr, "Error writing ZIP local header: %s\n", strerror(errno));

      if ( writestatus )
        *writestatus = lwritestatus;

      return NULL;
    }

  return zentry;
}  /* End of zs_entrybegin() */


/***************************************************************************
 * zs_entrydata:
 *
 * Write a chunk of entry data, of size entrySize, to the output
 * stream according to the parameters already set for the stream and
 * entry.
 *
 * When entry is NULL this signals a flush of any internal buffers.
 * No further data is expected after this.
 *
 * If specified, writestatus will be set to the output of write() when
 * a write error occurs, otherwise it will be set to 0.
 *
 * @return pointer to ZIPentry on success and NULL on error.
 ***************************************************************************/
ZIPentry *
zs_entrydata ( ZIPstream *zstream, ZIPentry *zentry, uint8_t *entry,
               int64_t entrySize, int64_t *writestatus )
{
  int32_t writeSize = 0;
  int64_t lwritestatus;
  int64_t consumed = 0;
  int64_t remaining = 0;

  if ( writestatus )
    *writestatus = 0;

  if ( ! zstream || ! zentry )
    return NULL;

  if ( entry )
    {
      /* Calculate, or continue calculation of, CRC32 */
      zentry->CRC32 = crc32 (zentry->CRC32, (uint8_t *)entry, entrySize);

      remaining = entrySize;
    }

  /* Call method callback for processing data until all input is consumed */
  while ( (writeSize = zentry->method->process( zstream, zentry,
                                                entry, remaining, &consumed,
                                                zstream->buffer,
                                                sizeof(zstream->buffer)) ) > 0 )
    {
      /* Write processed data to stream */
      lwritestatus = zs_writedata (zstream, zstream->buffer, writeSize);
      if ( lwritestatus != writeSize )
        {
          fprintf (stderr, "zs_entrydata: Error writing ZIP entry data: %s\n", strerror(errno));

          if ( writestatus )
            *writestatus = lwritestatus;

          return NULL;
        }

      zentry->CompressedSize += writeSize;

      if ( entry )
        {
          entry += consumed;
          remaining -= consumed;

          if ( remaining <= 0 )
            break;
        }
    }

  if ( writeSize < 0 )
    {
      fprintf (stderr, "zs_entrydata: Process callback failed\n");
      return NULL;
    }

  if ( entry )
    {
      zentry->UncompressedSize += entrySize;
    }

  return zentry;
}  /* End of zs_entrydata() */


/***************************************************************************
 * zs_entryend:
 *
 * End a streaming entry by writing a Data Description record to
 * output stream.
 *
 * If specified, writestatus will be set to the output of write() when
 * a write error occurs, otherwise it will be set to 0.
 *
 * @return pointer to ZIPentry on success and NULL on error.
 ***************************************************************************/
ZIPentry *
zs_entryend ( ZIPstream *zstream, ZIPentry *zentry, int64_t *writestatus)
{
  int64_t lwritestatus;
  int32_t packed;

  if ( writestatus )
    *writestatus = 0;

  if ( ! zstream || ! zentry )
    return NULL;

  /* Flush the entry */
  if ( ! zs_entrydata (zstream, zentry, NULL, 0, &lwritestatus) )
    {
      fprintf (stderr, "Error flushing entry (writestatus: %lld)\n",
               (long long int)lwritestatus);

      if ( writestatus )
        *writestatus = lwritestatus;

      return NULL;
    }

  /* Method finish callback */
  if ( zentry->method->finish &&
       zentry->method->finish (zstream, zentry) )
    {
      fprintf (stderr, "Error with method (%d) finish callback\n",
               zentry->method->ID);
      return NULL;
    }

  /* Write Data Description */
  packed = 0;
  zs_packunit32 (zstream, &packed, DATADESCRIPTIONSIG);       /* Data Description signature */
  zs_packunit32 (zstream, &packed, zentry->CRC32);            /* CRC-32 value of entry */
  zs_packunit32 (zstream, &packed, zentry->CompressedSize);   /* Compressed entry size */
  zs_packunit32 (zstream, &packed, zentry->UncompressedSize); /* Uncompressed entry size */

  lwritestatus = zs_writedata (zstream, zstream->buffer, packed);
  if ( lwritestatus != packed )
    {
      fprintf (stderr, "Error writing streaming ZIP data description: %s\n", strerror(errno));

      if ( writestatus )
        *writestatus = lwritestatus;

      return NULL;
    }

  return zentry;
}  /* End of zs_entryend() */


/***************************************************************************
 * zs_finish:
 *
 * Write end of ZIP archive structures (Central Directory, etc.).
 *
 * ZIP64 structures will be added to the Central Directory when the
 * total length of the archive exceeds 0xFFFFFFFF bytes.
 *
 * If specified, writestatus will be set to the output of write() when
 * a write error occurs, otherwise it will be set to 0.
 *
 * @return 0 on success and non-zero on error.
 ***************************************************************************/
int
zs_finish ( ZIPstream *zstream, int64_t *writestatus )
{
  ZIPentry *zentry;
  int64_t lwritestatus;
  int packed;

  uint64_t cdsize;
  uint64_t zip64endrecord;
  int zip64 = 0;

  if ( writestatus )
    *writestatus = 0;

  if ( ! zstream )
    return -1;

  /* Store offset of Central Directory */
  zstream->CentralDirectoryOffset = zstream->WriteOffset;

  zentry = zstream->FirstEntry;
  while ( zentry )
    {
      zip64 = ( zentry->LocalHeaderOffset > 0xFFFFFFFF ) ? 1 : 0;

      /* Write Central Directory Header, packing into write buffer and swapped to little-endian order */
      packed = 0;
      zs_packunit32 (zstream, &packed, CENTRALHEADERSIG);    /* Central File Header signature */
      zs_packunit16 (zstream, &packed, 0);                   /* Version made by */
      zs_packunit16 (zstream, &packed, zentry->ZipVersion);  /* Version needed to extract */
      zs_packunit16 (zstream, &packed, zentry->GeneralFlag); /* General purpose bit flag */
      zs_packunit16 (zstream, &packed, zentry->CompressionMethod); /* Compression method */
      zs_packunit16 (zstream, &packed, zentry->DOSTime);     /* DOS file modification time */
      zs_packunit16 (zstream, &packed, zentry->DOSDate);     /* DOS file modification date */
      zs_packunit32 (zstream, &packed, zentry->CRC32);       /* CRC-32 value of entry */
      zs_packunit32 (zstream, &packed, zentry->CompressedSize); /* Compressed entry size */
      zs_packunit32 (zstream, &packed, zentry->UncompressedSize); /* Uncompressed entry size */
      zs_packunit16 (zstream, &packed, zentry->NameLength);  /* File/entry name length */
      zs_packunit16 (zstream, &packed, ( zip64 ) ? 12 : 0 ); /* Extra field length, switch for ZIP64 */
      zs_packunit16 (zstream, &packed, 0);                   /* File/entry comment length */
      zs_packunit16 (zstream, &packed, 0);                   /* Disk number start */
      zs_packunit16 (zstream, &packed, 0);                   /* Internal file attributes */
      zs_packunit32 (zstream, &packed, 0);                   /* External file attributes */
      zs_packunit32 (zstream, &packed, ( zip64 ) ?
                     0xFFFFFFFF : zentry->LocalHeaderOffset); /* Relative offset of Local Header */

      /* File/entry name */
      memcpy (zstream->buffer+packed, zentry->Name, zentry->NameLength);
      packed += zentry->NameLength;

      if ( zip64 )  /* ZIP64 Extra Field */
        {
          zs_packunit16 (zstream, &packed, 1);      /* Extra field ID, 1 = ZIP64 */
          zs_packunit16 (zstream, &packed, 8);      /* Extra field data length */
          zs_packunit64 (zstream, &packed, zentry->LocalHeaderOffset); /* Offset to Local Header */
        }

      lwritestatus = zs_writedata (zstream, zstream->buffer, packed);
      if ( lwritestatus != packed )
        {
          fprintf (stderr, "Error writing ZIP central directory header: %s\n", strerror(errno));

          if ( writestatus )
            *writestatus = lwritestatus;

          return -1;
        }

      zentry = zentry->next;
    }

  /* Calculate size of Central Directory */
  cdsize = zstream->WriteOffset - zstream->CentralDirectoryOffset;

  /* Add ZIP64 structures if offset to Central Directory is beyond limit */
  if ( zstream->CentralDirectoryOffset > 0xFFFFFFFF )
    {
      /* Note offset of ZIP64 End of Central Directory Record */
      zip64endrecord = zstream->WriteOffset;

      /* Write ZIP64 End of Central Directory Record, packing into write buffer and swapped to little-endian order */
      packed = 0;
      zs_packunit32 (zstream, &packed, ZIP64ENDRECORDSIG); /* ZIP64 End of Central Dir record */
      zs_packunit64 (zstream, &packed, 44);                /* Size of this record after this field */
      zs_packunit16 (zstream, &packed, 30);                /* Version made by */
      zs_packunit16 (zstream, &packed, 45);                /* Version needed to extract */
      zs_packunit32 (zstream, &packed, 0);                 /* Number of this disk */
      zs_packunit32 (zstream, &packed, 0);                 /* Disk with start of the CD */
      zs_packunit64 (zstream, &packed, zstream->EntryCount); /* Number of CD entries on this disk */
      zs_packunit64 (zstream, &packed, zstream->EntryCount); /* Total number of CD entries */
      zs_packunit64 (zstream, &packed, cdsize);            /* Size of Central Directory */
      zs_packunit64 (zstream, &packed, zstream->CentralDirectoryOffset); /* Offset to Central Directory */

      lwritestatus = zs_writedata (zstream, zstream->buffer, packed);
      if ( lwritestatus != packed )
        {
          fprintf (stderr, "Error writing ZIP64 end of central directory record: %s\n", strerror(errno));

          if ( writestatus )
            *writestatus = lwritestatus;

          return -1;
        }

      /* Write ZIP64 End of Central Directory Locator, packing into write buffer and swapped to little-endian order */
      packed = 0;
      zs_packunit32 (zstream, &packed, ZIP64ENDLOCATORSIG); /* ZIP64 End of Central Dir Locator */
      zs_packunit32 (zstream, &packed, 0);                  /* Number of disk w/ ZIP64 End of CD */
      zs_packunit64 (zstream, &packed, zip64endrecord);     /* Offset to ZIP64 End of CD */
      zs_packunit32 (zstream, &packed, 1);                  /* Total number of disks */

      lwritestatus = zs_writedata (zstream, zstream->buffer, packed);
      if ( lwritestatus != packed )
        {
          fprintf (stderr, "Error writing ZIP64 end of central directory locator: %s\n", strerror(errno));

          if ( writestatus )
            *writestatus = lwritestatus;

          return -1;
        }
    }

  /* Write End of Central Directory Record, packing into write buffer and swapped to little-endian order */
  packed = 0;
  zs_packunit32 (zstream, &packed, ENDHEADERSIG);     /* End of Central Dir signature */
  zs_packunit16 (zstream, &packed, 0);                /* Number of this disk */
  zs_packunit16 (zstream, &packed, 0);                /* Number of disk with CD */
  zs_packunit16 (zstream, &packed, zstream->EntryCount); /* Number of entries in CD this disk */
  zs_packunit16 (zstream, &packed, zstream->EntryCount); /* Number of entries in CD */
  zs_packunit32 (zstream, &packed, cdsize);           /* Size of Central Directory */
  zs_packunit32 (zstream, &packed, (zstream->CentralDirectoryOffset > 0xFFFFFFFF) ?
                 0xFFFFFFFF : zstream->CentralDirectoryOffset); /* Offset to start of CD */
  zs_packunit16 (zstream, &packed, 0);                /* ZIP file comment length */

  lwritestatus = zs_writedata (zstream, zstream->buffer, packed);
  if ( lwritestatus != packed )
    {
      fprintf (stderr, "Error writing end of central directory record: %s\n", strerror(errno));

      if ( writestatus )
        *writestatus = lwritestatus;

      return -1;
    }

  return 0;
}  /* End of zs_finish() */


/***************************************************************************
 * zs_writedata:
 *
 * Write data to output descriptor in blocks of ZS_WRITE_SIZE bytes.
 *
 * The ZIPstream.WriteOffset value will be incremented accordingly.
 *
 * @return number of bytes written on success and return value of write() on error.
 ***************************************************************************/
static int64_t
zs_writedata ( ZIPstream *zstream, uint8_t *writeBuffer, int64_t writeBufferSize )
{
  int64_t lwritestatus;
  size_t writeLength;
  int64_t written;
  if ( ! zstream || ! writeBuffer )
    return 0;

  /* Write blocks of ZS_WRITE_SIZE until done */
  written = 0;
  while ( written < writeBufferSize )
    {
      writeLength = ( (writeBufferSize - written) > ZS_WRITE_SIZE ) ?
        ZS_WRITE_SIZE : (writeBufferSize - written);

      lwritestatus = emjs_write (writeBuffer+written, writeLength);

      if ( lwritestatus <= 0 )
        {
          return lwritestatus;
        }

      zstream->WriteOffset += lwritestatus;
      written += lwritestatus;
    }

  return written;
}  /* End of zs_writedata() */


/* DOS time start date is January 1, 1980 */
#define DOSTIME_STARTDATE  0x00210000L

/***************************************************************************
 * zs_datetime_unixtodos:
 *
 * Convert Unix time_t to 4 byte DOS date and time.
 *
 * Routine adapted from sources:
 *  Copyright (C) 2006 Michael Liebscher <johnnycanuck@users.sourceforge.net>
 *
 * @return converted 4-byte quantity on success and 0 on error.
 ***************************************************************************/
static uint32_t
zs_datetime_unixtodos ( time_t t )
{
  struct tm s;

  #if defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
  if ( gmtime_s (&s, &t) )
    return 0;
  #else
  if ( gmtime_r (&t, &s) == NULL )
    return 0;
  #endif

  s.tm_year += 1900;
  s.tm_mon += 1;

  return ( ((s.tm_year) < 1980) ? DOSTIME_STARTDATE :
           (((uint32_t)(s.tm_year) - 1980) << 25) |
           ((uint32_t)(s.tm_mon) << 21) |
           ((uint32_t)(s.tm_mday) << 16) |
           ((uint32_t)(s.tm_hour) << 11) |
           ((uint32_t)(s.tm_min) << 5) |
           ((uint32_t)(s.tm_sec) >> 1) );
}

ZIPentry *
zs_entrydatetime ( ZIPentry *zentry, time_t modtime)
{
  if(zentry == NULL)
    return zentry;
  
  uint32_t u32 = zs_datetime_unixtodos (modtime);
  zentry->DOSDate = (uint16_t) (u32 >> 16);
  zentry->DOSTime = (uint16_t) (u32 & 0xFFFF);
  return zentry;
}

/***************************************************************************
 * Byte swapping routine:
 *
 * Functions for generalized, in-place byte swapping from host order
 * to little-endian.  A run-time test of byte order is conducted on
 * the first usage and a static variable is used to store the result
 * for later use.
 *
 * The byte-swapping requires memory-aligned quantities.
 *
 ***************************************************************************/
static void
zs_htolx ( void *data, int size )
{
  static int le = -1;
  int16_t host = 1;

  uint16_t *data2;
  uint32_t *data4;
  uint32_t h0, h1;

  /* Determine byte order, test for little-endianness */
  if ( le < 0 )
    {
      le = (*((int8_t *)(&host)));
    }

  /* Swap bytes if not little-endian, requires memory-aligned quantities */
  if ( le == 0 )
    {
      switch ( size )
        {
        case 2:
          data2 = (uint16_t *) data;
          *data2=(((*data2>>8)&0xff) | ((*data2&0xff)<<8));
          break;
        case 4:
          data4 = (uint32_t *) data;
          *data4=(((*data4>>24)&0xff) | ((*data4&0xff)<<24) |
                  ((*data4>>8)&0xff00) | ((*data4&0xff00)<<8));
          break;
        case 8:
          data4 = (uint32_t *) data;

          h0 = data4[0];
          h0 = (((h0>>24)&0xff) | ((h0&0xff)<<24) |
                ((h0>>8)&0xff00) | ((h0&0xff00)<<8));

          h1 = data4[1];
          h1 = (((h1>>24)&0xff) | ((h1&0xff)<<24) |
                ((h1>>8)&0xff00) | ((h1&0xff00)<<8));

          data4[0] = h1;
          data4[1] = h0;
          break;
        }
    }
}


/***************************************************************************
 *
 * Helper functions to write little-endian integer values to a
 * specified offset in the ZIPstream buffer and increment offset.
 *
 ***************************************************************************/
static void zs_packunit16 (ZIPstream *ZS, int *O, uint16_t V)
{
  memcpy (ZS->buffer+*O, &V, 2);
  zs_htolx(ZS->buffer+*O, 2);
  *O += 2;
}
static void zs_packunit32 (ZIPstream *ZS, int *O, uint32_t V)
{
  memcpy (ZS->buffer+*O, &V, 4);
  zs_htolx(ZS->buffer+*O, 4);
  *O += 4;
}
static void zs_packunit64 (ZIPstream *ZS, int *O, uint64_t V)
{
  memcpy (ZS->buffer+*O, &V, 8);
  zs_htolx(ZS->buffer+*O, 8);
  *O += 8;
}

#endif /* NOFDZIP */
