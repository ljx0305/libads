/*
 Implementation of BECKHOFF's ADS protocol.
 ADS = Automation Device Specification
 Implemented according to specifications given in TwinCAT Information System
 May 2011.
 TwinCAT, ADS and maybe other terms used herein are registered trademarks of
 BECKHOFF Company. www.beckhoff.de

 Copyright (C) Thomas Hergenhahn (thomas.hergenhahn@web.de) 2003.
 Copyright (C) Luis Matos (gass@otiliamatos.ath.cx) 2009.
 Copyright (C) Gerhard Schiller (gerhard.schiller@gmail.com) 2013.

 This file is part of libads.  
 Libads is free software: you can redistribute it and/or modify
 it under the terms of the Lesser GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 libads is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 Lesser GNU General Public License for more details.

 You should have received a copy of the Lesser GNU General Public License
 along with libads.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifdef __cplusplus

extern "C" {
#endif

#ifndef __ADS_H__
#define __ADS_H__

#pragma pack (push)
#pragma pack (1)

#include "AdsDEF.h"
#include <stdio.h>

/* 
    Some simple types:
*/

#ifdef __linux__
	typedef struct dost {
		int rfd;
		int wfd;
//    int connectionType;
	} _ADSOSserialType;
#include <stdlib.h>
#define tmotype int
#endif

#define MAXDATALEN 8192

/* define heade sizes aond positions */
#define AMSTCPHeaderSIZE 6
#define AMSHeaderSIZE 32
#define ADSTCPDataStart (AMSTCPHeaderSIZE + AMSHeaderSIZE)

typedef struct _AMS_TCPheader {
	unsigned short	reserved;	/** must be zero */
	unsigned int 	length;	 	/** length in bytes: AMSheader + ADS data */
} AMS_TCPheader;

/**
    The AMS header.
*/
typedef struct _AMSheader {
	AmsNetId 		targetId;
	unsigned short	targetPort;
	AmsNetId 		sourceId;
	unsigned short	sourcePort;
	unsigned short	commandId;
	unsigned short	stateFlags;
	unsigned int	dataLength; /** bytes in data section of an ADS packet */
	unsigned int	errorCode;
	unsigned int	invokeId;
} AMSheader;


/**
 * \brief Ads Command Id for AmsHeader
 * Other commands are not defined or are used internally.
 * Therefore the Command Id is only allowed to contain the above
 * enumerated values!
 */
enum nAdsCommandId {
	cmdADSinvalid =					 0x0000, /**Invalid*/
	cmdADSreadDevInfo =				 0x0001, /**ADS Read Device Info*/
	cmdADSread =					 0x0002, /**ADS Read*/
	cmdADSwrite =					 0x0003, /**ADS Write*/
	cmdADSreadState =				 0x0004, /**ADS Read State*/
	cmdADSwriteControl =	 		 0x0005, /**ADS Write Control*/
	cmdADSaddDeviceNotification =	 0x0006, /**ADS Add Device Notification*/
	cmdADSdeleteDeviceNotification = 0x0007, /**ADS Delete Device Notification*/
	cmdADSdevNotify =				 0x0008, /**ADS Device Notification*/
	cmdADSreadWrite =				 0x0009	 /**ADS Read Write*/
} AdsCommandId;
/*
    Definitions of bits in stateFlags:
*/
#define sfAMSresponse	0x001	// zero means request
#define sfAMScommand	0x004	// indicates data exchange via ADS command
#define sfAMSudp		0x400	// indicates transport over UDP (0 mens TCP)

typedef struct _ADSpacket {
	AMS_TCPheader adsHeader;
	AMSheader	  amsHeader;
	char		  data[MAXDATALEN];
} ADSpacket;

typedef struct _ADSdeviceInfo {
	unsigned int result;
	AdsVersion 	Version;
	char 		name[24];		// rely on 24 bytes, not a terminating \0
} ADSdeviceInfo;

typedef struct _ADSreadRequest {
	unsigned int indexGroup;		// index Group the data belongs to
	unsigned int indexOffset;	// offset within index Group
	unsigned int length;			// length in bytes to be read
} ADSreadRequest;

/*
    Definitions for index Group:
 */
#define igADSfirstReserved			0x0000
#define igADSlastReserved			0x0FFF
#define igADSParameter				0x1000
#define igADSState					0x2000
#define igADSdevFunction			0x3000
#define igADScommonFunction			0x4000	// contains functions to access
											// PLC process data image
#define igADSfirstReservedExt		0x6000	// first iG reserved for extensions
#define igADSlastReservedExt		0xEFFF	// last iG reserved for extensions
#define igADSfirstTwinCATsysService	0xF000	// general TwinCAT ADS
											// system services...
#define igADSlastTwinCATsysService	0xFFFF	// ...containing functions to
											// access PLC I/O image
#define errADSok					0x0000
#define errADSInternal				0x0001
#define errADSNoRuntime				0x0002
#define errADSAllocLockedMemory		0x0003

typedef struct _ADSreadResponse {
	unsigned int result;		// error code
	unsigned int length;		// length in bytes of response
	char 		 data[MAXDATALEN];
} ADSreadResponse;

typedef struct _ADSwriteRequest {
	unsigned int indexGroup;
	unsigned int indexOffset;
	unsigned int length;		// length in bytes of response
	char 		 data[MAXDATALEN];
} ADSwriteRequest;

typedef struct _ADSwriteResponse {
	unsigned int result;		// error code
} ADSwriteResponse;

typedef struct _ADSstateResponse {
	unsigned int   result;
	unsigned short ADSstate;
	unsigned short devState;
} ADSstateResponse;

typedef struct _ADSwriteControlRequest {
	unsigned short ADSstate;
	unsigned short devState;
	unsigned int   length;
	unsigned char  data[];
} ADSwriteControlRequest;

typedef struct _ADSaddNotificationRequest {
	unsigned int  indexGroup;		// index Group the data belongs to
	unsigned int  indexOffset;		// offset within index Group
	unsigned int  length;			// length in bytes to be read
	unsigned int  transmissionMode;	//
	unsigned int  maxDelay;			// after this time the notification will be
									// called up. unit is 100ns
	unsigned int  cycleTime;		// after this time the server tests whether
									// values did change, unit is 100ns
	unsigned char reserved[16];
} ADSaddDeviceNotificationRequest;

typedef struct _ADSaddNotificationResponse {
	unsigned int result;
	unsigned int notificationHandle;
} ADSaddDeviceNotificationResponse;

typedef struct _ADSreadWriteRequest {
	unsigned int indexGroup;
	unsigned int indexOffset;
	unsigned int readLength;	// length in bytes of response
	unsigned int writeLength;	// length in bytes of request
	char 		 data[MAXDATALEN];
} ADSreadWriteRequest;

typedef struct _ADSreadWriteResponse {
	unsigned int result;
	unsigned int length;
	char 		 data[MAXDATALEN];
} ADSreadWriteResponse;

/*
    Library specific stuff:
*/

#define ADSDebugOpen 0x10
#define ADSDebugPacket 0x20
#define ADSDebugByte 0x40
#define ADSDebugAnalyze 0x80
#define ADSDebugPrintErrors 0x8

/* debug by command */
#define ADSDebugReadState 0x90
#define ADSDebugDeviceInfo 0x91
#define ADSDebugCommands (ADSDebugReadState|ADSDebugDeviceInfo)

/* all or none */
#define ADSDebugAll (ADSDebugOpen|ADSDebugPacket|ADSDebugByte|ADSDebugAnalyze|ADSDebugPrintErrors|ADSDebugCommands)
#define ADSDebugNone 0x0

#ifndef ADSDebug
#define ADSDebug ADSDebugNone
#endif

/* 
    This is a wrapper for the serial or network interface. 
*/
typedef struct {
	int			sd;			// socket descriptor for the network interface
	int			error;		// Set when read/write errors occur.
							// You will have to do something specific to your
							// OS to make transort work again.
	int			timeout;	// Timeout in milliseconds used in transort.
	char		*name;		// this name is used in error output, so you can
							// identify the interface
	AmsNetId	me;			// local netID (NOT the one open on  sd!!!)
	int			AMSport;	// local port (NOT the one open on  sd!!!)
} ADSInterface;

/* 
    This holds data for a connection;
*/
typedef struct {
	ADSInterface  *iface;			// pointer to used interface
	int			  AnswLen;			// length of last message
 	int			  invokeId;			// packetNumber in transport layer
	void		  *dataPointer;		// pointer to result data im msgIn, if present
	unsigned char msgIn[MAXDATALEN];
	unsigned char msgOut[MAXDATALEN];
	AmsNetId	  partner;			// netID of the device open on iface->sd
	int			  AMSport;			// port of the device open on iface->sd
} ADSConnection;

	void ads_debug(int type, const char *fmt, ...);
/** 
    This will setup a new interface structure from an initialized
    serial interface's handle and a name.
*/
	ADSInterface *_ADSNewInterface(int sd, AmsNetId me, int port, char *nname);

/** 
    This will setup a new connection structure using an initialized
    ADSInterface and PLC's MPI address.
*/
	ADSConnection *_ADSNewConnection(ADSInterface *di, AmsNetId partner, int port);

/**
Hex dump:
*/
        void _ADSDump(char *name, void *v, int len);

        void _ADSDumpAMSNetId(AMSNetID * id);

        void _ADSDumpAMSheader(AMSheader * h);
        
/**
    Naming
*/
	char *ADSCommandName(int c);

	char *ADSerrorText(int err);

long AdsGetMeAddress(PAmsAddr pAddr, int AmsPort);
	int ADSreadBytes(ADSConnection *dc,
				 unsigned long indexGroup, unsigned long offset,
				 unsigned long length, void *buffer,
				 unsigned long *pnRead);
	int ADSreadDeviceInfo(ADSConnection * dc,
			      char *pDevName, PAdsVersion pVersion);
	int ADSwriteBytes(ADSConnection *dc, int indexGroup, int offset,
								int length, void *data);
	int ADSreadState(ADSConnection * dc,
			 unsigned short *ADSstate, unsigned short *devState);
	int ADSwriteControl(ADSConnection * dc, int ADSstate,
			    int devState, void *data, int length);
	int ADSaddDeviceNotification(ADSConnection * dc,
				     int indexGroup,
				     int offset, int length,
				     int transmissionMode,
				     int maxDelay, int cycleTime);
	int ADSreadWriteBytes(ADSConnection * dc,
					  unsigned long indexGroup, unsigned long offset,
					  unsigned long readLength, void *readBuffer,
					  unsigned long writeLength, void *writeBuffer,
					  unsigned long *pnRead);
	int _ADSwrite(ADSConnection * dc);

	int _ADSparseNetID(const char *netIDstring, AmsNetId *id);

	int ADSGetLocalAMSId(AmsNetId * id);

	int ADSFreeInterface(ADSInterface * di);

	void ADSFreeConnection(ADSConnection * dc);

int _ADStranslateWrError(int rc, int nErr);
int _ADStranslateRdError(int rc, int nErr);

#endif	/* __ADS_H__ */

#ifdef __cplusplus
}
#endif
