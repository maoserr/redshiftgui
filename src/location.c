#include "common.h"
#include "location.h"
#include "netutils.h"

// Use hostip.info to look up lat/lon
int location_geocode_hostip(
		/*@out@*/ float *lat,
		/*@out@*/ float *lon,
		/*@out@*/ char *city,int bsize)
{
	char url[]="http://api.hostip.info/get_html.php?position=true";
	char *result;

	(*lat)=0.0;
	(*lon)=0.0;
	strcpy(city,"(Error)");

	result = download2buffer(url);
	if( !result ){
		LOG(LOGERR,_("Error during IP lookup"));
		return RET_FUN_FAILED;
	}
	LOG(LOGVERBOSE,_("Download content:\n %s"),result);

	(*lat)=parse_tag_float(result,"Latitude: ");
	(*lon)=parse_tag_float(result,"Longitude: ");

	if( parse_tag_str(result,"City: ","\n",city,bsize)==0 ){
		LOG(LOGERR,_("Error parsing city."));
		free(result);
		return RET_FUN_FAILED;
	}
	free(result);
	LOG(LOGINFO,_("You location: (%f,%f) %s"),*lat,*lon,city);
	return RET_FUN_SUCCESS;
}

// Use Geobytes to look up lat/lon
int location_geocode_geobytes(
		/*@out@*/ float *lat,
		/*@out@*/ float *lon,
		/*@out@*/ char *city,int bsize)
{
	char url[]="http://www.geobytes.com/IpLocator.htm?GetLocation&template=json.txt";
	char *result;
	int size_copied=0;
	int size_used=0;

	(*lat)=0.0;
	(*lon)=0.0;
	strcpy(city,"(Error)");

	result = download2buffer(url);
	if( !result ){
		LOG(LOGERR,_("Error during IP lookup"));
		return RET_FUN_FAILED;
	}
	LOG(LOGVERBOSE,_("Download content:\n %s"),result);
	
	(*lat)=parse_tag_float(result,"\"latitude\":");
	(*lon)=parse_tag_float(result,"\"longitude\":");

	size_copied=parse_tag_str(result,"\"city\":\"","\",",city,bsize);
	if( size_copied==0 ){
		free(result);
		return RET_FUN_FAILED;
	}
	size_used+=size_copied;
	if( (bsize-size_used)<=1 ){
		free(result);
		return RET_FUN_SUCCESS; /* Truncated */
	}
	city[size_used++]=',';
	size_copied=parse_tag_str(result,"\"region\":\"","\",",city+size_used,
			bsize-size_used);
	if( size_copied==0 ){
		free(result);
		return RET_FUN_FAILED;
	}
	size_used+=size_copied;
	if( (bsize-size_used)<=1 ){
		free(result);
		return RET_FUN_SUCCESS; /* Truncated */
	}
	city[size_used++]=',';
	size_copied=parse_tag_str(result,"\"country\":\"","\",",city+size_used,
			bsize-size_used);
	if( size_copied==0 ){
		free(result);
		return RET_FUN_FAILED;
	}
	free(result);
	return RET_FUN_SUCCESS;
}

// Use address input to look up lat/lon (probably could use an XML parser
//		if this gets any more complicated)
int location_address_lookup(const char *address,
		/*@out@*/ float *lat,
		/*@out@*/ float *lon,
		/*@out@*/ char *city,int bsize)
{
	char baseurl[]=
		"http://maps.google.com/maps/api/geocode/xml?sensor=false&address=";
	char *url;
	char *escaped_url;
	char *result;

	(*lat)=0.0;
	(*lon)=0.0;
	strcpy(city,"(Error)");

	escaped_url=escape_url(address);
	if( !escaped_url )
		return RET_FUN_FAILED;

	url=malloc((strlen(baseurl)
		+strlen(escaped_url)+1)*sizeof(char));
	if( !url ){
		LOG(LOGERR,_("Allocation of URL failed"));
		free(escaped_url);
		return RET_FUN_FAILED;
	}
	strcpy(url,baseurl);
	strcpy(url+strlen(baseurl),escaped_url);
	free(escaped_url);
	LOG(LOGVERBOSE,_("Created url: %s"),url);

	strcpy(city,"(Error)");

	result = download2buffer(url);
	free(url);
	if( !result ){
		LOG(LOGERR,_("Error during address search"));
		return RET_FUN_FAILED;
	}
	LOG(LOGVERBOSE,_("Downloaded content:\n %s"),result);

	(*lat)=parse_tag_float(result,"<lat>");
	(*lon)=parse_tag_float(result,"<lng>");

	if( parse_tag_str(result,"<formatted_address>",
				"</formatted_address>",city,bsize)
			!= RET_FUN_SUCCESS ){
		free(result);
		return RET_FUN_FAILED;
	}

	free(result);
	return RET_FUN_SUCCESS;
}

