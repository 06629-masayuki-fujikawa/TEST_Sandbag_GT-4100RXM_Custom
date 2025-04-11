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
 * Description: FTP functions used by both the FTP server and NAT
 * Filename: trftplib.c
 * Author: Odile
 * Date Created: 03/22/01
 * $Source: source/sockapi/trftplib.c $
 *
 * Modification History
 * $Revision: 6.0.2.2 $
 * $Date: 2010/01/18 21:43:09JST $
 * $Author: odile $
 * $ProjectName: /home/mks/proj/tcpip.pj $
 * $ProjectRevision: 6.0.1.33 $
 */

#include <trsocket.h>
#include <trmacro.h>
#include <trtype.h>
#include <trproto.h>

/*
 * Local types
 */
typedef union tuInet16Bit
{
    tt16Bit     inetShort;
    tt8Bit      inetArr[2];
} ttInet16Bit;

typedef union tuInet32Bit
{
    tt32Bit     inetLong;
    tt8Bit      inetArr[4];
} ttInet32Bit;


/*
 * tfFtpHxPxFormatToSockAddr function description:
 * Common between FTPD, and NAT.
 * FTPD uses this function to scan an FTP PORT command.
 * NAT uses this function to scan an FTP port, and PASV command.
 * FTP could use this function to scan a PASV command.
 * Convert from "h1,h2,h3,h4,p1,p2" format to a sockaddr.
 * Parameters
 * Value          Meaning
 * bufferPtr      Pointer to h1.
 * sockAddrPtr    Pointer to a socket address structure where to store the
 *                IP address, and port.
 * Return value:
 * length of the "h1,h2,h3,h4,p1,p2"    success.
 * 0                                    failure
 */
int tfFtpHxPxFormatToSockAddr ( ttCharPtr       bufferPtr,
                                ttSockAddrInPtr sockAddrInPtr )
{
    int                         commandIndex;
    int                         commaIndex;
    int                         errorCode;
    int                         digitCount;
    ttInet16Bit                 inet16Bit;
    ttInet32Bit                 inet32Bit;
    tt16Bit                     currentShort;
    tt8Bit                      digitCh;

    inet16Bit.inetShort = 0;
    inet32Bit.inetLong = 0;
    commaIndex = 0;
    commandIndex = 0;
    errorCode = TM_ENOERROR;
    commaIndex = 0;
    currentShort = (tt16Bit)0;
    digitCount = 0;
    errorCode = TM_ENOERROR;
    while (    ( bufferPtr[commandIndex] != '\0' )
            && ( (unsigned)commaIndex < sizeof(tt4IpAddress)
                 + sizeof(ttIpPort))
            && ( errorCode == TM_ENOERROR ) )
    {
/* skip over spaces before digit */
        while (bufferPtr[commandIndex] == ' ')
        {
            commandIndex++;
        }
        digitCh = (tt8Bit)bufferPtr[commandIndex];
/* PRQA: QAC Message 3416: The calling function doesn't have the side */
/* PRQA: QAC Message 3416: effect for the comparison processing */
/* PRQA S 3416 L1 */
        while (tm_isdigit(digitCh))
/* PRQA L:L1 */
        {
            digitCh = (tt8Bit)(digitCh - '0');
            currentShort = (tt16Bit)
                              (   (tt16Bit)digitCh
                                + (tt16Bit)((tt16Bit)currentShort * 10) );
            digitCount++;
            commandIndex++;
            digitCh = (tt8Bit)bufferPtr[commandIndex];
        }
/* skip over spaces after digit and before comma */
        while (bufferPtr[commandIndex] == ' ')
        {
            commandIndex++;
        }
        if (    (digitCount != 0)
             && (currentShort <= 255) )
/* At leat one digit, number less or equal to 255 */
        {
            if ((unsigned)commaIndex < sizeof(tt4IpAddress))
            {
                if (bufferPtr[commandIndex] == ',' )
/* Separator has to be a comma */
                {
                    inet32Bit.inetArr[commaIndex] = (tt8Bit)currentShort;
                    digitCount = 0;
                    commaIndex++;
                    currentShort = (tt16Bit)0;
                    commandIndex++;
                }
                else
                {
                    errorCode = TM_EINVAL;
                }
            }
            else
            {
                if (    (unsigned)commaIndex == sizeof(tt4IpAddress)
                     && (bufferPtr[commandIndex] != ',') )
/* Separator has to be a comma between the 2 port fields (p1,p2). */
                {
                    errorCode = TM_EINVAL;
                }
                else
                {
                    inet16Bit.inetArr[
                        (unsigned)commaIndex - sizeof(tt4IpAddress)]
                        = (tt8Bit)currentShort;
                    digitCount = 0;
                    commaIndex++;
                    currentShort = (tt16Bit)0;
                    commandIndex++;
                }
            }
        }
        else
        {
            errorCode = TM_EINVAL;
        }
    }
    if (  (errorCode == TM_ENOERROR)
        &&((unsigned)commaIndex == (sizeof(tt4IpAddress) + sizeof(ttIpPort))) )
    {
        sockAddrInPtr->ssin_port = inet16Bit.inetShort;
        sockAddrInPtr->ssin_addr.s_addr = inet32Bit.inetLong;
        commandIndex--;
    }
    else
    {
        commandIndex = 0;
    }
    return commandIndex;
}

/*
 * tfFtpSockAddrToHxPxFormat function description:
 * Common between FTP, FTPD, and NAT.
 * Convert a socket address to the "h1,h2,h3,h4,p1,p2\0" format. 
 * The server uses this function to build a reply to a PASV command.
 * NAT uses this function to fix a PORT, or PASV command
 * The FTP client could use this function to build a PORT command.
 * Parameters
 * Value          Meaning
 * sockAddrPtr    Pointer to a socket address structure containing the
 *                IP address, and port.
 * bufferPtr      Pointer to a buffer, where the resulting string will be
 *                stored.
 *
 * Return value:
 * length of the converted string.
 */
int tfFtpSockAddrToHxPxFormat( ttSockAddrInPtr   sockAddrPtr,
                               ttCharPtr         bufferPtr )
{
    ttCharPtr aHostPtr;
    ttCharPtr aPortPtr;
    int       commandIndex;
    int       commaIndex;
    int       tempInt;
    int       digit;
    static const char  TM_CONST_QLF tlAsciiDigit[10] = \
                {'0','1','2','3','4','5','6','7','8','9'};

    aHostPtr = (ttCharPtr)&sockAddrPtr->ssin_addr;
    aPortPtr = (ttCharPtr)&sockAddrPtr->ssin_port;
    commandIndex = 0;
    commaIndex = 0;
    while (   (unsigned)commaIndex
            < (sizeof(tt4IpAddress)+sizeof(ttIpPort)) )
    {
        if ((unsigned)commaIndex < sizeof(tt4IpAddress))
        {
            tempInt = ((int)aHostPtr[commaIndex]) & 0xFF;
        }
        else
        {
            tempInt =   ( (int)aPortPtr[   commaIndex
                                         - (int)sizeof(tt4IpAddress)] )
                       & 0xFF;
        }
        if (tempInt >= 100)
        {
            digit = tempInt/100;
            tempInt = tempInt % 100;
            bufferPtr[commandIndex] = tlAsciiDigit[digit];
            commandIndex++;
            if (tempInt < 10)
/* Middle digit is zero */
            {
                bufferPtr[commandIndex] = tlAsciiDigit[0];
                commandIndex++;
            }
        }
        if (tempInt >= 10)
        {
            digit = tempInt/10;
            tempInt = tempInt % 10;
            bufferPtr[commandIndex] = tlAsciiDigit[digit];
            commandIndex++;
        }
        bufferPtr[commandIndex] = tlAsciiDigit[tempInt];
        commandIndex++;
        commaIndex++;
        if( (unsigned)commaIndex != (sizeof(tt4IpAddress) + sizeof(ttIpPort)) )
        {
            bufferPtr[commandIndex++] = ',';
        }
        else
        {
            bufferPtr[commandIndex] = '\0';
        }
    }
    return commandIndex;
}

/*
 * The above routines have been tested with the following test program:
 */
#ifdef TM_TESTING_FTP_LIB
char buffer1[] = "129,200,0,3,40,0";
char buffer2[] = "2500,200,1,3,40,0";
char buffer3[] = "129,2500,2500,3,40,0";
char buffer4[] = "129,200,2500,3,40,0";
char buffer5[] = "129,200,1,2500,40,0";
char buffer6[] = "129,200,1,3,2500,0";
char buffer7[] = "129,200,1,3,0,2500";
char buffer8[] = "  129 , 200, 0, 3, 40, 0  ";
char buffer9[] = " .. 129 , 200, 0, 3, 40, 0  ";
char buffera[] = "129, ...200,0,3,40,0   ";
char bufferb[] = "129,\r200,0,3,40,0";
char bufferc[] = "129,\\r200,0,3,40,0";
char bufferd[] = "\\r129,200,0,3,40,0";
char buffere[] = " 129 , 200, 0, 3, 40, 0  )";
char bufferf[] = " 129 , 200, 0, 3, 40, 0\\r";
char bufferg[] = " 129 , 200, 0, 3, 40, 0  \\r\\n";
char bufferh[] = "129,200,0,0,0,0";
char bufferi[] = "129,200,0,0,0,20";
char bufferj[] = "0,200,10,0,0,20";
char bufferk[] = "0, \\0200,10,0,0,20";
char bufferl[] = "0, 000,00,0,0,00";
char bufferm[] = "0\0, 000,00,0,0,00";
char buffern[] = "\00, 000,00,0,0,00";
char buffero[] = "\r0, 000,00,0,0,00";
char bufferp[] = "129,200,,3,40,0";
char bufferq[] = "129,200,,3,,0";
char bufferr[] = "129,200,0010,3,10,10";
char buffers[] = "129,200,0010,3,10 0";

void test (ttCharPtr bufferPtr, ttSockAddrInPtr sockAddrInPtr);

void main (void)
{

    ttSockAddrIn sockAddrIn;

    test(buffer1, &sockAddrIn);
    test(buffer2, &sockAddrIn);
    test(buffer3, &sockAddrIn);
    test(buffer4, &sockAddrIn);
    test(buffer5, &sockAddrIn);
    test(buffer6, &sockAddrIn);
    test(buffer7, &sockAddrIn);
    test(buffer8, &sockAddrIn);
    test(buffer9, &sockAddrIn);
    test(buffera, &sockAddrIn);
    test(bufferb, &sockAddrIn);
    test(bufferc, &sockAddrIn);
    test(bufferd, &sockAddrIn);
    test(buffere, &sockAddrIn);
    test(bufferf, &sockAddrIn);
    test(bufferg, &sockAddrIn);
    test(bufferh, &sockAddrIn);
    test(bufferi, &sockAddrIn);
    test(bufferj, &sockAddrIn);
    test(bufferk, &sockAddrIn);
    test(bufferl, &sockAddrIn);
    test(bufferm, &sockAddrIn);
    test(buffern, &sockAddrIn);
    test(buffero, &sockAddrIn);
    test(bufferp, &sockAddrIn);
    test(bufferq, &sockAddrIn);
    test(bufferr, &sockAddrIn);
    test(buffers, &sockAddrIn);
}

void test (ttCharPtr bufferPtr, ttSockAddrInPtr sockAddrInPtr)
{
    char buffer[128];
    int  len;

    len = tfFtpHxPxFormatToSockAddr(bufferPtr, sockAddrInPtr);
    if (len != 0)
    {
        printf("%d, HxPxToSockAddr success '%s' '%s' %x\n",
               len,
               bufferPtr,
               inet_ntoa(sockAddrInPtr->ssin_addr),
               ntohs(sockAddrInPtr->ssin_port));
        len = tfFtpSockAddrToHxPxFormat(sockAddrInPtr, buffer);
        if (len != 0)
        {
            printf("%d, SockAddrToHxPx success '%s'\n", len, buffer);
        }
        else
        {
            printf("SockAddrToHxPx failed\n");
        }
    }
    else
    {
        printf("HxPxToSockAddr failed '%s'\n", bufferPtr);
    }
}
#endif /* TM_TESTING_FTP_LIB */
