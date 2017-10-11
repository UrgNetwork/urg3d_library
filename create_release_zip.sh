#!/bin/sh

RELEASE_DIR=urg3d_library-1.0.3
PACKAGE_NAME=${RELEASE_DIR}.zip

copy_files () {
    targetDIR=${@:$#:1}
    targetFILE=${@:$#-1:1}
    echo targetDIR = ${targetDIR} targetFILE = ${targetFILE}
    if [ -e $1 ]; then
        for path in ${@:1:$#-2}
        do
            echo mkdir -p /${path}
            mkdir -p ${targetDIR}/${path}
            cp -rf ${path}/${targetFILE} ${targetDIR}/${path}
        done
    fi
}

rm -rf ${PACKAGE_NAME} ${RELEASE_DIR}
mkdir ${RELEASE_DIR}
copy_files include *.h ${RELEASE_DIR}
copy_files samples *.c ${RELEASE_DIR}
copy_files samples *.sh ${RELEASE_DIR}
copy_files samples Makefile ${RELEASE_DIR}
copy_files src *.c ${RELEASE_DIR}
copy_files vs20*/high* *.vcxproj ${RELEASE_DIR}
copy_files vs20*/low* *.vcxproj ${RELEASE_DIR}
copy_files vs20*/urg3d *.vcxproj ${RELEASE_DIR}
copy_files vs20* *.sln ${RELEASE_DIR}
copy_files windowsexe *.bat ${RELEASE_DIR}
cp build_rule.mk *.txt Makefile *.pri *.in ${RELEASE_DIR}
zip ${PACKAGE_NAME} ${RELEASE_DIR} -r
