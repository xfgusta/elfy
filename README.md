# elfy

A tool for displaying information about ELF files.

It currently support parsing the:

+ file header
+ program headers
+ section headers
+ dynamic section

Run `elfy -H` to print a description of the command-line options. I plan to add more options soon.

## Building from source

**Requirements**

+ make
+ gcc
+ libelf

```ansi
git clone https://github.com/xfgusta/elfy
cd elfy
make
```
