#include <algorithm>
#include <cctype>
#include <sstream>
#include <string>
#include <time.h>
#include "tools.h"

/*
char *triml(char *data,int count){
	data=data+count;
	return data;
}

void nullify(char *ptr,int len){
	int i;
	for(i=0;i<len;i++)ptr[i]=0;

}
*/

void printchex(std::string in, bool spaces=true){
	unsigned int len=0,i;
    len=in.length();
	printf("[");
	if(spaces)
		for(i=0;i<len;i++)printf("%x ",(unsigned char)in[i]); 
	else
		for(i=0;i<len;i++)printf("%x",(unsigned char)in[i]); 
	printf("]\n");
}

void printchex(char *in, uint32 len, bool spaces=true){
	unsigned int i;
	printf("[");
	if(spaces)
		for(i=0;i<len;i++)printf("%x ",(unsigned char)in[i]); 
	else
		for(i=0;i<len;i++)printf("%x",(unsigned char)in[i]); 
	printf("]\n");
}

/*

char *strl(char *str,int count){
	int i;
	
	char *newstr;newstr=NewTempString(count);
count--;
	for(i=0;i<=count;i++)newstr[i]=str[i];
	return newstr;
} // do not forget to delete[]newstr; !!

char *strr(char *str,int count){
	return triml(str,strlen(str)-count);
}

char *trimr(char *str,int count){
	return strl(str,strlen(str)-count);
} // do not forget to delete[]newstr; , it uses strl(...) function!

char *genrndstr(int len){
	char *str;str=NewNullString(len);
	char rnd;
	int i;
	for(i=0;i<=len-1;i++){
		rnd=(char)rand()%255;
		str[i]=rnd;
	}
	return str;
} // do not forget to delete[]str; !!


char *StrToHex(char *in,int len=-1){ // converts a string like "*,6%<1" to hex like "AB451Df51BE58A",
int i,i2=0;					 // where the resulting string is double the length!
char *newin,*buf;
if(len<0)len=strlen(in);
newin=NewTempString(len*2);
buf=NewTempString(2);
for(i=0;i<=len-1;i++){
	sprintf(buf,"%x",(unsigned char)in[i]);
	if(buf[0]==0&&buf[1]==0){
		buf[0]='0';buf[1]='0';
	}else if(buf[1]==0&&buf[0]!=0){
		newin[i2++]='0';newin[i2++]=buf[0];
	}else{		
		newin[i2++]=buf[0];newin[i2++]=buf[1];
	}
	nullify(buf,2);
}
//printf("## Converted '%s' to '%s'\n",in,newin);
return newin;
}


char *HexToStr(char *in,int len=-1){
	int i=0,i2=0,i3=0,newlen=0;char code;
	char *newin,*buf;
	buf=NewTempString(4);buf[0]='0';buf[1]='x'; // 0x??
	bool odd=false;
	if(len<0)len=strlen(in);
	if(len%2!=0){odd=true;printf("HexToStr:: is ODD!\n");}
	newlen=(odd)?((len/2)+1):(len/2);
	newin=NewTempString(newlen);
	
	if(odd){
		buf[2]='0';buf[3]=in[i2++];
		newin[0]=(char)strtol(buf,NULL,16);
		for(i=1;i<newlen;i++){
			buf[0]=in[i2++];buf[1]=in[i2++];
			code=(char)strtol(buf,NULL,16);
			newin[i]=code;
		}
	}else{
		for(i=0;i<newlen;i++){
			buf[0]=in[i2++];buf[1]=in[i2++];
			code=(char)strtol(buf,NULL,16);
			newin[i]=code;
		}
	}
	return newin;
}

*/
/*
char *NewNullString(int len){
	char *out=(char*)malloc((len+1)*sizeof(char));
	if(out==NULL){
		printf("ERROR: NewNullString:: malloc(%d) failed!",(len+1)*sizeof(char));
		quitproc_error();
		//throw("ERROR: NewNullString:: malloc() failed!");
	}
	//nullify(out,len);
	return out;
}
*/
/*
char *Reverse(char *in,int len=-1){
char *out;
int i=0;
if(len<1)len=strlen(in);
out=NewTempString(len);
for(i=0;i<len;i++)out[i]=in[(len-1)-i];
return out;
}

void rawcpy(char *dest,char *src,int len=-1){
	int i=0;
	if(len<1)
		strcpy(dest,src);
	else {
		for(i=0;i<len;i++)dest[i]=src[i]; // < or <= ?!
	}
}

void rawcat(char *dest,char *more,int destlen=-1,int morelen=-1){
	int i=0;
	if(morelen<1)morelen=strlen(more);
	if(destlen<1)destlen=strlen(dest);
	for(i=0;i<morelen;i++)dest[i+destlen]=more[i];
}

void TrimQuotes(char *in){
	unsigned int len=0,pos=0;
	len=strlen(in)-1; // -1 because we want to access an array, which starts with 0, not 1
	if(len<0)return;
	if(len==0&&in[0]=='\"'){in[0]=0;return;}
	if(in[0]=='\"'&&in[len]=='\"'){
		for(pos=0;pos<=len;pos++){
			in[pos]=in[pos+1];
			if(in[pos]=='\"'){
				in[pos]=0;
				break;
			}
		}
	}
}
*/
std::string stringToLower(std::string s)
{
	std::transform(s.begin(), s.end(), s.begin(), std::tolower);
	return s;
}

std::string stringToUpper(std::string s)
{
	std::transform(s.begin(), s.end(), s.begin(), std::toupper);
	return s;
}

std::string toString(uint64 num){
	std::stringstream ss;
	ss << num;
	return ss.str();
}

std::string getDateString(void)
{
    time_t t = time(NULL);
    tm* aTm = localtime(&t);
    char str[30];
    //       YYYY   year
    //       MM     month (2 digits 01-12)
    //       DD     day (2 digits 01-31)
    //       HH     hour (2 digits 00-23)
    //       MM     minutes (2 digits 00-59)
    //       SS     seconds (2 digits 00-59)
    sprintf(str,"%-4d-%02d-%02d %02d:%02d:%02d ",aTm->tm_year+1900,aTm->tm_mon+1,aTm->tm_mday,aTm->tm_hour,aTm->tm_min,aTm->tm_sec);
    return std::string(str);
}

