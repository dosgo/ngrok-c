##ngrok-c

- ngrok client for c language.  
- Before compiling you need to generate libpolarssl.a static library Reference polarssl official website.  
- contact email dosgo@qq.com.




##build  

ddns.cpp with https.cpp not used.


#windows
Build.bat run in cmd.


#linux
Because not use makefile, so write a script sh compile, run build.sh


#openwrt
run openwrtbuild.sh




##use 

ngrok-polarssl -SER[Shost:ngrokd.ngrok.com,Sport:443,Atoken:xxx] -AddTun[Type:http,Lhost:127.0.0.1,Lport:80,Sdname:xxdosgo]


Shost   -Server host.  
Sport   -server port.  
Atoken  -ngrok authtoken.


type    -tcp or http or https.  
Lhost   -local address.  
Lport   -local port.  
sdname  -Subdomain.


##Example
- ngrokc -SER[Shost:ngrokd.ngrok.com,Sport:443] -AddTun[Type:http,Lhost:127.0.0.1,Lport:80,Sdname:Example]  
- ngrokc -AddTun[Type:http,Lhost:127.0.0.1,Lport:80,Sdname:Example]

You can also register multiple Tunnel, but can only have one of each type.  
- ngrokc -AddTun[Type:http,Lhost:127.0.0.1,Lport:80,Sdname:Example] -AddTun[Type:https,Lhost:127.0.0.1,Lport:81,Sdname:Example1]

 
##2015/7/10更新增加


- 如，你只要把xx.xxx.org解析到tunnel.mobi，那么访问xx.xxx.org就可以了..比Sdname好。。

ngrokc.exe -SER[Shost:tunnel.mobi,Sport:44433] -AddTun[Type:http,Lhost:127.0.0.1,Lport:80,Hostname:xx.xxx.org] -AddTun[Type:tcp,Lhost:127.0.0.1,Lport:80,Rport:55556] 
- 有空的话增加openssl的支持


- 感谢981213的makefile文件
- 感谢maz-1 修复ubuntu下编译bug



##2015/7/28更新
- 增加openssl支持，
- 如果编译openssl版本，修改config.h文件把define OPENSSL 改为1,
- 如果编译polarssl版本，修改config.h文件把define OPENSSL 改为0,

##2015/8/4
- make -f Makefile.openssl编译openssl版本
- make -f Makefile.polarssl编译polarssl版本
- openssl版本有bug，运行久了会崩溃...慎用。。

##2015/10/7
- 增加PolarSSL v2.0.0++版本兼容

###注意如果编译polarssl版本
如果你使用的PolarSSL 版本是v2.0.0及其以上的版本编译，请把config.h文件的define ISMBEDTLS 改为1


##2015/10/17
 - 修复内存泄漏
 - 增加编译详细说明
 - 发现bug，路由上CPU偶尔9%,暂未修复。

##2015/10/20
 - 大幅减少内存占用
 - 如无重大bug，不更新了。。
 - 
 
##2015/11/5
- 修复没网导致的内存泄漏问题。会导致路由不断重启
- 增加版本号。

##2015/12/8
- 做些小优化，避免嵌入式设备，栈溢出，主要是嵌入式设备栈内存太小。。多个连接就会导致崩溃。

###关于编译对应路由的版本的ngrokc。
##一。去http://downloads.openwrt.org/下载你路由对应的SDK版本 ，如OpenWrt-SDK-ar71xx-for-linux-x86_64-gcc-4.8-linaro_uClibc-0.9.33.2.tar.bz2，并且解压。
##二.需要先编译polarssl或者opnessl库（取决你想用啥库,2选1）。
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
    

##三.编译ngrokc
 - 1.openssl版本
      - 把下载的openssl里面的include/openssl，文件夹复制到，你SDK里面的staging_dir/toolchain-mips_r2_gcc-4.6-linaro_uClibc-0.9.33.2/include里面，（可能根据SDK路径有所不同）。
      - 然后把第二步生成的，libssl.a,libcrypto.a,放到ngrok-c目录。
      - 再修改，ngrok-c里面的config.h，#define OPENSSL 0，改成#define OPENSSL 1;
      - 继续修改openwrtbuild.sh文件export STAGING_DIR export PATH,把里面的路径改成你下载的SDK，所在的目录，注意是staging_dir目录对应STAGING_DIR，bin目录对应PATH。
      - 还有，最后一行的libpolarssl-mips.a改成libssl.a,libcrypto.a.
      - 执行openwrtbuild.sh，就行了。。
      - 就会在build-mips生成ngrokc文件。。你用ssh，上传到路由的/bin目录，并且加入执行权限。。就可以了。。跑了。。


- 2.openssl版本
      - 把下载的polarssl里面的include/polarssl，或者include/mbedtls 文件夹复制到，你SDK里面的staging_dir/toolchain-mips_r2_gcc-4.6-linaro_uClibc-0.9.33.2/include里面，（可能根据SDK路径有所不同）。
      - 然后把第二步生成的，2.0版本（libmbedtls.a libmbedcrypto.a libmbedx5.9.a）1.3版本（libpolarssl.a）放到ngrok-c目录。
      - 再修改，ngrok-c里面的config.h，#define OPENSSL 1，改成#define OPENSSL 0，如你的是2.0版本，请ISMBEDTLS 0改成ISMBEDTLS 1，如果是1.3，ISMBEDTLS 0
      - 继续修改openwrtbuild.sh文件export STAGING_DIR export PATH,把里面的路径改成你下载的SDK，所在的目录，注意是staging_dir目录对应STAGING_DIR，bin目录对应PATH。
      - 还有，最后一行的libpolarssl-mips.a改成2.0版本（libmbedtls.a libmbedcrypto.a libmbedx5.9.a）1.3版本（libpolarssl.a）.
      - 执行openwrtbuild.sh，就行了。。
      - 就会在build-mips生成ngrokc文件。。你用ssh，上传到路由的/bin目录，并且加入执行权限。。就可以了。。跑了。。


编译就这样了，以后请不要邮件问我怎么编译了，有bug可以联系。





现在tunnel.mobi不能用了。。，哪里还有国内的服务器的。。
