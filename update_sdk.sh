#!/bin/bash
proj_path_nv=$(cd $(dirname $0); pwd)

GADGET_TYPE_ID=`cat $proj_path_nv/param_tool/dev.conf | jq '.gadget_type_id'`
echo "Current GADGET_TYPE_ID : $GADGET_TYPE_ID"
SDK_VERSION=`cat $proj_path_nv/iot_sdk/sdk_version.h |grep SDK_VERSION| awk '{print $3}'`
echo "Current IOT SDK VERSION : $SDK_VERSION"

rm -fr ledm-rtk8710-latest.tgz
echo "download ledm-rtk8710-latest.tgz"
wget -q http://10.103.67.213/rtk8710/release/ledm-rtk8710-latest.tgz
[ $? -ne 0 ] && echo "download sdk failed." && exit 1

mkdir ledmsdk
tar xzf ledm-rtk8710-latest.tgz -C ./ledmsdk/ --strip-components 1

var=`cat ledmsdk/VERSION`
UPDATE_SDK_VERSION=${var:0:6}
echo "UPDATE IOT SDK VERSION : $UPDATE_SDK_VERSION"

#if [ $UPDATE_ADA_VERSION -gt $SDK_VERSION ];
#then
   rm -rf iot_sdk/lib
   rm -rf iot_sdk/include/
   cp -rf ledmsdk/lib/ iot_sdk/
   cp -rf ledmsdk/include/ iot_sdk/

   sed -i s/$SDK_VERSION/$UPDATE_SDK_VERSION/g $proj_path_nv/iot_sdk/sdk_version.h
   SDK_VERSION=`cat $proj_path_nv/iot_sdk/sdk_version.h |grep SDK_VERSION| awk '{print $3}'`
   cd iot_sdk 
   GIT_BRANCH=`git symbolic-ref --short -q HEAD`
   git add $proj_path_nv/iot_sdk/*
   git commit -s -m "update $GADGET_TYPE_ID project iot sdk version to $SDK_VERSION"
   git push RTK8710 HEAD:refs/for/master

   echo " update sdk success"
   cd ../
   rm -rf ledm-rtk8710-latest.tgz ledmsdk
   echo " remove ledm-rtk8710-latest.tgz ledmsdk"
#fi
