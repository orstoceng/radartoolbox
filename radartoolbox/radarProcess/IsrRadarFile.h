/*******************************************************************************
	Program:		Radar Process

	File:			IsrRadarFile.h

	Function:		Contains declarations for class IsrRadarFile

	Description:	See IsrRadarFile.cpp.

*******************************************************************************/

#include "tinyxml.h"
#include "radarrename.h"
#include "anyoption.h"
#include "tinyxml.h"

#include <boost/filesystem.hpp>
#include <boost/interprocess/file_mapping.hpp>
#include <boost/interprocess/mapped_region.hpp>
#include <string>
#include <windows.h>
#include <tchar.h>
#include <stdio.h>
#include <strsafe.h>
#include <iostream>
#include <fstream>
#include <stdlib.h>



#ifndef ISRRADARFILE_H
#define ISRRADARFILE_H


class IsrRadarFile {
	int sampleRate;
	int numberOfRotations;
	int numberOfRangeBins;
	int numberOfCollectionsPerRotation; //always even even if set to odd!
	int rangeGateCount;
	int numberOfWaveSumed;
	int totalCaptureTime;
	int imageGrid;
	int pulseRepetitionFrequency;
	float *azimuthAngle,*collectionTime;
	boost::interprocess::file_mapping *m_file;
	boost::interprocess::mapped_region *region;
	void * dataAddress;
	int dataSize,doughNut;
	float latitude,longitude,northing,easting;
	std::string fileName,netCdfFileName,outputPath;
	TiXmlDocument *metaXml;
	TiXmlElement *root;
	std::string projectCode,experimentCode,deploymentCode,project,experiment,conventions,title,
	       institution,dateCreated,dateModified,xmlAbstract,comment,model,transmitPulseWidth,
	       antennaType,horizontalBeamWidth,verticalBeamWidth,
	       rotationSpeed,keywords,references,netcdfVersion,siteCode,platformCode,
	       namingAuthority,fileVersion,fileVersionQualityControl,history,geospatialLatMin,
	       geospatialLatMax,geospatialLonMin,geospatialLonMax,geospatialVerticalMin,
	       geospatialVerticalMax,dataCentre,dataCentreEmail,authorEmail,author,principalInvestigator,
	       principalInvestigatorEmail,institutionReferences,citation,acknowledgement,distributionStatement;
	int netCdfStat,netCdfFileId,productcode;
	void readRadarParams();
	bool readAFile();
	bool readMFile();
	void makeNetCdfFileName(std::string productCode, std::string extension);
	bool readMetaData();
	void check_err(const int stat, const int line, const char *file);
	int defineNetCdfVariables(int  ncid);
	void pushFrameToMatlab(int index);
	bool zipperCheck(int frameIndex);
	bool zipperFound,filesLoaded;
	void getFrameAngles(int frameIndex,float *frameAngle);
	void interpFrameAngles(double *frameAngle,double *interpAngles,int interpCount,double stepSize);
	void gpuInterplatePloarFrame(int frameIndex,float *frameBuffer,double stepSize);
	char *getAzimuthLine(char *source,int *lineBuffer, char *eof);
	void sumImageArray(float *meanImage,float *interpframe,int xSize,int ySize,int frameCount);
	void saveNormalizedMeanJPG(const char *fileName,float *meanImage,bool gridOn,int xSize,int ySize,int pixelSize,int xgrid,int ygrid,float cLim);
public:
	IsrRadarFile(std::string fileName);
	~IsrRadarFile();
	bool loadBinaryFile();
	int getFileSize();
	void processFile();
	int getSampleRate() {
		return(sampleRate);
	}
	int getScanCount() {
		return(numberOfRotations);
	}
	int getNumberOfRangeBins() {
		return(numberOfRangeBins);
	}
	int getNumberOfCollectionsPerRotation() {
		return(numberOfCollectionsPerRotation);
	}
	int getRangeGateCount() {
		return(rangeGateCount);
	}
	int getNumberOfWaveSumed() {
		return(numberOfWaveSumed);
	}
	std::string getfileName() {
		return(fileName);
	};
	void getFrame(int frameIndex,float *frameBuffer, double *frameTime, float *frameAngle);
	//void getGriddedFrame(int frameIndex,short *frameBuffer,int xGridCount,int yGridCount, int xOffset, int yOffset, int gridSize);
	time_t getStartTime();
	void saveAsFixedPolarNetCdfFile(const char *netCdfFileName);
	void processGriddedData(const char *fileName,int startFrame, int frameCount,int xSize,int ySize,int xOffset, int yOffset,float gridSize, float heading, bool xyCoordinates,bool summedImage,bool outputNcdf,float imageGridSize,float cLim);
	void saveAsSummedImage(const char *fileName,int startFrame, int frameCount,int xSize,int ySize,int xOffset, int yOffset,float gridSize,float heading,  float cLim);
	bool renameSource(int year);
	void zipfiles();
};

#endif