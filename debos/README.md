The purpose of this document is basically explain how to create a `Debian` image with the modifications necessary to enable the development of the oz2 software.

The `Debian` image is assembled using the [debos](https://github.com/go-debos/debos) utility, which uses the `Debian` package feed beneath. Stuff not available in official `Debian` packages will be built from sources or downloaded into the final image.

# INSTALL DEBOS (under Debian)

To install [debos](https://github.com/go-debos/debos) you can do the following steps:

```sh
$ sudo apt install golang git libglib2.0-dev libostree-dev qemu-system-x86 qemu-user-static debootstrap systemd-container xz-utils bmap-tools

$ export GOPATH=`pwd`/gocode
$ go get -u github.com/go-debos/debos/cmd/debos
```

# USAGE

Example run:

```
$GOPATH/bin/debos debos/debimage.yaml
```

will create two outputs:

- debian-sid-oz2-boneblack-armhf.img.gz, a gz-compressed image file for a Beaglebone Black board.
- debian-sid-oz2-boneblack-armhf.img.bmap, a bitmap summary for faster flashing via bmaptools

To flash it, assuming your SD card is /dev/mmcblk0, use:

```
bmaptool copy debian-sid-oz2-boneblack-armhf.img.gz /dev/mmcblk0
```

The bmap file is automatically looked for in the current directory.

