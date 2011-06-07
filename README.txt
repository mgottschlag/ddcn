
 Dynamic Distributed Compiler Network
======================================

1) Build

To build the program in a build/ subdirectory run the following commands:

mkdir build
cd build
cmake ..
make

Optionally, to install the programs system-wide:

sudo make install

ccmake or cmake-gui can be used to enable/disable parts of the program.

2) Dependencies

Necessary for compiling the program are:

- Qt 4.x (TODO: Insert correct version number here, only for building
  ddcn_control)
- TODO: SpoVNet libraries, MCPO

3) Directory structure

docs/
  Various documentation about the interfaces
ddcn_service/
  Service which handles and forwards local compiler jobs and executes remote
  jobs. Can be talked to via DBUS.
ddcn_control/
  Graphical interface to show the status of the compiler network and change
  settings.
ddcn_gcc/
  Drop-in replacement for the GCC C compiler. Can be used for compiling via
  "CC=ddcn_gcc make -j20" (where a high number of parallel jobs is chosen
  to enable a high degree of parallelization via the network).
ddcn_make/
  Wrapper for GNU make which sets the environment variables necessary to
  use ddcn_gcc before starting GNU make.

