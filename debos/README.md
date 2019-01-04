The purpose of this document is basically explain how to create a `Debian` image with the modifications necessary to enable the development of the oz2 software.

The `Debian` image is assembled using the [debos](https://github.com/go-debos/debos) utility, which uses the `Debian` package feed beneath. Stuff not available in official `Debian` packages will be built from sources or downloaded into the final image.

# INSTALL DEBOS (under Debian)

To install [debos](https://github.com/go-debos/debos) you can do the following steps:

```sh
$ sudo apt install golang git libglib2.0-dev libostree-dev qemu-system-x86 qemu-user-static debootstrap systemd-container xz-utils bmap-tools

$ export GOPATH=`pwd`/gocode
$ go get -u github.com/go-debos/debos/cmd/debos
```

# Build an image and run it in the QEMU emulator.

First, make sure you have KVM installed:

```sh
$ sudo apt install qemu-kvm ovmf
```

Now that that’s done, let’s create the images, run:

```sh
$GOPATH/bin/debos -t suite:sid debos/debian-base.yaml
$GOPATH/bin/debos -t suite:sid debos/debian-uefi.yaml
```

Will create the following output:

- debian-base-sid-amd64.tar.gz, a tarball with the debian base filesystem.
- debian-uefi-sid-amd64.img, an image file for QEMU emulator.
- debian-uefi-sid-amd64.vdi, a VirtualBox image.

The final command runs the image using the QEMU emulator:

```sh
$ scripts/qemu-boot-efi debian-uefi-sid-amd64.img
```

# Build an image and flash it into the Beaglebone Black.

To create the images, run:

```sh
$GOPATH/bin/debos -t suite:sid debos/debian-base.yaml
$GOPATH/bin/debos -t suite:sid debos/debian-boneblack.yaml
```

Will create the following output:

- debian-base-sid-armhf.tar.gz, a tarball with the debian base filesystem.
- debian-boneblack-sid-armhf.img.gz, a gz-compressed image file for a Beaglebone Black board.
- debian-boneblack-sid-armhf.img.gz.md5, the image checksum.
- debian-boneblack-sid-armhf.img.bmap, a bitmap summary for faster flashing via bmaptools.

To flash it, assuming your SD card is /dev/mmcblk0, use:

```
bmaptool copy debian-boneblack-sid-armhf.img.gz /dev/mmcblk0
```

The bmap file is automatically looked for in the current directory.

