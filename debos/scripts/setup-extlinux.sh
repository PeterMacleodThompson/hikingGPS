#!/bin/bash

set -e
set -x

if [ -z  "${ROOTDIR}" ] ; then
  echo "ROOTDIR not given"
  exit 1
fi

cat ${ROOTDIR}/boot/extlinux/extlinux.conf 
sed -i "/append/c\        append console=ttyO0,115200 rootwait rw root=/dev/mmcblk0p1 rootfstype=ext4" ${ROOTDIR}/boot/extlinux/extlinux.conf
cat ${ROOTDIR}/boot/extlinux/extlinux.conf 

