#include "common.h"
#ifdef _WIN32
#define CURL_STATICLIB
#endif
#include <curl/curl.h>
#include <curl/types.h>
#include <curl/easy.h>

/**\brief cURL structure to store downloaded data */
struct MemoryStruct {
	/**\brief Buffer */
	char *memory;
	/**\brief Size of buffer */
	size_t size;
};
static CURL *curl;

// Allocate or re-size buffer
static void *myrealloc(void *ptr, size_t size){
	if(ptr)
		return realloc(ptr, size);
	else
		return malloc(size);
}

// Callback for curl download
static size_t WriteMemoryCallback(void *ptr, size_t size,
		size_t nmemb, void *data){
	size_t realsize = size*nmemb;
	struct MemoryStruct *mem = (struct MemoryStruct *)data;

	mem->memory = myrealloc(mem->memory, mem->size+realsize+1);
	if( mem->memory ){
		memcpy(&(mem->memory[mem->size]),ptr,realsize);
		mem->size += realsize;
		mem->memory[mem->size] = 0;
	}
	LOG(LOGINFO,_("Downloading data..."));
	return realsize;
}

// Escape special character in URL
static char *_escape_url(char url[]){
	struct{char ch;char code[3];} specials[]={
		{' ', "20"},
		{'<', "3C"},
		{'>', "3E"},
		{'#', "23"},
		{'%', "25"},
		{'{', "7B"},
		{'}', "7D"},
		{'|', "7C"},
		{'\\',"5C"},
		{'^', "5E"},
		{'~', "7E"},
		{'[', "5B"},
		{']', "5D"},
		{'`', "60"},
		{';', "3B"},
		{'/', "2F"},
		{'?', "3F"},
		{':', "3A"},
		{'@', "40"},
		{'=', "3D"},
		{'&', "26"},
		{'$', "24"},
	};
	int i,j,k;
	int size=strlen(url);
	int newsize=size+1;
	char *escaped_url=NULL;

	for( i=0; i<size; ++i ){
		for( j=0; j<SIZEOF(specials); ++j ){
			if( url[i]==specials[j].ch ){
				newsize+=2;
				break;
			}
		}
	}
	escaped_url = malloc(sizeof(char)*newsize);
	if( !escaped_url ){
		LOG(LOGERR,_("Unable to allocate new url"));
		return NULL;
	}
	i=0;
	k=0;
	while( i<newsize ){
		escaped_url[i]=url[k];
		for( j=0; j<SIZEOF(specials); ++j ){
			if( url[k]==specials[j].ch ){
				escaped_url[i]='%';
				escaped_url[++i]=specials[j].code[0];
				escaped_url[++i]=specials[j].code[1];
				break;
			}
		}
		++i;
		++k;
	}
	LOG(LOGVERBOSE,_("Old URL is %s"),url);
	LOG(LOGVERBOSE,_("Escaped URL is: %s"),escaped_url);
	return escaped_url;
}

// Downloads url to buffer, need to free returned buffer after use
// returns NULL on error
static char *_download2buffer(char url[]){
	CURLcode res;
	struct MemoryStruct chunk;

	chunk.memory = NULL;
	chunk.size = 0;
	curl_easy_setopt(curl,CURLOPT_URL,url);
	curl_easy_setopt(curl,CURLOPT_WRITEFUNCTION,WriteMemoryCallback);
	curl_easy_setopt(curl,CURLOPT_WRITEDATA,(void*)&chunk);
	res = curl_easy_perform(curl);
	if( res != 0  ){
		// Error occurred
		LOG(LOGERR,_("Error occurred while access url: %s"),url);
		if( chunk.memory )
			free(chunk.memory);
		return NULL;
	}else{
		LOG(LOGINFO,_("Download successful."));
		return chunk.memory;
	}
}

// Use hostip.info to look up lat/lon
int location_geocode_hostip(float *lat,float *lon,char *city,int bsize){
	char url[]="http://api.hostip.info/get_html.php?position=true";
	char *result;
	char *searchind=NULL;
	*lat = 0.0f;
	*lon = 0.0f;
	strcpy(city,"(Error)");

	result = _download2buffer(url);
	if( !result ){
		LOG(LOGERR,_("Error during IP lookup"));
		return RET_FUN_FAILED;
	}
	LOG(LOGVERBOSE,_("Download content:\n %s"),result);

	if( (searchind=strstr(result, "City: ")) ){
		char *begin = searchind+strlen("City: ");
		char *end = strchr(begin,'\n');
		if( end ){
			int length = end-begin;
			if( length >= bsize ){
				LOG(LOGWARN,_("City buffer size too small"));
				length = bsize-1;
			}
			strncpy(city,begin,length);
			city[length]='\0';
		}else{
			LOG(LOGWARN,_("Could not parse city name."));
			return RET_FUN_FAILED;
		}
	}else{
		LOG(LOGWARN,_("Could not parse city name."));
		return RET_FUN_FAILED;
	}

	if( (searchind=strstr(result, "Latitude: ")) )
		*lat = (float)atof(searchind+10);

	if( (searchind=strstr(result, "Longitude: ")) )
		*lon = (float)atof(searchind+11);

	if(result)
		free(result);
	return RET_FUN_SUCCESS;
}

// Use address input to look up lat/lon (probably could use an XML parser
//		if this gets any more complicated)
int location_address_lookup(char *address,float *lat,float *lon,
		char *city,int bsize){
	char baseurl[]=
		"http://maps.google.com/maps/api/geocode/xml?sensor=false&address=";
	char *url;
	char *escaped_url;
	char *result;
	char *searchind=NULL;

	escaped_url=_escape_url(address);
	if( !escaped_url )
		return RET_FUN_FAILED;

	url=malloc((strlen(baseurl)+strlen(escaped_url)+1)*sizeof(char));
	if( !url ){
		LOG(LOGERR,_("Allocation of URL failed"));
		return RET_FUN_FAILED;
	}
	strcpy(url,baseurl);
	strcpy(url+strlen(baseurl),escaped_url);
	free(escaped_url);
	LOG(LOGVERBOSE,_("Created url: %s"),url);

	*lat = 0.0f;
	*lon = 0.0f;
	strcpy(city,"(Error)");

	result = _download2buffer(url);
	free(url);
	if( !result ){
		LOG(LOGERR,_("Error during address search"));
		return RET_FUN_FAILED;
	}
	LOG(LOGVERBOSE,_("Downloaded content:\n %s"),result);
	if( (searchind=strstr(result,"<formatted_address>")) ){
		char *begin = searchind+strlen("<formatted_address>");
		char *end = strchr(begin,'<');
		if( end ){
			int length = end-begin;
			if( length >= bsize ){
				LOG(LOGWARN,_("City buffer size too small"));
				length = bsize-1;
			}
			strncpy(city,begin,length);
			city[length]='\0';
		}else{
			LOG(LOGWARN,_("Could not parse city name."));
			return RET_FUN_FAILED;
		}
	}else{
		LOG(LOGWARN,_("Could not parse city name."));
		return RET_FUN_FAILED;
	}

	if( (searchind=strstr(result,"<lat>")) )
		*lat = (float)atof(searchind+strlen("<lat>"));

	if( (searchind=strstr(result,"<lng>")) )
		*lon = (float)atof(searchind+strlen("<lng>"));

	if(result)
		free(result);
	return RET_FUN_SUCCESS;
}

int location_init(void){
	curl_global_init(CURL_GLOBAL_ALL);
	curl = curl_easy_init();
	if( curl )
		return RET_FUN_SUCCESS;
	else{
		LOG(LOGERR,_("Error initializing cURL library"));
		curl_global_cleanup();
		return RET_FUN_FAILED;
	}
}

int location_end(void){
	curl_easy_cleanup(curl);
	curl_global_cleanup();
	return RET_FUN_SUCCESS;
}
