#! /bin/sh

# Because of the autotools bug
cd ZenLib/Project/GNU/Library
./autogen.sh
cd ../../../../MediaInfoLib/Project/GNU/Library
./autogen.sh
cd ../../../../dvrescue/Project/GNU/CLI
./autogen.sh
cd ../../../..

./CLI_Compile.sh --enable-arch-x86_64
