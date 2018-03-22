#编译测试程序
gcc hualongtest.c -o hualongtest
#安装模块
sudo insmod mymodule.ko
#查看安装日志
dmesg -T -l 1
#查看是否成功加载模块
cat /proc/devices |grep 231
#新建对应设备文件
sudo mknod /dev/hualong c 231 0
#查看设备文件是否建立成
ll /dev/hualong
#运行测试程序
./hualongtest
#卸载设备
sudo rmmod mymodule
#查看设备是否成功卸载
cat /proc/devices |grep hualong
#删除字符设备
rm -f /dev/hualong
#查看字符设备是否被删除
ll /dev/hualong


