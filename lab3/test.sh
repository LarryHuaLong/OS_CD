#2.编译测试程序
gcc hualongtest.c -o hualongtest
#3.安装模块
sudo insmod hualong.ko
#4.查看安装日志
dmesg -T -l 1
#5.查看是否成功加载模块
cat /proc/devices |grep 231
#6.新建对应设备文件
sudo mknod /dev/hualong c 231 0
#7.查看设备文件是否建立成
ll /dev/hualong
#8.运行测试程序
./hualongtest
#9.卸载设备
sudo rmmod hualong
#10.查看设备是否成功卸载
cat /proc/devices |grep hualong
#11.删除字符设备
rm -f /dev/hualong
#12查看字符设备是否被删除
ll /dev/hualong


