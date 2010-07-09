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
	LOG(LOGINFO,_("Download data..."));
	return realsize;
}

int location_geocode_hostip(void){
	char url[]="http://api.hostip.info/get_html.php?position=true";
	CURLcode res;
	struct MemoryStruct chunk;
	int ret=RET_FUN_SUCCESS;

	chunk.memory = NULL;
	chunk.size = 0;

	curl_easy_setopt(curl,CURLOPT_URL,url);
	curl_easy_setopt(curl,CURLOPT_WRITEFUNCTION,WriteMemoryCallback);
	curl_easy_setopt(curl,CURLOPT_WRITEDATA,(void*)&chunk);
	res = curl_easy_perform(curl);
	if( res != 0  ){
		// Error occurred
		LOG(LOGERR,_("Error occurred while looking up IP"));
		ret=RET_FUN_FAILED;
	}else{
		LOG(LOGINFO,_("Download successful."));
		printf("%s\n",chunk.memory);
	}
	if(chunk.memory)
		free(chunk.memory);
	return ret;
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
