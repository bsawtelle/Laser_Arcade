#ifndef _SD_H_
#define _SD_H_

#define RESPONSE_BYTES 5 //Sd cards respond with at most 5 bytes of data


#define SDCS_PORT 1
#define SDCS_PIN 5

#define SDSSIBASE SSI0_BASE

//--------SD Response Flags --------

#define SDFLAG_IDLE_STATE             0x01
#define SDFLAG_ERASE_RESET						0x02
#define SDFLAG_ILLEGAL_COMMAND        0x04
#define SDFLAG_CRC_ERROR              0x08
#define SDFLAG_ERASE_SEQUENCE_ERROR   0x10
#define SDFLAG_ADDRESS ERROR					0x20
#define SDFlAG_PARAMETER_ERROR				0x40


extern uint32_t SSIBaseSD;

extern uint32_t SPICommandSD(uint8_t command_index, uint32_t argument, bool CRC);

extern bool InitialiseSD(void); //add gpio arguments?

extern uint32_t ReadSD(void);

extern void WriteSD(void); // add data argument

#endif // _SD_H_
