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
in the dependencies as well:

$ git clone https://github.com/stian-svedenborg/xyuv.git
$ mkdir xyuv_build
$ cd xyuv_build
$ cmake ../xyuv
$ make


===================================================
||                Dependencies                   ||
===================================================
Requirements:
 - CMake 3.2
 - Rapidjson (version xx included in the source).
 - (Optional) gtest testing framework.

For changing the minicalc internal expression parser
library you may also require the following tools:
 - re2c lexer generator.
 - The Lemon parser generator (svn version 3080900 in tree).

