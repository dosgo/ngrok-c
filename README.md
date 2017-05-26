## ngrok-c

- the client of ngrok in language C
- Before compiling, you need to generate the libpolarssl.a static library(via polarssl official website).
- contact me via email: dosgo@qq.com


不会编译的伙伴可以用，hauntek大神写的python版本，https://github.com/hauntek/python-ngrok

# openwrt 编译方法。
- 看教程http://www.jianshu.com/p/8428949d946c


## build  

ddns.cpp and https.cpp are abondoned 


# windows
run Build.bat in CLI.


# linux
run build.sh instead of makefile


# openwrt
run openwrtbuild.sh

# tomatoware
### openssl
 cp Makefile.openssl.static Makefile
 
 make
### porlarssl
 cp Makefile.porlarssl.static Makefile
 
 make


## 使用说明

命令

ngrok-polarssl -SER[Shost:ngrokd.ngrok.com,Sport:443,Atoken:xxx] -AddTun[Type:http,Lhost:127.0.0.1,Lport:80,Sdname:xxdosgo]


Shost   -Server host.   //服务器host

Sport   -server port.   //服务器端口

Atoken  -ngrok authtoken. //服务器认证串


type    -tcp or http or https.   //要映射的类型，tcp,http,https

Lhost   -local address.     //本地地址，如果是本机直接127.0.0.1

Lport   -local port.     //本地端口

sdname  -Subdomain.     //子域名

Hostname -hostname      //自定义域名映射      

Rport    -remote port  //远程端口，tcp映射的时候，制定端口使用。


## Example
- ngrokc -SER[Shost:ngrokd.ngrok.com,Sport:443] -AddTun[Type:http,Lhost:127.0.0.1,Lport:80,Sdname:Example]  
- ngrokc -AddTun[Type:http,Lhost:127.0.0.1,Lport:80,Sdname:Example]

You can also register multiple Tunnel, but can only have one of each type.  
- ngrokc -AddTun[Type:http,Lhost:127.0.0.1,Lport:80,Sdname:Example] -AddTun[Type:https,Lhost:127.0.0.1,Lport:81,Sdname:Example1]

 
## 2015/7/10更新增加


- 如，你只要把xx.xxx.org解析到tunnel.mobi，那么访问xx.xxx.org就可以了..比Sdname好。。

ngrokc.exe -SER[Shost:tunnel.mobi,Sport:44433] -AddTun[Type:http,Lhost:127.0.0.1,Lport:80,Hostname:xx.xxx.org] -AddTun[Type:tcp,Lhost:127.0.0.1,Lport:80,Rport:55556] 
- 有空的话增加openssl的支持


- 感谢981213的makefile文件
- 感谢maz-1 修复ubuntu下编译bug



## 2015/7/28更新
- 增加openssl支持，
- 如果编译openssl版本，修改config.h文件把define OPENSSL 改为1,
- 如果编译polarssl版本，修改config.h文件把define OPENSSL 改为0,

## 2015/8/4
- make -f Makefile.openssl编译openssl版本
- make -f Makefile.polarssl编译polarssl版本
- openssl版本有bug，运行久了会崩溃...慎用。。

## 2015/10/7
- 增加PolarSSL v2.0.0++版本兼容

### 注意如果编译polarssl版本
如果你使用的PolarSSL 版本是v2.0.0及其以上的版本编译，请把config.h文件的define ISMBEDTLS 改为1


## 2015/10/17
 - 修复内存泄漏
 - 增加编译详细说明
 - 发现bug，路由上CPU偶尔9%,暂未修复。

## 2015/10/20
 - 大幅减少内存占用
 - 如无重大bug，不更新了。。
 - 
 
## 2015/11/5
- 修复没网导致的内存泄漏问题。会导致路由不断重启
- 增加版本号。

## 2015/12/8
- 做些小优化，避免嵌入式设备，栈溢出，主要是嵌入式设备栈内存太小。。多个连接就会导致崩溃。
- 对了，记得编译的时候用-O2参数，不然这些，优化完全无效。。电脑可以忽略，但是嵌入式设备，记得用，我的8M，20个连接就溢出了。优化以后没测试，感觉没那么脆弱了。。

## 2015/12/13
- 修复一个古老的bug,10月份以后的版本都有这问题,如果是，请务必升级到最新版本，加载速度大幅提升。

## 2015/12/20
- 关闭。。tcp缓存区。。以修复http连接，保存失败问题。
- 打开tcp keeplive避免死链问题。
- 这个版本相对完善，没啥，大问题了。。不更新了。

## 2015/12/30
- 修复一个超长url在大端下,无法使用的bug。

## 2016/3/12
- 增加http://www.ngrok.cc/服务器支持

## 2016/3/29
- 增加openssl库动态加载，以方便编译openwrt版本。
- 增加openwrt傻瓜编译教程。
- 关于需要luci界面的，恩山有大神写了。我提交了。叫luci-app-ngrokc_git-15.290.16504-8c2fd44-1_all.ipk,下载安装好以后把编译好的ngrokc替换/usr/bin/里面的ngrokc，改权限，重启就可以了。。

## 2016/3/31
- 修复openssl版本连接不了sslv2服务器问题。


## 2016/4/4
- 去掉线程减少内存占用。
- 修改发送接收缓存区大小，避免阻塞。

## 2016/04/19
- 修复一个bug，增大缓存区。
- 大幅减少CPU消耗，一般10%以下


## 2016/04/20
- 修复1.22版本的polarssl下的bug，如果你使用的是ngrokc1.22版本，请更新，那个版本有重大bug，openssl版本没问题。
- 全部建议到1.24稳定版本。

## 2016/04/25
-1.25 版本（测试）增加多个通道功能，避免多个进程，可以注册N个通道，如- ngrokc -AddTun[Type:http,Lhost:127.0.0.1,Lport:80,Sdname:Example] -AddTun[Type:http,Lhost:127.0.0.1,Lport:80,Sdname:Example1] -AddTun[Type:http,Lhost:127.0.0.1,Lport:80,Sdname:Example2]
- 这是一个支持ngrok完全协议的版本。go语言以后可能员生支持mips，这个就没人用了。可能不更新了。

## 2016/05/2
-1.32稳定版本，修复1.25版本tcp映射出错问题。修复1.25的崩溃bug。

## 2016/05/11
-1.33版本 修复重连子域名bug。

## 2016/05/25
-1.37版本 修复一个崩溃bug，建议所有的都升级到这个版本。

## 2016/07/26
-1.39版本 为了增加自定义cid。其它没用

## 2016/10/27
-1.40版本 增加password功能支持。

## 2016/11/26
  准备新功能开发，UDP端口映射功能，有没有有兴趣的，一起开发，目前最大的问题在于，我的go语言很烂，别指望官方会加这功能，自己自己动手，又兴趣的加群192182463（QQ群）

## 2017/1/9
  -1.41版本 修复一个网络不好可能导致的内存泄漏问题。
  
### 关于编译对应路由的版本的ngrokc。
## 一。去http://downloads.openwrt.org/下载你路由对应的SDK版本 ，如OpenWrt-SDK-ar71xx-for-linux-x86_64-gcc-4.8-linaro_uClibc-0.9.33.2.tar.bz2，并且解压。
## 二.需要先编译polarssl或者opnessl库（取决你想用啥库,2选1）。
- 1.编译polarssl库
    - 去https://tls.mbed.org/download-archive下载，polarssl版本，然后解压
    - 再把buildlib.sh复制进去，并且修改export STAGING_DIR export PATH,把里面的路径改成你下载的SDK，所在的目录，注意是staging_dir目录对应STAGING_DIR，bin目录对应PATH。
    - 还得修改CC=mips-openwrt-linux-gcc CXX=mips-openwrt-linux-g++ AR=mips-openwrt-linux-ar RANLIB=mips-openwrt-linux-ranlib,把这些参数分别对应你的，编译器。名称。。
    - 然后执行，buildlib.sh
    - 如果一切顺利的话，就会在library目录下生成，2.0版本（libmbedtls.a libmbedcrypto.a libmbedx5.9.a）1.3版本（libpolarssl.a）
    - 这就编译好了polarssl库。
- 2.编译 openssl库
    - 去https://www.openssl.org/source/ 下载openssl，然后解压。
    - 再把buildlib.sh复制进去，并且修改export STAGING_DIR export PATH,把里面的路径改成你下载的SDK，所在的目录，注意是staging_dir目录对应STAGING_DIR，bin目录对应PATH。
    - 然后执行，buildlib.sh
    - 如果一切顺利的话，就会在当前目录下生成，libssl.a,libcrypto.a
    - 这就编译好了openssl库库。
    

## 三.编译ngrokc
 - 1.openssl版本
      - 把下载的openssl里面的include/openssl，文件夹复制到，你SDK里面的staging_dir/toolchain-mips_r2_gcc-4.6-linaro_uClibc-0.9.33.2/include里面，（可能根据SDK路径有所不同）。
      - 然后把第二步生成的，libssl.a,libcrypto.a,放到ngrok-c目录。
      - 再修改，ngrok-c里面的config.h，#define OPENSSL 0，改成#define OPENSSL 1;
      - 继续修改openwrtbuild.sh文件export STAGING_DIR export PATH,把里面的路径改成你下载的SDK，所在的目录，注意是staging_dir目录对应STAGING_DIR，bin目录对应PATH。
      - 还有，最后一行的libpolarssl-mips.a改成libssl.a,libcrypto.a.
      - 执行openwrtbuild.sh，就行了。。
      - 就会在build-mips生成ngrokc文件。。你用ssh，上传到路由的/bin目录，并且加入执行权限。。就可以了。。跑了。。


- 2.polarssl版本
      - 把下载的polarssl里面的include/polarssl，或者include/mbedtls 文件夹复制到，你SDK里面的staging_dir/toolchain-mips_r2_gcc-4.6-linaro_uClibc-0.9.33.2/include里面，（可能根据SDK路径有所不同）。
      - 然后把第二步生成的，2.0版本（libmbedtls.a libmbedcrypto.a libmbedx5.9.a）1.3版本（libpolarssl.a）放到ngrok-c目录。
      - 再修改，ngrok-c里面的config.h，#define OPENSSL 1，改成#define OPENSSL 0，如你的是2.0版本，请ISMBEDTLS 0改成ISMBEDTLS 1，如果是1.3，ISMBEDTLS 0
      - 继续修改openwrtbuild.sh文件export STAGING_DIR export PATH,把里面的路径改成你下载的SDK，所在的目录，注意是staging_dir目录对应STAGING_DIR，bin目录对应PATH。
      - 还有，最后一行的libpolarssl-mips.a改成2.0版本（libmbedtls.a libmbedcrypto.a libmbedx5.9.a）1.3版本（libpolarssl.a）.
      - 执行openwrtbuild.sh，就行了。。
      - 就会在build-mips生成ngrokc文件。。你用ssh，上传到路由的/bin目录，并且加入执行权限。。就可以了。。跑了。。
      
- 3.openwrt傻瓜编译教程。
      - 去http://downloads.openwrt.org/下载你路由对应的SDK版本 ，如OpenWrt-SDK-ar71xx-for-linux-x86_64-gcc-4.8-linaro_uClibc-0.9.33.2.tar.bz2，并且解压。
      - 编辑openwrtbuildv2.sh 修改export STAGING_DIR export PATH,把里面的路径改成你下载的SDK。
      - 把include/openssl，复制到你的下载的sdk,/home/dosgo/OpenWrt-SDK/staging_dir/toolchain-mips_r2_gcc-4.6-linaro_uClibc-0.9.33.2/include目录，
      - 运行编辑openwrtbuildv2.sh,就会在build-mips生成一个叫ngrokc的文件。成功了。。把它复制到bin目录。就可以运行ngrokc了。
      - 记得安装，libopenssl.ipk，官网有下载。
      


编译就这样了，以后请不要邮件问我怎么编译了，有bug可以联系。





现在tunnel.mobi不能用了。。，哪里还有国内的服务器的。。


