
#ifndef __EELINK_H
#define __EELINK_H

#ifdef __cplusplus
 extern "C" {
#endif

#include "stm32f10x.h"

#define PROTO_EELINK_PACKETLEN_MAX 255u

#define PROTO_EELINK_HEADER 0x70      // normal protocal header
#define PROTO_EELINK_HEADER_SIM 0x68
#define PROTO_EELINK_BUF_MAX 256
 
#define PROTO_LOGIN_BUF 17           // login buffer length
#define PROTO_GPS_BUF  128            // gps buffer length
#define IMEI_INFO_LEN 15             // imei buffer length
#define IMSI_INFO_LEN 15             // imsi buffer length
#define PHONE_NUMBER_LEN 15          // phone number buffer length
#define GSMBASESTATION_INFO_LEN 9

#define PROTO_FACTORY_HEADER   0x72    // factory test header


typedef enum protoEelinkPacketType
{
    PACKET_EELINK_MIN = 0,
    PACKET_EELINK_MAX = 0xFF,
    PACKET_EELINK_LOGIN = 0x1,
    PACKET_EELINK_GPS = 0x2,
    PACKET_EELINK_HEARTBEAT = 0x3,
    PACKET_EELINK_WARNING = 0x4,
    PACKET_EELINK_TERMSTATUS = 0x5,
    PACKET_EELINK_MESSAGEUPLOAD = 0x6,
    PACKET_EELINK_INTERACTIVE = 0x80,
    PACKET_EELINK_MESSAGEDOWNLOAD = 0x81,
    PACKET_EELINK_PHOTOINFO = 0x0E,
    PACKET_EELINK_PHOTODATA = 0x0F,
    PACKET_EELINK_OBDDATA = 0x07,
    PACKET_EELINK_OBDERR = 0x08,
    PACKET_FACTORY_REPORT = 0x77,
} PROTO_EELINK_PACKET_TYPE;

typedef struct eelink_packet_header
{
    uint8_t header[2];
    uint8_t type;
    uint8_t len[2];
    uint8_t seq[2];
} EELINK_PACKET_HEADER;

typedef struct eelink_sim_packet_login
{
    EELINK_PACKET_HEADER hdr;

    uint8_t imei[IMEI_INFO_LEN];
    uint8_t lang;
    uint8_t zone;
} EELINK_SIM_PACKET_LOGIN;

typedef struct eelink_packet_gps
{
    EELINK_PACKET_HEADER hdr;

    uint8_t utctime[4];
    uint8_t lati[4];
    uint8_t longi[4];
    uint8_t speed;
    uint8_t course[2];
    uint8_t basestation[GSMBASESTATION_INFO_LEN];
    uint8_t gps_valid;
    uint8_t dev_status[2];
    uint8_t battery_voltage[2];
    uint8_t signal_strength[2];
    uint8_t analog_input1[2];
    uint8_t analog_input2[2];
} EELINK_PACKET_GPS;


typedef struct eelink_term_info
{
    uint8_t imei[IMEI_INFO_LEN];
    uint32_t rtime;
    uint32_t longi;
    uint32_t lati;
    uint8_t speed;
    uint16_t direction;
    uint8_t pos_sts;
    uint16_t dev_sts;
    uint8_t gsm_station[GSMBASESTATION_INFO_LEN];
    uint16_t gsm_strength;
    uint16_t  battery;
} EELINK_TERM_INFO;

#ifdef __cplusplus
}
#endif

#endif // __EELINK_H

