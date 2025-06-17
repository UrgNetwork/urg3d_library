#!/bin/sh

if [ "${MSYSTEM}" = "MINGW32" ] || [ "${MSYSTEM}" = "MSYS" ] ; then
  echo "-lws2_32"
fi
