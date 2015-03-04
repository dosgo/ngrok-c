#ifndef DDNS_H_INCLUDED
#define DDNS_H_INCLUDED
#include <string>
#include "cJSON.h"
#include "https.h";
using namespace std;
string getDomainId(string user,string password,string domain);
string getRecordList(string user,string password,string domain_id);
int _ddns(string user,string password,string domain_id,string record_id,string sub_domain,string record_line,string value);
int ddns(string user,string password,string domain,string sub_domain,string value);
int strrpos(const char *haystack,const char *needle, int ignorecase);
int strlpos(const char *haystack,const char *needle, int ignorecase);
#endif // DDNS_H_INCLUDED
