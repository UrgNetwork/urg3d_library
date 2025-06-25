#!/bin/sh

if [ "${MSYSTEM}" = "MINGW32" ] || [ "${MSYSTEM}" = "MINGW64" ] || [ "${MSYSTEM}" = "MSYS" ] ; then
  echo "-lws2_32"
fi
