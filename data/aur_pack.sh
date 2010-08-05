#!/bin/sh
# $1 - package version
# $2 - i686 path
# $3 - x86_64 path
# $4 - output PKGBUILD

if [ ! -f $2 ]; then
	echo "i686 package not built!"
	exit 1
fi
if [ ! -f $3 ]; then
	echo "x86_64 package not built!"
	exit 1
fi
mdsum32_sum=`md5sum $2|cut -f -1 -d ' '`
mdsum64_sum=`md5sum $3|cut -f -1 -d ' '`

echo "Package version: $1"
echo "Md5sum of 32bit: $mdsum32_sum"
echo "Md5sum of 64bit: $mdsum64_sum"

cat <<EOF >$4
# Maintainer: maoserr

pkgname=redshiftgui
pkgver=$1
pkgrel=1
pkgdesc="Monitor color adjustment tool"
arch=('i686' 'x86_64')
url="http://github.com/maoserr/redshiftgui/downloads"
license=('GPL2')
depends=('gtk2' 'curl' 'libxcb' 'libxrandr' 'libxxf86vm')
source=(http://github.com/downloads/maoserr/redshiftgui/RedshiftGUI-\${pkgver}-Linux-\${CARCH}.tar.gz)
md5sums=('$mdsum32_sum')
[ \$CARCH = 'x86_64' ] && md5sums=('$mdsum64_sum')

build() {
  cd \$srcdir/RedshiftGUI-\${pkgver}-Linux-\${CARCH}
  mkdir -p \$pkgdir/usr/{bin,share}
  install -m755 bin/redshiftgui \$pkgdir/usr/bin
  mv share/* \$pkgdir/usr/share/
}
EOF
