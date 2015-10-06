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



###注意如果要编译polarssl版本，请使用polarssl 1.3.10版本编译，其它版本可能会出现兼容问题，下载地址:https://tls.mbed.org/download-archive
