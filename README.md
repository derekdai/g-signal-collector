g-signal-collector
==================

A simple utility to collect GObject signals

If you wish to trace the signal emitted by gobjects, try my g-signal-collector (Linux only).

How to install
==================
First, you have to clone the source tree from github

$ git clone git@github.com:derekdai/g-signal-collector.git

 Generate makefiles and build (cmake must be installed prior this step)

$ mkdir g-signal-collector-build

$ cmake ../g-signal-collector
-- The C compiler identification is GNU 4.7.2
-- The CXX compiler identification is GNU 4.7.2
-- Check for working C compiler: /usr/bin/gcc
-- Check for working C compiler: /usr/bin/gcc -- works
-- Detecting C compiler ABI info
-- Detecting C compiler ABI info - done
-- Check for working CXX compiler: /usr/bin/c++
-- Check for working CXX compiler: /usr/bin/c++ -- works
-- Detecting CXX compiler ABI info
-- Detecting CXX compiler ABI info - done
-- Found PkgConfig: /usr/bin/pkg-config (found version "0.26")
-- checking for module 'gobject-2.0'
--   found gobject-2.0, version 2.34.1
-- checking for module 'clutter-1.0'
--   found clutter-1.0, version 1.12.0
-- checking for module 'gtk+-3.0'
--   found gtk+-3.0, version 3.6.0
-- Configuring done
-- Generating done
-- Build files have been written to: /home/derekdai/Projects/g-signal-collector-build

$ make
Scanning dependencies of target g-signal-collector
[ 16%] Building C object src/CMakeFiles/g-signal-collector.dir/collector.c.o
[ 33%] Building C object src/CMakeFiles/g-signal-collector.dir/signal-info.c.o
[ 50%] Building C object src/CMakeFiles/g-signal-collector.dir/signal-info-pool.c.o
[ 66%] Building C object src/CMakeFiles/g-signal-collector.dir/signal-info-pool-dumper.c.o
Linking C shared library libg-signal-collector.so
[ 66%] Built target g-signal-collector
Scanning dependencies of target clutter-rectangles
[ 83%] Building C object test/CMakeFiles/clutter-rectangles.dir/clutter-rectangles.c.o
Linking C executable clutter-rectangles
[ 83%] Built target clutter-rectangles
Scanning dependencies of target gtk-rectangles
[100%] Building C object test/CMakeFiles/gtk-rectangles.dir/gtk-rectangles.c.o
Linking C executable gtk-rectangles
[100%] Built target gtk-rectangles

If build successively, you can find a .so in the src/ named libg-signal-collector.so

$ ls src
CMakeFiles  cmake_install.cmake  libg-signal-collector.so  Makefile

How to use it
==================

Now, you can run any application based on gobject to capture signals
$ LD_PRELOAD=$PWD/src/libg-signal-collector.so gedit

The command above run gedit with g-signal-collector, now you can close gedit and found the result in current directory.

$ ls gedit*
gedit.0.gsc

.gsc file is the text file. You can view it with any text editor you want. File content looks like
0    0x8489500         0.013648     0.000017    0x84a6040    GdkX11Display::opened
1    0x8489500         0.013666     0.019902    0x847e930    GdkX11DisplayManager::display-opened
2    0x8489500         0.014341     0.000001    0x847eb60    &gt;GtkStyleCascade::-gtk-private-changed
3    0x8489500         0.014345     0.000000    0x847eb60    &gt;GtkStyleCascade::-gtk-private-changed
4    0x8489500         0.015162     0.000001    0x84bc2e8    &gt;GSocketClient::event
5    0x8489500         0.015163     0.000001    0x84bc2e8    &gt;GSocketClient::event
6    0x8489500         0.015265     0.000000    0x84bc2e8    &gt;GSocketClient::event

1st column is serial number, 2nd column is the thread ID, 3rd column is timestamp, 4th column is elapsed time, 5th column is the signal description (nest leve&gt;class-name::sign-name).
