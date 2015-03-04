SOCKET listenSocket;  



struct sockinfo
{
    ssl_context * ssl;
    int isssl;
    int isconnect;
    int localsock;
    int remotesock;
};


map<int,sockinfo> socklist;


FD_SET writeSet;  
FD_SET readSet;  
  

while(true)  
{  
    FD_ZERO(&readSet);  
    FD_ZERO(&writeSet);  
    FD_SET(listenSocket,&socketSet);  
  
    //同时检查套接字的可读可写性。  
    int ret=select(0,&readSet,&writeSet,NULL,NULL);//为等待时间传入NULL，则永久等待。传入0立即返回。不要勿用。  
    if(ret==SOCKET_ERROR)  
    {  
        return false;  
    }  
    sockaddr_in addr;  
    int len=sizeof(addr);  
    //是否存在客户端的连接请求。  
    if(FD_ISSET(listenSocket,&readSet))//在readset中会返回已经调用过listen的套接字。  
    {  
        acceptSocket=accept(listenSocket,(sockaddr*)&addr,&len);  
        if(acceptSocket==INVALID_SOCKET)  
        {  
            return false;  
        }  
        else  
        {  
            FD_SET(acceptSocket,&socketSet);  
        }  
    }  
  
    for(int i=0;i<socketSet.fd_count;i++)  
    {  
        if(FD_ISSET(socketSet.fd_array[i],&readSet))  
        {  
            //调用recv，接收数据。  
        }  
        if(FD_ISSET(socketSet.fd_array[i]),&writeSet)  
        {  
            //调用send，发送数据。  
        }  
    }  
}  