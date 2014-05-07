# Copyright 2012 Cloudera Inc.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

cmake_minimum_required(VERSION 2.6)
project(thirdparty)

include(ExternalProject)

set(THIRDPARTY_ROOT_DIR ${CMAKE_SOURCE_DIR})
set(THIRDPARTY_INSTALL ${THIRDPARTY_ROOT_DIR}/install)

set(THRIFT_DIR ${THIRDPARTY_ROOT_DIR}/thrift-0.9.0)
ExternalProject_Add(thrift
  PREFIX ${THRIFT_DIR}
  DOWNLOAD_COMMAND ""
  DOWNLOAD_DIR ${THRIFT_DIR}
  SOURCE_DIR ${THRIFT_DIR}
  CONFIGURE_COMMAND JAVA_PREFIX=${THRIFT_DIR}/java
    PY_PREFIX=${THRIFT_DIR}/python ./configure --with-pic
    --prefix=${THRIFT_DIR}
    --with-php=no --with-java=no --with-perl=no --with-erlang=no
    --with-ruby=no --with-haskell=no --with-d=no --with-qt4=no
    --with-libevent=no
  BUILD_COMMAND make
  BUILD_IN_SOURCE 1
  INSTALL_COMMAND make install)

set(FB303_DIR ${THRIFT_DIR}/contrib/fb303)
ExternalProject_Add(fb303
  DEPENDS thrift
  PREFIX ${FB303_DIR}
  DOWNLOAD_COMMAND cd ${FB303_DIR} && chmod 755 bootstrap.sh
  DOWNLOAD_DIR ${FB303_DIR}
  SOURCE_DIR ${FB303_DIR}
  CONFIGURE_COMMAND CPPFLAGS=-I${THRIFT_DIR}/include
    PY_PREFIX=${THRIFT_DIR}/python ./bootstrap.sh --prefix=${THRIFT_DIR}
    --with-thriftpath=${THRIFT_DIR}
  BUILD_COMMAND make
  BUILD_IN_SOURCE 1
  INSTALL_COMMAND make install)

set(GFLAGS_DIR ${THIRDPARTY_ROOT_DIR}/gflags-2.0)
ExternalProject_Add(gflags
  PREFIX ${GFLAGS_DIR}
  DOWNLOAD_COMMAND ""
  DOWNLOAD_DIR ${GFLAGS_DIR}
  SOURCE_DIR ${GFLAGS_DIR}
  CONFIGURE_COMMAND ./configure --with-pic --prefix=${THIRDPARTY_INSTALL}
  BUILD_COMMAND make
  BUILD_IN_SOURCE 1
  INSTALL_COMMAND make install)

# Google perf tools indicates that adding --enable-frame-pointers might be
# necessary on 64-bit systems. We're not compiling the rest of our code to
# not omit frame pointers but it still seems to generate useful profiling data.
set(PPROF_DIR ${THIRDPARTY_ROOT_DIR}/gperftools-2.0)
ExternalProject_Add(pprof
  PREFIX ${PPROF_DIR}
  DOWNLOAD_COMMAND ""
  DOWNLOAD_DIR ${PPROF_DIR}
  SOURCE_DIR ${PPROF_DIR}
  CONFIGURE_COMMAND ./configure --enable-frame-pointers --with-pic
  BUILD_COMMAND make
  BUILD_IN_SOURCE 1
  INSTALL_COMMAND "")

# SLES's gcc45-c++ is required for sse2 suport (default is 4.3), but crashes
# when building logging_unittest-logging_unittest.o. Telling it to use the
# stabs format for debugging symbols instead of dwarf exercises a different
# code path to work around this issue.
set(GLOG_DIR ${THIRDPARTY_ROOT_DIR}/glog-0.3.2)
ExternalProject_Add(glog
  DEPENDS gflags
  PREFIX ${GLOG_DIR}
  DOWNLOAD_COMMAND ""
  DOWNLOAD_DIR ${GLOG_DIR}
  SOURCE_DIR ${GLOG_DIR}
  CONFIGURE_COMMAND ./configure --with-pic --with-gflags=${THIRDPARTY_INSTALL} &&
    echo logging_unittest-logging_unittest.o: CXXFLAGS=-gstabs -O2 >> Makefile.gcc45sles_workaround &&
    cat Makefile >> Makefile.gcc45sles_workaround &&
    mv Makefile.gcc45sles_workaround Makefile
  BUILD_COMMAND make -j4
  BUILD_IN_SOURCE 1
  INSTALL_COMMAND "")

set(GTEST_DIR ${THIRDPARTY_ROOT_DIR}/gtest-1.6.0)
ExternalProject_Add(gtest
  PREFIX ${GTEST_DIR}
  DOWNLOAD_COMMAND ""
  DOWNLOAD_DIR ${GTEST_DIR}
  SOURCE_DIR ${GTEST_DIR}
  CONFIGURE_COMMAND cmake .
  BUILD_COMMAND make -j4
  BUILD_IN_SOURCE 1
  INSTALL_COMMAND "")

set(SNAPPY_DIR ${THIRDPARTY_ROOT_DIR}/snappy-1.0.5)
ExternalProject_Add(snappy
  PREFIX ${SNAPPY_DIR}
  DOWNLOAD_COMMAND ""
  DOWNLOAD_DIR ${SNAPPY_DIR}
  SOURCE_DIR ${SNAPPY_DIR}
  CONFIGURE_COMMAND libtoolize -f && ./configure --with-pic --prefix=${SNAPPY_DIR}/build
  BUILD_COMMAND make install
  BUILD_IN_SOURCE 1
  INSTALL_COMMAND "")

set(RE2_DIR ${THIRDPARTY_ROOT_DIR}/re2)
ExternalProject_Add(re2
  PREFIX ${RE2_DIR}
  DOWNLOAD_COMMAND ""
  DOWNLOAD_DIR ${RE2_DIR}
  SOURCE_DIR ${RE2_DIR}
  CONFIGURE_COMMAND ""
  BUILD_COMMAND make -j4
  BUILD_IN_SOURCE 1
  INSTALL_COMMAND "")

set(LDAP_DIR ${THIRDPARTY_ROOT_DIR}/openldap-2.4.25)
ExternalProject_Add(ldap
  PREFIX ${LDAP_DIR}
  DOWNLOAD_COMMAND ""
  DOWNLOAD_DIR ${LDAP_DIR}
  SOURCE_DIR ${LDAP_DIR}
  CONFIGURE_COMMAND ./configure --enable-slapd=no
    --prefix=${THIRDPARTY_INSTALL} --enable-static --with-pic
  BUILD_COMMAND make -j4 && make -j4 depend
  BUILD_IN_SOURCE 1
  INSTALL_COMMAND make install)

set(SASL_DIR ${THIRDPARTY_ROOT_DIR}/cyrus-sasl-2.1.23)
ExternalProject_Add(sasl
  PREFIX ${SASL_DIR}
  DOWNLOAD_COMMAND ""
  DOWNLOAD_DIR ${SASL_DIR}
  SOURCE_DIR ${SASL_DIR}
  CONFIGURE_COMMAND ./configure
    --disable-sql --disable-otp --disable-ldap --disable-digest
    --with-saslauthd=no --prefix=${SASL_DIR}/build --enable-static
    --enable-staticdlopen CFLAGS=-fPIC\ -DPIC CXXFLAGS=-fPIC\ -DPIC
  BUILD_COMMAND make || true
  BUILD_IN_SOURCE 1
  INSTALL_COMMAND make install)

set(AVRO_C_DIR ${THIRDPARTY_ROOT_DIR}/avro-c-1.7.4)
ExternalProject_Add(avro-c
  PREFIX ${AVRO_C_DIR}
  DOWNLOAD_COMMAND ""
  DOWNLOAD_DIR ${AVRO_C_DIR}
  CONFIGURE_COMMAND cmake .
  SOURCE_DIR ${AVRO_C_DIR}
  BUILD_COMMAND make -j4
  BUILD_IN_SOURCE 1
  INSTALL_COMMAND "")

set(CDH4_EXTRAS_DIR ${THIRDPARTY_ROOT_DIR}/cdh4-extras)
ExternalProject_Add(cdh4-extras
  PREFIX ${CDH4_EXTRAS_DIR}
  DOWNLOAD_COMMAND ""
  DOWNLOAD_DIR ${CDH4_EXTRAS_DIR}
  CONFIGURE_COMMAND ""
  SOURCE_DIR ${CDH4_EXTRAS_DIR}
  BUILD_COMMAND mvn package -DskipTests
  BUILD_IN_SOURCE 1
  INSTALL_COMMAND "")
