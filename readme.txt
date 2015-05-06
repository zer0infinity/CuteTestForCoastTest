Testframework-Migration for COAST

Copyright © 2015

by

David Tran, Faculty of Computer Science,
University of Applied Sciences Rapperswil (HSR),
8640 Rapperswil, Switzerland


1. About
Open source C++ application server based on work done at Ubilab by André Weinand, Erich Gamma


2. Features
- COAST (C++ Open Application Server Toolkit)
* CUTE (C++ Unit Testing Easier)

3. System Requirements
- Ubuntu, Ubuntu x64
- Debian, Debian x64

4. Installation
- Extract the zip file
- In Terminal:
* sudo apt-get install build-essential gcc-4.9-multilib g++-4.9-multilib git libboost-all-dev
* export PIP_CERT=$HOME/coast-project.pem
* export PIP_INDEX_URL=https://devpi.coast-project.org/coast/CoastSconsider/+simple/
* git clone https://github.com/pypa/virtualenv
* python virtualenv/virtualenv.py --python=python2.7 .venv27scons
* . .venv27scons/bin/activate
* export GIT_SSL_NO_VERIFY=true
* git clone https://gerrit.coast-project.org/r/p/boost.git 3rdparty/boost
* git clone https://gerrit.coast-project.org/r/p/zlib.git 3rdparty/zlib
* pip install --use-wheel --requirement *require*.txt
* scons -u --jobs=4 --with-src-zlib=3rdparty/zlib --with-src-boost=3rdparty/boost --run-force CuteFoundationBaseTest

5. Known issues
-

6. Credits
David Tran (HSR), 2015

7. Thanks
Mirko Stocker (m1stocke@hsr.ch), HSR, IFS, 2015
Marcel Huber (m1huber@hsr.ch), HSR, IFS, 2015

8. Website
http://www.coast-project.org/
http://www.cute-test.com/

9. Licence
Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the distribution.
    * Neither the name of IFS INSTITUTE FOR SOFTWARE nor the
      names of its contributors may be used to endorse or promote products
      derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR
CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
