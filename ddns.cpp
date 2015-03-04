#include "ddns.h"
static char * headstring="Accept:*/*\r\nUser-Agent:dnspodc/0.1(dosgo@qq.com)";

string getDomainId(string user,string password,string domain)
{
    string DomainId="";
    string poststring="login_email="+user+"&login_password="+password+"&format=json&domain="+domain;
    struct http_response *hresp = http_post("https://dnsapi.cn/Domain.Info",headstring,(char*)poststring.c_str());
    if(hresp!=NULL){
        int start=strlpos(hresp->body,"{",0);
        int end=strrpos(hresp->body,"}",0);
        char*jsonstr=hresp->body+start;
        jsonstr[end+1]='\0';
        cJSON *json = cJSON_Parse(jsonstr);
        if(json)
        {
            cJSON *status = cJSON_GetObjectItem(json, "status");
            if(status)
            {
                cJSON *code= cJSON_GetObjectItem(status, "code");
                if(code)
                {
                    char *codestr=code->valuestring;
                    if ( strcmp(codestr,"1")==0)
                    {
                        cJSON *domain = cJSON_GetObjectItem(json, "domain");
                        //
                        char * domain_id=cJSON_GetObjectItem(domain, "id")->valuestring;
                        DomainId=string(domain_id);
                    }
                }
            }
        }
        cJSON_Delete(json);
   }
   free(hresp);
   return DomainId;
}

string getRecordList(string user,string password,string domain_id)
{
    string jsonstr="";
    string poststring="login_email="+user+"&login_password="+password+"&format=json&domain_id="+domain_id;
    struct http_response *hresp = http_post("https://dnsapi.cn/Record.List",headstring,(char *)poststring.c_str());
    if(hresp!=NULL){
        int start=strlpos(hresp->body,"{",0);
        int end=strrpos(hresp->body,"}",0);
        char*json=hresp->body+start;
        json[end+1]='\0';
        jsonstr=string(json);
    }
    free(hresp);
    return jsonstr;
}

int _ddns(string user,string password,string domain_id,string record_id,string sub_domain,string record_line,string value)
{
    string poststring="login_email="+user+"&login_password="+password+"&format=json&lang=en&domain_id="+domain_id+"&record_id="+record_id+"&sub_domain="+sub_domain+"&record_line="+record_line+"&value="+value;
    printf("poststring:%s\r\n",poststring.c_str());
    struct http_response *hresp = http_post("https://dnsapi.cn/Record.Ddns",headstring,(char *)poststring.c_str());
    if(hresp!=NULL){
        int start=strlpos(hresp->body,"{",0);
        int end=strrpos(hresp->body,"}",0);
        char*jsonstr=hresp->body+start;
        jsonstr[end+1]='\0';
      //  printf("jsonstr:%s\r\n",jsonstr);
        cJSON *json = cJSON_Parse(jsonstr);
        if(!json)
        {
            cJSON_Delete(json);
            return -1;
        }
        cJSON *status = cJSON_GetObjectItem(json, "status");
        if(!status)
        {
            cJSON_Delete(json);
            return -1;
        }
        char *code= cJSON_GetObjectItem(status, "code")->valuestring;
        if ( strcmp(code,"1")==0)
        {
            cJSON_Delete(json);
            return 0;
        }
        cJSON_Delete(json);
    }
    free(hresp);
    return -1;
}

int ddns(string user,string password,string domain,string sub_domain,string value)
{
    string  DomainId=getDomainId(user,password,domain);
    if(!DomainId.empty())
    {
        string jsonstr=getRecordList(user,password,DomainId);
        if(!jsonstr.empty())
        {
            cJSON *json = cJSON_Parse(jsonstr.c_str());
            if(!json)
            {
                cJSON_Delete(json);
                return -1;
            }
            cJSON *status = cJSON_GetObjectItem(json, "status");
            if(!status)
            {
                cJSON_Delete(json);
                return -1;
            }
            cJSON *code= cJSON_GetObjectItem(status, "code");
            if(!status)
            {
                cJSON_Delete(json);
                return -1;
            }
            char *codestr=code->valuestring;
            if (strcmp(codestr,"1")==0)
            {
                cJSON*records = cJSON_GetObjectItem(json, "records");
                if(!records)
                {
                    cJSON_Delete(json);
                    return -1;
                }
                for(int i=0;i<cJSON_GetArraySize(records);i++)
                {
                    cJSON *recordsitem=cJSON_GetArrayItem(records,i);
                    if(!recordsitem)
                    {
                        cJSON_Delete(json);
                        return -1;
                    }
                    char *item_sub_domain =cJSON_GetObjectItem(recordsitem, "name")->valuestring;
                    char *item_value =cJSON_GetObjectItem(recordsitem, "value")->valuestring;
                    if(strcmp(sub_domain.c_str(),item_sub_domain)==0&&strcmp(value.c_str(),item_value)!=0)
                    {
                        string item_record_line=string(cJSON_GetObjectItem(recordsitem, "line")->valuestring);
                        string item_record_id =string(cJSON_GetObjectItem(recordsitem, "id")->valuestring);
                        int err=_ddns(user,password,DomainId,item_record_id,sub_domain,item_record_line,value);
                        if(err==0)
                        {
                            printf("DDNS update OK \r\n");
                            return 0;
                        }
                        printf("DDNS update err \r\n");
                        return -1;
                    }
                }
            }

            cJSON_Delete(json);

        }

    }
    printf("DDNS not edit \r\n");
    return 0;
}


int strrpos(const char *haystack,const char *needle, int ignorecase = 0)
{
    int back=0;
    register unsigned char c, needc;
    unsigned char const *from, *end;
    int len = strlen(haystack);
    int needlen = strlen(needle);
    from = (unsigned char *)haystack;
    end = (unsigned char *)haystack + len;
    const char *findreset = needle;
    for (int i = 0; from < end; ++i) {
        c = *from++;
        needc = *needle;
        if (ignorecase) {
            if (c >= 65 && c < 97)
                c += 32;
            if (needc >= 65 && needc < 97)
                needc += 32;
        }
        if(c == needc) {
            ++needle;
            if(*needle == '\0') {
                if (len == needlen)
                {
                     back=0;
                    //return 0;
                }
                else
                {
                    back=i - needlen+1;
                    //return i - needlen+1;
                }
            }
        }
        else {
            if(*needle == '\0' && needlen > 0)
            {
                back=i - needlen+1;
                //return i - needlen +1;
            }
            needle = findreset;
        }
    }
    return  back;
}

int strlpos(const char *haystack,const char *needle, int ignorecase = 0)
{
    register unsigned char c, needc;
    unsigned char const *from, *end;
    int len = strlen(haystack);
    int needlen = strlen(needle);
    from = (unsigned char *)haystack;
    end = (unsigned char *)haystack + len;
    const char *findreset = needle;
    for (int i = 0; from < end; ++i) {
        c = *from++;
        needc = *needle;
        if (ignorecase) {
            if (c >= 65 && c < 97)
                c += 32;
            if (needc >= 65 && needc < 97)
                needc += 32;
        }
        if(c == needc) {
            ++needle;
            if(*needle == '\0') {
                if (len == needlen)
                    return 0;
                else
                    return i - needlen+1;
            }
        }
        else {
            if(*needle == '\0' && needlen > 0)
                return i - needlen +1;
            needle = findreset;
        }
    }
    return  0;
}

