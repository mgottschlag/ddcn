
TARGET=i686-linux-gnu
PREFIX=/opt/ddcn_toolchains
GCC_VER=4.4.6
GCC_SHORT_VER=4.4
BINUTILS_VER=2.19
NEWLIB_VER=1.16.0
GLIBC_VER=2.14

mkdir build_toolchain
cd build_toolchain

# Download files
if [ ! -e gcc-$GCC_VER.tar.bz2 ];
then
	wget ftp://ftp.fu-berlin.de/unix/languages/gcc/releases/gcc-$GCC_VER/gcc-$GCC_VER.tar.bz2
fi
if [ ! -e binutils-$BINUTILS_VER.tar.bz2 ];
then
	wget http://ftp.gnu.org/gnu/binutils/binutils-$BINUTILS_VER.tar.bz2
fi
#if [ ! -e newlib-$NEWLIB_VER.tar.gz ];
#then
#	wget ftp://sources.redhat.com/pub/newlib/newlib-$NEWLIB_VER.tar.gz
#fi
if [ ! -e glibc-$GLIBC_VER.tar.bz2 ];
then
	wget http://ftp.gnu.org/gnu/glibc/glibc-$GLIBC_VER.tar.bz2
fi


# Unpack files
if [ ! -e gcc-$GCC_VER ];
then
	tar -xvf gcc-$GCC_VER.tar.bz2
fi
if [ ! -e binutils-$BINUTILS_VER ];
then
	tar -xvf binutils-$BINUTILS_VER.tar.bz2
fi
#if [ ! -e newlib-$NEWLIB_VER ];
#then
#	tar -xvf newlib-$NEWLIB_VER.tar.gz
#fi
if [ ! -e glibc-$GLIBC_VER ];
then
	tar -xvf glibc-$GLIBC_VER.tar.bz2
fi

# Create build directories
rm -r build-gcc build-binutils build-newlib build-glibc
mkdir build-gcc build-binutils build-newlib build-glibc

cd build-binutils
../binutils-$BINUTILS_VER/configure --target=$TARGET --prefix=$PREFIX --disable-werror
make all -j4
make install

cd ../build-gcc
../gcc-$GCC_VER/configure --target=$TARGET --prefix=$PREFIX \
--enable-languages=c,c++ \
--with-headers=../glibc-$GLIBC_VER/include \
--program-suffix=-$GCC_SHORT_VER \
--program-prefix=$TARGET-
make all-gcc install-gcc

#cd ../build-newlib
#../newlib-$NEWLIB_VER/configure --target=$TARGET --prefix=$PREFIX

#make all
#make install

#cd ../build-gcc
#make all install

cd ..

