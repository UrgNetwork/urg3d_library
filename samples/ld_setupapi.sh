#!/bin/sh

if [ "${MSYSTEM}" = "MINGW32" ] || [ "${MSYSTEM}" = "MSYS" ] ; then
  echo "-lsetupapi"
fi
