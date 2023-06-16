/***************************************************************************
 * zipfiles.c
 *
 * Create a ZIP archive from all files specified on the command line
 * and write archive to stdout.  All diagnostics are printed to stderr.
 *
 * Compile with:
 *   cc -Wall fdzipstream.c zipfiles.c -o zipfiles -lz
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

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <errno.h>
#include <sys/stat.h>

#if defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
  #include <fcntl.h>
#endif

#include "fdzipstream.h"

#define MAXIMUM_READ 10485760

int main (int argc, char *argv[])
{
  ZIPstream *zstream = NULL;
  ZIPentry *zentry = NULL;

  unsigned char *buffer = NULL;
  uint64_t bufferlength = 0;
  int64_t writestatus;

  int method = ZS_DEFLATE;
  int fd;
  int idx;

  FILE *input;
  struct stat st;

  uint64_t readsize;

  if ( argc < 2 )
    {
      fprintf (stderr, "zipfiles: write a ZIP archive to stdout containing specified files\n");
      fprintf (stderr, "Usage: zipfiles [-0] <file1> [file2] ... > output.zip\n");
      fprintf (stderr, "  -0  Store archive entries, default is to deflate entries\n");
      fprintf (stderr, "\n");
      return 0;
    }

  /* Set stdout to binary mode for Windows platforms */
  #if defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
  _setmode( _fileno( stdout ), _O_BINARY );
  #endif

  /* Set output stream to stdout */
  fd = fileno (stdout);

  /* Initialize ZIP container */
  if ( (zstream = zs_init (fd, NULL)) == NULL )
    {
      fprintf (stderr, "Error initializing ZIP archive\n");
      return 1;
    }

  /* Loop through input arguments and process options */
  for ( idx=1; idx < argc; idx++ )
    {
      if ( ! strncmp (argv[idx], "-0", 2) )
        {
          method = ZS_STORE;
          fprintf (stderr, "Storing archive entries, no compression\n");
          continue;
        }
    }

  /* Loop through input files, skip options */
  for ( idx=1; idx < argc; idx++ )
    {
      if ( ! strcmp (argv[idx], "-0") )
        continue;

      if ( (input = fopen (argv[idx], "rb")) == NULL )
        {
          fprintf (stderr, "Cannot open %s: %s\n", argv[idx], strerror(errno));
          return 1;
        }

      if ( fstat (fileno(input), &st) )
        {
          fclose(input);
          fprintf (stderr, "Cannot stat %s: %s\n", argv[idx], strerror(errno));
          return 1;
        }

      /* Allocate buffer */
      if ( ! buffer )
        {
          bufferlength = 1048576;
          if ( (buffer = malloc (bufferlength)) == NULL )
            {
              fclose(input);
              fprintf (stderr, "Cannot allocate %lld bytes\n",
                       (long long int) bufferlength);
              return 1;
            }
        }

      /* Begin ZIP entry */
      if ( ! (zentry = zs_entrybegin (zstream, argv[idx], st.st_mtime,
                                      method, &writestatus)) )
        {
          fclose(input);
          zs_free (zstream);
          free (buffer);
          fprintf (stderr, "Cannot begin ZIP entry for %s (writestatus: %lld)\n",
                   argv[idx], (long long int) writestatus);
          return 1;
        }

      /* Read file into buffer */
      while ( ! feof (input) )
        {
          readsize = fread (buffer, 1, bufferlength, input);

          /* Add data to ZIP entry */
          if ( ! zs_entrydata (zstream, zentry, buffer, readsize, &writestatus) )
            {
              zs_free (zstream);
              free (buffer);
              fclose(input);
              fprintf (stderr, "Error adding entry data to ZIP for %s (writestatus: %lld)\n",
                       argv[idx], (long long int) writestatus);
              return 1;
            }
        }

      /* End ZIP entry */
      if ( ! zs_entryend (zstream, zentry, &writestatus) )
        {
          zs_free (zstream);
          free (buffer);
          fclose(input);
          fprintf (stderr, "Cannot end ZIP entry for %s (writestatus: %lld)\n",
                   argv[idx], (long long int) writestatus);
          return 1;
        }

      fprintf (stderr, "Added %s: %lld -> %lld (%.1f%%)\n",
               zentry->Name,
               (long long int) zentry->UncompressedSize,
               (long long int) zentry->CompressedSize,
               (100.0 * zentry->CompressedSize / zentry->UncompressedSize));

      fclose (input);
    } /* Done looping over input files */

  /* Finish ZIP archive */
  if ( zs_finish (zstream, &writestatus) )
    {
      zs_free (zstream);
      free (buffer);
      fprintf (stderr, "Error finishing ZIP archive (writestatus: %lld)\n",
               (long long int) writestatus);
      return 1;
    }

  fprintf (stderr, "Success, created archive with %d entries\n",
           zstream->EntryCount);

  /* Cleanup */
  zs_free (zstream);

  if ( buffer )
    free (buffer);

  return 0;
}
