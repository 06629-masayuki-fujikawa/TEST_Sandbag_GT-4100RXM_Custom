/*
 * Copyright Notice:
 * Copyright Treck Incorporated  1997 - 2016
 * Copyright Zuken Elmic Japan   1997 - 2016
 * No portions or the complete contents of this file may be copied or
 * distributed in any form (including but not limited to printed or electronic
 * forms) without the expressed written consent of Treck Incorporated OR
 * Zuken Elmic.  Copyright laws and International Treaties protect the
 * contents of this file.  Unauthorized use is prohibited.
 * All rights reserved.
 *
 * Description: Time Functionality
 * Filename: trtime.c
 * Author: Bryan Wang
 * Date Created: 02/16/2004
 * $Source: source/trtime.c $
 *
 * Modification History
 * $Revision: 6.0.2.2 $
 * $Date: 2010/01/18 21:48:55JST $
 * $Author: odile $
 * $ProjectName: /home/mks/proj/tcpip.pj $
 * $ProjectRevision: 6.0.1.33 $
 */


/*
 * Include
 */
#include <trsocket.h>
#include <trmacro.h>
#include <trtype.h>
#include <trproto.h>
#include <trglobal.h>

#define TM_YEAR_BASE    1900
#define EPOCH_YEAR      1970
#define SECSPERMIN      60
#define MINSPERHOUR     60
#define HOURSPERDAY     24
#define DAYSPERWEEK     7
#define THURSDAY        4       /* 0..6 is Sun..Sat (see struct tsTime) */
#define DAYSPERNYEAR    TM_UL(365)
#define DAYSPER400YEARS TM_UL(146097)
#define DAYSPER100YEARS TM_UL(36524)
#define DAYSPER4YEARS   TM_UL(1461)
#define DAYSTO1970      TM_UL(719468)

static ttCharPtr tfCopyString (ttCharPtr destPtr, ttConstCharPtr stringPtr);

static ttConstCharPtr const TM_CONST_QLF tlMonthString[] = 
{
    "Jan", "Feb", "Mar", "Apr", "May", "Jun",
    "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
};

static ttConstCharPtr const TM_CONST_QLF tlWeekString[] = 
{
    "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"
};

/* How many days prior to each month (month index = 0-11).  */
static const tt16Bit TM_CONST_QLF tlMonYearDay[] =
{
    0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334 /* Normal years.*/
};

/* Get the UTC 'ttTime' representation of seconds since mid night 1970 */
int tfGetUtcTime (
    ttConst32BitPtr daysPtr, 
    ttConst32BitPtr secondsPtr, 
    ttTimePtr       timePtr)
{
    int             errorCode;
    tt32Bit         calc;
    tt32Bit         years1, years4, years100, years400;
    tt32Bit         day, month, year;

/* Sanity check. If sign bit of *daysPtr is set, days could overflow. */
    if ( (secondsPtr == (tt32BitPtr)0) || (timePtr == (ttTimePtr) 0) ||
            (daysPtr && ((long)*daysPtr < 0)) )
    {
        errorCode = TM_EINVAL;
    }
    else
    {
        errorCode = TM_ENOERROR;

/* Calculate hours, minutes, seconds and days  */
        calc = *secondsPtr;
        timePtr->tm_sec = (int)(calc % SECSPERMIN);
        calc /= SECSPERMIN;
        timePtr->tm_min = (int)(calc % MINSPERHOUR);
        calc /= MINSPERHOUR;
        timePtr->tm_hour = (int)(calc % HOURSPERDAY);
        calc /= HOURSPERDAY;

/* Get the total number of days since January 1, 1970  */
        if (daysPtr != (tt32BitPtr)0) {
            calc += *daysPtr;
            }

/* January 1, 1970 was a Thursday. Compute day = 0..6 = Sun..Sat  */
        timePtr->tm_wday = (int)((THURSDAY + calc) % DAYSPERWEEK);

/* Use year 0 as reference date for year, month, day calculation  */
        calc += DAYSTO1970;
        years400 = calc / DAYSPER400YEARS;
        calc -= years400 * DAYSPER400YEARS;
        years100 = calc / DAYSPER100YEARS - calc / (DAYSPER400YEARS - 1);
        calc -= years100 * DAYSPER100YEARS;
        years4 = calc / DAYSPER4YEARS;
        calc -= years4 * DAYSPER4YEARS;
        years1 = calc / DAYSPERNYEAR - calc / (DAYSPER4YEARS - 1);
        calc -= years1 * DAYSPERNYEAR;

        day = calc;
        calc = (111 * calc + 41) / 3395;
        day = day - (30 * calc) - (7 * (calc + 1) / 12) + 1;

        calc += 3;
        month = (calc + 11) % 12;
        year = (400 * years400) + (100 * years100) + (4 * years4) + years1
                + (calc / 13);

        timePtr->tm_year = (int)(year - TM_YEAR_BASE);
        timePtr->tm_mon = (int)month;   /* 0..11 = Jan..Dec     */
        timePtr->tm_mday = (int)day;    /* Day of month = 1..31 */

        /* Compute number of days to date in this year  */
        timePtr->tm_yday = (int)day - 1 + (int)tlMonYearDay[(int)month] +
                            (( (int)month >= 2 &&
                              ((year & 3) == 0) &&
                              ((year % 100 != 0) || (year % 400 == 0))
                            ) ? 1 : 0);
    }
    return errorCode;
}


static ttConstCharPtr const TM_CONST_QLF tlTimeHttpZoneFormat = " GMT";
static ttConstCharPtr const TM_CONST_QLF tlTimeSmtpZoneFormat = " 0000";

#define TM_CHAR_SPACE ' '
#define TM_CHAR_COLON ':'
#define TM_CHAR_COMMA ','
#define TM_CHAR_ZERO  '0'

/*
 * Returns pointer to the end of the time string.
 */
ttCharPtr tfGetTimeString(ttTimePtr timePtr, ttCharPtr timeStrPtr,
                          int timeStrLen, int format)
{
/*
 * HTTP time: Thu, 1 Aug 2003 13:00:00 GMT
 * SMTP time: Thu 1, Aug 2003 06:00:00 -0700
 */
    ttConstCharPtr timeZoneStrPtr;


    if (timeStrLen >= 32)
    {
        timeStrPtr = tfCopyString(timeStrPtr, tlWeekString[timePtr->tm_wday]);
        if (format == TM_TIME_FORMAT_HTTP)
        {
            *timeStrPtr++ = TM_CHAR_COMMA; 
            timeZoneStrPtr = tlTimeHttpZoneFormat;
        }
        else /* if (format == TM_TIME_FORMAT_SMTP) */
        {
            timeZoneStrPtr = tlTimeSmtpZoneFormat;
        }
        *timeStrPtr++ = TM_CHAR_SPACE; 
        timeStrPtr = tfUlongDecimalToString( timeStrPtr,
                                             (tt32Bit)timePtr->tm_mday );
        if (format == TM_TIME_FORMAT_SMTP)
        {
            *timeStrPtr++ = TM_CHAR_COMMA; 
        }

        *timeStrPtr++ = TM_CHAR_SPACE; 
        timeStrPtr = tfCopyString(timeStrPtr, tlMonthString[timePtr->tm_mon]);

        *timeStrPtr++= TM_CHAR_SPACE;
        timeStrPtr = tfUlongDecimalToString(
                                timeStrPtr,
                                (tt32Bit)timePtr->tm_year + TM_YEAR_BASE);
        *timeStrPtr++ = TM_CHAR_SPACE;
        if (timePtr->tm_hour <= 9)
        {
            *timeStrPtr++ = TM_CHAR_ZERO;
        }
        timeStrPtr = tfUlongDecimalToString( timeStrPtr,
                                             (tt32Bit)timePtr->tm_hour );

        *timeStrPtr++ = TM_CHAR_COLON;
        if (timePtr->tm_min <= 9)
        {
            *timeStrPtr++ = TM_CHAR_ZERO;
        }
        timeStrPtr = tfUlongDecimalToString( timeStrPtr,
                                             (tt32Bit)timePtr->tm_min );

        *timeStrPtr++ = TM_CHAR_COLON;
        if (timePtr->tm_sec <= 9)
        {
            *timeStrPtr++ = TM_CHAR_ZERO;
        }

        timeStrPtr = tfUlongDecimalToString( timeStrPtr,
                                             (tt32Bit)timePtr->tm_sec );

        timeStrPtr = tfCopyString(timeStrPtr, timeZoneStrPtr);
    }
    else
    {
/* Buffer too small. No conversion done */
        *timeStrPtr = '\0';
    }
    return timeStrPtr;
}

static ttCharPtr tfCopyString (ttCharPtr destPtr, ttConstCharPtr stringPtr)
{
    int size;

    size = (int)tm_strlen(stringPtr);
    tm_strcpy(destPtr, stringPtr);
    destPtr[size] = '\0';
    return (destPtr + size);
}
