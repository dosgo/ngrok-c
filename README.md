##ngrok-c

ngrok client for c language.
Before compiling you need to generate libpolarssl.a static library Reference polarssl official website.




##build

#windows
Build.bat run in cmd.


#linux
Because not use makefile, so write a script sh compile, run build.sh


#openwrt
run cpopenwrt.sh




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
ngrok-polarssl -SER[Shost:ngrokd.ngrok.com,Sport:443] -AddTun[Type:http,Lhost:127.0.0.1,Lport:80,Sdname:Example] ngrok-polarssl -AddTun[Type:http,Lhost:127.0.0.1,Lport:80,Sdname:Example]

You can also register multiple Tunnel, but can only have one of each type.  

ngrok-polarssl -AddTun[Type:http,Lhost:127.0.0.1,Lport:80,Sdname:Example] -AddTun[Type:https,Lhost:127.0.0.1,Lport:81,Sdname:Example1]

 










