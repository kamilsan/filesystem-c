## File system

This repository contains an implementation of a very basic file system, that I've made for Operating Systems course. It is probably very unsafe, poorly written and contains bugs so **please don't use it for anything serious**.

Here are some features of this file system:
  * continuous file allocation (no blocks of constant sizes)
  * file name must have 20 characters or less
  * all sizes and offsets must fit into 64-bit number; there is no direct limitation on file size or similar
  * support for nested directories and links
  * avoids fragmentation by consolidating neighboring free segments

Very user friendly command line interface is also provided. 

### Build instructions

```bash
make
./program
```
