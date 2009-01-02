#!/bin/sh

intltool-merge -d ../po/ ../src/whiteboard.desktop.in ../src/whiteboard.desktop
dpkg-buildpackage -us -uc -b -rfakeroot
