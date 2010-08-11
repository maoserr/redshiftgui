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
static void *_myrealloc(void *ptr, size_t size){
	if(ptr)
		return realloc(ptr, size);
	else
		return malloc(size);
}

// Callback for curl download
static size_t _writememcb(void *ptr, size_t size,
		size_t nmemb, void *data){
	size_t realsize = size*nmemb;
	struct MemoryStruct *mem = (struct MemoryStruct *)data;

	mem->memory = _myrealloc(mem->memory, mem->size+realsize+1);
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
char *download2buffer(char url[]){
	CURLcode res;
	struct MemoryStruct chunk;

	chunk.memory = NULL;
	chunk.size = 0;
	LOG(LOGINFO,_("Downloading URL: %s"),url);
	curl_easy_setopt(curl,CURLOPT_URL,url);
	curl_easy_setopt(curl,CURLOPT_WRITEFUNCTION,_writememcb);
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

// Escape special character in URL
char *escape_url(char url[]){
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

/* Copies content of tag to buffer
 * Returns the number of characters copied, minus null terminator
 */
int parse_tag_str(char content[],char start_tag[],char end_tag[],
		char *buffer,int bsize){
	char *searchind;
	if( (searchind=strstr(content,start_tag)) ){
		char *begin = searchind+strlen(start_tag);
		char *end = strstr(begin,end_tag);
		if( end ){
			int length = end-begin;
			if( length >= bsize ){
				LOG(LOGWARN,_("Buffer to small for [%s]-[%s]"),start_tag,end_tag);
				length = bsize-1;
			}
			strncpy(buffer,begin,length);
			buffer[length]='\0';
			return length;
		}else{
			LOG(LOGWARN,_("Could not find end tag [%s]-[%s]"),start_tag,end_tag);
			return 0;
		}
	}else{
		LOG(LOGWARN,_("Could not parse tag [%s]-[%s]"),start_tag,end_tag);
		return 0;
	}
}

/* Parses floats from buffer based on tag
 */
float parse_tag_float(char content[],char start[]){
	char *searchind;
	int size=strlen(start);

	searchind=strstr(content,start);
	if( searchind )
		return (float)atof(searchind+size);

	return 0.0f;
}

int net_init(void){
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

int net_end(void){
	curl_easy_cleanup(curl);
	curl_global_cleanup();
	return RET_FUN_SUCCESS;
}
