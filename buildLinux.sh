#/bin/bash

WORK_DIR=$(cd $(dirname $0); pwd)
INSTALL_PATH=${WORK_DIR}/bin/linux

buildFunc(){
    echo "---------------"
    echo "Start building $1 ..."
    cd ${WORK_DIR}/LocalCTP/ctp_file
    rm -fr ./current
    cp -fr $1 ./current
    cd ${WORK_DIR}/GenScript
    python3 ParseCTPHeaders.py
    cd ${WORK_DIR}/LocalCTP
    make clean && make -j 30
    mv ${INSTALL_PATH}/libthosttraderapi_se.so ${INSTALL_PATH}/libthosttraderapi_se_v$1.so
}

buildDemoFunc(){
    echo "---------------"
    echo "Start building DEMO ..."
    cd ${WORK_DIR}/TestLocalCTP
    make clean && make -j 30
}

# choose the CTP version you use

# v6.3.15
#buildFunc 6.3.15

# v6.3.19
#buildFunc 6.3.19

# v6.6.1
#buildFunc 6.6.1

# v6.6.9
#buildFunc 6.6.9

# v6.7.0
#buildFunc 6.7.0

# v6.7.2
#buildFunc 6.7.2

# v6.7.7
#buildFunc 6.7.7

# v6.7.8
#buildFunc 6.7.8

# v6.5.1, put it at last, because we want the code to be set as v6.5.1 finally
buildFunc 6.5.1

cp ${INSTALL_PATH}/libthosttraderapi_se_v6.5.1.so ${INSTALL_PATH}/libthosttraderapi_se.so

buildDemoFunc

