Nonzip - simple ZIP writing library
====================================

#### Allows opening a file, then writing files to it and closing the ZIP file, while trying to use as little memory as possible, without using any serious hacks.

## Usage:
```C
// Writing a simple zip with one text document:
const char data[] = "test\n1234\nąśćń\n";
nonzip_t z;
z = nonzip_open("test.zip");
nonzip_addfile(&z, "file.txt, data, sizeof(data), time(0));
nonzip_write_close(&z);
```

### nonzip.[ch] license:
    
    Copyright (c) 2023 Michał Stoń <michal.ston@mobica.com>
    
    Permission is hereby granted, free of charge, to any person obtaining a copy of
    this software and associated documentation files (the "Software"), to deal in
    the Software without restriction, including without limitation the rights to
    use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
    the Software, and to permit persons to whom the Software is furnished to do so,
    subject to the following conditions:
    
    The above copyright notice and this permission notice shall be included in all
    copies or substantial portions of the Software.
    
    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
    FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
    COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
    IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
    CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.


### crc32.h license:
Modified for another usecase, original code by Spencer Garett:

    This code implements the AUTODIN II polynomial used by Ethernet,
    and can be used to calculate multicast address hash indices.
    It assumes that the low order bits will be transmitted first,
    and consequently the low byte should be sent first when
    the crc computation is finished.  The crc should be complemented
    before transmission.
    The variable corresponding to the macro argument "crc" should
    be an unsigned long and should be preset to all ones for Ethernet
    use.  An error-free packet will leave 0xDEBB20E3 in the crc.
        Spencer Garrett <srg@quick.com>
