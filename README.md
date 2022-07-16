# elfy

A tool for displaying information about ELF files.

It currently support parsing the:

+ file header
+ program headers
+ section headers
+ dynamic section
+ symbol table
+ dynamic symbol table

## Installation

### Arch Linux

[**elfy**](https://aur.archlinux.org/packages/elfy) package from AUR

```text
git clone https://aur.archlinux.org/elfy.git
cd elfy
makepkg -si
```

### Fedora Linux

[**elfy**](https://copr.fedorainfracloud.org/coprs/xfgusta/elfy/) package from Copr

```text
dnf copr enable xfgusta/elfy
dnf install elfy
```

### From source

**Requirements**

+ make
+ gcc
+ libelf

The install directory defaults to `/usr/local`:

```text
make install
```

You can install **elfy** in a different directory using the `PREFIX` variable:

```text
make PREFIX=/usr install
```

## Screenshot

![screenshot](screenshot.png)

## License

Copyright (c) 2022 Gustavo Costa. Distributed under the MIT license.
