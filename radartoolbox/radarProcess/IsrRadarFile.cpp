/*******************************************************************************
	Program:		Radar Process

	File:			IsrRadarFile.cpp

	Function:		Contains methods for class IsrRadarFile

	Description:	IsrRadarFile processes the M*.bin and A*.txt files generated
					by Dennis Trizna of Imaging Science Research (ISR).

*******************************************************************************/

#include "stdafx.h"
#include "radarrename.h"
#include "anyoption.h"
#include "tinyxml.h"
#include "IsrRadarFile.h"
#include "GpuGrid.h"

#include <windows.h>
#include <tchar.h>
#include <stdio.h>
#include <strsafe.h>
#include <iostream>
#include <string>
#include <fstream>
#include <stdlib.h>
#include <netcdfcpp.h>
#include <boost/filesystem.hpp>
#include <boost/lexical_cast.hpp>
#include <IL/il.h>
#include <GL/gl.h>

//#include "cuda.h"
//#include "cuda_runtime.h"
//#include "device_launch_parameters.h"
//#include "cufft.h"
//#include "kernel.cu"
#define SPEED_OF_LIGHT 299702547
#define cimg_use_tiff 1
//#include "CImg.h"

using namespace boost::filesystem;
using namespace boost::interprocess;
using namespace std;


IsrRadarFile::IsrRadarFile(string fileName)
{
	this->fileName = fileName;
	filesLoaded =false;
	readRadarParams();
	collectionTime = NULL;
	azimuthAngle = NULL;
	metaXml = NULL;
	//	ep = NULL;

}

bool IsrRadarFile::zipperCheck(int frameIndex)
{
	const short *framePtr = static_cast<short*>(dataAddress)+20+numberOfCollectionsPerRotation*numberOfRangeBins*frameIndex;
	short *checkByte;
	for (int i=0; i<numberOfCollectionsPerRotation; i++) {
		if (*framePtr+(i*numberOfRangeBins)+18!=0)
			zipperFound=true;
		return(true);
	}
	zipperFound=false;
	return(false);
}


void IsrRadarFile::getFrameAngles(int frameIndex,float *frameAngle)
{
	memcpy(frameAngle,azimuthAngle+(numberOfCollectionsPerRotation*frameIndex),numberOfCollectionsPerRotation*sizeof(float));
}


void IsrRadarFile::getFrame(int frameIndex,float *frameBuffer, double *frameTime, float *frameAngle)
{
	// Set the frame pointer to the start of the current frame in the binary file
	// (dataAddress points to the start of the memory-mapped binary file)
	const short *framePtr = static_cast<short*>(dataAddress)+10+numberOfCollectionsPerRotation*numberOfRangeBins*frameIndex;
	// Make sure the frame pointer has not moved past the end of the data
	if (framePtr < (static_cast<short*>(dataAddress) + dataSize - (numberOfCollectionsPerRotation*numberOfRangeBins) ) ) {
		// go through each azimuth ("collection") in the frame
		for (int i=0; i<numberOfCollectionsPerRotation; i++) {
			// check for zipper
			// If the 19th sample of the azimuth is not zero, then the zipper effect is present
			if (*(framePtr+(i*numberOfRangeBins) + 18) != 0) { // zipper found
				for (int k=0; k<(numberOfRangeBins-doughNut-4); k++)
					frameBuffer[(i*(numberOfRangeBins-doughNut))+k] =(float)framePtr[(i*numberOfRangeBins)+doughNut-4+k];
				for (int z =numberOfRangeBins-doughNut-4; z<(numberOfRangeBins-doughNut); z++)
					frameBuffer[(i*(numberOfRangeBins-doughNut))+z] =0;
				//memcpy(frameBuffer+(i*(numberOfRangeBins-doughNut)),framePtr+(i*numberOfRangeBins)+doughNut-4,(numberOfRangeBins-doughNut-4)*sizeof(short));
				zipperFound=true;
			} else
				for (int k=0; k<(numberOfRangeBins-doughNut); k++)
					frameBuffer[(i*(numberOfRangeBins-doughNut))+k] =(float)framePtr[(i*numberOfRangeBins)+doughNut+k];

			//memcpy(frameBuffer+(i*(numberOfRangeBins-doughNut)),framePtr+(i*numberOfRangeBins)+doughNut,(numberOfRangeBins-doughNut)*sizeof(short));
		}
		memcpy(frameTime,collectionTime+(numberOfCollectionsPerRotation*frameIndex),numberOfCollectionsPerRotation*sizeof(float));
		getFrameAngles(frameIndex,frameAngle);
	} else
		cout << "over reading frames\r\n";

}


void IsrRadarFile::getGriddedFrame(int frameIndex,short *frameBuffer,int xGridCount,int yGridCount, int xOffset, int yOffset, int gridSize)
{
	float *frameAngle = new float[numberOfCollectionsPerRotation];
	getFrameAngles(frameIndex,frameAngle);
	double interpAngles[1440];
	float y0,y1,x,x0,x1;
	int sumCount =1;
	interpAngles[0] =0;
	for (int i=1; (i<1440); i++) {
		x = (float)i*0.25;
		while ((x>frameAngle[sumCount]) && (sumCount<numberOfCollectionsPerRotation))
			sumCount++;
		if(sumCount<numberOfCollectionsPerRotation) {
			x0 = frameAngle[sumCount-1];
			x1 = frameAngle[sumCount];
			y0 = sumCount-1;
			y1 = sumCount;
			interpAngles[i] = y0 +(x-x0)*((y1-y0)/(x1-x0));
		} else
			interpAngles[i] =-1;
	}
}


void IsrRadarFile::interpFrameAngles(double *frameAngle,double *interpAngles,int interpCount,double stepsize)
{
	float y0,y1,x,x0,x1;
	int sumCount =1;
	interpAngles[0]=0;
	for (int i=1; (i<interpCount); i++) {
		x = (float)i*stepsize;
		while ((x>frameAngle[sumCount]) && (sumCount<numberOfCollectionsPerRotation))
			sumCount++;
		if(sumCount<numberOfCollectionsPerRotation) {
			x0 = frameAngle[sumCount-1];
			x1 = frameAngle[sumCount];
			y0 = sumCount-1;
			y1 = sumCount;
			interpAngles[i] = y0 +(x-x0)*((y1-y0)/(x1-x0));
		} else
			interpAngles[i] =-1;
	}
}


time_t IsrRadarFile::getStartTime()
{
	path p(fileName);
	time_t t = boost::filesystem::last_write_time(p)-totalCaptureTime;
	return(t);
}


bool IsrRadarFile::loadBinaryFile()
{
	filesLoaded = false;
	cout << "\r\nreading Mfile\r\n";
	if (!readMFile())
		return(false);
	cout << "reading Afile\r\n";
	if (!readAFile())
		return(false);
	cout << "reading OK\r\n";
	filesLoaded = true;
	return(true);

}


void IsrRadarFile::pushFrameToMatlab(int index)
{
	/*	if (ep==NULL) {
	//	  if (!(ep = engOpen("\0"))) {
	//		fprintf(stderr, "\nCan't start MATLAB engine\n");
	}
	short *frameBuffer = new short[numberOfCollectionsPerRotation*numberOfRangeBins];
	double *collectionTimes = new double[numberOfCollectionsPerRotation];
	double *collectionAngle = new double[numberOfCollectionsPerRotation];
	getFrame(index,frameBuffer,collectionTimes,collectionAngle);
	mwSize dims[2];
	dims[0] =numberOfCollectionsPerRotation;
	dims[1] =numberOfRangeBins;
	//	   mxArray *data = mxCreateNumericArray(2,dims,mxINT16_CLASS,mxREAL);
	//	   memcpy((void *)mxGetPr(data), (void *)frameBuffer, sizeof(frameBuffer));
	//	   engPutVariable(ep,"fameBuffer",data);
	}*/
}


IsrRadarFile::~IsrRadarFile()
{
	delete(metaXml);
	if (filesLoaded) {

		delete(m_file);
		delete(region);
		delete(collectionTime);
		delete(azimuthAngle);
	}
}


void IsrRadarFile::zipfiles()
{
	string zipFileName =  fileName.substr(0, fileName.length()-3) + "zip";
	string aFileName =  fileName.substr(0, fileName.length()-3) + "txt";
	aFileName[aFileName.find_last_of('M')]='A';
	string zip = "7z a -mx0  -tzip "+zipFileName+" "+fileName.substr(0, fileName.length()-3)+"* "+aFileName+" -x!"+zipFileName+" >>output.txt";
	cout << "\n\rZipping :" <<zip;
	system(zip.c_str());
	cout << "\n\rzip complete";
}


bool IsrRadarFile::readMetaData()
{
	path p(fileName);
	string xmlFileName =  fileName.substr(0, fileName.length()-3) + "xml";
	path x(xmlFileName);
	if (!exists(x)) { // OK lets copy the default xml file!
		string defaultxmlFileName = p.parent_path().string() + "\\" +"metadata.xml";
		string command = "copy "+defaultxmlFileName+" "+xmlFileName;
		system(command.c_str());
	}
	if (exists(x)) {
		metaXml = new TiXmlDocument(xmlFileName.c_str());
		metaXml->LoadFile();
		root = metaXml->RootElement();
		projectCode = root->FirstChild("project_code")->FirstChild()->Value();
		experimentCode = root->FirstChild("experiment_code")->FirstChild()->Value();
		deploymentCode = root->FirstChild("deployment_code")->FirstChild()->Value();
		project =root->FirstChild("project")->FirstChild()->Value();
		experiment =root->FirstChild("experiment")->FirstChild()->Value();
		conventions =root->FirstChild("conventions")->FirstChild()->Value();
		title =root->FirstChild("title")->FirstChild()->Value();
		institution =root->FirstChild("institution")->FirstChild()->Value();
		dateCreated =root->FirstChild("date_created")->FirstChild()->Value();
		dateModified =root->FirstChild("date_modified")->FirstChild()->Value();
		xmlAbstract =root->FirstChild("abstract")->FirstChild()->Value();
		comment =root->FirstChild("comment")->FirstChild()->Value();
		model =root->FirstChild("radar")->FirstChild("model")->FirstChild()->Value();
		transmitPulseWidth =root->FirstChild("radar")->FirstChild("transmit_pulse_width")->FirstChild()->Value();
		doughNut =atoi(root->FirstChild("radar")->FirstChild("dough_nut")->FirstChild()->Value());
		pulseRepetitionFrequency = atoi(root->FirstChild("radar")->FirstChild("pulse_repetition_Frequency")->FirstChild()->Value());
		antennaType =root->FirstChild("radar")->FirstChild("antenna")->FirstChild("type")->FirstChild()->Value();
		horizontalBeamWidth =root->FirstChild("radar")->FirstChild("antenna")->FirstChild("horizontal_beam_width")->FirstChild()->Value();
		verticalBeamWidth =root->FirstChild("radar")->FirstChild("antenna")->FirstChild("vertical_beam_width")->FirstChild()->Value();
		rotationSpeed =root->FirstChild("radar")->FirstChild("rotation_speed")->FirstChild()->Value();
		keywords =root->FirstChild("keywords")->FirstChild()->Value();
		references =root->FirstChild("references")->FirstChild()->Value();
		netcdfVersion =root->FirstChild("netcdf_version")->FirstChild()->Value();
		siteCode =root->FirstChild("site_code")->FirstChild()->Value();
		platformCode =root->FirstChild("platform_code")->FirstChild()->Value();
		namingAuthority =root->FirstChild("naming_authority")->FirstChild()->Value();
		fileVersion =root->FirstChild("file_version")->FirstChild()->Value();
		fileVersionQualityControl =root->FirstChild("file_version_quality_control")->FirstChild()->Value();
		history =root->FirstChild("history")->FirstChild()->Value();
		geospatialLatMin =root->FirstChild("geospatial_lat_min")->FirstChild()->Value();
		geospatialLatMax =root->FirstChild("geospatial_lat_max")->FirstChild()->Value();
		geospatialLonMin =root->FirstChild("geospatial_lon_min")->FirstChild()->Value();
		geospatialLonMax =root->FirstChild("geospatial_lon_max")->FirstChild()->Value();
		geospatialVerticalMin =root->FirstChild("geospatial_vertical_min")->FirstChild()->Value();
		geospatialVerticalMax =root->FirstChild("geospatial_vertical_max")->FirstChild()->Value();
		latitude = atof(root->FirstChild("radar")->FirstChild("position")->FirstChild("lat")->FirstChild()->Value());
		longitude = atof(root->FirstChild("radar")->FirstChild("position")->FirstChild("lon")->FirstChild()->Value());
		northing = atof(root->FirstChild("radar")->FirstChild("position")->FirstChild("northing")->FirstChild()->Value());
		easting = atof(root->FirstChild("radar")->FirstChild("position")->FirstChild("easting")->FirstChild()->Value());
		dataCentre =root->FirstChild("data_centre")->FirstChild()->Value();
		dataCentreEmail =root->FirstChild("data_centre_email")->FirstChild()->Value();
		authorEmail =root->FirstChild("author_email")->FirstChild()->Value();
		author =root->FirstChild("author")->FirstChild()->Value();
		principalInvestigator =root->FirstChild("principal_investigator")->FirstChild()->Value();
		principalInvestigatorEmail =root->FirstChild("principal_investigator_email")->FirstChild()->Value();
		institutionReferences =root->FirstChild("institution_references")->FirstChild()->Value();
		citation =root->FirstChild("citation")->FirstChild()->Value();
		acknowledgement =root->FirstChild("acknowledgement")->FirstChild()->Value();
		distributionStatement =root->FirstChild("distribution_statement")->FirstChild()->Value();
		return (true);
	}
	return(false);
}


int IsrRadarFile::getFileSize()
{
	return(file_size(fileName.c_str()));
}


void IsrRadarFile::makeNetCdfFileName(string productCode, string extension)
{
	time_t t = getStartTime();
	tm *ptm = localtime ( &t );
	char fn[MAX_PATH];
	sprintf(fn,"%s\\%s_%s-%s_RADAR_%04u-%02u-%02u_%02u-%02u_%s.%s",outputPath.c_str(),projectCode.c_str(),experimentCode.c_str(),deploymentCode.c_str(),ptm->tm_year+1900,
	        ptm->tm_mon+1,ptm->tm_mday,ptm->tm_hour,ptm->tm_min,productCode.c_str(),extension.c_str());
	netCdfFileName = string(fn);
}

bool IsrRadarFile::readMFile()
{
	path p(fileName);
	m_file = NULL;
	region = NULL;
	if (exists(p)) {
		m_file = new file_mapping(fileName.c_str(), read_only);
		region = new mapped_region(*m_file, read_only);
		//Get the address of the mapped region
		dataAddress  = region->get_address();
		dataSize  = region->get_size();
		return(true);
	} else
		return(false);
}


char * IsrRadarFile::getAzimuthLine(char *source,int *lineBuffer, char *eof)
{
	lineBuffer[0] =0;
	int numbercount=0;
	while ((*source==' ') && (source<eof) && (numbercount<6))
		source++;
	while ((*source!='\r') && (source<eof) && (numbercount<6)) {
		if ((*source<'0') ||(*source>'9')) {
			while (((*source<'0') ||(*source>'9')) && (*source!='\r') && (source<eof))
				source++;
			numbercount++;
			if (numbercount<6)
				lineBuffer[numbercount] =0;
		}
		if ((source<eof) && (numbercount<6))
			lineBuffer[numbercount]=lineBuffer[numbercount]*10 + ((int)*source++ - '0');
	}
	if (numbercount<6)
		for (int i=numbercount+1; i<6; i++)
			lineBuffer[i] =-999;
	while ((*source!='\r') && (source<eof))
		source++;
	source += 2;
	return(source);
}


bool IsrRadarFile::readAFile()
{
	string aFileName =  fileName.substr(0, fileName.length()-3) + "txt";
	aFileName[aFileName.find_last_of('M')]='A';
	path p(aFileName);
	if (exists(p)) {
		file_mapping *a_file = new file_mapping(aFileName.c_str(), read_only);
		mapped_region *a_region = new mapped_region(*a_file, read_only);
		//		 *m_file;
		//Get the address of the mapped region
		void * dataAddress  = a_region->get_address();
		int dataSize  = a_region->get_size();
		int lineBuffer[6], *lineBuffPointer;
		char * source = (char *)dataAddress;
		char * eof = (char *)dataAddress+dataSize;
		source = getAzimuthLine(source,lineBuffer,eof);
		source = getAzimuthLine(source,lineBuffer,eof);
		source = getAzimuthLine(source,lineBuffer,eof);
		int lineCount = 0;
		int collectionCounter =0;
		int azimuthZero =0;
		if (collectionTime!=NULL) {
			delete[] collectionTime;
			delete[] azimuthAngle;
		}
		collectionTime = new float[numberOfRotations*numberOfCollectionsPerRotation];
		azimuthAngle = new float[numberOfRotations*numberOfCollectionsPerRotation];

		while ((source!=NULL) && (lineCount<numberOfRotations*numberOfCollectionsPerRotation)) {
			source = getAzimuthLine(source,lineBuffer,eof);
			int hour = -999;
			int second = -999;
			int minuet = -999;
			int miliSecond = -999;
			int shaftCount, scanCount;
			//	  sscanf(lineBuffer,"%d %d %d %d %d %d",&scanCount,&shaftCount,&hour,&minuet,&second,&miliSecond);
			if (lineCount % numberOfCollectionsPerRotation==0)
				azimuthZero = lineBuffer[1];
			if (lineBuffer[2]==-999)
				collectionTime[lineCount]=-999;
			else
				collectionTime[lineCount] = ((double)lineBuffer[2]*1/24)+((double)lineBuffer[3]*1/(60*24))+((double)lineBuffer[4]*1/(60*24*60))+((double)lineBuffer[5]*1/(60*24*60*1000));
			azimuthAngle[lineCount] = ((double)(lineBuffer[1]-azimuthZero))*360/4096;
			lineCount++;
		}
		if  (source!=NULL) {
			source = getAzimuthLine(source,lineBuffer,eof);
			totalCaptureTime = lineBuffer[0];
			//		sscanf(lineBuffer,"%d",&totalCaptureTime);
		}
		if (collectionTime[0] == -999) { // no times for radar
			float ratotionInterval =(float)totalCaptureTime / (float)numberOfRotations;
			float incTime = (float) numberOfWaveSumed /(float) pulseRepetitionFrequency;
			float startTime =getStartTime();
			for (int i=0; i<numberOfCollectionsPerRotation*numberOfRotations; i++)

				collectionTime[i]= startTime+ (incTime *float(i % numberOfCollectionsPerRotation)) +(ratotionInterval* (float)(i /  numberOfCollectionsPerRotation));

		}
		delete(a_file);
		delete(a_region);
		return(true);
	}
}


/*	FILE *fp;
			int scanCount,shaftCount;
	char AFileName[MAX_PATH];
	string lineBuffer;
	strncpy(AFileName, path,MAX_PATH);
	strcat(AFileName,"A");
	strncat(AFileName,binFileName+1,7);
	strcat(AFileName,".txt");

	ifstream file (AFileName);
	if (file.is_open())
	{
		while (file.good()) {
			getline(file,lineBuffer);

		}
	}

//	int * angleCount = (int*) malloc((5+colectionsPerRotation*scans)*sizeof(int));
//	if((fp=fopen(AFileName, "r")) == NULL) {
//		 printf("Cannot open file.\n");
		return(false);
//    }
//	fscanf(fp,"%d %d"); // BLOW THE FIRST 4 LINES AWAY
//	fscanf(fp,"%d %d");
//	fscanf(fp,"%d %d");
//	fscanf(fp,"%d %d");
//	for (int line=0;line<colectionsPerRotation*scans;line++) {
//		fscanf(fp,"%d %d",&scanCount,&shaftCount);
//	}

//	return (true); */


// read in the header information from the bin file
void IsrRadarFile::readRadarParams()
{
	numberOfRangeBins = -1;
	ifstream file (fileName, ios::in|ios::binary|ios::ate);
	if (file.is_open()) {
		short *headerblock = new short [10];
		file.seekg (0, ios::beg);
		file.read ((char *)headerblock, 20);
		file.close();
		sampleRate = headerblock[0];
		numberOfRotations = headerblock[1];
		numberOfRangeBins = headerblock[3];
		numberOfCollectionsPerRotation = (headerblock[4]/2)*2; //always even even if set to odd!
		rangeGateCount = headerblock[5];
		numberOfWaveSumed = headerblock[6];
		delete[] headerblock;

	} else
		cout << "cannot read header!";
}


void IsrRadarFile::check_err(const int stat, const int line, const char *file)
{
	if (stat != NC_NOERR) {
		(void)fprintf(stderr,"line %d of %s: %s\n", line, file, nc_strerror(stat));
		fflush(stderr);
		exit(1);
	}
}


void IsrRadarFile::processFile()
{
	cout << "reading metadata\r\n";
	readMetaData();
	cout << "processing";
	TiXmlNode *processing =root->FirstChild("processing");
	TiXmlNode*node;
	for( node = root->FirstChild("processing")->FirstChildElement(); node; node = node->NextSibling()) {
		if (strcmp(node->Value(),"grid")==0) {
			if (!filesLoaded)
				loadBinaryFile();
			string name =node->FirstChild("name")->FirstChild()->Value();
			cout << "\r\nname:" << name;
			string code =node->FirstChild("code")->FirstChild()->Value();
			cout << "\r\ncode:" << code;
			float gridSize = atof(node->FirstChild("grid_size")->FirstChild()->Value());
			cout << "\r\ngridSize:" << gridSize;
			float xSize =atof(node->FirstChild("x_size")->FirstChild()->Value());
			cout << "\r\nxSize:" << xSize;
			float ySize =atof(node->FirstChild("y_size")->FirstChild()->Value());
			cout << "\r\nySize:" << ySize;
			float imageGridSize =atof(node->FirstChild("image_grid_size")->FirstChild()->Value());
			cout << "\r\nimageGridSize:" << imageGridSize;
			float xOffset =atof(node->FirstChild("x_offset")->FirstChild()->Value());
			cout << "\r\nxOffset:" << xOffset;
			float yOffset =atof(node->FirstChild("y_offset")->FirstChild()->Value());
			cout << "\r\nyOffset:" << yOffset;
			float heading =atof(node->FirstChild("rotation_about_radar")->FirstChild()->Value());
			cout << "\r\nheading:" << heading;
			int startFrame =atoi(node->FirstChild("start_frame")->FirstChild()->Value());
			cout << "\r\nstartFrame:" << startFrame;
			int frameCount =atoi(node->FirstChild("frame_count")->FirstChild()->Value());
			cout << "\r\nframeCount:" << frameCount;
			bool xyCoordinates =strcmp(node->FirstChild("coordinates")->FirstChild()->Value(),"xy")==0;
			cout << "\r\nxyCoordinates:" << xyCoordinates;
			bool sumImage =strcmp(node->FirstChild("sum_image")->FirstChild()->Value(),"1")==0;
			cout << "\r\nsumImage:" << sumImage;
			bool outputNcdf =strcmp(node->FirstChild("write_netcdf")->FirstChild()->Value(),"1")==0;
			cout << "\r\noutputNcdf:" << outputNcdf;
			float cLim =atof(node->FirstChild("color_limit")->FirstChild()->Value());
			cout << "\r\ncLim:" << cLim;
			outputPath=node->FirstChild("output_path")->FirstChild()->Value();
			cout << "\r\noutputPath:" << outputPath;
			makeNetCdfFileName(code,"nc");
			processGriddedData(NULL,startFrame,frameCount,xSize,ySize,xOffset,yOffset,gridSize,heading,xyCoordinates,sumImage,outputNcdf,imageGridSize,cLim);
		} else if (strcmp(node->Value(),"polar")==0) {
		} else if (strcmp(node->Value(),"fixedpolar")==0) {

		}
		//	assert( node );
		//	TiXmlElement* itemElement = node->ToElement();
		//	assert( itemElement  );
		//	itemElement->Attribute("
	}
	//  root->FirstChild("experiment_code")->FirstChild()->Value()

}


void IsrRadarFile::saveAsFixedPolarNetCdfFile(const char *fileName)
{
	/*
	if (fileName==NULL)
		fileName = netCdfFileName.c_str();;
	NcFile ncFile(fileName, NcFile::replace);
	NcVar *rangeVar, *timeVar, *collectionVar, *azimuthVar,*collectionTimeVar,*magVar,*latVar,*longVar,*northingVar,*eastingVar;
		/* assign global attributes */
	/*	   ncFile.add_att( "project", project.length(),project.c_str());
	ncFile.add_att( "experiment",experiment.c_str());
	ncFile.add_att( "experiment_code",experimentCode.c_str());
	ncFile.add_att( "conventions",conventions.c_str());
	ncFile.add_att( "title",title.c_str());
	ncFile.add_att( "institution",experimentCode.c_str());
	ncFile.add_att( "date_created",dateCreated.c_str());
	ncFile.add_att( "date_modified",dateModified.c_str());
	ncFile.add_att( "abstract",xmlAbstract.c_str());
	ncFile.add_att( "comment",comment.c_str());
	ncFile.add_att( "radar_model",model.c_str());
	ncFile.add_att( "radar_transmit_pulse_width",transmitPulseWidth.c_str());
	ncFile.add_att( "radar_pulse_repetition_Frequency",pulseRepetitionFrequency);
	ncFile.add_att( "radar_antenna_type",antennaType.c_str());
	ncFile.add_att( "radar_antenna_horizontal_beam_width",horizontalBeamWidth.c_str());
	ncFile.add_att( "radar_antenna_vertical_beam_width",verticalBeamWidth.c_str());
	ncFile.add_att( "radar_rotation_speed",rotationSpeed.c_str());
	ncFile.add_att( "keywords",keywords.c_str());
	ncFile.add_att( "references",references.c_str());
	ncFile.add_att("netcdf_version", atof(netcdfVersion.c_str()));
	ncFile.add_att("site_code",siteCode.c_str());
	ncFile.add_att( "platform_code",platformCode.length(),platformCode.c_str());
	ncFile.add_att( "naming_authority",namingAuthority.length(),namingAuthority.c_str());
	ncFile.add_att( "file_version", fileVersion.length(),fileVersion.c_str());
	ncFile.add_att( "file_version_quality_control",fileVersionQualityControl.length(),fileVersionQualityControl.c_str());
	ncFile.add_att( "history",history.length(),history.c_str());
	ncFile.add_att("geospatial_lat_min",atof(geospatialLatMin.c_str()));
	ncFile.add_att("geospatial_lat_max", atof(geospatialLatMax.c_str()));
	ncFile.add_att("geospatial_lon_min",atof(geospatialLonMin.c_str()));
	ncFile.add_att("geospatial_lon_max",atof(geospatialLonMax.c_str()));
	ncFile.add_att("geospatial_vertical_min",atof(geospatialVerticalMin.c_str()));
	ncFile.add_att("geospatial_vertical_max", atof(geospatialVerticalMax.c_str()));
	ncFile.add_att( "data_centre",dataCentre.length(),dataCentre.c_str());
	ncFile.add_att( "data_centre_email",dataCentreEmail.c_str());
	ncFile.add_att( "author_email",authorEmail.c_str());
	ncFile.add_att( "author",author.c_str());
	ncFile.add_att( "principal_investigator",principalInvestigator.c_str());
	ncFile.add_att( "principal_investigator_email", principalInvestigatorEmail.c_str());
	ncFile.add_att( "institution_references", institutionReferences.c_str());
	ncFile.add_att( "citation",citation.c_str());
	ncFile.add_att( "acknowledgement",acknowledgement.c_str());
	ncFile.add_att( "distribution_statement",distributionStatement.c_str());
	ncFile.add_att("dough_nut", doughNut);
	NcDim *rangeDim, *timeDim, *collectionDim;
	rangeDim =ncFile.addDim("range",numberOfRangeBins-doughNut);
	timeDim =ncFile.addDim("time");

	int inpterpbins = 360.0/0.25;
	collectionDim =ncFile.addDim("collection",inpterpbins);

	rangeVar = ncFile.addVar("range",NcVar:ncFloat,rangeDim);
	ncFile.
	ncFile.addVariableAttribute("range", "units", "m");
	//   units = “days since 1970-01-01T00:00:00Z”
	timeVar = ncFile.add_var("time",ncFloat,timeDim);
	collectionVar = ncFile.add_var("collcection",ncInt,collectionDim);
	// magVar = ncFile.add_var("radar_magnitude",ncShort,timeDim,collectionDim,rangeDim);
	magVar = ncFile.add_var("radar_magnitude",ncFloat,timeDim,collectionDim,rangeDim);
	azimuthVar =ncFile.add_var("azimuth",ncFloat,timeDim,collectionDim);
	collectionTimeVar =ncFile.add_var("collection_time",ncDouble,timeDim,collectionDim);
	latVar =ncFile.add_var("lat",ncFloat);
	latVar->put(&latitude,1);
	longVar =ncFile.add_var("long",ncFloat);
	longVar->put(&longitude,1);
	eastingVar =ncFile.add_var("easting",ncFloat);
	eastingVar->put(&easting,1);
	northingVar =ncFile.add_var("northing",ncFloat);
	northingVar->put(&northing,1);
	// calculate the range bins
	float *distances = new float[numberOfRangeBins-doughNut];
	//
	for (int i=0;i <numberOfRangeBins-doughNut;i++)
		distances[i] = (1+i)*SPEED_OF_LIGHT/(sampleRate*1000000)/2;
	rangeVar->put(distances);
	delete distances;
	//  int *collection = new int[numberOfCollectionsPerRotation];
	//  for (int i=0;i<numberOfCollectionsPerRotation;i++)
	//		   collection[i]=i;
			int *collection = new int[inpterpbins];
	for (int i=0;i<inpterpbins;i++)
		collection[i]=i;

	collectionVar->put(collection);
	delete collection;
	float *times = new float[10];
	for (int i=0;i<10;i++)
		times[i]= collectionTime[i*numberOfCollectionsPerRotation];
	timeVar->put(times);
	// initalise all the GPU Stuff
	GpuGrid gridder(numberOfCollectionsPerRotation,numberOfRangeBins-doughNut,0.25,2000,1000,(float)sampleRate,true);
	float *frameBuffer = gridder.getRawFrameBuffer(); //new float[numberOfCollectionsPerRotation*(numberOfRangeBins-doughNut)];
	double *collectionTimes = new double[inpterpbins];

	float *collectionAngle =  gridder.getRawAngleBuffer(); //new float[inpterpbins];
	zipperFound = false;

	for (int i=0;i<10;i++){ //numberOfRotations
	getFrame(i,frameBuffer,collectionTimes,collectionAngle);
	float * interpframe =gridder.interpolateCartFrame(frameBuffer,collectionAngle,0,0,3,0);
	//   cout << "frame :" << i;
	if (interpframe!=NULL) {
		magVar->put_rec(interpframe,i);
		azimuthVar->put_rec(collectionAngle,i);
		collectionTimeVar->put_rec(collectionTimes,i);
	}
	ncFile.add_att( "zipper_found",zipperFound);
	ncFile.add_att( "dough_nut_removed",(doughNut>0));
	}
	delete (collectionAngle);
	delete (collectionTimes);
	delete (frameBuffer);
	delete (times);
	//  delete (collection);


	*/

}


void IsrRadarFile::processGriddedData(const char *fileName,int startFrame, int frameCount,int xSize,int ySize,int xOffset, int yOffset,float gridSize,float heading, bool xyCoordinates,bool summedImage,bool outputNcdf,float imageGridSize,float cLim)
{
	ILuint texid;
	ILboolean success;
	NcVar *northingVar ,*timeVar, *eastingVar, *magVar;

	//  if (fileName==NULL)
	//	  fileName = netCdfFileName.c_str();;
	//   if (frameCount<0)
	//	   frameCount =numberOfRotations-startFrame;
	//size_t buffer =1024*1024;
	if (frameCount<0)
		frameCount =numberOfRotations-startFrame;
	int inpterpbins = 360.0/0.25;
	NcFile *ncFile = NULL;
	float *times = new float[frameCount];
	if (outputNcdf) {
		size_t t = 1024*1024;
		size_t total = 1024*102481024;
		ncFile = new NcFile(netCdfFileName.c_str(), NcFile::Replace,&t,total,NcFile::Netcdf4);
		/* assign global attributes */
		ncFile->add_att( "project",project.c_str());
		ncFile->add_att( "project",project.c_str());
		ncFile->add_att( "experiment",experiment.c_str());
		ncFile->add_att( "experiment_code",experimentCode.c_str());
		ncFile->add_att( "conventions",conventions.c_str());
		ncFile->add_att( "title",title.c_str());
		ncFile->add_att( "institution",experimentCode.c_str());
		ncFile->add_att( "date_created",dateCreated.c_str());
		ncFile->add_att( "date_modified",dateModified.c_str());
		ncFile->add_att( "abstract",xmlAbstract.c_str());
		ncFile->add_att( "comment",comment.c_str());
		ncFile->add_att( "radar_model",model.c_str());
		ncFile->add_att( "radar_transmit_pulse_width",transmitPulseWidth.c_str());
		ncFile->add_att( "radar_pulse_repetition_Frequency",pulseRepetitionFrequency);
		ncFile->add_att( "radar_antenna_type",antennaType.c_str());
		ncFile->add_att( "radar_antenna_horizontal_beam_width",horizontalBeamWidth.c_str());
		ncFile->add_att( "radar_antenna_vertical_beam_width",verticalBeamWidth.c_str());
		ncFile->add_att( "radar_rotation_speed",rotationSpeed.c_str());
		ncFile->add_att( "keywords",keywords.c_str());
		ncFile->add_att( "references",references.c_str());
		ncFile->add_att("netcdf_version", atof(netcdfVersion.c_str()));
		ncFile->add_att("site_code",siteCode.c_str());
		ncFile->add_att( "platform_code",platformCode.c_str());
		ncFile->add_att( "naming_authority",namingAuthority.c_str());
		ncFile->add_att( "file_version",fileVersion.c_str());
		ncFile->add_att( "file_version_quality_control",fileVersionQualityControl.c_str());
		ncFile->add_att( "history",history.c_str());
		ncFile->add_att("geospatial_lat_min",atof(geospatialLatMin.c_str()));
		ncFile->add_att("geospatial_lat_max", atof(geospatialLatMax.c_str()));
		ncFile->add_att("geospatial_lon_min",atof(geospatialLonMin.c_str()));
		ncFile->add_att("geospatial_lon_max",atof(geospatialLonMax.c_str()));
		ncFile->add_att("geospatial_vertical_min",atof(geospatialVerticalMin.c_str()));
		ncFile->add_att("geospatial_vertical_max",atof(geospatialVerticalMax.c_str()));
		ncFile->add_att( "data_centre",dataCentre.c_str());
		ncFile->add_att( "data_centre_email",dataCentreEmail.c_str());
		ncFile->add_att( "author_email",authorEmail.c_str());
		ncFile->add_att( "author",author.c_str());
		ncFile->add_att( "principal_investigator",principalInvestigator.c_str());
		ncFile->add_att( "principal_investigator_email", principalInvestigatorEmail.c_str());
		ncFile->add_att( "institution_references", institutionReferences.c_str());
		ncFile->add_att( "citation",citation.c_str());
		ncFile->add_att( "acknowledgement",acknowledgement.c_str());
		ncFile->add_att( "distribution_statement",distributionStatement.c_str());
		ncFile->add_att("dough_nut", doughNut);

		NcDim *northingDim =ncFile->add_dim("northing",ySize);
		NcDim *eastingDim = ncFile->add_dim("easting",xSize);
		NcDim *timeDim = ncFile->add_dim("time",frameCount);

		northingVar = ncFile->add_var("northing",ncFloat,northingDim);
		timeVar = ncFile->add_var("time",ncFloat,timeDim);
		timeVar->add_att("units","days since 1970-01-01T00:00:00Z");
		eastingVar = ncFile->add_var("easting",ncFloat,eastingDim);


		// magVar = ncFile->add_var("radar_magnitude",ncShort,timeDim,collectionDim,rangeDim);
		magVar = ncFile->add_var("radar_magnitude",ncShort,timeDim,eastingDim,northingDim);

		// calculate the range bins
		float *northings = new float[ySize];
		//
		if (xyCoordinates) {
			for (int i=0; i<ySize; i++)
				northings[i] = -(yOffset*gridSize) + (gridSize*i);
			northingVar->put(northings,ySize);
			delete northings;
			float *eastngs = new float[xSize];
			for (int i=0; i<xSize; i++)
				eastngs[i]= (gridSize*i)-(xOffset*gridSize);
			eastingVar->put(eastngs,xSize);
			delete eastngs;
		} else {
			for (int i=0; i<ySize; i++)
				northings[i] =northing -(yOffset*gridSize) + (gridSize*i);
			northingVar->put(northings,ySize);
			delete northings;
			float *eastngs = new float[xSize];
			for (int i=0; i<xSize; i++)
				eastngs[i]=easting + (xOffset*gridSize) - (gridSize*i);
			eastingVar->put(eastngs,xSize);
			delete eastngs;
		}

		for (int i=0; i<frameCount; i++)
			times[i]= collectionTime[i*numberOfCollectionsPerRotation];

	}
	timeVar->put(times,frameCount);
	double *collectionTimes = new double[inpterpbins];
	zipperFound = false;
	GpuGrid gridder(numberOfCollectionsPerRotation,numberOfRangeBins-doughNut,0.25,xSize,ySize,(float)sampleRate,true);
	float *frameBuffer = gridder.getRawFrameBuffer();
	float *collectionAngle = gridder.getRawAngleBuffer();
	short *output = new short[xSize*ySize];
	float *meanImage = new float[xSize*ySize];
	for (int i =0; i<xSize*ySize; i++)
		meanImage[i]=0;
	ilInit(); /* Initialization of DevIL */
	ilGenImages(1, &texid); /* Generation of one image name */
	ilEnable(IL_FILE_OVERWRITE);
	ilBindImage(texid); /* Binding of image name */
	cout << "\n\rProcessing frames :";
	for (int i=startFrame; i<frameCount; i++) { //numberOfRotations

		if (i % 50 == 0)
			cout << ".";
		getFrame(i,frameBuffer,collectionTimes,collectionAngle);
		//   cout << "frame :" << i << gridder.isGpuReady();
		float * interpframe =gridder.interpolateCartFrame(frameBuffer,collectionAngle,xOffset,yOffset,gridSize,heading);
		//   cout << "Grid :" << i;
		float *interpFramePointer =interpframe;
		for (int ci=0; ci<xSize*ySize; ci++)
			output[ci]=short(interpframe[ci]);
		if (interpframe!=NULL) {
			if (ncFile!=NULL) {
				magVar->put_rec(output,i);
				//	timeVar->put(&times[i],i);

			}
			if (summedImage)
				sumImageArray(meanImage,interpframe,xSize,ySize,frameCount);

		}
	}
	cout << ".end\r\n";
	if (ncFile!=NULL) {
		ncFile->add_att( "zipper_found",zipperFound);
		ncFile->add_att( "dough_nut_removed",(doughNut>0));
		cout << "close file\r\n";
		cout << "delete ncFile\r\n";
		ncFile->close();
		delete (ncFile);
	}
	if (summedImage) {
		path p(netCdfFileName);
		p.replace_extension(".jpg");
		saveNormalizedMeanJPG(p.string().c_str(),meanImage,1,xSize,ySize,gridSize,imageGridSize,imageGridSize,cLim);
		cout << "Image written:" << p.string() << "\r\n" ;
	}
	delete (collectionTimes);
	delete (times);
	delete (output);
	delete (meanImage);
	//  delete (collection);




}


void IsrRadarFile::sumImageArray(float *meanImage,float *interpframe,int xSize,int ySize,int frameCount)
{
	float *meanp = meanImage;
	float *interpFramePointer =interpframe;
	for (int c=0; c<xSize*ySize; c++) {
		*meanp++ =*meanp+(*interpFramePointer/frameCount);
		interpFramePointer++;
	}
}


bool IsrRadarFile::renameSource(int year)
{
	path p(fileName);
	char newFileName[MAX_PATH];
	if (exists(p)) {
		time_t t = boost::filesystem::last_write_time(p);
		tm *ptm = localtime ( &t );
		if (p.filename().string().length()==12) { //correct length to rename!
			string time =p.filename().string().substr(4,4);
			string exten =p.extension().string();
			sprintf(newFileName,"%s\\%c%04u%02u%02u%s%s",p.parent_path().string().c_str(),p.filename().string().at(0),ptm->tm_year+1900,
			        ptm->tm_mon+1,ptm->tm_mday,time.c_str(),exten.c_str());

			path newFile(newFileName);
			if (!exists(newFile)) // lets do some renameing!
				rename(p,newFile);
			fileName =newFileName;
		}
	}
	string afile = p.parent_path().string()+"\\A"+p.filename().string().substr(1,100);

	path oldAfile(afile);
	oldAfile.replace_extension(".txt");
	if (exists(oldAfile) && (oldAfile.filename().string().length()==12)) {
		time_t t = boost::filesystem::last_write_time(oldAfile);
		tm *ptm = localtime ( &t );
		string time =oldAfile.filename().string().substr(4,4);
		string exten =oldAfile.extension().string();
		sprintf(newFileName,"%s\\%c%04u%02u%02u%s%s",oldAfile.parent_path().string().c_str(),oldAfile.filename().string().at(0),ptm->tm_year+1900,
		        ptm->tm_mon+1,ptm->tm_mday,time.c_str(),exten.c_str());
		path newAfile(newFileName);
		rename(oldAfile,newAfile);
	}
	return(true);
}


void IsrRadarFile::saveNormalizedMeanJPG(const char *fileName,float *meanImage,bool gridOn,int xSize,int ySize,int pixelSize,int xgrid,int ygrid,float cLim)
{
	float *meanp = meanImage;
	for (int c=0; c<xSize*ySize; c++) {
		*meanp =*meanp/cLim; //
		int row = c / ySize;
		int col = (c - row *ySize)*pixelSize;
		row = (row * pixelSize);
		if ((ygrid>0) && (xgrid>0)) {
			if ((row  % ygrid)<pixelSize)
				*meanp = 1-*meanp;
			if ((col % xgrid)<pixelSize)
				*meanp = 1-*meanp;
		}
		meanp++;
	}
	ilTexImage( ySize, xSize, 0, 1,IL_LUMINANCE,IL_FLOAT,meanImage);
	ilSaveImage(fileName);
}


void IsrRadarFile::saveAsSummedImage(const char *fileName,int startFrame, int frameCount,int xSize,int ySize,int xOffset, int yOffset,float gridSize,float heading, float cLim)
{
	ILuint texid;
	ILboolean success;


	if (fileName==NULL)
		fileName = netCdfFileName.c_str();
	if (frameCount<0)
		frameCount =numberOfRotations-startFrame;
	//size_t buffer =1024*1024;

	int inpterpbins = 360.0/0.25;
	GpuGrid *gridder = new GpuGrid(numberOfCollectionsPerRotation,numberOfRangeBins-doughNut,0.25,xSize,ySize,(float)sampleRate,true);
	float *frameBuffer = gridder->getRawFrameBuffer();

	double *collectionTimes = new double[inpterpbins];
	float *collectionAngle = gridder->getRawAngleBuffer();
	zipperFound = false;


	float *meanImage = new float[xSize*ySize];
	for (int i =0; i<xSize*ySize; i++)
		meanImage[i]=0;
	ilInit(); /* Initialization of DevIL */
	ilGenImages(1, &texid); /* Generation of one image name */
	ilEnable(IL_FILE_OVERWRITE);
	ilBindImage(texid); /* Binding of image name */
	for (int i=startFrame; i<frameCount; i++) { //numberOfRotations
		getFrame(i,frameBuffer,collectionTimes,collectionAngle);
		float * interpframe =gridder->interpolateCartFrame(frameBuffer,collectionAngle,xOffset,yOffset,gridSize,heading);
		if (interpframe!=NULL)
			sumImageArray(meanImage,interpframe,xSize,ySize,frameCount);
	}
	frameBuffer = NULL;
	collectionAngle = NULL;
	//   saveNormalizedMeanJPG(fileName,meanImage,1,xSize,ySize,100/gridSize,100/gridSize,cLim);
	delete (gridder);
	//  delete (collectionAngle);
	delete (collectionTimes);
	//  delete (frameBuffer);

	delete (meanImage);
}