/*******************************************************************************
	Program:		Radar Process

	File:			radarprocess.cpp

	Function:		Contains main()

	Description:	main() processes the command-line arguments using the
					AnyOption class, renames the input files (M*.bin and A*.txt)
					and then processes the files using the IsrRadarFile class.

*******************************************************************************/

#include "stdafx.h"
#include "Resource.h"
#include "anyoption.h"
#include "tinyxml.h"
#include "IsrRadarFile.h"
#include "GpuGrid.h"

#include <windows.h>
#include <tchar.h>
#include <stdio.h>
#include <strsafe.h>
#include <iostream>
#include <fstream>
#include <stdlib.h>
#include <netcdf.h>
#include <ctime>
#include <sstream>
#include <string>
#include <boost/filesystem.hpp>
#include <boost/lexical_cast.hpp>


#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// The one and only application object

using namespace std;
using namespace boost::filesystem;

void DisplayErrorBox(LPTSTR  lpszFunction);
void MakeNetCdfFile(char *netCdfFileName, short sampleRate, short scans, short bins, short collectionsPerRotation, short rangeGate, short waveSums,TiXmlDocument metaXml);
void MakeFileName(char *binFileName,char *metaFileName, TCHAR *netCdfFileName);
void JulianDayToYearMonthDay( const long julianDay, int *year, int *month, int *day );
long YearMonthDayToJulianDay( const long year, const long month, const long day );
void readAzimuthFile(TCHAR *binFileName,int *azimuthCount,float *angle, float *time);
void SplitNameAndPathname( const char*  fullFileName, int length, char *fileName, char *path);
void GetFileDateTimeStamp(char *fileName,SYSTEMTIME *stUTC,SYSTEMTIME *stLocal);
void MakeNetCdfFileName(char *fileName,char *basePath,TiXmlDocument metaXml,SYSTEMTIME fileTime);

int main(int argc, char *argv[])
{
	//	Engine *Eg;
	//	Eg = engOpen("\0");

	// We use AnyOption to set and read the command-line arguments.
	AnyOption *opt = new AnyOption();
	// set up usage/help with addUsage
	opt->addUsage( "" );
	opt->addUsage( "Usage: radarprocess [-y year] [-m:metadata file] [--file binaryfilename]" );
	opt->addUsage( "" );
	opt->addUsage( " -h  --help            Prints this help " );
	opt->addUsage( " -y  --year 2010       force year for naming" );
	opt->addUsage( " -r  --rename          rename ISR files to a datebased name" );
	opt->addUsage( " -z  --zip             zip files up into a single file (no compression)" );
	opt->addUsage( " --file  ISR binary file" );
	//opt->addUsage( " -m  --meta meta.xml   select metadata file (default metadata.xml)" );
	//opt->addUsage( " -n  --nav navfile     select nav file
	opt->addUsage( " " );
	opt->addUsage( " Assumptions Azimuth file start with A" );
	opt->addUsage( " Assumptions magnitudes start with M or N" );
	opt->addUsage( " Assumptions velocity starts with a P (phase)" );
	opt->addUsage( "" );
	// Create the possible flags and options
	opt->setFlag(  "help", 'h' );   // create a flag to show the help with both long (--help) and short (-h) forms
	opt->setOption(  "year", 'y' ); // an option to set the year, supporting long and short form
	opt->setFlag(  "rename",'r' );  /* a flag that, if set, causes the ISR files (A*.txt, M*.bin) to
									be renamed to a date-based name. Long and short forms */
	opt->setFlag(  "zip",'z' );     // a flag to cause the files to be sipped into a single file.
	opt->setOption(  "file"  );     // option to provide the ISR binary file name. Long form only.
	//opt->setOption(  "meta",'m' );  // an option to provide the metadata file,
	//								supporting long and short form (currently unused) */
	//opt->setOption(  "nav",'n' );   /* an option to provide the navigation file,
	//								supporting long and short form (currently unused) */
	opt->processCommandArgs( argc,(char **)  argv ); // Process the command-line arguments with AnyOpt.
	if (opt->getFlag( "help" ) || opt->getFlag( 'h' ) || (argc == 1 ) ) { // No args or help print option
		opt->printUsage();
		delete opt;
		return(-1);
	}

	//----------------------------------------------------------------
	// Process the binary file
	//----------------------------------------------------------------
	if (opt->getValue("file")!=NULL) {
		// print the provided ISR binary file name
		cout << "output:\n\rsourcefile:" << opt->getValue("file") <<"\n\r";
		path x(opt->getValue("file"));
		if (exists(x)) {
			// Create the IsrRadarFile object (calls the constructor).
			// The file header is read in here
			IsrRadarFile isr(opt->getValue("file"));

			// Rename the binary file, if the rename flag was set.
			if (opt->getFlag( "rename" ) || opt->getFlag( 'r' )) {
				// Use the year set in the year option, if provided.
				if (opt->getValue("year")!=NULL)
					isr.renameSource(atoi(opt->getValue("year")));
				else
					isr.renameSource(-1);
			}

			// Main processing block
			{
				// print the filename and the start time of the capture
				time_t t =  isr.getStartTime();
				cout << "output_" << opt->getValue("file") <<" " << asctime(localtime( &t));
				// Process the binary file
				isr.processFile();
			}
			// If the zip flag is set, put the A*.txt and M*.bin files into a (not-compressed) ZIP file.
			if (opt->getFlag( "zip" ) || opt->getFlag( 'z' )) {
				isr.zipfiles();
			}
		} else {
			cout << "file not found!";
		}

	} else
		cout << "need --file filename!";



	//  addPulseData(netCdfFileName,sampleRate,scans,bins,collectionsPerRotation);
	//   addAzimuth(netCdfFileName,binfileName);
	//  addNavigation(netCdfFileName,navFileName);
	delete(opt);
	exit(1);
}

// NB - I don't think any of the functions listed in the remainder of this file (below) are ever used. They appear to be legacy code. - R. Pittman
//
//void DisplayErrorBox(LPTSTR lpszFunction)
//{
//	// Retrieve the system error message for the last-error code
//
//	LPVOID lpMsgBuf;
//	LPVOID lpDisplayBuf;
//	DWORD dw = GetLastError();
//
//	FormatMessage(
//	    FORMAT_MESSAGE_ALLOCATE_BUFFER |
//	    FORMAT_MESSAGE_FROM_SYSTEM |
//	    FORMAT_MESSAGE_IGNORE_INSERTS,
//	    NULL,
//	    dw,
//	    MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
//	    (LPTSTR) &lpMsgBuf,
//	    0, NULL );
//
//	// Display the error message and clean up
//
//	lpDisplayBuf = (LPVOID)LocalAlloc(LMEM_ZEROINIT,
//	                                  (lstrlen((LPCTSTR)lpMsgBuf)+lstrlen((LPCTSTR)lpszFunction)+40)*sizeof(TCHAR));
//	StringCchPrintf((LPTSTR)lpDisplayBuf,
//	                LocalSize(lpDisplayBuf) / sizeof(TCHAR),
//	                TEXT("%s failed with error %d: %s"),
//	                lpszFunction, dw, lpMsgBuf);
//	MessageBox(NULL, (LPCTSTR)lpDisplayBuf, TEXT("Error"), MB_OK);
//
//	LocalFree(lpMsgBuf);
//	LocalFree(lpDisplayBuf);
//}
//
//void GetFileDateTimeStamp(char *fileName,SYSTEMTIME *stUTC,SYSTEMTIME *stLocal)
//{
//	HANDLE hfile = CreateFile(fileName, GENERIC_READ, FILE_SHARE_READ, NULL,
//	                          OPEN_EXISTING, 0, NULL);
//	FILETIME ftCreate,ftAccess,ftWrite;
//	GetFileTime(hfile, &ftCreate, &ftAccess, &ftWrite);
//	FileTimeToSystemTime(&ftWrite, stUTC);
//	SystemTimeToTzSpecificLocalTime(NULL, stUTC, stLocal);
//}
//
//const char * getIndent( unsigned int numIndents )
//{
//	static const char * pINDENT="                                      + ";
//	static const unsigned int LENGTH=strlen( pINDENT );
//	unsigned int n=numIndents*2;
//	if ( n > LENGTH ) n = LENGTH;
//
//	return &pINDENT[ LENGTH-n ];
//}
//const char * getIndentAlt( unsigned int numIndents )
//{
//	static const char * pINDENT="                                        ";
//	static const unsigned int LENGTH=strlen( pINDENT );
//	unsigned int n=numIndents*2;
//	if ( n > LENGTH ) n = LENGTH;
//
//	return &pINDENT[ LENGTH-n ];
//}
//int dump_attribs_to_stdout(TiXmlElement* pElement, unsigned int indent)
//{
//	if ( !pElement ) return 0;
//
//	TiXmlAttribute* pAttrib=pElement->FirstAttribute();
//	int i=0;
//	int ival;
//	double dval;
//	const char* pIndent=getIndent(indent);
//	printf("\n");
//	while (pAttrib)
//	{
//		printf( "%s%s: value=[%s]", pIndent, pAttrib->Name(), pAttrib->Value());
//
//		if (pAttrib->QueryIntValue(&ival)==TIXML_SUCCESS)    printf( " int=%d", ival);
//		if (pAttrib->QueryDoubleValue(&dval)==TIXML_SUCCESS) printf( " d=%1.1f", dval);
//		printf( "\n" );
//		i++;
//		pAttrib=pAttrib->Next();
//	}
//	return i;
//}
//// Make up net cdf file name
//void MakeNetCdfFileName(char *fileName,char *basePath,TiXmlDocument metaXml,SYSTEMTIME fileTime)
//{
//	TiXmlElement *root = metaXml.RootElement();
//	const char * projectCode = root->FirstChild("project_code")->FirstChild()->Value();
//	const char * experimentCode = root->FirstChild("experiment_code")->FirstChild()->Value();
//	const char * deploymentCode = root->FirstChild("deployment_code")->FirstChild()->Value();
//	//		sprintf(fileName,"%s%s_%s-%s_RADAR_%04u-%02u-%02u_%02u:%02u.nc",basePath,projectCode,experimentCode,deploymentCode,fileTime.wYear,
//	//						fileTime.wMonth,fileTime.wDay,fileTime.wHour,fileTime.wMinute);
//
//	//		sprintf(fileName,"%s%s_%s-%s_RADAR",basePath,projectCode,experimentCode,deploymentCode);
//
//}
//
//
//
//
//
//
//
//void check_err(const int stat, const int line, const char *file) {
//	if (stat != NC_NOERR) {
//		(void)fprintf(stderr,"line %d of %s: %s\n", line, file, nc_strerror(stat));
//		fflush(stderr);
//		exit(1);
//	}
//}
//void MakeNetCdfFile(char *netCdfFileName, short sampleRate, short scans, short bins, short collectionsPerRotation, short rangeGate, short waveSums,TiXmlDocument metaXml)
//{
//	int  stat;  /* return status */
//	int  ncid;  /* netCDF id */
//
//	/* dimension ids */
//	int range_dim;
//	int time_dim;
//	int navtime_dim;
//	int numSystems_dim;
//
//	/* dimension lengths */
//	size_t range_len = bins;
//	size_t time_len = scans*collectionsPerRotation;
//	size_t navtime_len = 1;
//	size_t numSystems_len = 1;
//
//	/* variable ids */
//	int lat_id;
//	int lon_id;
//	int elevation_id;
//	int altitude_id;
//	int heading_id;
//	int navtime_id;
//	int time_id;
//	int azimuth_id;
//	int range_id;
//	int ant_gain_id;
//	int bm_width_id;
//	int pulse_width_id;
//	int pulse_rep_freq_id;
//	int peak_pwr_id;
//	int sample_rate_id;
//	int number_of_rotations_id;
//	int number_of_bins_id;
//	int number_of_collects_id;
//	int number_of_wave_sums_id;
//
//	/* rank (number of dimensions) for each variable */
//#   define RANK_lat 1
//#   define RANK_lon 1
//#   define RANK_elevation 1
//#   define RANK_altitude 0
//#   define RANK_heading 1
//#   define RANK_navtime 1
//#   define RANK_time 1
//#   define RANK_azimuth 1
//#   define RANK_range 1
//#   define RANK_ant_gain 1
//#   define RANK_bm_width 1
//#   define RANK_pulse_width 1
//#   define RANK_pulse_rep_freq 1
//#   define RANK_peak_pwr 1
//#   define RANK_sample_rate 1
//#   define RANK_number_of_rotations 1
//#   define RANK_number_of_bins 1
//#   define RANK_number_of_collects 1
//#   define RANK_number_of_wave_sums 1
//
//	/* variable shapes */
//	int lat_dims[RANK_lat];
//	int lon_dims[RANK_lon];
//	int elevation_dims[RANK_elevation];
//	int heading_dims[RANK_heading];
//	int navtime_dims[RANK_navtime];
//	int time_dims[RANK_time];
//	int azimuth_dims[RANK_azimuth];
//	int range_dims[RANK_range];
//	int ant_gain_dims[RANK_ant_gain];
//	int bm_width_dims[RANK_bm_width];
//	int pulse_width_dims[RANK_pulse_width];
//	int pulse_rep_freq_dims[RANK_pulse_rep_freq];
//	int peak_pwr_dims[RANK_peak_pwr];
//	int sample_rate_dims[RANK_sample_rate];
//	int number_of_rotations_dims[RANK_number_of_rotations];
//	int number_of_bins_dims[RANK_number_of_bins];
//	int number_of_collects_dims[RANK_number_of_collects];
//	int number_of_wave_sums_dims[RANK_number_of_wave_sums];
//
//	const char *value;
//	TiXmlElement *root = metaXml.RootElement();
//
//	/* enter define mode */
//	stat = nc_create(netCdfFileName, NC_CLOBBER, &ncid);
//	check_err(stat,__LINE__,__FILE__);
//
//	/* define dimensions */
//	stat = nc_def_dim(ncid, "range", range_len, &range_dim);
//	check_err(stat,__LINE__,__FILE__);
//	stat = nc_def_dim(ncid, "time", time_len, &time_dim);
//	check_err(stat,__LINE__,__FILE__);
//	stat = nc_def_dim(ncid, "navtime", navtime_len, &navtime_dim);
//	check_err(stat,__LINE__,__FILE__);
//	stat = nc_def_dim(ncid, "numSystems", numSystems_len, &numSystems_dim);
//	check_err(stat,__LINE__,__FILE__);
//
//	/* define variables */
//
//	lat_dims[0] = navtime_dim;
//	stat = nc_def_var(ncid, "lat", NC_FLOAT, RANK_lat, lat_dims, &lat_id);
//	check_err(stat,__LINE__,__FILE__);
//
//	lon_dims[0] = navtime_dim;
//	stat = nc_def_var(ncid, "lon", NC_FLOAT, RANK_lon, lon_dims, &lon_id);
//	check_err(stat,__LINE__,__FILE__);
//
//	elevation_dims[0] = navtime_dim;
//	stat = nc_def_var(ncid, "elevation", NC_FLOAT, RANK_elevation, elevation_dims, &elevation_id);
//	check_err(stat,__LINE__,__FILE__);
//
//	stat = nc_def_var(ncid, "altitude", NC_DOUBLE, RANK_altitude, 0, &altitude_id);
//	check_err(stat,__LINE__,__FILE__);
//
//	heading_dims[0] = navtime_dim;
//	stat = nc_def_var(ncid, "heading", NC_FLOAT, RANK_heading, heading_dims, &heading_id);
//	check_err(stat,__LINE__,__FILE__);
//
//	navtime_dims[0] = navtime_dim;
//	stat = nc_def_var(ncid, "navtime", NC_DOUBLE, RANK_navtime, navtime_dims, &navtime_id);
//	check_err(stat,__LINE__,__FILE__);
//
//	time_dims[0] = time_dim;
//	stat = nc_def_var(ncid, "time", NC_DOUBLE, RANK_time, time_dims, &time_id);
//	check_err(stat,__LINE__,__FILE__);
//
//	azimuth_dims[0] = time_dim;
//	stat = nc_def_var(ncid, "azimuth", NC_FLOAT, RANK_azimuth, azimuth_dims, &azimuth_id);
//	check_err(stat,__LINE__,__FILE__);
//
//	range_dims[0] = range_dim;
//	stat = nc_def_var(ncid, "range", NC_FLOAT, RANK_range, range_dims, &range_id);
//	check_err(stat,__LINE__,__FILE__);
//
//	ant_gain_dims[0] = numSystems_dim;
//	stat = nc_def_var(ncid, "ant_gain", NC_FLOAT, RANK_ant_gain, ant_gain_dims, &ant_gain_id);
//	check_err(stat,__LINE__,__FILE__);
//
//	bm_width_dims[0] = numSystems_dim;
//	stat = nc_def_var(ncid, "bm_width", NC_FLOAT, RANK_bm_width, bm_width_dims, &bm_width_id);
//	check_err(stat,__LINE__,__FILE__);
//
//	pulse_width_dims[0] = numSystems_dim;
//	stat = nc_def_var(ncid, "pulse_width", NC_FLOAT, RANK_pulse_width, pulse_width_dims, &pulse_width_id);
//	check_err(stat,__LINE__,__FILE__);
//
//	pulse_rep_freq_dims[0] = numSystems_dim;
//	stat = nc_def_var(ncid, "pulse_rep_freq", NC_FLOAT, RANK_pulse_rep_freq, pulse_rep_freq_dims, &pulse_rep_freq_id);
//	check_err(stat,__LINE__,__FILE__);
//
//	peak_pwr_dims[0] = numSystems_dim;
//	stat = nc_def_var(ncid, "peak_pwr", NC_FLOAT, RANK_peak_pwr, peak_pwr_dims, &peak_pwr_id);
//	check_err(stat,__LINE__,__FILE__);
//
//	sample_rate_dims[0] = numSystems_dim;
//	stat = nc_def_var(ncid, "sample_rate", NC_FLOAT, RANK_sample_rate, sample_rate_dims, &sample_rate_id);
//	check_err(stat,__LINE__,__FILE__);
//
//	number_of_rotations_dims[0] = numSystems_dim;
//	stat = nc_def_var(ncid, "number_of_rotations", NC_SHORT, RANK_number_of_rotations, number_of_rotations_dims, &number_of_rotations_id);
//	check_err(stat,__LINE__,__FILE__);
//
//	number_of_bins_dims[0] = numSystems_dim;
//	stat = nc_def_var(ncid, "number_of_bins", NC_SHORT, RANK_number_of_bins, number_of_bins_dims, &number_of_bins_id);
//	check_err(stat,__LINE__,__FILE__);
//
//	number_of_collects_dims[0] = numSystems_dim;
//	stat = nc_def_var(ncid, "number_of_collects", NC_SHORT, RANK_number_of_collects, number_of_collects_dims, &number_of_collects_id);
//	check_err(stat,__LINE__,__FILE__);
//
//	number_of_wave_sums_dims[0] = numSystems_dim;
//	stat = nc_def_var(ncid, "number_of_wave_sums", NC_SHORT, RANK_number_of_wave_sums, number_of_wave_sums_dims, &number_of_wave_sums_id);
//	check_err(stat,__LINE__,__FILE__);
//
//	/* assign global attributes */
//	{	/* project */
//		value =root->FirstChild("project")->FirstChild()->Value();
//		stat = nc_put_att_text(ncid, NC_GLOBAL, "project", strlen(value),value);
//		check_err(stat,__LINE__,__FILE__);
//	}
//	{	/* experiment */
//		value =root->FirstChild("experiment")->FirstChild()->Value();
//		stat = nc_put_att_text(ncid, NC_GLOBAL, "experiment", strlen(value),value);
//		check_err(stat,__LINE__,__FILE__);
//	}
//	{	/* experiment_code */
//		value =root->FirstChild("experiment_code")->FirstChild()->Value();
//		stat = nc_put_att_text(ncid, NC_GLOBAL, "experiment_code",strlen(value),value);
//		check_err(stat,__LINE__,__FILE__);
//	}
//	{	/* conventions */
//		value =root->FirstChild("conventions")->FirstChild()->Value();
//		stat = nc_put_att_text(ncid, NC_GLOBAL, "conventions",strlen(value),value);
//		check_err(stat,__LINE__,__FILE__);
//	}
//	{	/* title */
//		value =root->FirstChild("title")->FirstChild()->Value();
//		stat = nc_put_att_text(ncid, NC_GLOBAL, "title", strlen(value),value);
//		check_err(stat,__LINE__,__FILE__);
//	}
//	{	/* institution */
//		value =root->FirstChild("institution")->FirstChild()->Value();
//		stat = nc_put_att_text(ncid, NC_GLOBAL, "institution",strlen(value),value);
//		check_err(stat,__LINE__,__FILE__);
//	}
//	{	/* date_created */
//		value =root->FirstChild("date_created")->FirstChild()->Value();
//		stat = nc_put_att_text(ncid, NC_GLOBAL, "date_created", strlen(value),value);
//		check_err(stat,__LINE__,__FILE__);
//	}
//	{	/* date_modified */
//		value =root->FirstChild("date_modified")->FirstChild()->Value();
//		stat = nc_put_att_text(ncid, NC_GLOBAL, "date_modified",strlen(value),value);
//		check_err(stat,__LINE__,__FILE__);
//	}
//	{	/* abstract */
//		value =root->FirstChild("abstract")->FirstChild()->Value();
//		stat = nc_put_att_text(ncid, NC_GLOBAL, "abstract", strlen(value),value);
//		check_err(stat,__LINE__,__FILE__);
//	}
//	{	/* comment */
//		value =root->FirstChild("comment")->FirstChild()->Value();
//		stat = nc_put_att_text(ncid, NC_GLOBAL, "comment",strlen(value),value);
//		check_err(stat,__LINE__,__FILE__);
//	}
//	{	/* radar_model */
//		value =root->FirstChild("radar")->FirstChild("model")->FirstChild()->Value();
//		stat = nc_put_att_text(ncid, NC_GLOBAL, "radar_model", strlen(value),value);
//		check_err(stat,__LINE__,__FILE__);
//	}
//	{	/* radar_transmit_pulse_width */
//		value =root->FirstChild("radar")->FirstChild("transmit_pulse_width")->FirstChild()->Value();
//		stat = nc_put_att_text(ncid, NC_GLOBAL, "radar_transmit_pulse_width",strlen(value),value);
//		check_err(stat,__LINE__,__FILE__);
//	}
//	{	/* radar_pulse_repetition_Frequency */
//		value =root->FirstChild("radar")->FirstChild("pulse_repetition_Frequency")->FirstChild()->Value();
//		stat = nc_put_att_text(ncid, NC_GLOBAL, "radar_pulse_repetition_Frequency", strlen(value),value);
//		check_err(stat,__LINE__,__FILE__);
//	}
//	{	/* radar_antenna_type */
//		value =root->FirstChild("radar")->FirstChild("antenna")->FirstChild("type")->FirstChild()->Value();
//		stat = nc_put_att_text(ncid, NC_GLOBAL, "radar_antenna_type",strlen(value),value);
//		check_err(stat,__LINE__,__FILE__);
//	}
//	{	/* radar_antenna_horizontal_beam_width */
//		value =root->FirstChild("radar")->FirstChild("antenna")->FirstChild("horizontal_beam_width")->FirstChild()->Value();
//		stat = nc_put_att_text(ncid, NC_GLOBAL, "radar_antenna_horizontal_beam_width",strlen(value),value);
//		check_err(stat,__LINE__,__FILE__);
//	}
//	{	/* radar_antenna_vertical_beam_width */
//		value =root->FirstChild("radar")->FirstChild("antenna")->FirstChild("vertical_beam_width")->FirstChild()->Value();
//		stat = nc_put_att_text(ncid, NC_GLOBAL, "radar_antenna_vertical_beam_width", strlen(value),value);
//		check_err(stat,__LINE__,__FILE__);
//	}
//	{	/* radar_rotation_speed */
//		value =root->FirstChild("radar")->FirstChild("rotation_speed")->FirstChild()->Value();
//		stat = nc_put_att_text(ncid, NC_GLOBAL, "radar_rotation_speed", strlen(value),value);
//		check_err(stat,__LINE__,__FILE__);
//	}
//	{	/* keywords */
//		value =root->FirstChild("keywords")->FirstChild()->Value();
//		stat = nc_put_att_text(ncid, NC_GLOBAL, "keywords", strlen(value),value);
//		check_err(stat,__LINE__,__FILE__);
//	}
//	{	/* references */
//		value =root->FirstChild("references")->FirstChild()->Value();
//		stat = nc_put_att_text(ncid, NC_GLOBAL, "references",  strlen(value),value);
//		check_err(stat,__LINE__,__FILE__);
//	}
//	{	/* netcdf_version */
//		value =root->FirstChild("netcdf_version")->FirstChild()->Value();
//		double netcdf_version_att[1] ;
//		sscanf(value, "%lf", &netcdf_version_att);
//		stat = nc_put_att_double(ncid, NC_GLOBAL, "netcdf_version", NC_DOUBLE, 1, netcdf_version_att);
//		check_err(stat,__LINE__,__FILE__);
//	}
//	{	/* site_code */
//		value =root->FirstChild("site_code")->FirstChild()->Value();
//		stat = nc_put_att_text(ncid, NC_GLOBAL, "site_code",   strlen(value),value);
//		check_err(stat,__LINE__,__FILE__);
//	}
//	{	/* platform_code */
//		value =root->FirstChild("platform_code")->FirstChild()->Value();
//		stat = nc_put_att_text(ncid, NC_GLOBAL, "platform_code",  strlen(value),value);
//		check_err(stat,__LINE__,__FILE__);
//	}
//	{	/* naming_authority */
//		value =root->FirstChild("naming_authority")->FirstChild()->Value();
//		stat = nc_put_att_text(ncid, NC_GLOBAL, "naming_authority", strlen(value),value);
//		check_err(stat,__LINE__,__FILE__);
//	}
//	{	/* file_version */
//		value =root->FirstChild("file_version")->FirstChild()->Value();
//		stat = nc_put_att_text(ncid, NC_GLOBAL, "file_version", strlen(value),value);
//		check_err(stat,__LINE__,__FILE__);
//	}
//	{	/* file_version_quality_control */
//		value =root->FirstChild("file_version_quality_control")->FirstChild()->Value();
//		stat = nc_put_att_text(ncid, NC_GLOBAL, "file_version_quality_control",strlen(value),value);
//		check_err(stat,__LINE__,__FILE__);
//	}
//	{	/* history */
//		value =root->FirstChild("history")->FirstChild()->Value();
//		stat = nc_put_att_text(ncid, NC_GLOBAL, "history", strlen(value),value);
//		check_err(stat,__LINE__,__FILE__);
//	}
//	{	/* geospatial_lat_min */
//		value =root->FirstChild("geospatial_lat_min")->FirstChild()->Value();
//		double geospatial_lat_min_att[1];
//		sscanf(value, "%lf", &geospatial_lat_min_att);
//		stat = nc_put_att_double(ncid, NC_GLOBAL, "geospatial_lat_min", NC_DOUBLE, 1, geospatial_lat_min_att);
//		check_err(stat,__LINE__,__FILE__);
//	}
//	{	/* geospatial_lat_max */
//		value =root->FirstChild("geospatial_lat_max")->FirstChild()->Value();
//		double geospatial_lat_max_att[1] ;
//		sscanf(value, "%lf", &geospatial_lat_max_att);
//		stat = nc_put_att_double(ncid, NC_GLOBAL, "geospatial_lat_max", NC_DOUBLE, 1, geospatial_lat_max_att);
//		check_err(stat,__LINE__,__FILE__);
//	}
//	{	/* geospatial_lon_min */
//		value =root->FirstChild("geospatial_lon_min")->FirstChild()->Value();
//		double geospatial_lon_min_att[1];
//		sscanf(value, "%lf", &geospatial_lon_min_att);
//		stat = nc_put_att_double(ncid, NC_GLOBAL, "geospatial_lon_min", NC_DOUBLE, 1, geospatial_lon_min_att);
//		check_err(stat,__LINE__,__FILE__);
//	}
//	{	/* geospatial_lon_max */
//		value =root->FirstChild("geospatial_lon_max")->FirstChild()->Value();
//		double geospatial_lon_max_att[1] ;
//		sscanf(value, "%lf", &geospatial_lon_max_att);
//		stat = nc_put_att_double(ncid, NC_GLOBAL, "geospatial_lon_max", NC_DOUBLE, 1, geospatial_lon_max_att);
//		check_err(stat,__LINE__,__FILE__);
//	}
//	{	/* geospatial_vertical_min */
//		value =root->FirstChild("geospatial_vertical_min")->FirstChild()->Value();
//		double geospatial_vertical_min_att[1];
//		sscanf(value, "%lf", &geospatial_vertical_min_att);
//		stat = nc_put_att_double(ncid, NC_GLOBAL, "geospatial_vertical_min", NC_DOUBLE, 1, geospatial_vertical_min_att);
//		check_err(stat,__LINE__,__FILE__);
//	}
//	{	/* geospatial_vertical_max */
//		value =root->FirstChild("geospatial_vertical_max")->FirstChild()->Value();
//		double geospatial_vertical_max_att[1] ;
//		sscanf(value, "%lf", &geospatial_vertical_max_att);
//		stat = nc_put_att_double(ncid, NC_GLOBAL, "geospatial_vertical_max", NC_DOUBLE, 1, geospatial_vertical_max_att);
//		check_err(stat,__LINE__,__FILE__);
//	}
//	{	/* data_centre */
//		value =root->FirstChild("data_centre")->FirstChild()->Value();
//		stat = nc_put_att_text(ncid, NC_GLOBAL, "data_centre", strlen(value),value);
//		check_err(stat,__LINE__,__FILE__);
//	}
//	{	/* data_centre_email */
//		value =root->FirstChild("data_centre_email")->FirstChild()->Value();
//		stat = nc_put_att_text(ncid, NC_GLOBAL, "data_centre_email",strlen(value),value);
//		check_err(stat,__LINE__,__FILE__);
//	}
//	{	/* author_email */
//		value =root->FirstChild("author_email")->FirstChild()->Value();
//		stat = nc_put_att_text(ncid, NC_GLOBAL, "author_email", strlen(value),value);
//		check_err(stat,__LINE__,__FILE__);
//	}
//	{	/* author */
//		value =root->FirstChild("author")->FirstChild()->Value();
//		stat = nc_put_att_text(ncid, NC_GLOBAL, "author",strlen(value),value);
//		check_err(stat,__LINE__,__FILE__);
//	}
//	{	/* principal_investigator */
//		value =root->FirstChild("principal_investigator")->FirstChild()->Value();
//		stat = nc_put_att_text(ncid, NC_GLOBAL, "principal_investigator",strlen(value),value);
//		check_err(stat,__LINE__,__FILE__);
//	}
//	{	/* principal_investigator_email */
//		value =root->FirstChild("principal_investigator_email")->FirstChild()->Value();
//		stat = nc_put_att_text(ncid, NC_GLOBAL, "principal_investigator_email", strlen(value),value);
//		check_err(stat,__LINE__,__FILE__);
//	}
//	{	/* institution_references */
//		value =root->FirstChild("institution_references")->FirstChild()->Value();
//		stat = nc_put_att_text(ncid, NC_GLOBAL, "institution_references", strlen(value),value);
//		check_err(stat,__LINE__,__FILE__);
//	}
//	{	/* citation */
//		value =root->FirstChild("citation")->FirstChild()->Value();
//		stat = nc_put_att_text(ncid, NC_GLOBAL, "citation", strlen(value),value);
//		check_err(stat,__LINE__,__FILE__);
//	}
//	{	/* acknowledgement */
//		value =root->FirstChild("acknowledgement")->FirstChild()->Value();
//		stat = nc_put_att_text(ncid, NC_GLOBAL, "acknowledgement",strlen(value),value);
//		check_err(stat,__LINE__,__FILE__);
//	}
//	{	/* distribution_statement */
//		value =root->FirstChild("distribution_statement")->FirstChild()->Value();
//		stat = nc_put_att_text(ncid, NC_GLOBAL, "distribution_statement",strlen(value),value);
//		check_err(stat,__LINE__,__FILE__);
//	}
//
//
//	/* assign per-variable attributes */
//	{	/* long_name */
//		stat = nc_put_att_text(ncid, lat_id, "long_name", 8, "latitude");
//		check_err(stat,__LINE__,__FILE__);
//	}
//	{	/* units */
//		stat = nc_put_att_text(ncid, lat_id, "units", 13, "degrees_north");
//		check_err(stat,__LINE__,__FILE__);
//	}
//	{	/* axis */
//		stat = nc_put_att_text(ncid, lat_id, "axis", 1, "Y");
//		check_err(stat,__LINE__,__FILE__);
//	}
//	{	/* standard_name */
//		stat = nc_put_att_text(ncid, lat_id, "standard_name", 8, "latitude");
//		check_err(stat,__LINE__,__FILE__);
//	}
//	{	/* missing_value */
//		static const float lat_missing_value_att[1] = {-999} ;
//		stat = nc_put_att_float(ncid, lat_id, "missing_value", NC_FLOAT, 1, lat_missing_value_att);
//		check_err(stat,__LINE__,__FILE__);
//	}
//	{	/* valid_range */
//		static const float lat_valid_range_att[2] = {-90, 90} ;
//		stat = nc_put_att_float(ncid, lat_id, "valid_range", NC_FLOAT, 2, lat_valid_range_att);
//		check_err(stat,__LINE__,__FILE__);
//	}
//	{	/* long_name */
//		stat = nc_put_att_text(ncid, lon_id, "long_name", 9, "longitude");
//		check_err(stat,__LINE__,__FILE__);
//	}
//	{	/* units */
//		stat = nc_put_att_text(ncid, lon_id, "units", 12, "degrees_east");
//		check_err(stat,__LINE__,__FILE__);
//	}
//	{	/* axis */
//		stat = nc_put_att_text(ncid, lon_id, "axis", 1, "X");
//		check_err(stat,__LINE__,__FILE__);
//	}
//	{	/* missing_value */
//		static const float lon_missing_value_att[1] = {-999} ;
//		stat = nc_put_att_float(ncid, lon_id, "missing_value", NC_FLOAT, 1, lon_missing_value_att);
//		check_err(stat,__LINE__,__FILE__);
//	}
//	{	/* valid_range */
//		static const float lon_valid_range_att[2] = {-360, 360} ;
//		stat = nc_put_att_float(ncid, lon_id, "valid_range", NC_FLOAT, 2, lon_valid_range_att);
//		check_err(stat,__LINE__,__FILE__);
//	}
//	{	/* long_name */
//		stat = nc_put_att_text(ncid, lon_id, "long_name", 15, "Radar elevation");
//		check_err(stat,__LINE__,__FILE__);
//	}
//	{	/* units */
//		stat = nc_put_att_text(ncid, lon_id, "units", 1, "m");
//		check_err(stat,__LINE__,__FILE__);
//	}
//	{	/* datum */
//		stat = nc_put_att_text(ncid, lon_id, "datum", 22, "mean sea level (AHD94)");
//		check_err(stat,__LINE__,__FILE__);
//	}
//	{	/* axis */
//		stat = nc_put_att_text(ncid, lon_id, "axis", 1, "Z");
//		check_err(stat,__LINE__,__FILE__);
//	}
//	{	/* standard_name */
//		stat = nc_put_att_text(ncid, lon_id, "standard_name", 9, "longitude");
//		check_err(stat,__LINE__,__FILE__);
//	}
//	{	/* long_name */
//		stat = nc_put_att_text(ncid, altitude_id, "long_name", 42, "Altitude in meters (asl) of the instrument");
//		check_err(stat,__LINE__,__FILE__);
//	}
//	{	/* units */
//		stat = nc_put_att_text(ncid, altitude_id, "units", 6, "meters");
//		check_err(stat,__LINE__,__FILE__);
//	}
//	{	/* valid_range */
//		static const float altitude_valid_range_att[2] = {-10000, 90000} ;
//		stat = nc_put_att_float(ncid, altitude_id, "valid_range", NC_FLOAT, 2, altitude_valid_range_att);
//		check_err(stat,__LINE__,__FILE__);
//	}
//	{	/* missing_value */
//		static const float altitude_missing_value_att[1] = {-99999} ;
//		stat = nc_put_att_float(ncid, altitude_id, "missing_value", NC_FLOAT, 1, altitude_missing_value_att);
//		check_err(stat,__LINE__,__FILE__);
//	}
//	{	/* _FillValue */
//		static const double altitude_FillValue_att[1] = {-88888} ;
//		stat = nc_put_att_double(ncid, altitude_id, "_FillValue", NC_DOUBLE, 1, altitude_FillValue_att);
//		check_err(stat,__LINE__,__FILE__);
//	}
//	{	/* long_name */
//		stat = nc_put_att_text(ncid, heading_id, "long_name", 7, "heading");
//		check_err(stat,__LINE__,__FILE__);
//	}
//	{	/* units */
//		stat = nc_put_att_text(ncid, heading_id, "units", 13, "degrees_north");
//		check_err(stat,__LINE__,__FILE__);
//	}
//	{	/* standard_name */
//		stat = nc_put_att_text(ncid, navtime_id, "standard_name", 4, "time");
//		check_err(stat,__LINE__,__FILE__);
//	}
//	{	/* axis */
//		stat = nc_put_att_text(ncid, navtime_id, "axis", 1, "T");
//		check_err(stat,__LINE__,__FILE__);
//	}
//	{	/* units */
//		stat = nc_put_att_text(ncid, navtime_id, "units", 19, "days since 1970-1-1");
//		check_err(stat,__LINE__,__FILE__);
//	}
//	{	/* long_name */
//		stat = nc_put_att_text(ncid, navtime_id, "long_name", 15, "navigation time");
//		check_err(stat,__LINE__,__FILE__);
//	}
//	{	/* standard_name */
//		stat = nc_put_att_text(ncid, time_id, "standard_name", 4, "time");
//		check_err(stat,__LINE__,__FILE__);
//	}
//	{	/* axis */
//		stat = nc_put_att_text(ncid, time_id, "axis", 1, "T");
//		check_err(stat,__LINE__,__FILE__);
//	}
//	{	/* units */
//		stat = nc_put_att_text(ncid, time_id, "units", 19, "days since 1970-1-1");
//		check_err(stat,__LINE__,__FILE__);
//	}
//	{	/* long_name */
//		stat = nc_put_att_text(ncid, time_id, "long_name", 4, "time");
//		check_err(stat,__LINE__,__FILE__);
//	}
//	{	/* standard_name */
//		stat = nc_put_att_text(ncid, azimuth_id, "standard_name", 7, "azimuth");
//		check_err(stat,__LINE__,__FILE__);
//	}
//	{	/* units */
//		stat = nc_put_att_text(ncid, azimuth_id, "units", 17, "degrees_clockwise");
//		check_err(stat,__LINE__,__FILE__);
//	}
//	{	/* long_name */
//		stat = nc_put_att_text(ncid, azimuth_id, "long_name", 13, "radar azimuth");
//		check_err(stat,__LINE__,__FILE__);
//	}
//	{	/* comment */
//		stat = nc_put_att_text(ncid, azimuth_id, "comment", 57, "Degrees are measured clockwise from the radar start pulse");
//		check_err(stat,__LINE__,__FILE__);
//	}
//	{	/* standard_name */
//		stat = nc_put_att_text(ncid, range_id, "standard_name", 5, "range");
//		check_err(stat,__LINE__,__FILE__);
//	}
//	{	/* units */
//		stat = nc_put_att_text(ncid, range_id, "units", 6, "meters");
//		check_err(stat,__LINE__,__FILE__);
//	}
//	{	/* long_name */
//		stat = nc_put_att_text(ncid, range_id, "long_name", 5, "range");
//		check_err(stat,__LINE__,__FILE__);
//	}
//	{	/* missing_value */
//		static const float range_missing_value_att[1] = {-99999} ;
//		stat = nc_put_att_float(ncid, range_id, "missing_value", NC_FLOAT, 1, range_missing_value_att);
//		check_err(stat,__LINE__,__FILE__);
//	}
//	{	/* long_name */
//		stat = nc_put_att_text(ncid, ant_gain_id, "long_name", 12, "Antenna Gain");
//		check_err(stat,__LINE__,__FILE__);
//	}
//	{	/* units */
//		stat = nc_put_att_text(ncid, ant_gain_id, "units", 2, "db");
//		check_err(stat,__LINE__,__FILE__);
//	}
//	{	/* missing_value */
//		static const float ant_gain_missing_value_att[1] = {-999} ;
//		stat = nc_put_att_float(ncid, ant_gain_id, "missing_value", NC_FLOAT, 1, ant_gain_missing_value_att);
//		check_err(stat,__LINE__,__FILE__);
//	}
//	{	/* _FillValue */
//		static const float ant_gain_FillValue_att[1] = {-888} ;
//		stat = nc_put_att_float(ncid, ant_gain_id, "_FillValue", NC_FLOAT, 1, ant_gain_FillValue_att);
//		check_err(stat,__LINE__,__FILE__);
//	}
//	{	/* long_name */
//		stat = nc_put_att_text(ncid, bm_width_id, "long_name", 10, "Beam Width");
//		check_err(stat,__LINE__,__FILE__);
//	}
//	{	/* units */
//		stat = nc_put_att_text(ncid, bm_width_id, "units", 7, "degrees");
//		check_err(stat,__LINE__,__FILE__);
//	}
//	{	/* missing_value */
//		static const float bm_width_missing_value_att[1] = {-999} ;
//		stat = nc_put_att_float(ncid, bm_width_id, "missing_value", NC_FLOAT, 1, bm_width_missing_value_att);
//		check_err(stat,__LINE__,__FILE__);
//	}
//	{	/* _FillValue */
//		static const float bm_width_FillValue_att[1] = {-888} ;
//		stat = nc_put_att_float(ncid, bm_width_id, "_FillValue", NC_FLOAT, 1, bm_width_FillValue_att);
//		check_err(stat,__LINE__,__FILE__);
//	}
//	{	/* long_name */
//		stat = nc_put_att_text(ncid, pulse_width_id, "long_name", 11, "Pulse Width");
//		check_err(stat,__LINE__,__FILE__);
//	}
//	{	/* units */
//		stat = nc_put_att_text(ncid, pulse_width_id, "units", 12, "nano_seconds");
//		check_err(stat,__LINE__,__FILE__);
//	}
//	{	/* missing_value */
//		static const float pulse_width_missing_value_att[1] = {-999} ;
//		stat = nc_put_att_float(ncid, pulse_width_id, "missing_value", NC_FLOAT, 1, pulse_width_missing_value_att);
//		check_err(stat,__LINE__,__FILE__);
//	}
//	{	/* _FillValue */
//		static const float pulse_width_FillValue_att[1] = {-888} ;
//		stat = nc_put_att_float(ncid, pulse_width_id, "_FillValue", NC_FLOAT, 1, pulse_width_FillValue_att);
//		check_err(stat,__LINE__,__FILE__);
//	}
//	{	/* long_name */
//		stat = nc_put_att_text(ncid, pulse_rep_freq_id, "long_name", 24, "Pulse Repation Frequency");
//		check_err(stat,__LINE__,__FILE__);
//	}
//	{	/* units */
//		stat = nc_put_att_text(ncid, pulse_rep_freq_id, "units", 5, "Hertz");
//		check_err(stat,__LINE__,__FILE__);
//	}
//	{	/* missing_value */
//		static const float pulse_rep_freq_missing_value_att[1] = {-999} ;
//		stat = nc_put_att_float(ncid, pulse_rep_freq_id, "missing_value", NC_FLOAT, 1, pulse_rep_freq_missing_value_att);
//		check_err(stat,__LINE__,__FILE__);
//	}
//	{	/* _FillValue */
//		static const float pulse_rep_freq_FillValue_att[1] = {-888} ;
//		stat = nc_put_att_float(ncid, pulse_rep_freq_id, "_FillValue", NC_FLOAT, 1, pulse_rep_freq_FillValue_att);
//		check_err(stat,__LINE__,__FILE__);
//	}
//	{	/* long_name */
//		stat = nc_put_att_text(ncid, peak_pwr_id, "long_name", 10, "Peak Power");
//		check_err(stat,__LINE__,__FILE__);
//	}
//	{	/* units */
//		stat = nc_put_att_text(ncid, peak_pwr_id, "units", 5, "watts");
//		check_err(stat,__LINE__,__FILE__);
//	}
//	{	/* missing_value */
//		static const float peak_pwr_missing_value_att[1] = {-999} ;
//		stat = nc_put_att_float(ncid, peak_pwr_id, "missing_value", NC_FLOAT, 1, peak_pwr_missing_value_att);
//		check_err(stat,__LINE__,__FILE__);
//	}
//	{	/* _FillValue */
//		static const float peak_pwr_FillValue_att[1] = {-888} ;
//		stat = nc_put_att_float(ncid, peak_pwr_id, "_FillValue", NC_FLOAT, 1, peak_pwr_FillValue_att);
//		check_err(stat,__LINE__,__FILE__);
//	}
//	{	/* long_name */
//		stat = nc_put_att_text(ncid, sample_rate_id, "long_name", 11, "Sample Rate");
//		check_err(stat,__LINE__,__FILE__);
//	}
//	{	/* units */
//		stat = nc_put_att_text(ncid, sample_rate_id, "units", 5, "Hertz");
//		check_err(stat,__LINE__,__FILE__);
//	}
//	{	/* missing_value */
//		static const float sample_rate_missing_value_att[1] = {-999} ;
//		stat = nc_put_att_float(ncid, sample_rate_id, "missing_value", NC_FLOAT, 1, sample_rate_missing_value_att);
//		check_err(stat,__LINE__,__FILE__);
//	}
//	{	/* long_name */
//		stat = nc_put_att_text(ncid, number_of_rotations_id, "long_name", 27, "Number of Antenna Rotations");
//		check_err(stat,__LINE__,__FILE__);
//	}
//	{	/* missing_value */
//		static const float number_of_rotations_missing_value_att[1] = {-999} ;
//		stat = nc_put_att_float(ncid, number_of_rotations_id, "missing_value", NC_FLOAT, 1, number_of_rotations_missing_value_att);
//		check_err(stat,__LINE__,__FILE__);
//	}
//	{	/* long_name */
//		stat = nc_put_att_text(ncid, number_of_bins_id, "long_name", 20, "Number of Range Bins");
//		check_err(stat,__LINE__,__FILE__);
//	}
//	{	/* long_name */
//		stat = nc_put_att_text(ncid, number_of_collects_id, "long_name", 34, "Number of collections per rotation");
//		check_err(stat,__LINE__,__FILE__);
//	}
//	{	/* long_name */
//		stat = nc_put_att_text(ncid, number_of_wave_sums_id, "long_name", 34, "Number of wave sums per collection");
//		check_err(stat,__LINE__,__FILE__);
//	}
//
//
//	/* leave define mode */
//	stat = nc_enddef (ncid);
//	check_err(stat,__LINE__,__FILE__);
//
//	/* assign variable data */
//	{
//		float bm_width_data[1] = {0.80000001} ;
//		size_t bm_width_startset[1] = {0} ;
//		size_t bm_width_countset[1] = {1} ;
//		stat = nc_put_vara(ncid, bm_width_id, bm_width_startset, bm_width_countset, bm_width_data);
//		stat = nc_put_vara(ncid, bm_width_id, bm_width_startset, bm_width_countset, bm_width_data);
//		check_err(stat,__LINE__,__FILE__);
//	}
//
//	{
//		float pulse_width_data[1] = {80} ;
//		size_t pulse_width_startset[1] = {0} ;
//		size_t pulse_width_countset[1] = {1} ;
//		stat = nc_put_vara(ncid, pulse_width_id, pulse_width_startset, pulse_width_countset, pulse_width_data);
//		stat = nc_put_vara(ncid, pulse_width_id, pulse_width_startset, pulse_width_countset, pulse_width_data);
//		check_err(stat,__LINE__,__FILE__);
//	}
//
//	{
//		float pulse_rep_freq_data[1] = {2000} ;
//		size_t pulse_rep_freq_startset[1] = {0} ;
//		size_t pulse_rep_freq_countset[1] = {1} ;
//		stat = nc_put_vara(ncid, pulse_rep_freq_id, pulse_rep_freq_startset, pulse_rep_freq_countset, pulse_rep_freq_data);
//		stat = nc_put_vara(ncid, pulse_rep_freq_id, pulse_rep_freq_startset, pulse_rep_freq_countset, pulse_rep_freq_data);
//		check_err(stat,__LINE__,__FILE__);
//	}
//	//	{ short * Mag = new short[
//
//	{
//		float peak_pwr_data[1] = {25000} ;
//		size_t peak_pwr_startset[1] = {0} ;
//		size_t peak_pwr_countset[1] = {1} ;
//		stat = nc_put_vara(ncid, peak_pwr_id, peak_pwr_startset, peak_pwr_countset, peak_pwr_data);
//		stat = nc_put_vara(ncid, peak_pwr_id, peak_pwr_startset, peak_pwr_countset, peak_pwr_data);
//		check_err(stat,__LINE__,__FILE__);
//	}
//
//	{
//		float bm_width_data[1] = {0.80} ;
//		size_t bm_width_startset[1] = {0} ;
//		size_t bm_width_countset[1] = {1} ;
//		stat = nc_put_vara(ncid, bm_width_id, bm_width_startset, bm_width_countset, bm_width_data);
//		stat = nc_put_vara(ncid, bm_width_id, bm_width_startset, bm_width_countset, bm_width_data);
//		check_err(stat,__LINE__,__FILE__);
//	}
//
//	stat = nc_close(ncid);
//	check_err(stat,__LINE__,__FILE__);
//
//
//}
//
//
//long YearMonthDayToJulianDay( const long year, const long month, const long day )
//{
//	long julianDay;
//
//	julianDay = day - 32075L +
//	            1461L * ( year + 4800L + ( month - 14L ) / 12L ) / 4L +
//	            367L * ( month - 2L - ( month - 14L ) / 12L * 12L ) / 12L -
//	            3L * ( ( year + 4900L + ( month - 14L ) / 12L ) / 100L ) / 4L;
//	return julianDay;
//}
//
//
//void JulianDayToYearMonthDay( const long julianDay, int *year, int *month, int *day )
//{
//
//	long t1, t2, yr, mo;
//
//	t1 = julianDay + 68569L;
//	t2 = 4L * t1 / 146097L;
//	t1 = t1 - ( 146097L * t2 + 3L ) / 4L;
//	yr = 4000L * ( t1 + 1L ) / 1461001L;
//	t1 = t1 - 1461L * yr / 4L + 31L;
//	mo = 80L * t1 / 2447L;
//	*day = (int) ( t1 - 2447L * mo / 80L );
//	t1 = mo / 11L;
//	*month = (int) ( mo + 2L - 12L * t1 );
//	*year = (int) ( 100L * ( t2 - 49L ) + yr + t1 );
//}
//
//
//// Retrieves the pathpath from a pathname.
////
//// Returns: SUCCESS if the basepath is present and successfully copied to the p_base_path buffer
////          FAILURE_NULL_ARGUMENT if any arguments are NULL
////          FAILURE_INVALID_ARGUMENTS if either buffer size is less than 1
////          FAILURE_BUFFER_TOO_SMALL if the p_basepath buffer is too small
////          FAILURE_INVALID_PATH if the p_pathname doesn't have a path (e.g. C:, calc.exe, ?qwa)
////          FAILURE_API_CALL if there is an error from the underlying API calls
//
//
//void SplitNameAndPathname( const char*  fullFileName, int length, char *fileName, char *path)
//{
//	int  lastSlash;
//
//	for (int i=0; i<length; i++)
//		if (fullFileName[i]=='\\')
//			lastSlash =i+1;
//	for (int i=0; i<lastSlash; i++)
//		path[i]=fullFileName[i];
//	path[lastSlash]='\0';
//	for (int i=lastSlash; i<length; i++)
//		*fileName++ = fullFileName[i];
//	*fileName = '\0';
//}
