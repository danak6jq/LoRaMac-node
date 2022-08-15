/*!
 * \file      cli.h
 *
 * \brief     Command Line Interface handling implementation
 *
 * \copyright Revised BSD License, see section \ref LICENSE.
 *
 * \code
 *                ______                              _
 *               / _____)             _              | |
 *              ( (____  _____ ____ _| |_ _____  ____| |__
 *               \____ \| ___ |    (_   _) ___ |/ ___)  _ \
 *               _____) ) ____| | | || |_| ____( (___| | | |
 *              (______/|_____)_|_|_| \__)_____)\____)_| |_|
 *              (C)2013-2020 Semtech
 *
 * \endcode
 */
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include "NvmDataMgmt.h"
#include "cli.h"

#include <stdlib.h>
#include <string.h>
#include <math.h>

extern Gpio_t LedRx;

#define GPS_MAX_FIELDS  30

static char gpsBuffer[82];

static enum { GP_BEGIN, GP_COMMA, GP_CHECK } gpsState = GP_BEGIN;
static uint8_t gpsIndex;
static char *gpsField[GPS_MAX_FIELDS];
static char *gpsPtr;

#define htonl(x) ( ((x)<<24 & 0xFF000000UL) | \
                   ((x)<< 8 & 0x00FF0000UL) | \
                   ((x)>> 8 & 0x0000FF00UL) | \
                   ((x)>>24 & 0x000000FFUL) )

#define htons(x) ( ((x)<< 8 & 0x0000FF00UL) | \
                   ((x)>> 8 & 0x000000FFUL) )

uint8_t coords[18];

static float
scanDDMM(char *p)
{
    float v, d;

    v = strtof(p, NULL);
    d = floorf(v / 100.0f);
    v -= d * 100.0f;
    d += v / 60.0f;
    
    return (d);
}

static void
processGpsSentence(uint8_t check)
{
    uint8_t checkVal;
    volatile float lat, lon, hdop, altitude;

    checkVal = strtol(gpsField[gpsIndex], NULL, 16);
    if (checkVal != check || gpsIndex < 2) {
        // invalid sentence, return
        return;
    }

    if (strncmp(gpsField[0], "GPGGA", strlen("GPGGA")) != 0) {
        // not the sentence we want
        return;
    }

    // GGA has 16 total fields
    // 2, 3: lat, 4, 5: long, 6: fix, 8: hdop, 9, 10: altitude
    if (gpsIndex < 15) {
        return;
    }

    *((uint32_t *)(coords + 0)) = 0xffffffff;
    *((uint32_t *)(coords + 4)) = 0xffffffff;

    if (atoi(gpsField[6]) < 1) {
        // no fix
        return;
    }

    lat = scanDDMM(gpsField[2]);
    if (*gpsField[3] == 'S') {
        lat = -lat;
    }

    lon = scanDDMM(gpsField[4]);
    if (*gpsField[5] == 'W') {
        lon = -lon;
    }

    hdop = strtof(gpsField[8], NULL);

    altitude = strtof(gpsField[9], NULL);

    GpioToggle(&LedRx);

    *((uint32_t *)(coords + 0)) = htonl( (uint32_t)(lat * 1000000UL));
    *((uint32_t *)(coords + 4)) = htonl( (uint32_t)(lon * 1000000UL));

    altitude = roundf(altitude);
  
    *((uint16_t *)(coords + 8)) = htons(4000); // mV
    coords[10] = 0x04; // FLAG, LED off, no movement mode, version 1.6.4
    *((uint16_t *)(coords + 11)) = htons(0); // roll
    *((uint16_t *)(coords + 13)) = htons(0); // pitch
    coords[15] = (int8_t)(hdop * 100.0);
    *((uint16_t *)(coords + 16)) = htons((int16_t)(altitude * 100.0)); // altitude
}

static void
processGpsChar(uint8_t c)
{
    static uint8_t checkVal = 0;

    switch (gpsState) {
    case GP_BEGIN:
        if (c != '$') {
            break;
        }
        gpsIndex = 0;
        gpsPtr = gpsBuffer;
        gpsField[gpsIndex] = gpsPtr;
        gpsState = GP_COMMA;
        checkVal = 0;
        break;

    case GP_COMMA:
        if (gpsPtr >= (gpsBuffer + sizeof(gpsBuffer))) {
            // invalid
            gpsState = GP_BEGIN;
            break;
        }

        if (c == ',' || c == '*') {
            *gpsPtr++ = '\0';
            if (c == '*') {
                gpsState = GP_CHECK;
            } else {
                checkVal ^= c;
            }
            gpsField[++gpsIndex] = gpsPtr;
            break;
        } else {
            *gpsPtr++ = c;
            checkVal ^= c;
            break;
        }
        break;

    case GP_CHECK:
        if (c == '\n' || c == '\r') {
            *gpsPtr++ = '\0';
            processGpsSentence(checkVal);
            gpsState = GP_BEGIN;
            break;
        } else {
            *gpsPtr++ = c;
            break;
        }
        break;

    default:
        gpsState = GP_BEGIN;
        break;
    }

}


void CliProcess( Uart_t* uart )
{
    uint8_t data = 0;
    
    if( UartGetChar( uart, &data ) == 0 ) {
        processGpsChar(data);
    }
}