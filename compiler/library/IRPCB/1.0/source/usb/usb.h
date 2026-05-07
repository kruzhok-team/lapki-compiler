/*
 * This module provides only low-level services, such as transmitting, receiving
 * and default Setup Request handling. All other must provide higher-level driver.
 */

#ifndef USB_USB_H_
#define USB_USB_H_

#include "types.h"
#include "board.h"
#include "kl_buf.h"

namespace Usb {
#if 1 // ==== Setup pkt ====
#pragma pack(push, 1)
union SetupPkt_t {
    uint64_t W64;
    uint8_t Buf8[8];
    struct {
        union {                   // [0]
            uint8_t bmRequestType;
            struct { // From LSB to MSB
                uint8_t Recipient: 5;
                uint8_t Type: 2;
                uint8_t Direction: 1;
            };
        };
        uint8_t bRequest;         // [1]
        union {
            uint16_t wValue;      // [2;3]
            struct { // From LSB to MSB
                uint8_t dscIndex; // [2]
                uint8_t dscType;  // [3]
            };
            uint8_t cfgNumber;    // [2]
            uint8_t epFeatureSelector; // [2]
        };
        union {
            uint16_t wIndex;      // [4;5]
            struct { // From LSB to MSB
                uint8_t Number : 4;
                uint8_t __reserved46 : 3;
                uint8_t dir : 1;
            } Ep;
        };
        uint16_t wLength;   // [6;7]
    };
};
#pragma pack(pop)
extern SetupPkt_t SetupPkt;

#define USB_REQDIR_HOST2DEV     0UL
#define USB_REQDIR_DEV2HOST     1UL

#define USB_REQ_RECPNT_DEVICE   0UL
#define USB_REQ_RECPNT_INTRFCE  1UL
#define USB_REQ_RECPNT_EP       2UL
#define USB_REQ_RECPNT_OTHER    3UL

#define USB_REQTYPE_STD         0UL
#define USB_REQTYPE_CLASS       1UL
#define USB_REQTYPE_VENDOR      2UL
#endif

enum class Evt { Reset = 0, Address = 1, Configured = 2, Unconfigured = 3, Suspend = 4, Wakeup = 5, Stalled = 6 };

// Callbacks. Must be implemented at higher level.
void EventCallback(Evt event);
#if USB_SOF_CB_EN
void SOFCallback();
#endif

// Setup request callback: returns OK if request processed, or !Ok if standard processing required
struct StpReqCbRpl_t {
    retv Retval = retv::NotFound;
    uint8_t *Buf = nullptr;
    uint32_t Sz = 0;
    ftVoidVoid EndTransferCb = nullptr;
};
StpReqCbRpl_t SetupReqHookCallback();

// Getting descriptors. Must be implemented in descriptors_xxx.cpp
Buf_t GetDescriptor(uint8_t dtype,  uint8_t dindex, uint16_t lang);

// ==== Endpoints ====
#define EP0_SZ      64  // Control Endpoint must have a packet size of 64 bytes. Do not touch this.
enum class EpType { Ctrl, Iso, Bulk, Interrupt };
// Default values for Ep0
struct EpConfig_t {
    ftVoidU32 OutTransferEndCallback = nullptr;
    ftVoidVoid InTransferEndCallback = nullptr;
    EpType Type = EpType::Ctrl;
    uint32_t OutMaxPktSz = EP0_SZ; // FS: up to 1023 bytes, HS: up to 1024 bytes. Must be 0 if not used
    uint32_t InMaxPktSz = EP0_SZ;  // FS: up to 1023 bytes, HS: up to 1024 bytes. Must be 0 if not used
    uint32_t InMultiplier = 1;
};

// Call this when USB is configured: from EventCallback
void InitEp(uint32_t ep, const EpConfig_t *pCfg);

// Endpoints transfer starting
void StartReceiveI(uint32_t ep, uint8_t *pBuf, uint32_t MaxSz);
void StartTransmitI(uint32_t ep, uint8_t *pBuf, uint32_t Sz);
void StartTransmit(uint32_t ep, uint8_t *pBuf, uint32_t Sz);

// Endpoints transfer status
bool IsEpTransmitting(uint32_t ep);
bool IsEpReceiving(uint32_t ep);

// GPIO init is not required. Don't forget to setup USB clock = 48MHz
void Connect();
void Disconnect();
bool IsActive();

} // namespace

#endif /* USB_USB_H_ */
