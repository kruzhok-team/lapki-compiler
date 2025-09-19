#pragma once
#include "../stm32g431xx.h"
#include "core.h"

#include "protocol.c"

#define __USBBUF_BEGIN 0x40006000
#define __MEM2USB(X) (((int)X - __USBBUF_BEGIN))
#define __USB2MEM(X) (((int)X + __USBBUF_BEGIN))
#define MIN(X, Y) (((X) < (Y)) ? (X) : (Y))
extern volatile bool needToJump; // main.c
extern "C" {

    
// Взято из примера, что именно делает – непонятно 
#define USB_TXTIMEOUT 5

struct {
  bool isUp; // индикатор того, что произошло подключение по usb протоколу
} usbStatus = 
{ .isUp = false 
};

// далее идёт чёрная магия
typedef struct {
    unsigned short ADDR_TX;
    unsigned short COUNT_TX;
    unsigned short ADDR_RX;
    unsigned short COUNT_RX;
} USB_BTABLE_ENTRY;

typedef struct {
    unsigned short Length;
    unsigned short BytesSent;
    #ifdef USB_TXTIMEOUT
    unsigned int Timeout;
    #endif
    unsigned char *Buffer;
} USB_TRANSFER_STATE;

typedef struct {
    USB_SETUP_PACKET Setup;
    USB_TRANSFER_STATE Transfer;
    USB_TRANSFER_STATE Receive;
} USB_CONTROL_STATE;

typedef struct {
    volatile unsigned char *Buffer;
    char Size;
    void (*CompleteCallback)(char ep, short length);
} USB_BufferConfig;

static USB_CONTROL_STATE ControlState;

// значения этих двух структур кладётся в область памяти USB
__attribute__((aligned(8)))
__attribute__((section(".usbbuf")))
volatile
static
USB_BTABLE_ENTRY BTable[8] = {0};

__attribute__((aligned(2)))
__attribute__((section(".usbbuf")))
volatile static char EP0_Buf[2][64] = {0};

// Копирование из ОЗУ в память USB
static void USB_CopyMemory(volatile uint16_t *source, volatile uint16_t *target, uint16_t length);
// Обнуление USB-SRAM
static void USB_ClearSRAM();
// Обработчик сообщений в EP0 (конфигурация и управление)
static void USB_HandleControl();
// Обработчик пакетов SETUP в EP0
static void USB_HandleSetup(USB_SETUP_PACKET *setup);

static void AddToDescriptor(char *data, short *offset);
char *USB_GetConfigDescriptor(short *length);
static void USB_PrepareTransfer(USB_TRANSFER_STATE *transfer, short *ep, char *txBuffer, short *txBufferCount, short txBufferSize);
// Get the device descriptor
USB_DESCRIPTOR_DEVICE *USB_GetDeviceDescriptor();

//USB_QUALIFIER_DESCRIPTOR_DEVICE *USB_GetQualifierDeviceDescriptor();

__weak void USB_SuspendDevice();
__weak void USB_WakeupDevice();
void USB_ResetClass(char interface, char alternateId);
char *USB_GetString(char index, short lcid, short *length);
char *USB_GetOSDescriptor(short *length);
static void USB_DistributeBuffers();
void USB_SetEPConfig(USB_CONFIG_EP config);
void USB_ConfigureEndpoints();
void USB_Transmit(char ep, char *buffer, short length);
char USB_IsTransmitPending(char ep);

void Rx_HandleSystem(char ep, short length);
void Rx_HandleData  (char ep, short length);
char buffer[2048];
char lineCoding[7];

// Example definition for a Virtual COM Port
static const USB_DESCRIPTOR_DEVICE DeviceDescriptor = {
    .Length = 18,
    .Type = 0x01,
    .USBVersion = 0x0200,
    .DeviceClass = 0x02,
    .DeviceSubClass = 0x00,
    .DeviceProtocol = 0x00,
    .MaxPacketSize = 64,
    .VendorID = 0x21BB,
    .ProductID = 0xF001,
    .DeviceVersion = 0x0101,
    .strManufacturer = 1,
    .strProduct = 2,
    .strSerialNumber = 3,
    .Configurations = 1
  };
    
USB_DESCRIPTOR_DEVICE *USB_GetDeviceDescriptor() {
  return (USB_DESCRIPTOR_DEVICE *)&DeviceDescriptor;
};

// We need two interfaces. One for the CDC-Definition and one for the Data
static const USB_DESCRIPTOR_CONFIG ConfigDescriptor = {
    .Length = 9,
    .Type = 0x02,
    .TotalLength = 46,  // 55 для двух bulk интерфейсов, 4 эндпойнта; 62 для одного CDC (2 интерфейса, 3 эндпойнта); 85 для (3 интерфейсов, 5 эндпойнтов)
    .Interfaces = 1,
    .ConfigurationID = 1,
    .strConfiguration = 0,
    .Attributes = (1 << 7),
    .MaxPower = 125};  //x2 mA
    
// The Data-Interface with two Bulk-Endpoints
static const USB_DESCRIPTOR_INTERFACE InterfaceAll = {  
    .Length = 9,
    .Type = 0x04,
    .InterfaceID = 0,
    .AlternateID = 0,
    .Endpoints = 4,
    .Class = 0x00, //0x0A для CDC
    .SubClass = 0x00,
    .Protocol = 0x00,
    .strInterface = 0};
    
#define EP_SYSTEM 1
#define EP_DATA 2

static const USB_DESCRIPTOR_ENDPOINT SystemEndpoints[4] = { 
    {.Length = 7,
        .Type = 0x05,
        .Address = (1 << 7) | EP_SYSTEM,
        .Attributes = 0x02,  //0x01 - iso, 0x02 - bulk, 0x03 - interrupt
        .MaxPacketSize = 64,
     .Interval = 0x00},
    {.Length = 7,
        .Type = 0x05,
        .Address = EP_SYSTEM,
        .Attributes = 0x02,  //0x01 - iso, 0x02 - bulk, 0x03 - interrupt
     .MaxPacketSize = 64,
     .Interval = 0x00},
    {.Length = 7,
     .Type = 0x05,
     .Address = (1 << 7) | EP_DATA,
     .Attributes = 0x02,  //0x01 - iso, 0x02 - bulk, 0x03 - interrupt
     .MaxPacketSize = 64,
     .Interval = 0x00},
    {.Length = 7,
     .Type = 0x05,
     .Address = EP_DATA,
     .Attributes = 0x02,  //0x01 - iso, 0x02 - bulk, 0x03 - interrupt
     .MaxPacketSize = 64,
     .Interval = 0x00}
    };

static USB_BufferConfig Buffers[16] = {0};
static char ControlDataBuffer[USB_MaxControlData] = {0};

static USB_TRANSFER_STATE Transfers[7] = {0};

static char ConfigurationBuffer[78] = {0};  // 55 для двух bulk интерфейсов, 4 эндпойнта; 62 для одного CDC (2 интерфейса, 3 эндпойнта); 85 для (3 интерфейсов, 5 эндпойнтов)
static char ActiveConfiguration = 0x00;
static char DeviceState = 0x00; // 0 - Default, 1 - Address, 2 - Configured
static char EndpointState[USB_NumEndpoints] = {0};

static void AddToDescriptor(char *data, short *offset) {
    short length = data[0];

    for (int i = 0; i < length; i++) {
        ConfigurationBuffer[i + *offset] = data[i];
    }

    *offset += length;
}

char *USB_GetConfigDescriptor(short *length) {
    if (ConfigurationBuffer[0] == 0) {
        short offset = 0;
        AddToDescriptor((char*)&ConfigDescriptor, &offset);
        AddToDescriptor((char*)&InterfaceAll, &offset);
        AddToDescriptor((char*)&SystemEndpoints[0], &offset);
        AddToDescriptor((char*)&SystemEndpoints[1], &offset);
        AddToDescriptor((char*)&SystemEndpoints[2], &offset);
        AddToDescriptor((char*)&SystemEndpoints[3], &offset);
    }

    *length = sizeof(ConfigurationBuffer);
    return ConfigurationBuffer;
}

static void USB_SetEP(volatile short *ep, short value, short mask) {
  short toggle = 0b0111000001110000;
  short rc_w0 = 0b1000000010000000;
  short rw = 0b0000011100001111;

  short wr0 = rc_w0 & (~mask | value);
  short wr1 = (mask & toggle) & (*ep ^ value);
  short wr2 = rw & ((*ep & ~mask) | value);

  *ep = wr0 | wr1 | wr2;
}

static void USB_HandleControl() {
  if(USB->EP0R & USB_EP_CTR_RX) { // Получили сообщение в endpoint 0
    if(USB->EP0R & USB_EP_SETUP) { // Это SETUP пакет
      USB_SETUP_PACKET *setup = (USB_SETUP_PACKET*)EP0_Buf[0];
      USB_HandleSetup(setup);
      // USB_CopyMemory((volatile uint16_t*)setup, (volatile uint16_t*)&ControlState.Setup, sizeof(USB_SETUP_PACKET));
      ControlState.Transfer.Length = 0;
      ControlState.Receive.Length = 0;

      // If this is an OUT Transfer and we expect data, postpone handling the setup until the data arrives
      if ((setup->RequestType & DEFINE_bmRequestType_Dir_Mask) == DEFINE_bmRequestType_Dir_HostToDevice 
          && setup->Length > 0) {
        ControlState.Receive.Length = setup->Length;
        ControlState.Receive.BytesSent = 0;
      } else {
        // Не убирать! Проверили, при отключении не работает
        USB_HandleSetup(&ControlState.Setup);
      }

    } else {
      // Check if we are expecting data for a setup-packet. If so, read it and call the Setup-Handler once the transfer is complete
      if (ControlState.Receive.Length > 0) {
        if (ControlState.Receive.BytesSent < USB_MaxControlData) {
          USB_CopyMemory((volatile short*)EP0_Buf[0], (volatile short*)(ControlState.Receive.Buffer + ControlState.Receive.BytesSent), MIN(USB_MaxControlData - ControlState.Receive.BytesSent, BTable[0].COUNT_RX & 0x1FF));
          ControlState.Receive.BytesSent += MIN(USB_MaxControlData - ControlState.Receive.BytesSent, BTable[0].COUNT_RX & 0x1FF);
        }

        if (ControlState.Receive.BytesSent >= ControlState.Receive.Length) {
          USB_HandleSetup(&ControlState.Setup);
          ControlState.Receive.Length = 0;
        } else if (ControlState.Receive.BytesSent >= USB_MaxControlData) {
          USB_SetEP(&USB->EP0R, USB_EP_TX_STALL, USB_EP_TX_VALID);
          ControlState.Receive.Length = 0;
        }
      }
    }
    
    USB_SetEP(&USB->EP0R, USB_EP_RX_VALID, USB_EP_CTR_RX | USB_EP_RX_VALID);
  }

  if (USB->EP0R & USB_EP_CTR_TX) {
    // We just sent a control message
    if(ControlState.Setup.Request == 0x05) {
      USB->DADDR = USB_DADDR_EF | ControlState.Setup.Value;
    }
    // check for running transfers
    if (ControlState.Transfer.Length > 0) {
      if (ControlState.Transfer.Length > ControlState.Transfer.BytesSent) {
        USB_PrepareTransfer(&ControlState.Transfer, (short*)&USB->EP0R, (char*)&EP0_Buf[1], (short*)&BTable[0].COUNT_TX, 64);
      }
    }
    USB_SetEP(&USB->EP0R, 0x00, USB_EP_CTR_TX);
  }
}

short guidCounter = 0;
uint8_t guidRegistry[142] =
  { 0x8E, 0x00, 0x00, 0x00// DWORD (LE) Descriptor length (142 bytes)
  , 0x00, 0x01//	BCD WORD (LE)	Version ('1.0')
  , 0x05, 0x00//	WORD (LE)	Extended Property Descriptor index (5)
  , 0x01, 0x00//	WORD (LE)	Number of sections (1)
  , 0x84, 0x00, 0x00, 0x00//	DWORD (LE)	Size of the property section (132 bytes)
  , 0x01, 0x00, 0x00, 0x00//	DWORD (LE)	Property data type (1 = Unicode REG_SZ, see table below)
  , 0x28, 0x00//	WORD (LE)	Property name length (40 bytes)

  , 0x44, 0x00, 0x65, 0x00
  , 0x76, 0x00, 0x69, 0x00
  , 0x63, 0x00, 0x65, 0x00
  , 0x49, 0x00, 0x6e, 0x00
  , 0x74, 0x00, 0x65, 0x00
  , 0x72, 0x00, 0x66, 0x00
  , 0x61, 0x00, 0x63, 0x00
  , 0x65, 0x00, 0x47, 0x00
  , 0x55, 0x00, 0x49, 0x00
  , 0x44, 0x00, 0x00, 0x00//	NUL-terminated Unicode String (LE)	Property name "DeviceInterfaceGUID"

  , 0x4e, 0x00, 0x00, 0x00//	DWORD (LE)	Property data length (78 bytes)

  , 0x7b, 0x00, 0x30, 0x00
  , 0x43, 0x00, 0x30, 0x00
  , 0x34, 0x00, 0x30, 0x00
  , 0x30, 0x00, 0x43, 0x00
  , 0x39, 0x00, 0x2d, 0x00
  , 0x45, 0x00, 0x33, 0x00
  , 0x35, 0x00, 0x34, 0x00
  , 0x2d, 0x00, 0x34, 0x00
  , 0x33, 0x00, 0x31, 0x00
  , 0x30, 0x00, 0x2d, 0x00
  , 0x41, 0x00, 0x45, 0x00
  , 0x44, 0x00, 0x35, 0x00
  , 0x2d, 0x00, 0x45, 0x00
  , 0x46, 0x00, 0x45, 0x00
  , 0x38, 0x00, 0x43, 0x00
  , 0x39, 0x00, 0x46, 0x00
  , 0x41, 0x00, 0x38, 0x00
  , 0x33, 0x00, 0x30, 0x00
  , 0x38, 0x00, 0x7d, 0x00
  , 0x00, 0x00//NUL-terminated Unicode String (LE) Property name "{0c0400c9-e354-4310-aed5-efe8c9fa8308}"
  };

static
void
USB_HandleSetup
( USB_SETUP_PACKET *setup 
) {
    USB_CopyMemory((volatile uint16_t*)setup, (volatile uint16_t*)&ControlState.Setup, sizeof(USB_SETUP_PACKET));
    ControlState.Transfer.Length = 0;
    if ((setup->RequestType) == 0xC0
       ) {
      if ((setup->Request) == 0x77 ) {
        if ((setup->Index) == 0x0004 ) {
          volatile uint8_t tmp[40] =
            { 0x28, 0x00, 0x00, 0x00 //Descriptor length (40)
            , 0x00, 0x01 //Version (1.0)
            , 0x04, 0x00 //Compatibility ID Descriptor index (0x0004)
            , 0x01 //Number of sections
            , 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 //Reserved
            , 0x01	//Interface Number (Interface #0)
            , 0x01	//Reserved
            //, 0x4c, 0x49, 0x42, 0x55, 0x53, 0x42, 0x00, 0x00	//ASCII String Compatible ID ("WINUSB0\0" / "LIBUSB0\0" / "LIBUSBK\0" )
            , 0x57, 0x49, 0x4e, 0x55, 0x53, 0x42, 0x00, 0x00	//ASCII String Compatible ID ("WINUSB0\0" / "LIBUSB0\0" / "LIBUSBK\0" )
            , 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00	//ASCII String Sub-Compatible ID (unused)
            , 0x00, 0x00, 0x00, 0x00, 0x00, 0x00	//Reserved
            };
          USB_CopyMemory((volatile short*)tmp, (volatile short*)EP0_Buf[1], 40);
          BTable[0].COUNT_TX = setup->Length;
          USB_SetEP(&USB->EP0R, USB_EP_TX_VALID, USB_EP_TX_VALID);
          return;
        }
      }
    }
    /*
    if ((setup->RequestType) == 0xC0
     || (setup->RequestType) == 0xC1
       ) {
      status.debug1 = true;
      if ((setup->Request) == 0x77 ) {
        status.debug2 = true;
        if ((setup->Index) == 0x0005 ) {
          status.debug3 = true;
          if ( guidCounter == 0 ) guidCounter = 142;
          USB_CopyMemory((volatile short*)guidRegistry+(142-guidCounter), (volatile short*)EP0_Buf[1], setup->Length);
          BTable[0].COUNT_TX = setup->Length;
          USB_SetEP(&USB->EP0R, USB_EP_TX_VALID, USB_EP_TX_VALID);
          if ( guidCounter < setup -> Length ) guidCounter = 0; else guidCounter -= setup->Length;
          return;
        }
      }
    }
    */
    if ((setup->RequestType & 0x60) != 0) {
        // Class and interface setup packets are redirected to the class specific implementation
        //char ret = USB_HandleClassSetup(setup, ControlState.Receive.Buffer, ControlState.Receive.Length); USB_HandleClassSetup
    } else if ((setup->RequestType & 0x60) == 0) {
        if ((setup->RequestType & 0x0F) == 0) { // Device Requests
            switch (setup->Request) {
            case 0x00: // Get Status
                EP0_Buf[1][0] = USB_SelfPowered;
                EP0_Buf[1][1] = 0x00;
                BTable[0].COUNT_TX = 2;
                USB_SetEP(&USB->EP0R, USB_EP_TX_VALID, USB_EP_TX_VALID);
                break;
            case 0x01: // Clear Feature
                USB_SetEP(&USB->EP0R, USB_EP_TX_STALL, USB_EP_TX_VALID);
                break;
            case 0x03: // Set Feature
                BTable[0].COUNT_TX = 0;
                USB_SetEP(&USB->EP0R, USB_EP_TX_VALID, USB_EP_TX_VALID);
                break;
            case 0x05: // Set Address
                BTable[0].COUNT_TX = 0;
                USB_SetEP(&USB->EP0R, USB_EP_TX_VALID, USB_EP_TX_VALID);
                DeviceState = 1;
                break;
            case 0x06: // Get Descriptor
                switch (setup->DescriptorType) {
                case 0x01: { // Device Descriptor
                    USB_DESCRIPTOR_DEVICE *descriptor = USB_GetDeviceDescriptor();
                    USB_CopyMemory((volatile short*)descriptor, (volatile short*)EP0_Buf[1], sizeof(USB_DESCRIPTOR_DEVICE));
                    BTable[0].COUNT_TX = sizeof(USB_DESCRIPTOR_DEVICE);

                    USB_SetEP(&USB->EP0R, USB_EP_TX_VALID, USB_EP_TX_VALID);
                } break;
                case 0x02: { // Configuration Descriptor
                    short length;
                    char *descriptor = USB_GetConfigDescriptor(&length);
                    ControlState.Transfer.Buffer = descriptor;
                    ControlState.Transfer.BytesSent = 0;
                    ControlState.Transfer.Length = MIN(length, setup->Length);

                    USB_PrepareTransfer(&ControlState.Transfer, (short*)&USB->EP0R, (char*)&EP0_Buf[1], (short*)&BTable[0].COUNT_TX, 64);
                } break;
                case 0x03:                             // String Descriptor
                    if (setup->DescriptorIndex == 0) { // Get supported Languages
                        USB_DESCRIPTOR_STRINGS data = {
                            .Length = 4,
                            .Type = 0x03,
                        };
                        // Don't use prepare transfer, as the variable (buffer) will become invalid once we leave this block
                        USB_CopyMemory((volatile short*)&data, (volatile short*)EP0_Buf[1], 2);
                        *((short *)(&EP0_Buf[1][2])) = 0x0419;

                        BTable[0].COUNT_TX = 4;
                        USB_SetEP(&USB->EP0R, USB_EP_TX_VALID, USB_EP_TX_VALID);
                    } else {
                        short length = 0;
                        char *data = 0;

                        if (setup->DescriptorIndex == 0xEE) { // Microsoft OS Descriptor
                            data = USB_GetOSDescriptor(&length);
                        } else {
                            data = USB_GetString(setup->DescriptorIndex, setup->Index, &length);
                        }

                        short txLength = length + 2;
                        txLength = MIN(length, setup->Length);

                        USB_DESCRIPTOR_STRINGS header = {
                            .Length = length,
                            .Type = 0x03};

                        USB_CopyMemory((volatile short*)data, (volatile short*)(EP0_Buf[1] + 2), txLength - 2);

                        USB_CopyMemory((volatile short*)&header, (volatile short*)EP0_Buf[1], 2);

                        BTable->COUNT_TX = txLength;
                        USB_SetEP(&USB->EP0R, USB_EP_TX_VALID, USB_EP_TX_VALID);
                    }
                    break;
                case 0x06: {// Device Qualifier Descriptor
                    /*USB_QUALIFIER_DESCRIPTOR_DEVICE *qdescriptor = USB_GetQualifierDeviceDescriptor();
                    USB_CopyMemory((volatile short*)qdescriptor, (volatile short*)EP0_Buf[1], sizeof(USB_QUALIFIER_DESCRIPTOR_DEVICE));
                    BTable[0].COUNT_TX = sizeof(USB_QUALIFIER_DESCRIPTOR_DEVICE);*/
                    USB_SetEP(&USB->EP0R, USB_EP_TX_STALL, USB_EP_TX_VALID);
                    //USB_SetEP(&USB->EP0R, USB_EP_TX_VALID, USB_EP_TX_VALID);
                    } break;
                }
                break;
            case 0x07: // Set Descriptor
                // Allows the Host to alter the descriptor. Not supported
                USB_SetEP(&USB->EP0R, USB_EP_TX_STALL, USB_EP_TX_VALID);
                break;
            case 0x08: // Get Configuration
                if (DeviceState == 1 || DeviceState == 2) {
                    if (DeviceState == 1) {
                        ActiveConfiguration = 0;
                    }

                    EP0_Buf[1][0] = ActiveConfiguration;
                    BTable[0].COUNT_TX = 1;
                    USB_SetEP(&USB->EP0R, USB_EP_TX_VALID, USB_EP_TX_VALID);
                } else {
                    USB_SetEP(&USB->EP0R, USB_EP_TX_STALL, USB_EP_TX_VALID);
                }
                break;
            case 0x09: // Set Configuration
                //DeviceState = 1;
                if (DeviceState == 1 || DeviceState == 2) {
                    BTable[0].COUNT_TX = 0;
                    switch (setup->Value & 0xFF) {
                    case 0:
                        DeviceState = 1;
                        ActiveConfiguration = 0;
                        USB_SetEP(&USB->EP0R, USB_EP_TX_VALID, USB_EP_TX_VALID);
                        break;
                    case 1:
                        DeviceState = 2;
                        ActiveConfiguration = 1;
                        USB_SetEP(&USB->EP0R, USB_EP_TX_VALID, USB_EP_TX_VALID);
                        break;
                    default:
                        USB_SetEP(&USB->EP0R, USB_EP_TX_STALL, USB_EP_TX_VALID);
                        break;
                    }

                    if (DeviceState == 2) {
                        USB_SetEP(&USB->EP1R, 0x00, USB_EP_DTOG_RX | USB_EP_DTOG_TX);
                        USB_SetEP(&USB->EP2R, 0x00, USB_EP_DTOG_RX | USB_EP_DTOG_TX);
                        USB_SetEP(&USB->EP3R, 0x00, USB_EP_DTOG_RX | USB_EP_DTOG_TX);
                        USB_SetEP(&USB->EP4R, 0x00, USB_EP_DTOG_RX | USB_EP_DTOG_TX);
                        USB_SetEP(&USB->EP5R, 0x00, USB_EP_DTOG_RX | USB_EP_DTOG_TX);
                        USB_SetEP(&USB->EP6R, 0x00, USB_EP_DTOG_RX | USB_EP_DTOG_TX);
                        USB_SetEP(&USB->EP7R, 0x00, USB_EP_DTOG_RX | USB_EP_DTOG_TX);
                    }
                } else {
                    USB_SetEP(&USB->EP0R, USB_EP_TX_STALL, USB_EP_TX_VALID);
                }
                break;
            }
        }
        else if ((setup->RequestType & 0x0F) == 0x01) { // Interface requests
            switch (setup->Request) {
            case 0x00: // Get Status
                if (DeviceState == 2) {
                    EP0_Buf[1][0] = 0x00;
                    EP0_Buf[1][1] = 0x00;
                    BTable[0].COUNT_TX = 2;
                    USB_SetEP(&USB->EP0R, USB_EP_TX_VALID, USB_EP_TX_VALID);
                } else {
                    USB_SetEP(&USB->EP0R, USB_EP_TX_STALL, USB_EP_TX_VALID);
                }
                break;
            case 0x01: // Clear Feature
                USB_SetEP(&USB->EP0R, USB_EP_TX_STALL, USB_EP_TX_VALID);
                break;
            case 0x03: // Set Feature
                USB_SetEP(&USB->EP0R, USB_EP_TX_STALL, USB_EP_TX_VALID);
                break;
            case 0x0A: // Get Interface
                if (DeviceState == 2 && setup->Index < USB_NumInterfaces) {
                    EP0_Buf[1][0] = 0x00;
                    BTable[0].COUNT_TX = 1;
                    USB_SetEP(&USB->EP0R, USB_EP_TX_VALID, USB_EP_TX_VALID);
                } else {
                    USB_SetEP(&USB->EP0R, USB_EP_TX_STALL, USB_EP_TX_VALID);
                }
                break;
            case 0x0B: // Set Interface /!\ USB InANutshell is wrong here, it confuses the decimal value (11) with the hex one (0x0B)
                if (DeviceState == 2 && setup->Index < USB_NumInterfaces) {
                    BTable[0].COUNT_TX = 0;
                    USB_SetEP(&USB->EP0R, USB_EP_TX_VALID, USB_EP_TX_VALID);
                    USB_ResetClass(setup->Index, setup->Value);
                } else {
                    USB_SetEP(&USB->EP0R, USB_EP_TX_STALL, USB_EP_TX_VALID);
                }
                break;
            }
        }
        else if ((setup->RequestType & 0x0F) == 0x02) { // Endpoint requests
            switch (setup->Request) {
            case 0x00: // Get Status
                if ((DeviceState == 2 || (DeviceState == 1 && setup->Index == 0x00)) && setup->Index < USB_NumEndpoints) {
                    if (setup->Value == 0x00) {
                        EP0_Buf[1][0] = EndpointState[setup->Index];
                        EP0_Buf[1][1] = 0x00;
                        BTable[0].COUNT_TX = 2;
                        USB_SetEP(&USB->EP0R, USB_EP_TX_VALID, USB_EP_TX_VALID);
                    } else {
                        USB_SetEP(&USB->EP0R, USB_EP_TX_STALL, USB_EP_TX_VALID);
                    }
                } else {
                    USB_SetEP(&USB->EP0R, USB_EP_TX_STALL, USB_EP_TX_VALID);
                }
                break;
            case 0x01: // Clear Feature
                if ((DeviceState == 2 || (DeviceState == 1 && setup->Index == 0x00)) && setup->Index < USB_NumEndpoints) {
                    if (setup->Value == 0x00) {
                        EndpointState[setup->Index] = 0;
                        BTable[0].COUNT_TX = 0;
                        USB_SetEP(&USB->EP0R, USB_EP_TX_VALID, USB_EP_TX_VALID);
                    } else {
                        USB_SetEP(&USB->EP0R, USB_EP_TX_STALL, USB_EP_TX_VALID);
                    }
                } else {
                    USB_SetEP(&USB->EP0R, USB_EP_TX_STALL, USB_EP_TX_VALID);
                }
                break;
            case 0x03: // Set Feature
                if ((DeviceState == 2 || (DeviceState == 1 && setup->Index == 0x00)) && setup->Index < USB_NumEndpoints) {
                    if (setup->Value == 0x00) {
                        EndpointState[setup->Index] = 1;
                        BTable[0].COUNT_TX = 0;
                        USB_SetEP(&USB->EP0R, USB_EP_TX_VALID, USB_EP_TX_VALID);
                    } else {
                        USB_SetEP(&USB->EP0R, USB_EP_TX_STALL, USB_EP_TX_VALID);
                    }
                } else {
                    USB_SetEP(&USB->EP0R, USB_EP_TX_STALL, USB_EP_TX_VALID);
                }
                break;
            case 0x0C: // Sync Frame /!\ USB InANutshell is wrong here again, as it confuses the decimal value (12) with the hex one (0x0C)
                USB_SetEP(&USB->EP0R, USB_EP_TX_STALL, USB_EP_TX_VALID);
                break;
            }
        }
    }   
}

// FIXME: Убрать арифметику указателей (сделать switch по номерам внутри)
static
void
USB_PrepareTransfer
( USB_TRANSFER_STATE *transfer
, short *ep
, char *txBuffer
, short *txBufferCount
, short txBufferSize
) {
  *txBufferCount = MIN(txBufferSize, transfer->Length - transfer->BytesSent);
  #ifdef USB_TXTIMEOUT
  transfer->Timeout = counter;
  #endif
  //НЕ выставляем NAK, если нет готовых данных
  USB_CopyMemory((volatile short*)(transfer->Buffer + transfer->BytesSent), (volatile short*)txBuffer, *txBufferCount);
  transfer->BytesSent += *txBufferCount;
  USB_SetEP(ep, USB_EP_TX_VALID, USB_EP_TX_VALID);
  return;

  //Выставляем NAK, если нет готовых данных
  if (*txBufferCount > 0) {
    USB_CopyMemory((volatile short*)(transfer->Buffer + transfer->BytesSent), (volatile short*)txBuffer, *txBufferCount);
    transfer->BytesSent += *txBufferCount;
    USB_SetEP(ep, USB_EP_TX_VALID, USB_EP_TX_VALID);
  } else {
    USB_SetEP(ep, USB_EP_TX_NAK, USB_EP_TX_VALID);
  }
}

void 
USB_Init
( void ) 
{
  // Initialize the NVIC
  RCC -> APB1ENR1 |= RCC_APB1ENR1_USBEN;
  RCC -> AHB2ENR |= RCC_AHB2ENR_GPIOAEN;
  initPin_AF_PP(GPIOA,11,10);
  initPin_AF_PP(GPIOA,12,10);
  NVIC_SetPriority(USB_LP_IRQn, 8);
  NVIC_EnableIRQ(USB_LP_IRQn);
  
  ControlState.Receive.Buffer = ControlDataBuffer;

  // Enable USB macrocell
  USB->CNTR &= ~USB_CNTR_PDWN;

  // Wait 1μs until clock is stable
  delay(2);

  // Enable all interrupts & the internal pullup to put 1.5K on D+ for FullSpeed USB
  // USB->CNTR |= USB_CNTR_RESETM | USB_CNTR_CTRM; //=== Commit contains more interrupts that are not necessary
  // Enable all interrupts & the internal pullup to put 1.5K on D+ for FullSpeed USB
  USB->CNTR |= USB_CNTR_RESETM | USB_CNTR_CTRM | USB_CNTR_WKUPM | USB_CNTR_SUSPM;
  USB->BCDR |= USB_BCDR_DPPU;

  // Clear the USB Reset (D+ & D- low) to start enumeration
  USB->CNTR &= ~USB_CNTR_FRES;
  USB->ISTR = 0;
}

static
void
USB_ClearSRAM
( void ) 
{
  uint8_t *usbBuffer = (uint8_t*)__USBBUF_BEGIN;
  for (int i = 0; i < 1024; i++) {
    usbBuffer[i] = 0;
  }
}

void USB_HP_IRQHandler() {
    // Only take care of regular transmissions
    
    if ((USB->ISTR & USB_ISTR_CTR) != 0) {
        char ep = USB->ISTR & USB_ISTR_EP_ID;

        if (ep > 0 && ep < 8) {
            // On RX, call the registered callback if available
            if ((*(&USB->EP0R + ep * 2) & USB_EP_CTR_RX) != 0) {
                if (Buffers[ep * 2].CompleteCallback != 0) {
                    Buffers[ep * 2].CompleteCallback(ep, BTable[ep].COUNT_RX & 0x01FF);
                }

                USB_SetEP(&USB->EP0R + ep * 2, USB_EP_RX_VALID, USB_EP_CTR_RX | USB_EP_RX_VALID);
            }

            // On TX, check if there is some remaining data to be sent in the pending Transfers
            if ((*(&USB->EP0R + ep * 2) & USB_EP_CTR_TX) != 0) {
                if (Transfers[ep - 1].Length > 0) {
                    if (Transfers[ep - 1].Length > Transfers[ep - 1].BytesSent) {
                        USB_PrepareTransfer(&Transfers[ep - 1], (short*)(&USB->EP0R + ep * 2), (char*)(&Buffers[ep * 2 + 1].Buffer), (short*)(&BTable[ep].COUNT_TX), (short)(Buffers[ep * 2 + 1].Size));
                    } else if (Transfers[ep - 1].Length == Transfers[ep - 1].BytesSent) {
                        char length = Transfers[ep - 1].Length;
                        Transfers[ep - 1].Length = 0;

                        if (Buffers[ep * 2 + 1].CompleteCallback != 0) {
                            Buffers[ep * 2 + 1].CompleteCallback(ep, length);
                        }

                        // if complete and no new TX, add one empty packet to flush queue, send tx complete signal
                        if (Transfers[ep - 1].Length == 0) {
                            BTable[ep].COUNT_TX = 0;
                            USB_SetEP(&USB->EP0R + ep * 2, USB_EP_TX_VALID, USB_EP_TX_VALID);
                        }
                    }
                }

                USB_SetEP(&USB->EP0R + ep * 2, 0x00, USB_EP_CTR_TX);
            }
        }
    }
}

void USB_LP_IRQHandler() {
    if((USB->ISTR & USB_ISTR_RESET) != 0) {
        USB->ISTR = ~USB_ISTR_RESET;

        USB_ClearSRAM();

        //Prepare BTable
        USB->BTABLE = __MEM2USB(BTable);

        BTable[0].ADDR_RX = __MEM2USB(EP0_Buf[0]);
        BTable[0].ADDR_TX = __MEM2USB(EP0_Buf[1]);
        BTable[0].COUNT_TX = 0;
        BTable[0].COUNT_RX = (1 << 15) | (1 << 10);

        Buffers[0].Buffer = (char*)EP0_Buf[0];
        Buffers[0].Size = 64;
        Buffers[1].Buffer = (char*)EP0_Buf[1];
        Buffers[1].Size = 64;

        // Prepare for a setup packet (RX = Valid, TX = NAK)
        USB_SetEP(&USB->EP0R, USB_EP_CONTROL | USB_EP_RX_VALID | USB_EP_TX_NAK, USB_EP_TYPE_MASK | USB_EP_RX_VALID | USB_EP_TX_VALID);

        USB_ConfigureEndpoints();

        // Enable USB functionality and set address to 0
        DeviceState = 0;
        USB->DADDR = USB_DADDR_EF;
    }
    else if ((USB->ISTR & USB_ISTR_CTR) != 0) {
        //__BKPT(); // Breakpoint for debugging
        if((USB->ISTR & USB_ISTR_EP_ID) == 0) {
            USB_HandleControl();
        }
        else {
            USB_HP_IRQHandler();
        }
    }
    else if((USB->ISTR & USB_ISTR_SUSP) != 0){
        USB->ISTR = ~USB_ISTR_SUSP;
        USB_SuspendDevice();
        
        // On Suspend, the device should enter low power mode and turn off the USB-Peripheral
        USB->CNTR |= USB_CNTR_FSUSP;

        // If the device still needs power from the USB Host
        USB->CNTR |= USB_CNTR_LPMODE;        
    } 
    else if((USB->ISTR & USB_CLR_WKUP) != 0) {
      	USB->ISTR = ~USB_ISTR_WKUP;

        // Resume peripheral
        USB->CNTR &= ~(USB_CNTR_FSUSP | USB_CNTR_LPMODE);
        USB_WakeupDevice();
    }
}
  
// Копирование из ОЗУ в память USB
// Память USB состоит из слов по 16 бит, контроллер по умолчанию оперирует словами по 32 бита
// Поэтому заставляем его работать со словами по 16.
static
void 
USB_CopyMemory
( volatile uint16_t *source
, volatile uint16_t *target
, uint16_t length
)
{
  for (int i = 0; i < length / 2; i++) {
    target[i] = source[i];
  }

  if(length % 2 == 1) {
    ((char*)target)[length - 1] = ((char*)source)[length - 1];
  }
}

char *USB_GetString(char index, short lcid, short *length) {
    // Strings need to be in unicode (thus prefixed with u"...")
    // The length is double the character count + 2 — or use VSCode which will show the number of bytes on hover
    if (index == 1) {
        *length = 18;
        return (char*)u"Полюс-НТ";
    } else if (index == 2) {
        *length = 22;
        return (char*)u"КиберМишка";
    } else if(index == 3) {
        *length = 34;
        return (char*)utfSerial();
    }else if(index == 4) {
        *length = 32;
        return (char*)u"B0";
    }

    return 0;
}

//https://github.com/pbatard/libwdi/wiki/WCID-Devices#user-content-Implementation
char  *USB_GetOSDescriptor(short *length) {
  *length = 18;
  return (char*) u"MSFT100\x77";
  //return (char*) "\x4D\x00\x53\x00\x46\x00\x54\x00\x31\x00\x30\x00\x30\x00\x77";
  /*
    [ 0x12 //length
    , 0x03 //descroptor type (string)
    , 0x4D, 0x00, 0x53, 0x00, 0x46, 0x00, 0x54, 0x00, 0x31, 0x00, 0x30, 0x00, 0x30, 0x00 //"MSFT100", signature for WCID
    , 0x77 //vendor code
    , 0x00 //padding
    ];
  */
}

__weak void USB_SuspendDevice() {

}

__weak void USB_WakeupDevice(){
    
}

void USB_ResetClass(char interface, char alternateId) {
    // do nothing
}

static void USB_DistributeBuffers() {
    // This function will organize the USB-SRAM and assign RX- and TX-Buffers
    int addr = __USBBUF_BEGIN + sizeof(BTable);
    for (int i = 0; i < 16; i++) {
        if (Buffers[i].Size > 0) {
            Buffers[i].Buffer = (char*)addr;
            addr += Buffers[i].Size;

            if (addr & 0x01)
                addr++;
        } else {
            Buffers[i].Buffer = 0x00;
        }
    }
}

void USB_SetEPConfig(USB_CONFIG_EP config) {
    if (config.EP > 0 && config.EP < 8) {
        unsigned char rxSize = config.RxBufferSize;
        unsigned char txSize = config.TxBufferSize;

        if (rxSize & 0x01)
            rxSize++;
        if (txSize & 0x01)
            txSize++;

        Buffers[config.EP * 2].Size = config.RxBufferSize;
        Buffers[config.EP * 2 + 1].Size = config.TxBufferSize;
        Buffers[config.EP * 2].CompleteCallback = config.RxCallback;
        Buffers[config.EP * 2 + 1].CompleteCallback = 0;
        USB_DistributeBuffers();

        if (rxSize > 0) {
            BTable[config.EP].ADDR_RX = __MEM2USB(Buffers[config.EP * 2].Buffer);
        }
        if (txSize > 0) {
            BTable[config.EP].ADDR_TX = __MEM2USB(Buffers[config.EP * 2 + 1].Buffer);
        }

        BTable[config.EP].COUNT_TX = 0;
        if (rxSize < 64) {
            BTable[config.EP].COUNT_RX = (rxSize / 2) << 10;
        } else {
            BTable[config.EP].COUNT_RX = (1 << 15) | (((rxSize / 32) - 1) << 10);
        }

        // only allow to set ep type & kind
        short epConfig = config.Type & 0x0700;
        epConfig |= USB_EP_TX_NAK;
        epConfig |= config.EP;
        if (rxSize > 0) {
            epConfig |= USB_EP_RX_VALID;
        }

        USB_SetEP((&USB->EP0R) + 2 * config.EP, epConfig, USB_EP_DTOG_RX | USB_EP_RX_VALID | USB_EP_TYPE_MASK | USB_EP_KIND | USB_EP_DTOG_TX | USB_EP_TX_VALID | 0x000F);
    }
}

void USB_ConfigureEndpoints() {
    // Configure all endpoints and route their reception to the functions that need them
    USB_CONFIG_EP epSystem = {
        .EP = EP_SYSTEM,
        .RxBufferSize = 64,
        .TxBufferSize = 64,
        .Type = USB_EP_BULK,
        .RxCallback = Rx_HandleSystem,
    };
    USB_CONFIG_EP epData = {
        .EP = EP_DATA,
        .RxBufferSize = 64,
        .TxBufferSize = 64,
        .Type = USB_EP_BULK,
        .RxCallback = Rx_HandleData,
    };
        
    USB_SetEPConfig(epData);
    USB_SetEPConfig(epSystem);
    usbStatus.isUp = true;
}

void USB_Transmit(char ep, char *buffer, short length) {
    // Prepare the transfer metadata and initiate the chunked transfer
    if (ep == 0) {
        ControlState.Transfer.Buffer = buffer;
        ControlState.Transfer.BytesSent = 0;
        ControlState.Transfer.Length = length;
        USB_PrepareTransfer(&ControlState.Transfer, (short int*)&USB->EP0R, (char*)EP0_Buf[1], (short int*)&BTable[0].COUNT_TX, 64);
    } else if (ep < 8) {
        Transfers[ep - 1].Buffer = buffer;
        Transfers[ep - 1].Length = length;
        Transfers[ep - 1].BytesSent = 0;
        USB_PrepareTransfer(&Transfers[ep - 1], (short int*)((&USB->EP0R) + ep * 2),(char*) Buffers[ep * 2 + 1].Buffer, (short int*)&BTable[ep].COUNT_TX, Buffers[ep * 2 + 1].Size);
    }
}

char USB_IsTransmitPending(char ep) {
    USB_TRANSFER_STATE* tx;
    if (ep == 0) {
        tx = &ControlState.Transfer;
    } else {
        tx = &Transfers[ep - 1];
    }
    #ifdef USB_TXTIMEOUT
    if(counter - tx->Timeout > USB_TXTIMEOUT) {
        tx->Length = 0;
    }
    #endif
    return tx->Length > 0;
}

void USB_Fetch(char ep, char *buffer, short *length) {
    // Read data from the RX Buffer
    if (ep >= 0 && ep < 8) {
        short rxcount = BTable[ep].COUNT_RX & 0x1FF;
        //*length = MIN(rxcount, *length); //Ограничение на длину чтения
        *length = rxcount;
        USB_CopyMemory((volatile short*)Buffers[ep * 2].Buffer, (volatile short*)buffer, *length);
    }
}

#include "handlers.c"

void
usbClear
(void)
{
  // Отключаем прерывание
  NVIC_DisableIRQ(USB_LP_IRQn);

  // Отключаем USB
   
  // Сбрасываем USB (через бит RST)

  RCC -> APB1RSTR1 |=  RCC_APB1RSTR1_USBRST;
  RCC -> APB1RSTR1 &= ~RCC_APB1RSTR1_USBRST;
  RCC -> APB1ENR1 &= ~RCC_APB1ENR1_USBEN;
  
  // Зачищаем память
  USB_ClearSRAM();
}
}