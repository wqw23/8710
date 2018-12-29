#!/bin/bash
#lftp weixya_ftp:FiaeIfa92u3_3dd@54.222.179.232:22021

host="ftp://54.222.179.232:22021"
username="weixya_ftp"
password="FiaeIfa92u3_3dd"
local_path=$1
remote_path="./$2"
file="$3"

lftp -c "open $host
user $username $password && \
lcd $local_path && \
cd $remote_path && \
put $file "
