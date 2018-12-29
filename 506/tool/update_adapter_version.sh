#!/bin/bash

proj_path_nv=$(cd $(dirname $0); pwd)

GADGET_TYPE_ID=`cat $proj_path_nv/param_tool/dev.conf | jq '.gadget_type_id'`
echo "Current GADGET_TYPE_ID : $GADGET_TYPE_ID"
ADA_VERSION=`cat $proj_path_nv/product/version.h |grep ADA_VERSION| awk '{print $3}'`
echo "Current ADA VERSION : $ADA_VERSION"

((UPDATE_ADA_VERSION=${ADA_VERSION}+1))
sed -i s/$ADA_VERSION/$UPDATE_ADA_VERSION/g $proj_path_nv/product/version.h
ADA_VERSION=`cat $proj_path_nv/product/version.h |grep ADA_VERSION| awk '{print $3}'`
echo "UPDATE ADA VERSION TO : $ADA_VERSION"

cd product
GIT_BRANCH=$1
git add $proj_path_nv/product/version.h
git commit -s -m "update $GADGET_TYPE_ID project adapter version to $ADA_VERSION"
git push RTK8710 HEAD:refs/for/$GIT_BRANCH

