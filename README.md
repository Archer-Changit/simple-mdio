1.编译说明
  1.1 编译模块（smdio.ko）
    1）进入ko/kernel目录
    2）执行make，即可生成smdio.ko

    注意：a.对于想使用指定版本内核，编译时，可如此操作：make KDIR=/lib/modules/6.8.0-107-generic/build
         b.对于交叉编译，可能需要指明内核源代码树的路径（或者说安装路径），如：make KDIR=/home/ag/src/myir-imx-linux
  
  1.2 编译可执行文件（smdio）
    1）进入smdio目录
    2）执行make，即可生成用户程序smdio
  
2.使用说明
  1）插入KO文件
    insmod smdio.ko

    此操作会生成/dev/smdio字符设备文件。
    
  2）执行smdio，操作以太网PHY寄存器
     示例：smdio r stmmac-1 0x01 0x1e 0x0180

3. 其他事项
   1）ko/kernel/compat.h（若有版本保护请联系删除！！！）
     该文件直接来源于mdio-tools项目（https://github.com/wkz/mdio-tools），仅是为了兼容Linux 6.0版本前的mdio相关接口。将该文件删除，不影响使用。
   2）你的设备有哪些MDIO Bus
     ls -l /sys/bus/mdio_bus/devices/
  
