#!/bin/bash
proj_path_nv=$(pwd)

RELEASE_DATE=`date +%Y-%m-%d`

jq -h >/dev/null
if [ $? != 0 ];
then
   echo "The program 'jq' is currently not installed. You can install it by typing: sudo apt install jq"
   exit
fi

echo "Build Rtos Project ..."
ADA_VERSION=`cat $proj_path_nv/product/version.h |grep ADA_VERSION| awk '{print $3}'`
echo "Current ADA VERSION : $ADA_VERSION"
SDK_VERSION=`cat $proj_path_nv/iot_sdk/sdk_version.h |grep SDK_VERSION| awk '{print $3}'`
echo "Current SDK VERSION : $SDK_VERSION"
MODULE_TYPE=`cat $proj_path_nv/param_tool/dev.conf | jq '.module_type'|awk -F '["]' '{print$2}'`
echo "Current MODULE_TYPE : $MODULE_TYPE"
GADGET_TYPE_ID=`cat $proj_path_nv/param_tool/dev.conf | jq '.gadget_type_id'`
echo "Current GADGET_TYPE_ID : $GADGET_TYPE_ID"

OUT_DIR=$proj_path_nv/out
IMG_DIR=$proj_path_nv/image

rm $OUT_DIR -rf
rm $IMG_DIR -rf
mkdir -p $OUT_DIR
mkdir -p $IMG_DIR

cd $IMG_DIR
wget -q http://10.103.67.213/rtk8710/bin/bootload/boot_all.bin
[ $? -ne 0 ] && echo "Download boot_all.bin failed." && exit 1

wget -q http://10.103.67.213/rtk8710/bin/bootload/system-2M-2.bin
[ $? -ne 0 ] && echo "Download system-2M-2.bin failed." && exit 1

wget -q http://10.103.67.213/rtk8710/bin/image/factory_mp_0.0.1.bin
[ $? -ne 0 ] && echo "Download system-2M-2.bin failed." && exit 1

cp $proj_path_nv/LeIotProjLib/Debug/Exe/image2_all_ota1.bin $IMG_DIR
[ $? -ne 0 ] && echo "Copy image2_all_ota1.bin failed." && exit 1

cd $proj_path_nv
echo "Current Path : $proj_path_nv"

echo "----------------------------------Start combine------------------------------------------------"
for CLOUD in dev pvt api
do
	#CLOUD=`cat $proj_path_nv/param_tool/dev.conf | jq '.cloud'`
	echo "Current CLOUD : $CLOUD"

	echo "Param Rtos Project ..."
	cd $proj_path_nv/param_tool
	./param $CLOUD $proj_path_nv/param_tool/dev.conf param_$CLOUD.bin

	if [ $? != 0 ];
	then
	   echo "Param Rtos Project Error !!!"
	   exit
	fi
        mv $proj_path_nv/param_tool/param_$CLOUD.bin $IMG_DIR

        cd $proj_path_nv/combine_tool
	./combine combine_$CLOUD.conf
        mv combine.bin $OUT_DIR/${MODULE_TYPE}_${GADGET_TYPE_ID}_${CLOUD}_${SDK_VERSION}_${ADA_VERSION}.bin
done
echo "----------------------------------Combine success------------------------------------------------"

echo "----------------------------------Copy ota_all.bin------------------------------------------------"
cp $proj_path_nv/LeIotProjLib/Debug/Exe/ota_all.bin $OUT_DIR/update_${GADGET_TYPE_ID}_${ADA_VERSION}.bin
[ $? -ne 0 ] && echo "Copy ota_all.bin failed." && exit 1

echo "----------------------------------Make zip file------------------------------------------------"

cd $OUT_DIR
md5sum * > md5sum

cd $proj_path_nv
rm out.zip
zip -r out.zip out

DIR=RTK8710/Release/${GADGET_TYPE_ID}/${RELEASE_DATE}_${ADA_VERSION}

$proj_path_nv/tool/scp_to_ftp.sh $DIR out

echo "----------------------------------Update to ftp success------------------------------------------------"
echo ftp://10.103.67.198/$DIR

rm out.zip

UPDATE_BIN_MD5=`cat ${OUT_DIR}/md5sum |grep update_${GADGET_TYPE_ID}_${ADA_VERSION}.bin| awk '{print $1}'`
$proj_path_nv/tool/upload_image_to_server.sh ${OUT_DIR}/update_${GADGET_TYPE_ID}_${ADA_VERSION}.bin ${GADGET_TYPE_ID} ${ADA_VERSION} ${UPDATE_BIN_MD5} gadget

#$proj_path_nv/tool/lftp_put_upgrade_file.sh $OUT_DIR $GADGET_TYPE_ID update_${GADGET_TYPE_ID}_${ADA_VERSION}.bin
#local-path, remote-path, files
#if [ $? != 0 ];
#then
#   echo "lftp upload file fail, so that stop build projects"
#   exit
#fi
