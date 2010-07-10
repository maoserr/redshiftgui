#include "common.h"
#ifdef _WIN32
#define CURL_STATICLIB
#endif
#include <curl/curl.h>
#include <curl/types.h>
#include <curl/easy.h>

struct MemoryStruct {
	char *memory;
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
		char *begin = searchind+6;
		char *end = strchr(begin,'\n');
		if( end ){
			int length = end-begin;
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
		*lat = atof(searchind+10);

	if( (searchind=strstr(result, "Longitude: ")) )
		*lon = atof(searchind+11);

	if(result)
		free(result);
	return RET_FUN_SUCCESS;
}

int location_address_lookup(char address[]){

	return RET_FUN_FAILED;
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
