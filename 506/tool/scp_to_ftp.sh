#!/usr/bin/expect
set destdir "/data/lenovo_ftp"
set dir [lindex $argv 0]
set file [lindex $argv 1]

set serverIp "10.103.67.198"
set serverUser "jenkins"
set serverPwd "jenkins"
set address "~"

spawn scp $file.zip $serverUser@$serverIp:$address
expect {
  "*yes/no*" {send "yes\r"; exp_continue}
  "*password:" {send "$serverPwd\r"; exp_continue}
  "*Password:" {send "$serverPwd\r"}
}
#登录远程服务器
spawn  ssh $serverUser@$serverIp
expect {
  "*yes/no*" {send "yes\r"; exp_continue}
  "*password:" {send "$serverPwd\r"; exp_continue}
  "*Password:" {send "$serverPwd\r"}
}
#进入ftp目录
expect "$"
send "cd $destdir\r"
#创建版本目录
expect "$"
send "mkdir -p $dir\r"
expect "$"
#拷贝
send "mv ~/$file.zip ./$dir\r"
expect "$"

send "cd $dir\r"
expect "$"

send "unzip -o $file.zip\r"
expect "$"

send "chmod 766 $file/ \r"
expect "$"

send "mv $file/* .\r"
expect "$"

send "rm $file* -rf\r"
expect "$"

send "chmod 777 . -R\r"
expect "$"

#退出
send "exit\r"
expect eof
