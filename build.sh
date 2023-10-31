#!/bin/bash
#
# Script for Dev's daily work.  It is a good idea to use the exact same
# build options as the released version.

get_key_value()
{
  echo "$1" | sed 's/^--[a-zA-Z_-]*=//'
}

usage()
{
cat <<EOF
Usage: $0 [-t debug|release] [-d <dest_dir>] [-s <server_suffix>]
       Or
       $0 [-h | --help]
  -t                      Select the build type.
  -d                      Set the destination directory.
  -s                      Set the server suffix.
  -h, --help              Show this help message.

Note: this script is intended for internal use by MySQL developers.
EOF
}

parse_options()
{
  while test $# -gt 0
  do
    case "$1" in
    -t=*)
      build_type=`get_key_value "$1"`;;
    -t)
      shift
      build_type=`get_key_value "$1"`;;
    -d=*)
      dest_dir=`get_key_value "$1"`;;
    -d)
      shift
      dest_dir=`get_key_value "$1"`;;
    -s=*)
      server_suffix=`get_key_value "$1"`;;
    -s)
      shift
      server_suffix=`get_key_value "$1"`;;
    -h | --help)
      usage
      exit 0;;
    *)
      echo "Unknown option '$1'"
      exit 1;;
    esac
    shift
  done
}

dump_options()
{
  echo "Dumping the options used by $0 ..."
  echo "build_type=$build_type"
  echo "dest_dir=$dest_dir"
  echo "server_suffix=$server_suffix"
}

if test ! -f sql/mysqld.cc
then
  echo "You must run this script from the MySQL top-level directory"
  exit 1
fi

build_type="debug"
dest_dir=$HOME/tmp_run2
server_suffix="rds-dev"

parse_options "$@"
dump_options

if [ x"$build_type" = x"debug" ]; then
  build_type="Debug"
  debug=1
  debug_sync=1
elif [ x"$build_type" = x"release" ]; then
  # Release CMAKE_BUILD_TYPE is not compatible with mysql 8.0
  # build_type="Release"
  build_type="RelWithDebInfo"
  debug=0
  debug_sync=0
else
  echo "Invalid build type, it must be \"debug\" or \"release\"."
  exit 1
fi

server_suffix="-""$server_suffix"

if [ x"$build_type" = x"Release" ]; then
  COMMON_FLAGS="-O3 -g -fexceptions -fno-omit-frame-pointer -fno-strict-aliasing"
  CFLAGS="$COMMON_FLAGS"
  CXXFLAGS="$COMMON_FLAGS"
elif [ x"$build_type" = x"Debug" ]; then
  COMMON_FLAGS="-O0 -g3 -gdwarf-2 -fexceptions -fno-omit-frame-pointer -fno-strict-aliasing -fprofile-arcs -ftest-coverage"
  CFLAGS="$COMMON_FLAGS"
  CXXFLAGS="$COMMON_FLAGS"
fi

CC=gcc
CXX=g++

export CC CFLAGS CXX CXXFLAGS

rm -rf CMakeCache.txt
make clean
cat extra/boost/boost_1_77_0.tar.bz2.*  > extra/boost/boost_1_77_0.tar.bz2
cmake .                                    \
    -DCMAKE_BUILD_TYPE="$build_type"       \
    -DCMAKE_INSTALL_PREFIX="$dest_dir"     \
    -DCMAKE_COMPILE_COMMANDS=1             \
    -DCMAKE_EXPORT_COMPILE_COMMANDS=1      \
    -DBUILD_CONFIG=xtrabackup_release      \
    -DWITH_DEBUG=$debug                    \
    -DFORCE_INSOURCE_BUILD=1               \
    -DWITH_BOOST="./extra/boost/boost_1_77_0.tar.bz2" \
    -DMYSQL_SERVER_SUFFIX="$server_suffix"

make -j `cat /proc/cpuinfo | grep processor| wc -l`
# end of file
