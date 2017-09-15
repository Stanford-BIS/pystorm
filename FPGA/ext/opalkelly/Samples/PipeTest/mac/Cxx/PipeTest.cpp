//------------------------------------------------------------------------
// PipeTest.CPP
//
// This is the C++ source file for the PipeTest Sample.
//
//
// Copyright (c) 2004-2010 Opal Kelly Incorporated
// $Rev$ $Date$
//------------------------------------------------------------------------

#include <iostream>
#include <fstream>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#if defined(__QNX__)
	#include <stdint.h>
	#include <sys/syspage.h>
	#include <sys/neutrino.h>
	#include <sys/types.h>
	#include <sys/usbdi.h>
#elif defined(__linux__)
	#include <sys/time.h>
#elif defined(__APPLE__)
	#include <sys/time.h>
#endif

#include "okFrontPanelDLL.h"

// From mt_random.cpp
void mt_init();
unsigned long mt_random();


#if defined(_WIN32)
	#define sscanf  sscanf_s
#elif defined(__linux__) || defined(__APPLE__)
#endif
#if defined(__QNX__)
	#define clock_t   uint64_t
    #define clock()   ClockCycles()
	#define NUM_CPS   ((SYSPAGE_ENTRY(qtime)->cycles_per_sec))
#else
	#define NUM_CPS   CLOCKS_PER_SEC
#endif

typedef unsigned int UINT32;


#define MIN(a,b)   (((a)<(b)) ? (a) : (b))


#define OK_PATTERN_COUNT         0
#define OK_PATTERN_LFSR          1
#define OK_PATTERN_WALKING1      2
#define OK_PATTERN_WALKING0      3
#define OK_PATTERN_HAMMER        4
#define OK_PATTERN_NEIGHBOR      5


okTDeviceInfo  m_devInfo;
bool           m_bCheck;
bool           m_bInjectError;
int            m_ePattern;
UINT32         m_u32BlockSize;
UINT32         m_u32SegmentSize;
UINT32         m_u32TransferSize;
UINT32         m_u32ThrottleIn;
UINT32         m_u32ThrottleOut;
clock_t        m_cStart;
clock_t        m_cStop;


void
startTimer()
{
#if defined(_WIN32) || defined(__QNX__)
	m_cStart = clock();
#else
	timeval a;
	gettimeofday(&a, 0);
	m_cStart = a.tv_sec * 1000000 + a.tv_usec;
#endif
}



void
stopTimer()
{
#if defined(_WIN32) || defined(__QNX__)
	m_cStop = clock();
#else
	timeval a;
	gettimeofday(&a, 0);
	m_cStop = a.tv_sec * 1000000 + a.tv_usec;
#endif
}



// Sets the reset state of the pattern generator based on the 
// selected pattern.
void
patternReset(UINT32 *wordH, UINT32 *wordL, UINT32 u32Width)
{
	switch (m_ePattern) {
		case OK_PATTERN_COUNT:
			*wordH = 0x00000001;
			*wordL = 0x00000001;
			break;
		case OK_PATTERN_LFSR:
			*wordH = 0x0D0C0B0A;
			*wordL = 0x04030201;
			break;
		case OK_PATTERN_WALKING1:
			*wordH = 0x00000000;
			*wordL = 0x00000001;
			break;
		case OK_PATTERN_WALKING0:
			*wordH = 0xFFFFFFFF;
			*wordL = 0xFFFFFFFE;
			break;
		case OK_PATTERN_HAMMER:
			*wordH = 0x00000000;
			*wordL = 0x00000000;
			break;
		case OK_PATTERN_NEIGHBOR:
			*wordH = 0x00000000;
			*wordL = 0x00000000;
			break;
	}
}



// Computes the next word in the data pattern based on the 
// selected pattern and the output word width (in bits).
void
patternNext(UINT32 *wordH, UINT32 *wordL, UINT32 u32Width)
{
	static UINT32 neighborH=0xFFFFFFFF;
	static UINT32 neighborL=0xFFFFFFFE;
	UINT32 bit, hold;

	switch (m_ePattern) {
		case OK_PATTERN_COUNT:
			*wordH = *wordH + 1;
			*wordL = *wordL + 1;
			break;
		case OK_PATTERN_LFSR:
			bit = ((*wordH >> 31) ^ (*wordH >> 21) ^ (*wordH >> 1)) & 1;
			*wordH = (*wordH << 1) | bit;
			bit = ((*wordL >> 31) ^ (*wordL >> 21) ^ (*wordL >> 1)) & 1;
			*wordL = (*wordL << 1) | bit;
			break;
		case OK_PATTERN_WALKING0:
			if (64 == u32Width) {
				hold = *wordH;
				*wordH = (*wordH << 1) | ((*wordL >> 31) & 0x01);
				*wordL = (*wordL << 1) | ((  hold >> 31) & 0x01);
			} else if (32 == u32Width) {
				*wordH = 0xFFFFFFFF;
				*wordL = (*wordL << 1) | ((*wordL >> 31) & 0x01);
			} else if (16 == u32Width) {
				*wordH = 0xFFFFFFFF;
				*wordL = ((*wordL << 1) | ((*wordL >> 15) & 0x01)) & 0xFFFF;
			} else if (8 == u32Width) {
				*wordH = 0xFFFFFFFF;
				*wordL = ((*wordL << 1) | ((*wordL >> 7) & 0x01)) & 0xFF;
			}
			break;
		case OK_PATTERN_WALKING1:
			if (64 == u32Width) {
				hold = *wordH;
				*wordH = (*wordH << 1) | ((*wordL >> 31) & 0x01);
				*wordL = (*wordL << 1) | ((  hold >> 31) & 0x01);
			} else if (32 == u32Width) {
				*wordH = 0x00000000;
				*wordL = (*wordL << 1) | ((*wordL >> 31) & 0x01);
			} else if (16 == u32Width) {
				*wordH = 0x00000000;
				*wordL = ((*wordL << 1) | ((*wordL >> 15) & 0x01)) & 0xFFFF;
			} else if (8 == u32Width) {
				*wordH = 0x00000000;
				*wordL = ((*wordL << 1) | ((*wordL >> 7) & 0x01)) & 0xFF;
			}
			break;
		case OK_PATTERN_HAMMER:
			*wordH = ~(*wordH);
			*wordL = ~(*wordL);
			break;
		case OK_PATTERN_NEIGHBOR:
			if (0x0 == *wordH) {
				*wordH = neighborH;
				*wordL = neighborL;
				if (64 == u32Width) {
					hold = neighborH;
					neighborH = (neighborH << 1) | ((neighborL >> 31) & 0x01);
					neighborL = (neighborL << 1) | ((     hold >> 31) & 0x01);
				} else if (32 == u32Width) {
					neighborH = 0xFFFFFFFF;
					neighborL = (neighborL << 1) | ((neighborL >> 31) & 0x01);
				} else if (16 == u32Width) {
					neighborH = 0xFFFFFFFF;
					neighborL = ((neighborL << 1) | ((neighborL >> 15) & 0x01)) & 0xFFFF;
				} else if (8 == u32Width) {
					neighborH = 0xFFFFFFFF;
					neighborL = ((neighborL << 1) | ((neighborL >> 7) & 0x01)) & 0xFF;
				}
			} else {
				*wordH = 0x00000000;
				*wordL = 0x00000000;
			}
			break;
	}
}



// Generates a buffer of data following the selecetd data pattern
// and word width.
void
generateData(unsigned char *pucValid, UINT32 u32ByteCount, UINT32 u32Width)
{
	UINT32 i;
	UINT32 wordH, wordL;


	patternReset(&wordH, &wordL, u32Width);

	if (64 == u32Width) {
		UINT32 *pu32Valid = (UINT32 *)pucValid;
		for (i=0; i<u32ByteCount/8; i++) {
			pu32Valid[i*2 + 0] = wordL;
			pu32Valid[i*2 + 1] = wordH;
			patternNext(&wordH, &wordL, u32Width);
		}
	} else if (32 == u32Width) {
		UINT32 *pu32Valid = (UINT32 *)pucValid;
		for (i=0; i<u32ByteCount/4; i++) {
			pu32Valid[i] = wordL;
			patternNext(&wordH, &wordL, u32Width);
		}
	} else if (16 == u32Width) {
		for (i=0; i<u32ByteCount/2; i++) {
			pucValid[i*2 + 0] = (wordL >> 0) & 0xff;
			pucValid[i*2 + 1] = (wordL >> 8) & 0xff;
			patternNext(&wordH, &wordL, u32Width);
		}
	} else if (8 == u32Width) {
		for (i=0; i<u32ByteCount; i++) {
			pucValid[i] = wordL & 0xff;
			patternNext(&wordH, &wordL, u32Width);
		}
	}

	// Inject errors (optional)
	if (m_bInjectError)
		pucValid[7] = ~pucValid[7];
}



bool
checkData(unsigned char *pucBuffer, unsigned char *pucValid, UINT32 u32ByteCount)
{
	UINT32 i;
	UINT32 *pu32Buffer = (UINT32 *)pucBuffer;
	UINT32 *pu32Valid = (UINT32 *)pucValid;
	for (i=0; i<u32ByteCount/4; i++) {
		if (pu32Buffer[i] != pu32Valid[i]) {
			printf("[%d]  %08X  !=  %08X\n", i, pu32Buffer[i], pu32Valid[i]);
			return(false);
		}
	}
	return(true);
}



void
reportRateResults(UINT32 u32Count)
{
	clock_t clkInterval = m_cStop - m_cStart;
	printf("Duration: %.3f seconds -- %.2f calls/s\n",
	       (double) clkInterval / NUM_CPS,
	       (double) (u32Count) * NUM_CPS / clkInterval);
}



void
reportBandwidthResults(UINT32 u32Count)
{
	clock_t clkInterval = (m_cStop - m_cStart)/u32Count;
	printf("Duration: %.3f seconds -- %.2f MB/s\n",
	       (double) clkInterval / NUM_CPS,
	       (double) (m_u32TransferSize/1024/1024) * NUM_CPS / clkInterval);
}



void
Transfer(okCFrontPanel *dev, UINT32 u32Count, bool bWrite)
{
	unsigned char *pBuffer;
	unsigned char *pValid;
	UINT32  i;
	UINT32  u32SegmentSize, u32Remaining;
	long ret;


#if defined(__QNX__)
	pBuffer = (unsigned char *)usbd_alloc((size_t)m_u32SegmentSize);
	pValid = (unsigned char *)usbd_alloc((size_t)m_u32SegmentSize);
#else
	pBuffer = new unsigned char[m_u32SegmentSize];
	pValid = new unsigned char[m_u32SegmentSize];
#endif

	// Only COUNT and LFSR are supported on non-USB3 devices.
	if (OK_INTERFACE_USB3 != m_devInfo.deviceInterface) {
		switch (m_ePattern) {
			case OK_PATTERN_WALKING0:
			case OK_PATTERN_WALKING1:
			case OK_PATTERN_HAMMER:
			case OK_PATTERN_NEIGHBOR:
				printf("Unsupported pattern for device type.  Switching to LFSR.\n");
				m_ePattern = OK_PATTERN_LFSR;
				break;
		}
	}

	if (OK_INTERFACE_USB3 == m_devInfo.deviceInterface) {
		dev->SetWireInValue(0x02, m_u32ThrottleIn);     // Pipe In throttle
		dev->SetWireInValue(0x01, m_u32ThrottleOut);    // Pipe Out throttle
		dev->SetWireInValue(0x00, (m_ePattern<<2) | 1<<1 | 1);  // PATTERN | SET_THROTTLE=1 | RESET=1
		dev->UpdateWireIns();
		dev->SetWireInValue(0x00, (m_ePattern<<2) | 0<<1 | 0);  // PATTERN | SET_THROTTLE=0 | RESET=0
		dev->UpdateWireIns();
	} else {
		dev->SetWireInValue(0x02, m_u32ThrottleIn);     // Pipe In throttle
		dev->SetWireInValue(0x01, m_u32ThrottleOut);    // Pipe Out throttle
		dev->SetWireInValue(0x00, 1<<5 | ((m_ePattern==OK_PATTERN_LFSR ? 1 : 0)<<4) | 1<<2);  // SET_THROTTLE=1 | MODE=LFSR | RESET=1
		dev->UpdateWireIns();
		dev->SetWireInValue(0x00, 0<<5 | ((m_ePattern==OK_PATTERN_LFSR ? 1 : 0)<<4) | 0<<2);  // SET_THROTTLE=0 | MODE=LFSR | RESET=0
		dev->UpdateWireIns();
	}

	startTimer();
	for (i=0; i<u32Count; i++) {
		u32Remaining = m_u32TransferSize;
		while (u32Remaining > 0) {
			u32SegmentSize = MIN(m_u32SegmentSize, u32Remaining);
			u32Remaining -= u32SegmentSize;

			// If we're validating data, generate data per segment.
			if (m_bCheck) {
				if (OK_INTERFACE_USB3 == m_devInfo.deviceInterface) {
					dev->SetWireInValue(0x00, (m_ePattern<<2) | 0<<1 | 1);  // PATTERN | SET_THROTTLE=0 | RESET=1
					dev->UpdateWireIns();
					dev->SetWireInValue(0x00, (m_ePattern<<2) | 0<<1 | 0);  // PATTERN | SET_THROTTLE=0 | RESET=0
					dev->UpdateWireIns();
				} else {
					dev->SetWireInValue(0x00, 0<<5 | ((m_ePattern==OK_PATTERN_LFSR ? 1 : 0)<<4) | 1<<2);  // SET_THROTTLE=0 | MODE=LFSR | RESET=1
					dev->UpdateWireIns();
					dev->SetWireInValue(0x00, 0<<5 | ((m_ePattern==OK_PATTERN_LFSR ? 1 : 0)<<4) | 0<<2);  // SET_THROTTLE=0 | MODE=LFSR | RESET=0
					dev->UpdateWireIns();
				}
				generateData(pValid, u32SegmentSize, m_devInfo.pipeWidth);
			}

			if (bWrite) {
				if (0 == m_u32BlockSize) {
					ret = dev->WriteToPipeIn(0x80, u32SegmentSize, pValid);
				} else {
					ret = dev->WriteToBlockPipeIn(0x80, m_u32BlockSize, u32SegmentSize, pValid);
				}
			} else {
				if (0 == m_u32BlockSize) {
					ret = dev->ReadFromPipeOut(0xA0, u32SegmentSize, pBuffer);
				} else {
					ret = dev->ReadFromBlockPipeOut(0xA0, m_u32BlockSize, u32SegmentSize, pBuffer);
				}
			}

			if (ret < 0) {
				switch (ret) {
					case okCFrontPanel::InvalidBlockSize:
						printf("Block Size Not Supported\n");
						break;
					case okCFrontPanel::UnsupportedFeature:
						printf("Unsupported Feature\n");
						break;
					default:
						printf("Transfer Failed with error: %ld\n", ret);
						break;
				}

				if (dev->IsOpen() == false){
					printf("Device disconnected\n");
					exit(-1);
				}

				return;
			}

			if (m_bCheck) {
				if (false == bWrite) {
					if (false == checkData(pBuffer, pValid, u32SegmentSize)) {
						printf("ERROR: Data check failed!\n");
					}
				} else {
					dev->UpdateWireOuts();
					int n = dev->GetWireOutValue(0x21);
					if (0 != n) {
						printf("ERROR: Data check failed!  (%d errors)\n", n);
					}
				}
			}
		}
	}
	stopTimer();
#if defined(__QNX__)
	usbd_free(pValid);
	usbd_free(pBuffer);
#else
	delete [] pValid;
	delete [] pBuffer;
#endif

	reportBandwidthResults(u32Count);
}



void
BenchmarkWires(okCFrontPanel *dev)
{
	UINT32 i;

	printf("UpdateWireIns  (1000 calls)  ");
	startTimer();
	for (i=0; i<1000; i++)
		dev->UpdateWireIns();
	stopTimer();
	reportRateResults(1000);
	
	printf("UpdateWireOuts (1000 calls)  ");
	startTimer();
	for (i=0; i<1000; i++)
		dev->UpdateWireOuts();
	stopTimer();
	reportRateResults(1000);
}



void
BenchmarkTriggers(okCFrontPanel *dev)
{
	UINT32 i;

	printf("ActivateTriggerIns  (1000 calls)  ");
	startTimer();
	for (i=0; i<1000; i++)
		dev->ActivateTriggerIn(0x40, 0x01);
	stopTimer();
	reportRateResults(1000);
	
	printf("UpdateTriggerOuts (1000 calls)  ");
	startTimer();
	for (i=0; i<1000; i++)
		dev->UpdateTriggerOuts();
	stopTimer();
	reportRateResults(1000);
}



void
StressPipes(okCFrontPanel *dev)
{
	UINT32 i, j;
	bool bWrite;
	UINT32 matrix[][3] = { // SegmentSize,    TransferSize,   Pattern
	                        { 4*1024*1024,    64*1024*1024,   OK_PATTERN_COUNT    },
	                        { 4*1024*1024,    64*1024*1024,   OK_PATTERN_LFSR     },
	                        { 4*1024*1024,    64*1024*1024,   OK_PATTERN_WALKING1 },
	                        { 4*1024*1024,    64*1024*1024,   OK_PATTERN_WALKING0 },
	                        { 4*1024*1024,    64*1024*1024,   OK_PATTERN_HAMMER   },
	                        { 4*1024*1024,    64*1024*1024,   OK_PATTERN_NEIGHBOR },
	                        { 0,              0,              0                   } };

	m_bCheck = true;
	for (j=0; j<2; j++) {
		bWrite = (j==1);
		for (i=0; matrix[i][0]!=0; i++) {
			m_u32BlockSize     = 0;
			m_u32SegmentSize   = matrix[i][0];
			m_u32TransferSize  = matrix[i][1];
			m_ePattern         = matrix[i][2];
			if (0 != m_u32BlockSize) {
				m_u32SegmentSize  -= (m_u32SegmentSize % m_u32BlockSize);  // Segment size must be a multiple of block length
				m_u32TransferSize -= (m_u32TransferSize % m_u32BlockSize);  // Segment size must be a multiple of block length
			}
			printf("%s SS:%-10d  TS:%-10d   Pattern:%d   ", bWrite ? ("Write") : ("Read "), 
			       m_u32SegmentSize, m_u32TransferSize, m_ePattern);
			Transfer(dev, 1, bWrite);
		}
	}
}



void
BenchmarkPipes(okCFrontPanel *dev)
{
	UINT32 i, j, u32Count;
	bool bWrite;
	UINT32 matrix[][4] = { // BlockSize, SegmentSize,    TransferSize, Count
	                        {    0,      4*1024*1024,    64*1024*1024,     1 },
	                        {    0,      4*1024*1024,    32*1024*1024,     1 },
	                        {    0,      4*1024*1024,    16*1024*1024,     2 },
	                        {    0,      4*1024*1024,     8*1024*1024,     4 },
	                        {    0,      4*1024*1024,     4*1024*1024,     8 },
	                        {    0,      1*1024*1024,    32*1024*1024,     1 },
	                        {    0,         256*1024,    32*1024*1024,     1 },
	                        {    0,          64*1024,    16*1024*1024,     1 },
	                        {    0,          16*1024,     4*1024*1024,     1 },
	                        {    0,           4*1024,     1*1024*1024,     1 },
	                        {    0,           1*1024,     1*1024*1024,     1 },
	                        { 1024,           1*1024,     1*1024*1024,     1 },
	                        { 1024,      1*1024*1024,    32*1024*1024,     1 },
	                        {  900,      1*1024*1024,    32*1024*1024,     1 },
	                        {  800,      1*1024*1024,    32*1024*1024,     1 },
	                        {  700,      1*1024*1024,    32*1024*1024,     1 },
	                        {  600,      1*1024*1024,    32*1024*1024,     1 },
	                        {  512,      1*1024*1024,    32*1024*1024,     1 },
	                        {  500,      1*1024*1024,    32*1024*1024,     1 },
	                        {  400,      1*1024*1024,    16*1024*1024,     1 },
	                        {  300,      1*1024*1024,    16*1024*1024,     1 },
	                        {  256,      1*1024*1024,    16*1024*1024,     1 },
	                        {  200,      1*1024*1024,     8*1024*1024,     1 },
	                        {  128,      1*1024*1024,     8*1024*1024,     1 },
	                        {  100,      1*1024*1024,     8*1024*1024,     1 },
	                        { 9999,                0,               0,     0 } };

	for (j=0; j<2; j++) {
		bWrite = (j==1);
		for (i=0; matrix[i][0]!=9999; i++) {
			m_u32BlockSize     = matrix[i][0];
			m_u32SegmentSize   = matrix[i][1];
			m_u32TransferSize  = matrix[i][2];
			if (0 != m_u32BlockSize) {
				m_u32SegmentSize  -= (m_u32SegmentSize % m_u32BlockSize);  // Segment size must be a multiple of block length
				m_u32TransferSize -= (m_u32TransferSize % m_u32BlockSize);  // Segment size must be a multiple of block length
			}
			u32Count           = matrix[i][3];
			printf("%s BS:%-10d  SS:%-10d  TS:%-10d   ", bWrite ? ("Write") : ("Read "), 
			       m_u32BlockSize, m_u32SegmentSize, m_u32TransferSize);
			Transfer(dev, u32Count, bWrite);
		}
	}
}



bool
InitializeFPGA(okCFrontPanel *dev, char *bitfile, const char *serial)
{
	if (okCFrontPanel::NoError != dev->OpenBySerial(std::string(serial))) {
		printf("Device could not be opened.  Is one connected?\n");
		return(false);
	}
	dev->GetDeviceInfo(&m_devInfo);
	printf("Found a device: %s\n", m_devInfo.productName);

	dev->LoadDefaultPLLConfiguration();	

	// Get some general information about the XEM.
	printf("Device firmware version: %d.%d\n", m_devInfo.deviceMajorVersion, m_devInfo.deviceMinorVersion);
	printf("Device serial number: %s\n", m_devInfo.serialNumber);
	printf("Device device ID: %d\n", m_devInfo.productID);

	// Download the configuration file.
	if (okCFrontPanel::NoError != dev->ConfigureFPGA(bitfile)) {
		printf("FPGA configuration failed.\n");
		return(false);
	}

	// Check for FrontPanel support in the FPGA configuration.
	if (dev->IsFrontPanelEnabled()){
		printf("FrontPanel support is enabled.\n");
	}
	else{
		printf("FrontPanel support is not enabled.\n");
		return(false);
	}

	return(true);
}


static void
printUsage(char *progname)
{
	printf("Usage: %s bitfile [lfsr|sequential|walking1|walking0|hammer|neighbor]\n", progname);
	printf("                  [throttlein T] [throttleout T] [check] [inject]\n");
	printf("                  [blocksize B] [segmentsize S]\n");
	printf("                  [stress] [bench] [read N] [write N]\n\n");
	printf("   bitfile        - Configuration file to download.\n");
	printf("   lfsr           - [Default] Selects LFSR psuedorandom pattern generator\n");
	printf("   sequential     - Selects Counter pattern generator\n");
	printf("   walking1       - Selects Walking 1's pattern generator. (USB 3.0 only)\n");
	printf("   walking0       - Selects Walking 0's pattern generator. (USB 3.0 only)\n");
	printf("   hammer         - Selects Hammer pattern generator. (USB 3.0 only)\n");
	printf("   neighbor       - Selects Neighbor pattern generator. (USB 3.0 only)\n");
	printf("   throttlein     - Specifies a 32-bit hex throttle vector (writes).\n");
	printf("   throttleout    - Specifies a 32-bit hex throttle vector (reads).\n");
	printf("   check          - Turns on validity checks.\n");
	printf("   inject         - Injects an error during data generation.\n");
	printf("   blocksize B    - Sets the block size to B (for BTPipes).\n");
	printf("   segmentsize S  - Sets the segment size to S.\n");
	printf("   stress         - Performs a transfer stress test (validity checks on).\n");
	printf("   bench          - Runs a preset benchmark script and prints results.\n");
	printf("   read N         - Performs a read of N bytes and optionally checks for validity.\n");
	printf("   write N        - Performs a write of N bytes and optionally checks for validity.\n");
}


int
main(int argc, char *argv[])
{
	okCFrontPanel *dev;
	const char* serial;
	char dll_date[32], dll_time[32];
	UINT32 i;


	printf("---- Opal Kelly ---- PipeTest Application v2.0 ----\n");
	if (FALSE == okFrontPanelDLL_LoadLib(NULL)) {
		printf("FrontPanel DLL could not be loaded.\n");
		return(-1);
	}
	okFrontPanelDLL_GetVersion(dll_date, dll_time);
	printf("FrontPanel DLL loaded.  Built: %s  %s\n", dll_date, dll_time);

	if (argc < 2) {
		printUsage(argv[0]);
		return(-1);
	}

	// Initialize the FPGA with our configuration bitfile.
	dev = new okCFrontPanel;
	if ((argc>=3) && (!strcmp(argv[2], "serial"))) {
		serial = argv[2];
		i = 4;
	} else {
		serial = "";
		i = 2;
	}
	if (false == InitializeFPGA(dev, argv[1], serial)) {
		printf("FPGA could not be initialized.\n");
		goto done;
	}

	m_bCheck = false;
	m_ePattern = OK_PATTERN_LFSR;
	m_u32BlockSize = 0;
	m_u32SegmentSize = 4*1024*1024;
	m_u32ThrottleIn = 0xffffffff;
	m_u32ThrottleOut = 0xffffffff;
	for (; i<(UINT32)argc; i++) {
		if (!strcmp(argv[i], "blocksize")) {
			sscanf(argv[++i], "%u", &m_u32BlockSize);
		}
		if (!strcmp(argv[i], "segmentsize")) {
			sscanf(argv[++i], "%u", &m_u32SegmentSize);
		}
		if (!strcmp(argv[i], "check")) {
			m_bCheck = true;
		}
		if (!strcmp(argv[i], "random")) {
			m_ePattern = OK_PATTERN_LFSR;
			printf("Data pattern: LFSR\n");
		}
		if (!strcmp(argv[i], "sequential")) {
			m_ePattern = OK_PATTERN_COUNT;
			printf("Data pattern: Sequential\n");
		}
		if (!strcmp(argv[i], "walking0")) {
			m_ePattern = OK_PATTERN_WALKING0;
			printf("Data pattern: Walking 0's\n");
		}
		if (!strcmp(argv[i], "walking1")) {
			m_ePattern = OK_PATTERN_WALKING1;
			printf("Data pattern: Walking 1's\n");
		}
		if (!strcmp(argv[i], "hammer")) {
			m_ePattern = OK_PATTERN_HAMMER;
			printf("Data pattern: Hammer\n");
		}
		if (!strcmp(argv[i], "neighbor")) {
			m_ePattern = OK_PATTERN_NEIGHBOR;
			printf("Data pattern: Neighbor\n");
		}

		if (!strcmp(argv[i], "inject")) {
			m_bInjectError = true;
		}
		if (!strcmp(argv[i], "throttlein")) {
			sscanf(argv[++i], "%08x", &m_u32ThrottleIn);
		}
		if (!strcmp(argv[i], "throttleout")) {
			sscanf(argv[++i], "%08x", &m_u32ThrottleOut);
		}
		if (!strcmp(argv[i], "read")) {
			sscanf(argv[++i], "%d", &m_u32TransferSize);
			Transfer(dev, 1, false);
		}
		if (!strcmp(argv[i], "write")) {
			sscanf(argv[++i], "%d", &m_u32TransferSize);
			Transfer(dev, 1, true);
		}
		if (!strcmp(argv[i], "bench")) {
			BenchmarkWires(dev);
			BenchmarkTriggers(dev);
			BenchmarkPipes(dev);
		}
		if (!strcmp(argv[i], "stress")) {
			StressPipes(dev);
		}
	}

done:
	delete dev;
	return(0);
}
