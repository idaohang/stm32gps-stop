#ifndef __GPS_APP_H_
#define __GPS_APP_H_


#define GPS_SUCCESS 0
#define GPS_FAIL    1
#define GPS_ERROR   2
#define GPS_INVALID 3

typedef struct
{
  char time[10];
  unsigned char status;
  char latitude[9];
  unsigned char ns_indic;
  char longitude[10];
  unsigned char ew_indic;
  char speed[16];
  char course[16];
  char date[6];
}ST_GPSRMCINFO, *pST_GPSRMCINFO;


typedef struct
{
	union
	{
	unsigned char s[4];
	int i;
	}utc;
  union
  {
  	unsigned char s[4];
    int           i;   
  }latitude;   // 纬度
  union
  {
  	unsigned char s[4];
    int           i;    
  }longitude;  // 经度
  unsigned char speed;        // 0x00 - 0xFF 0~255km/h
  union
  {
  	unsigned char s[2];
  	unsigned short i;     
  }course;  // 0 - 360度 航向
  unsigned char status;  // 定位状态
}ST_GPSDATA, *pST_GPSDATA;


void GPSPowerOn(void);
void GPSPowerOff(void);
void GPSClearBuffer(void);
unsigned char GPSInfoAnalyze(pST_GPSRMCINFO pRmcInfo);
void ParseGPSInfo(ST_GPSRMCINFO rmcInfo, pST_GPSDATA pGpsData);


#endif // __GPS_APP_H_
