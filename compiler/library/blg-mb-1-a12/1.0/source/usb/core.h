#ifndef __USB_H
#define __USB_H

#define DEFINE_bmRequestType_Dir_Mask 0x80
#define DEFINE_bmRequestType_Dir_HostToDevice 0x00
#define DEFINE_bmRequestType_Dir_DeviceToHost 0x80

typedef struct {
    unsigned char RequestType;
    unsigned char Request;
    union {
        unsigned short Value;
        struct {
            unsigned char DescriptorIndex;
            unsigned char DescriptorType;
        };
    };
    unsigned short Index;
    unsigned short Length;
} USB_SETUP_PACKET;

typedef struct {
    unsigned char EP;
    unsigned char RxBufferSize;
    unsigned char TxBufferSize;
    unsigned short Type;
    void (*RxCallback)(char ep, short length);
    void (*TxCallback)(char ep, short length);
} USB_CONFIG_EP;

#ifndef __weak
#define __weak __attribute__((weak))
#endif

#define USB_SelfPowered 0
#define USB_NumInterfaces 1
#define USB_NumEndpoints 8
#define USB_MaxControlData 64

#define USB_OK 0
#define USB_BUSY 1
#define USB_ERR 2

#pragma pack(1)
typedef struct {
    unsigned char Length;
    unsigned char Type;
} USB_DESCRIPTOR_STRINGS;

typedef struct {
    unsigned char Length;
    unsigned char Type;
    unsigned short USBVersion;
    unsigned char DeviceClass;
    unsigned char DeviceSubClass;
    unsigned char DeviceProtocol;
    unsigned char MaxPacketSize;
    unsigned short VendorID;
    unsigned short ProductID;
    unsigned short DeviceVersion;
    unsigned char strManufacturer;
    unsigned char strProduct;
    unsigned char strSerialNumber;
    unsigned char Configurations;
} USB_DESCRIPTOR_DEVICE;
// FIXME: Этот дескриптор описывает, как будет работать устройство
// в другом скоростном режиме (Full-Speed или High-Speed)
/*typedef struct {
    unsigned char Length;
    unsigned char Type;
    unsigned short USBVersion;
    unsigned char DeviceClass;
    unsigned char DeviceSubClass;
    unsigned char DeviceProtocol;
    unsigned char MaxPacketSize;
    unsigned char Configurations;
    unsigned char Reserved;
} USB_QUALIFIER_DESCRIPTOR_DEVICE;*/
typedef struct {
    unsigned char Length;
    unsigned char Type;
    unsigned short TotalLength;
    unsigned char Interfaces;
    unsigned char ConfigurationID;
    unsigned char strConfiguration;
    unsigned char Attributes;
    unsigned char MaxPower;
} USB_DESCRIPTOR_CONFIG;

typedef struct {
    unsigned char Length;
    unsigned char Type;
    unsigned char InterfaceID;
    unsigned char AlternateID;
    unsigned char Endpoints;
    unsigned char Class;
    unsigned char SubClass;
    unsigned char Protocol;
    unsigned char strInterface;
} USB_DESCRIPTOR_INTERFACE;

typedef struct {
    unsigned char Length;
    unsigned char Type;
    unsigned char Address;
    unsigned char Attributes;
    unsigned short MaxPacketSize;
    unsigned char Interval;
} USB_DESCRIPTOR_ENDPOINT;  

#pragma pack()

#endif
