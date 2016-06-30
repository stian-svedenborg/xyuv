===================================================
||             CrossYUV Library                  ||
===================================================

Thank you for downloading the CrossYUV library, this
package contains rules to build both a static and a
shared library, together with a tool to create and
manage .xyuv files.

===================================================
||                   License                     ||
===================================================

The CrossYUV library and tools are licensed under the
MIT License, outlined below.
Furthermore, the CrossYUV source includes the rapidjson
library, also released under the MIT License. A copy
of the license may be found in the file LICENSE included
in this bundle.

===================================================
||                Installation                   ||
===================================================
Start by cloning the repository (recursively to bring
in the dependencies as well):

$ git clone --recursive https://github.com/stian-svedenborg/xyuv.git
$ mkdir xyuv_build
$ cd xyuv_build
$ cmake ../xyuv
$ make
# make install

You may also need to run ldconfig to update the shared library cache.
# ldconfig

The default installation directory can be overridden
by passing -DCMAKE_INSTALL_PREFIX:PATH=/path/to/somewhere
to cmake.

===================================================
||                Dependencies                   ||
===================================================
Requirements:
 - CMake 3.2
 - Rapidjson (submodule in source)
 - googletest (submodule in source)

For changing the minicalc internal expression parser
you may also require the following tools:
 - re2c lexer generator.
 - The Lemon parser generator (svn version 3080900 in tree).

