/***************************************************************************
 * zipexample.c
 *
 * Create a ZIP archive from data in memory.
 *
 * An example of how to use fdzipstream.[ch]
 *
 * Compile with:
 *   cc -Wall fdzipstream.c zipexample.c -o zipexample -lz
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
#include <time.h>
#include <errno.h>

#include "fdzipstream.h"

char buffer[200] = "They're just questions, Leon. In answer to your query, they're written down for me. It's a test, designed to provoke an emotional response... Shall we continue?";

int main (int argc, char *argv[])
{
  ZIPstream *zstream = NULL;
  ZIPentry *zentry = NULL;

  uint64_t buffersize = 0;
  int64_t writestatus;

  time_t now;
  int method = ZS_DEFLATE;
  int fd;
  int idx;

  if ( argc < 2 )
    {
      fprintf (stderr, "zipexample: write a ZIP archive to stdout from memory buffer\n");
      fprintf (stderr, "Usage: zipexample [-S] [-D] > output.zip\n");
      fprintf (stderr, "  -S  Store archive entries\n");
      fprintf (stderr, "  -D  Deflate archive entries\n");
      fprintf (stderr, "\n");
      fprintf (stderr, "One of -S or -D is required, make sure to redirect stdout.");
      fprintf (stderr, "\n");
      return 0;
    }

  /* Loop through input arguments and process options */
  for ( idx=1; idx < argc; idx++ )
    {
      if ( ! strncmp (argv[idx], "-S", 2) )
        {
          method = ZS_STORE;
          fprintf (stderr, "Storing archive entries, no compression\n");
          continue;
        }
      else if ( ! strncmp (argv[idx], "-D", 2) )
        {
          method = ZS_DEFLATE;
          fprintf (stderr, "Deflating archive entries, with compression\n");
          continue;
        }
    }

  /* Set output stream to stdout */
  fd = fileno (stdout);

  /* Initialize ZIP container, skip options */
  if ( (zstream = zs_init (fd, NULL)) == NULL )
    {
      fprintf (stderr, "Error initializing ZIP archive\n");
      return 1;
    }

  buffersize = strlen (buffer);
  now = time(NULL);

  /* Write entry containing buffer contents */
  zentry = zs_writeentry (zstream, (unsigned char *)buffer, buffersize,
                          "Leon.txt", now, method, &writestatus);

  if ( zentry == NULL )
    {
      fprintf (stderr, "Error adding entry to output ZIP (writestatus: %lld)\n",
               (long long int) writestatus);
      return 1;
    }

  fprintf (stderr, "Added %s: %lld -> %lld (%.1f%%)\n",
           zentry->Name,
           (long long int) zentry->UncompressedSize,
           (long long int) zentry->CompressedSize,
           (100.0 * zentry->CompressedSize / zentry->UncompressedSize));

  /* Write another entry containing the same buffer contents */
  zentry = zs_writeentry (zstream, (unsigned char *)buffer, buffersize,
                          "Deckard.txt", now, method, &writestatus);

  if ( zentry == NULL )
    {
      fprintf (stderr, "Error adding entry to output ZIP (writestatus: %lld)\n",
               (long long int) writestatus);
      return 1;
    }

  fprintf (stderr, "Added %s: %lld -> %lld (%.1f%%)\n",
           zentry->Name,
           (long long int) zentry->UncompressedSize,
           (long long int) zentry->CompressedSize,
           (100.0 * zentry->CompressedSize / zentry->UncompressedSize));

  /* Many more files/entries can be added to the ZIP archive ... */

  /* Finish ZIP archive */
  if ( zs_finish (zstream, &writestatus) )
    {
      fprintf (stderr, "Error finishing ZIP archive (writestatus: %lld)\n",
               (long long int) writestatus);
      return 1;
    }

  fprintf (stderr, "Success, created archive with %d entries\n",
           zstream->EntryCount);

  /* Cleanup */
  zs_free (zstream);

  return 0;
}
