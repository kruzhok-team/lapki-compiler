#include "usb.h"
#include "gd_lib.h"
#include "yartos.h"
#include "shell.h"

// Params
#define USB_RX_FIFO_SZ      512U // Why 512? Because why not. There is 1280 bytes total, for both RX & TX
#define USB_RX_FIFO_SZ32    (USB_RX_FIFO_SZ / 4) // rx_fifo_size in 32-bit words
#define USB_FIFO_MEM_SZ32   320 // Total FIFO size in 32-bit words; hardware is such as it is.
#define UTT_VALUE_FS        5 // Turnaround time in PHY clocks for Full Speed. No info why 5.
#define UTT_VALUE_HS        9 // Turnaround time in PHY clocks for High Speed. No info why 9.

// Standard request constants
#define USB_REQ_GET_STATUS                  0U
#define USB_REQ_CLEAR_FEATURE               1U
#define USB_REQ_SET_FEATURE                 3U
#define USB_REQ_SET_ADDRESS                 5U
#define USB_REQ_GET_DESCRIPTOR              6U
#define USB_REQ_SET_DESCRIPTOR              7U
#define USB_REQ_GET_CONFIGURATION           8U
#define USB_REQ_SET_CONFIGURATION           9U
#define USB_REQ_GET_INTERFACE               10U
#define USB_REQ_SET_INTERFACE               11U
#define USB_REQ_SYNCH_FRAME                 12U

#define USB_FEATURE_ENDPOINT_HALT           0U
#define USB_FEATURE_DEVICE_REMOTE_WAKEUP    1U
#define USB_FEATURE_TEST_MODE               2U

// Macro to construct single number out of two. To simplify processing of standard request.
#define REC_REQ(recipient, request)         (recipient | (request << 8))

enum class UsbSta { Stop, Ready, Selected, Active, Suspended };
static UsbSta UsbState= UsbSta::Stop, SavedState = UsbSta::Stop;
static uint8_t address; // Assigned USB address
static uint8_t configuration; // Current configuration
#if USB_REMOTE_WKUP_EN == 1U
static uint16_t status; // Contains wkup bit and self-powered bit
#endif

// Short constants to transmit USB statuses
static const uint8_t zero_status[2]   = {0x00, 0x00};
static const uint8_t active_status[2] = {0x00, 0x00};
static const uint8_t halted_status[2] = {0x01, 0x00};

static retv DefaultRequestHandler();

const Usb::EpConfig_t* EpCfg[USBFS_MAX_EP_COUNT]; // Include Ep0

#if 1 // ========================== Ep Buffers =================================
struct EpState_t {
    uint32_t Sz = 0;         // Requested transmit transfer size
    uint32_t cnt = 0;        // Transmitted bytes so far
    uint8_t *pBuf = nullptr; // Pointer to the transmission linear buffer
    uint32_t TotalSz = 0;    // Total transmit transfer size
};

// Include Ep0
EpState_t InEpState [USBFS_MAX_EP_COUNT];
EpState_t OutEpState[USBFS_MAX_EP_COUNT];
#endif

#if 1 // ======================== Memory Control ===============================
static class RxFifo_t {
public:
    void Flush() {
        USB->GRSTCTL = GRSTCTL_RXFF;
        while(USB->GRSTCTL & GRSTCTL_RXFF);
        DelayLoop(18); // Wait for 3 PHY Clocks
    }

    void ReadToBuf(uint8_t *buf, uint32_t n, uint32_t max) {
        uint32_t w = 0;
        uint32_t i = 0;
        while(i < n) {
            if((i & 3) == 0) w = USB->FIFO[0][0];
            if(i < max) {
                *buf++ = (uint8_t)w;
                w >>= 8;
            }
            i++;
        }
    }

    void ReadToFunc(ftVoidU8 Put, uint32_t n) {
        uint32_t w = 0;
        uint32_t i = 0;
        while(i < n) {
            if((i & 3) == 0) w = USB->FIFO[0][0];
            Put((uint8_t)w);
            w >>= 8;
            i++;
        }
    }
} RxFifo;

static class TxFifo_t {
private:
    uint32_t PtrMemNext; // Pointer to the next address in the packet memory
public:
    void Reset() { PtrMemNext = USB_RX_FIFO_SZ32; } // Tx RAM starts on top of Rx RAM

    void Flush(uint32_t fifo) {
        USB->GRSTCTL = GRSTCTL_TXFNUM(fifo) | GRSTCTL_TXFF;
        while(USB->GRSTCTL & GRSTCTL_TXFF); // Wait completion
        DelayLoop(18); // Wait for 3 PHY Clocks
    }

    uint32_t Allocate(uint32_t size) {
        uint32_t next = PtrMemNext; // Save current address
        PtrMemNext += size;
        Sys::DbgAssert(PtrMemNext <= USB_FIFO_MEM_SZ32, "OTG FIFO memory overflow");
        return next;
    }

    void Fill(uint32_t ep) { // otg_txfifo_handler
        EpState_t *isp = &InEpState[ep];
        // The TXFIFO is filled until there is space left and data to be transmitted
        while(isp->cnt < isp->Sz) { // Do until all the data will be sent
            // Number of bytes remaining in the current transaction
            uint32_t n = isp->Sz - isp->cnt;
            if(n > EpCfg[ep]->InMaxPktSz) n = EpCfg[ep]->InMaxPktSz;
            // Check that the TXFIFO has enough space for the next packet
            if(((USB->ie[ep].DIEPTFSTAT & DTXFSTS_INEPTFSAV_MASK) * 4) < n) return;
            // Fill it
            volatile uint32_t *fifop = USB->FIFO[ep]; // Destination
            uint8_t *buf = &isp->pBuf[isp->cnt]; // Src
            isp->cnt += n; // prepare for next time
            while(true) {
                *fifop = *((uint32_t*)buf);
                if(n <= 4) break;
                n -= 4;
                buf += 4;
            }
        } // while
        USB->DIEPFEINTEN &= ~DIEPEMPMSK_INEPTXFEM(ep); // Disable FIFO_EMPTY IRQ
    }

} TxFifo;
#endif

#if 1 // ============================= Endpoints ===============================
enum class EpSta { DISABLED, STALLED, ACTIVE };
static uint32_t EpTransmittingFlag = 0, EpReceivingFlag = 0;

static inline void CallOutTransferEndCallback(uint32_t ep, uint32_t Sz) {
    if(EpCfg[ep]->OutTransferEndCallback) EpCfg[ep]->OutTransferEndCallback(Sz);
}

static inline void CallInTransferEndCallback(uint32_t ep) {
    if(EpCfg[ep]->InTransferEndCallback) EpCfg[ep]->InTransferEndCallback();
}

static inline void StallIn (uint32_t ep) { USB->ie[ep].DIEPCTL |= DIEPCTL_STALL; }
static inline void StallOut(uint32_t ep) { USB->oe[ep].DOEPCTL |= DOEPCTL_STALL; }
static inline void ClearIn (uint32_t ep) { USB->ie[ep].DIEPCTL &= ~DIEPCTL_STALL; }
static inline void ClearOut(uint32_t ep) { USB->oe[ep].DOEPCTL &= ~DOEPCTL_STALL; }

void StartInTransfer(uint32_t ep) {
    EpState_t *isp = &InEpState[ep];
    // Transfer initialization
    isp->TotalSz = isp->Sz;
    // Special case, sending zero size packet
    if(isp->Sz == 0) USB->ie[0].DIEPLEN = DIEPTSIZ_PKTCNT(1) | DIEPTSIZ_XFRSIZ(0);
    else { // Ordinal case
        if(ep == 0 and isp->Sz > EP0_SZ) isp->Sz = EP0_SZ; // Single pkt only for Ep0
        uint32_t pktcnt = (isp->Sz + EpCfg[ep]->InMaxPktSz - 1UL) / EpCfg[ep]->InMaxPktSz;
        USB->ie[ep].DIEPLEN = DIEPTSIZ_MCNT(1) | DIEPTSIZ_PKTCNT(pktcnt) | DIEPTSIZ_XFRSIZ(isp->Sz);
    }
    // Special case for isochronous endpoint
    if(EpCfg[ep]->Type == Usb::EpType::Iso) { // Toggle odd/even bit
        if(USB->DSTS & DSTS_FNSOF_ODD) USB->ie[ep].DIEPCTL |= DIEPCTL_SEVNFRM;
        else USB->ie[ep].DIEPCTL |= DIEPCTL_SODDFRM;
    }
    // Start operation
    USB->ie[ep].DIEPCTL |= DIEPCTL_EPENA | DIEPCTL_CNAK;
    USB->DIEPFEINTEN |= DIEPEMPMSK_INEPTXFEM(ep);
}

    /* Transaction size is rounded to a multiple of packet size because the
     following requirement in the RM:
     "For OUT transfers, the transfer size field in the endpoint's transfer
     size register must be a multiple of the maximum packet size of the
     endpoint, adjusted to the Word boundary".*/
void StartOutTransfer(uint32_t ep) {
    EpState_t *osp = &OutEpState[ep];
    // Transfer initialization
    osp->TotalSz = osp->Sz;
    if(ep == 0 and osp->Sz > EP0_SZ) osp->Sz = EP0_SZ; // Single pkt only for Ep0
    uint32_t MaxSz = EpCfg[ep]->OutMaxPktSz;
    uint32_t pktcnt = (osp->Sz + MaxSz - 1UL) / MaxSz;
    uint32_t rxsize = (pktcnt * MaxSz + 3U) & 0xFFFFFFFCUL;
    // Check if size is not too large
//    Sys::DbgAssert(rxsize < 0x7FFFF, "Bad Size"); // rxsize is 19 bit value
//    Sys::DbgAssert(pcnt < 0x3FF, "Bad Size"); // len is 10 bit value
    // Setup transaction parameters in DOEPTSIZ
    USB->oe[ep].DOEPTSIZ = DOEPTSIZ_STUPCNT(3) | DOEPTSIZ_PKTCNT(pktcnt) | DOEPTSIZ_XFRSIZ(rxsize);
    // Special case of isochronous endpoint
    if(EpCfg[ep]->Type == Usb::EpType::Iso) { // Toggle odd/even bit
        if(USB->DSTS & DSTS_FNSOF_ODD) USB->oe[ep].DOEPCTL |= DOEPCTL_SEVNFRM;
        else USB->oe[ep].DOEPCTL |= DOEPCTL_SODDFRM;
    }
    // Start operation
    USB->oe[ep].DOEPCTL |= DOEPCTL_EPENA | DOEPCTL_CNAK;
}

namespace Ep0 { // ======== EP0 =========
enum class Sta {
    STP_WAITING,        // Waiting for SETUP data
    IN_TX,              // Transmitting
    IN_WAITING_TX0,     // Waiting transmit 0
    IN_SENDING_STS,     // Sending status
    OUT_WAITING_STS,    // Waiting status
    OUT_RX,             // Receiving
    ERROR               // Error, EP0 stalled
} State = Sta::STP_WAITING;

static union {
    uint64_t SetupBufW64;
    uint8_t SetupBuf[8];
};

uint8_t *ptrNext = nullptr;
uint32_t TransferLen = 0;
ftVoidVoid EndTransactionCallback = nullptr;

// Setup IN or OUT data phase after SETUP pkt
inline void PrepareSetupTransfer(uint8_t *pBuf, uint32_t Len, ftVoidVoid AEndTransactionCallback) {
    ptrNext = pBuf;
    TransferLen = Len;
    EndTransactionCallback = AEndTransactionCallback;
}

void SetupCallback() {
    // Is the EP0 state machine in the correct state for handling setup packets?
    if(State != Sta::STP_WAITING) { // Unexpected, could require handling with a warning event
        State = Sta::STP_WAITING; // Reset the EP0 state machine and proceed
    }
    // Read the setup data from buffer to SetupPkt struct
    Usb::SetupPkt.W64 = SetupBufW64;
    // Do not process if app handler processed the request
    Usb::StpReqCbRpl_t Rpl = Usb::SetupReqHookCallback();
    if(Rpl.Retval == retv::Ok) PrepareSetupTransfer(Rpl.Buf, Rpl.Sz, Rpl.EndTransferCb);
    else { // Callback did not process the request. Try to process standard req, stall if fail
        if(Usb::SetupPkt.Type != USB_REQTYPE_STD or DefaultRequestHandler() != retv::Ok) {
            /* Error response, the state machine goes into an error state, the low
             level layer will have to reset it to USB_EP0_WAITING_SETUP after
             receiving a SETUP packet.*/
            StallIn(0);
            StallOut(0);
            Usb::EventCallback(Usb::Evt::Stalled);
            State = Sta::ERROR;
            return;
        }
    }
#if USB_SET_ADDRESS_ACK_BY_HW // Zero-length packet sent by hardware
    if(Usb::SetupPkt.bRequest == USB_REQ_SET_ADDRESS) return;
#endif
    // Data to transfer is prepared either by StdReqHookCallback, or by DefaultRequestHandler using PrepareSetupTransfer
    uint32_t max = Usb::SetupPkt.wLength;
    // The transfer size cannot exceed the specified amount
    if(TransferLen > max) TransferLen = max;
    if(Usb::SetupPkt.Direction == USB_REQDIR_DEV2HOST) { // IN phase
        if(TransferLen != 0U) {
            // Start the transmit phase
            State = Sta::IN_TX;
            Sys::LockFromIRQ();
            Usb::StartTransmitI(0, ptrNext, TransferLen);
            Sys::UnlockFromIRQ();
        }
        else { // No transmission phase, directly receiving the zero-sized status packet
            State = Sta::OUT_WAITING_STS;
            Sys::LockFromIRQ();
            Usb::StartReceiveI(0, nullptr, 0);
            Sys::UnlockFromIRQ();
        }
    }
    else { // HOST2DEV, OUT phase
        if(TransferLen != 0U) { // There is something to receive
            // Start the receive phase
            State = Sta::OUT_RX;
            Sys::LockFromIRQ();
            Usb::StartReceiveI(0, ptrNext, TransferLen);
            Sys::UnlockFromIRQ();
        }
        else { // No receive phase, directly sending the zero sized status packet
            State = Sta::IN_SENDING_STS;
            Sys::LockFromIRQ();
            Usb::StartTransmitI(0, nullptr, 0);
            Sys::UnlockFromIRQ();
        }
    }
}

void InCallback() {
    uint32_t max;
    switch(State) {
        case Sta::IN_TX:
            max = Usb::SetupPkt.wLength;
            /* If the transmitted size is less than the requested size and it is a
             multiple of the maximum packet size then a zero size packet must be
             transmitted.*/
            if((TransferLen < max) and ((TransferLen % EP0_SZ) == 0U)) {
                Sys::LockFromIRQ();
                Usb::StartTransmitI(0, nullptr, 0);
                Sys::UnlockFromIRQ();
                State = Sta::IN_WAITING_TX0;
                return;
            }
            // Fall through
        // Transmit phase over, receiving the zero sized status packet
        case Sta::IN_WAITING_TX0:
            State = Sta::OUT_WAITING_STS;
            Sys::LockFromIRQ();
            Usb::StartReceiveI(0, nullptr, 0);
            Sys::UnlockFromIRQ();
            return;
        // Status packet sent, invoke the callback if defined
        case Sta::IN_SENDING_STS:
            if(EndTransactionCallback) EndTransactionCallback();
            State = Sta::STP_WAITING;
            return;
        // Invalid states in the IN phase
        case Sta::STP_WAITING:
        case Sta::OUT_WAITING_STS:
        case Sta::OUT_RX:
            Sys::DbgAssert(false, "EP0 IN error");
            // Fall through
        case Sta::ERROR:
            /* Error response, the state machine goes into an error state, the low
             level layer will have to reset it to USB_EP0_WAITING_SETUP after
             receiving a SETUP packet.*/
            StallIn(0);
            StallOut(0);
            Usb::EventCallback(Usb::Evt::Stalled);
            State = Sta::ERROR;
            return;
        default:
            Sys::DbgAssert(false, "EP0 IN invalid state");
    }
}

void OutCallback() {
    EpState_t *osp = &OutEpState[0];
    /* If the transaction only covers part of the total transfer,
     * another transaction is immediately initiated to cover the remainder */
    if(((osp->cnt % EP0_SZ) == 0) and (osp->Sz < osp->TotalSz)) {
        osp->Sz = osp->TotalSz - osp->Sz;
        osp->cnt = 0;
        Sys::LockFromIRQ();
        StartOutTransfer(0);
        Sys::UnlockFromIRQ();
        return;
    }
    // All data received
    EpReceivingFlag &= ~(1UL << 0);
    switch(State) {
        // Receive phase over, send the zero sized status packet
        case Sta::OUT_RX:
            State = Sta::IN_SENDING_STS;
            Sys::LockFromIRQ();
            Usb::StartTransmitI(0, nullptr, 0);
            Sys::UnlockFromIRQ();
            return;
        // Status packet received, it must be zero sized, invoking the callback if defined
        case Sta::OUT_WAITING_STS:
            if(osp->cnt != 0U) break;
            if(EndTransactionCallback) EndTransactionCallback();
            State = Sta::STP_WAITING;
            return;
        // Invalid states in the OUT phase
        case Sta::STP_WAITING:
        case Sta::IN_TX:
        case Sta::IN_WAITING_TX0:
        case Sta::IN_SENDING_STS:
            Sys::DbgAssert(false, "EP0 OUT error");
            // Fall through
        case Sta::ERROR:
            /* Error response, the state machine goes into an error state, the low
             level layer will have to reset it to USB_EP0_WAITING_SETUP after
             receiving a SETUP packet.*/
            StallIn(0);
            StallOut(0);
            Usb::EventCallback(Usb::Evt::Stalled);
            State = Sta::ERROR;
            return;
        default:
            Sys::DbgAssert(false, "EP0 OUT invalid state");
    }
}

void Reset() {
    InEpState[0].Sz = 0;
    InEpState[0].cnt = 0;
    InEpState[0].TotalSz = 0;
    OutEpState[0].Sz = 0;
    OutEpState[0].cnt = 0;
    OutEpState[0].TotalSz = 0;
}
} // Ep0 namespace

static void EpDisableAll() { // otg_disable_ep
    for(uint32_t i=0; i < USBFS_MAX_EP_COUNT; i++) {
        USB->ie[i].DIEPCTL = 0;
        USB->ie[i].DIEPLEN = 0;
        USB->ie[i].DIEPINT = 0xFFFFFFFF;
        USB->oe[i].DOEPCTL = 0;
        USB->oe[i].DOEPTSIZ = 0;
        USB->oe[i].DOEPINTF = 0xFFFFFFFF;
    }
    USB->DAEPINTEN = DAINTMSK_OEPM(0) | DAINTMSK_IEPM(0);
}

static EpSta GetStatusIn(uint32_t ep) {
    uint32_t ctl = USB->ie[ep].DIEPCTL;
    if(!(ctl & DIEPCTL_USBAEP)) return EpSta::DISABLED;
    if(ctl & DIEPCTL_STALL)     return EpSta::STALLED;
    return EpSta::ACTIVE;
}

static EpSta GetStatusOut(uint32_t ep) {
    uint32_t ctl = USB->oe[ep].DOEPCTL;
    if(!(ctl & DOEPCTL_USBAEP)) return EpSta::DISABLED;
    if(ctl & DOEPCTL_STALL)     return EpSta::STALLED;
    return EpSta::ACTIVE;
}
#endif

#if 1 // =========================== USB core ==================================
static void DisableEndpointsI() {
    Sys::DbgAssert(UsbState == UsbSta::Active, "invalid state");
    EpTransmittingFlag &= 1UL; // Do not touch Ep0
    EpReceivingFlag &= 1UL; // Do not touch Ep0
    TxFifo.Reset();
    EpDisableAll();
}

static void SetAddress() {
    address = (uint8_t)Usb::SetupPkt.wValue;
    USB->DCFG = (USB->DCFG & ~DCFG_DAD_MASK) | DCFG_DAD(address);
    Usb::EventCallback(Usb::Evt::Address);
    UsbState = UsbSta::Selected;
}

static retv DefaultRequestHandler() {
    Buf_t Dsc;
    // Construct Recipient and Request into single data word
    uint32_t RecipientAndReq = (uint32_t)Usb::SetupPkt.Recipient | ((uint32_t)Usb::SetupPkt.bRequest << 8);
    uint16_t W16;
    // Decode the request
    switch(RecipientAndReq) {
        /* The Get Status request directed at the device will return two bytes, only lesser bits D0 and D1 are used.
         * If D0 is set, then this indicates the device is self powered. If clear, the device is bus powered.
         * If D1 is set, the device has remote wakeup enabled and can wake the host up during suspend.
         * The remote wakeup bit can be controlled by the SetFeature and ClearFeature requests
         * with a feature selector of DEVICE_REMOTE_WAKEUP (0x01)
         */
        case REC_REQ(USB_REQ_RECPNT_DEVICE, USB_REQ_GET_STATUS): // Just return the current status word
            W16 = (USB_REMOTE_WKUP_EN << 1) | USB_SELF_POWERED;
            Ep0::PrepareSetupTransfer((uint8_t*)&W16, 2, nullptr);
            return retv::Ok;

        // Only the DEVICE_REMOTE_WAKEUP is handled here, any other feature number is handled as an error
#if USB_REMOTE_WKUP_EN == 1U
        case (USB_REQ_RECPNT_DEVICE | (USB_REQ_CLEAR_FEATURE << 8)):
            if(Usb::SetupPkt.wValue == USB_FEATURE_DEVICE_REMOTE_WAKEUP) {
                status &= ~(1U << 1); // Clear wkup bit
                Ep0.PrepareSetupTransfer(nullptr, 0, nullptr);
                return retv::Ok;
            }
            return retv::Fail;
        case REC_REQ(USB_REQ_RECPNT_DEVICE, USB_REQ_SET_FEATURE):
            if(Usb::SetupPkt.wValue == USB_FEATURE_DEVICE_REMOTE_WAKEUP) {
                status |= (1U << 1); // Set wkup bit
                Ep0.PrepareSetupTransfer(nullptr, 0, nullptr);
                return retv::Ok;
            }
            return retv::Fail;
#endif

        case REC_REQ(USB_REQ_RECPNT_DEVICE, USB_REQ_SET_ADDRESS):
            /* The SET_ADDRESS handling can be performed here or postponed after
             the status packet depending on the USB_SET_ADDRESS_MODE low
             driver setting.*/
#if USB_SET_ADDR_AFTER_ZEROPKT  // (1) Send ZeroPkt (2) Set Address
            Ep0.PrepareSetupTransfer(nullptr, 0, SetAddress);
#else // (1) Set Address (2) Send ZeroPkt
            SetAddress();
            Ep0::PrepareSetupTransfer(nullptr, 0, nullptr);
#endif
            return retv::Ok;

        case REC_REQ(USB_REQ_RECPNT_DEVICE,  USB_REQ_GET_DESCRIPTOR):
        case REC_REQ(USB_REQ_RECPNT_INTRFCE, USB_REQ_GET_DESCRIPTOR):
            Dsc = Usb::GetDescriptor(Usb::SetupPkt.dscType, Usb::SetupPkt.dscIndex, Usb::SetupPkt.wIndex);
            if(Dsc.ptr == nullptr) return retv::Fail;
            // Length is first byte of descriptor
            Ep0::PrepareSetupTransfer(Dsc.ptr, Dsc.sz, nullptr);
            return retv::Ok;

        case REC_REQ(USB_REQ_RECPNT_DEVICE, USB_REQ_GET_CONFIGURATION): // Return the last selected configuration
            Ep0::PrepareSetupTransfer(&configuration, 1, nullptr);
            return retv::Ok;

        case REC_REQ(USB_REQ_RECPNT_DEVICE, USB_REQ_SET_CONFIGURATION):
            // f the USB device is already active, we need to perform the clear procedure on the current configuration
            if(UsbState == UsbSta::Active) {
                // Clear current configuration
                Sys::LockFromIRQ();
                DisableEndpointsI();
                Sys::UnlockFromIRQ();
                configuration = 0U;
                UsbState = UsbSta::Selected;
                Usb::EventCallback(Usb::Evt::Unconfigured);
            }
            if(Usb::SetupPkt.cfgNumber != 0) { // Set new configuration
                configuration = Usb::SetupPkt.cfgNumber;
                UsbState = UsbSta::Active;
                Usb::EventCallback(Usb::Evt::Configured);
            }
            Ep0::PrepareSetupTransfer(nullptr, 0, nullptr);
            return retv::Ok;

        case REC_REQ(USB_REQ_RECPNT_INTRFCE, USB_REQ_GET_STATUS):
        case REC_REQ(USB_REQ_RECPNT_EP,      USB_REQ_SYNCH_FRAME):
            // Just send two zero bytes
            Ep0::PrepareSetupTransfer((uint8_t*)zero_status, 2, nullptr);
            return retv::Ok;

        case REC_REQ(USB_REQ_RECPNT_EP, USB_REQ_GET_STATUS):
            if(Usb::SetupPkt.Ep.dir == 1) { // IN ep
                switch(GetStatusIn(Usb::SetupPkt.Ep.Number)) {
                    case EpSta::STALLED:
                        Ep0::PrepareSetupTransfer((uint8_t*)halted_status, 2, nullptr);
                        return retv::Ok;
                    case EpSta::ACTIVE:
                        Ep0::PrepareSetupTransfer((uint8_t*) active_status, 2, nullptr);
                        return retv::Ok;
                    case EpSta::DISABLED:
                    default: return retv::Fail;
                }
            }
            else { // OUT ep
                switch(GetStatusOut(Usb::SetupPkt.Ep.Number)) {
                    case EpSta::STALLED:
                        Ep0::PrepareSetupTransfer((uint8_t*)halted_status, 2, nullptr);
                        return retv::Ok;
                    case EpSta::ACTIVE:
                        Ep0::PrepareSetupTransfer((uint8_t*)active_status, 2, nullptr);
                        return retv::Ok;
                    case EpSta::DISABLED:
                    default: return retv::Fail;
                }
            }

        case REC_REQ(USB_REQ_RECPNT_EP, USB_REQ_CLEAR_FEATURE): // Only ENDPOINT_HALT is handled as feature
            if(Usb::SetupPkt.epFeatureSelector != USB_FEATURE_ENDPOINT_HALT) return retv::Fail;
            // Clear the EP status, not valid for EP0, it is ignored in that case
            if(Usb::SetupPkt.Ep.Number != 0U) {
                if(Usb::SetupPkt.Ep.dir == 1) ClearIn(Usb::SetupPkt.Ep.Number);
                else ClearOut(Usb::SetupPkt.Ep.Number);
            }
            Ep0::PrepareSetupTransfer(nullptr, 0, nullptr);
            return retv::Ok;

        case REC_REQ(USB_REQ_RECPNT_EP, USB_REQ_SET_FEATURE): // Only ENDPOINT_HALT is handled as feature
            if(Usb::SetupPkt.epFeatureSelector != USB_FEATURE_ENDPOINT_HALT) return retv::Fail;
            // Stall the EP, not valid for EP0, it is ignored in that case
            if(Usb::SetupPkt.Ep.Number != 0U) {
                if(Usb::SetupPkt.Ep.dir == 1) StallIn(Usb::SetupPkt.Ep.Number);
                else StallOut(Usb::SetupPkt.Ep.Number);
            }
            Ep0::PrepareSetupTransfer(nullptr, 0, nullptr);
            return retv::Ok;

        default: return retv::Fail;
    } // switch
}

// Reset core and delay of at least 3 PHY cycles
static void ResetCore() {
    USB->GRSTCTL |= GRSTCTL_CSRST;
    DelayLoop(12);
    while(USB->GRSTCTL & GRSTCTL_CSRST);
    DelayLoop(18);
}

static void StartCore() {
    Sys::Lock();
    if(UsbState == UsbSta::Stop) {
        RCU->EnUSB(); // Enable USB periph clk
        RCU->ResUSB();
        Nvic::EnableVector(USB_IRQ_NUMBER, USB_IRQ_PRIO);
        // Forced device mode, USB turn-around time = TRDT_VALUE_FS, Full Speed 1.1 PHY
        USB->GUSBCS = GUSBCS_FDM | GUSBCS_UTT(UTT_VALUE_FS) | GUSBCS_EMBPHY; // EMBPHY marked as reserved, but who knows
        USB->DCFG = DCFG_DSPD_FS11; // 48MHz 1.1 PHY
        USB->PWRCLKCTL = 0; // En clocks
        ResetCore(); // Soft reset USB core
        // Consider VBUS voltage always valid, en VBUSA & B (required), pwron embd PHY
        USB->GCCFG = GCCFG_VBUSIG | GCCFG_VBUSACEN | GCCFG_VBUSBCEN | GCCFG_PWRON;
        USB->GCCFG |= GCCFG_SOFOEN; // Enable SOF output. May not be necessary, but who cares? But must be enabled for CTC.
        DelayLoop(20);
        USB->DisconnectBus(); DelayLoop(9);
        USB->GAHBCS = 0; // Interrupts on TXFIFOs half empty, global irqs dis
        EpDisableAll();
        // Clear all pending Device Interrupts, only the USB Reset interrupt is required initially
        USB->DIEPINTEN  = 0;
        USB->DOEPINTEN  = 0;
        USB->DAEPINTEN = 0;
#if USB_SOF_CB_EN
        USB->GINTEN = GINTEN_ENUMFIE | GINTEN_RSTIE | GINTEN_SPIE |
        GINTEN_ESPIE | GINTEN_SESIE | GINTEN_WKUPIE | GINTEN_ISOINCIE | GINTEN_ISOONCIE |
        GINTEN_SOFIE;
#else
        USB->GINTEN = GINTEN_ENUMFIE | GINTEN_RSTIE | GINTEN_SPIE |
            GINTEN_ESPIE | GINTEN_SESIE | GINTEN_WKUPIE | GINTEN_ISOINCIE | GINTEN_ISOONCIE;
#endif
        USB->GINTF  = 0xFFFFFFFF;     // Clear all pending IRQs, if any
        USB->GAHBCS |= GAHBCS_GINTEN; // Global interrupts enable
    }
    UsbState = UsbSta::Ready;
    Sys::Unlock();
}

static void StopCore() {
    Sys::Lock();
    if(UsbState != UsbSta::Stop) {
        EpDisableAll();
        USB->DAEPINTEN = 0;
        USB->GAHBCS   = 0;
        USB->GCCFG    = 0;
        Nvic::DisableVector(USB_IRQ_NUMBER);
        RCU->DisUSB();
    }
    UsbState = UsbSta::Stop;
    Ep0::Reset();
    Sys::RescheduleS();
    Sys::Unlock();
}
#endif

#if 1 // ============================ IRQ Handlers =============================
static void OnIrqReset() {
    UsbState = UsbSta::Ready;
    // Reset internal state
    EpTransmittingFlag = 0;
    EpReceivingFlag = 0;
    address = 0;
    configuration = 0;
    Ep0::Reset();
    TxFifo.Flush(0);
    // Clear and disable all ep irqs
    USB->DIEPFEINTEN = 0;
    USB->DAEPINTEN   = DAINTMSK_OEPM(0) | DAINTMSK_IEPM(0);

    // Set all endpoints in NAK mode, clear interrupts
    for(unsigned i = 0; i <= USB_NUM_EP_MAX; i++) {
        USB->ie[i].DIEPCTL = DIEPCTL_SNAK;
        USB->oe[i].DOEPCTL = DOEPCTL_SNAK;
        USB->ie[i].DIEPINT = 0xFFFFFFFF;
        USB->oe[i].DOEPINTF = 0xFFFFFFFF;
    }

    TxFifo.Reset(); // Reset the FIFO memory allocator

    // Init RX FIFO size, the address is always zero
    USB->GRFLEN = USB_RX_FIFO_SZ32;
    RxFifo.Flush();

    // Reset the device address to zero
    USB->DCFG = (USB->DCFG & ~DCFG_DAD_MASK) | DCFG_DAD(0);

    // Enable EP-related interrupt sources
    USB->GINTEN  |= GINTEN_RXFNEIE | GINTEN_OEPIE  | GINTEN_IEPIE;
    USB->DIEPINTEN  = DIEPMSK_TOCM   | DIEPMSK_XFRCM;
    USB->DOEPINTEN  = DOEPMSK_STUPM  | DOEPMSK_XFRCM;

    // EP0 initialization, it is a special case
    Ep0::State = Ep0::Sta::STP_WAITING; // EP0 state machine initialization
    USB->oe[0].DOEPTSIZ = DOEPTSIZ_STUPCNT(3);
    USB->oe[0].DOEPCTL = DOEPCTL_SD0PID | DOEPCTL_USBAEP | DOEPCTL_EPTYP_CTRL | DOEPCTL_MPSIZ(EP0_SZ);
    USB->ie[0].DIEPLEN = 0;
    USB->ie[0].DIEPCTL = DIEPCTL_SD0PID | DIEPCTL_USBAEP | DIEPCTL_EPTYP_CTRL |
                          DIEPCTL_TXFNUM(0) | DIEPCTL_MPSIZ(EP0_SZ);
    USB->DIEP0TFLEN = DIEPTFLEN_IEPTXFD(EP0_SZ / 4) | DIEPTFLEN_IEPTXRSAR(TxFifo.Allocate(EP0_SZ / 4));
    Usb::EventCallback(Usb::Evt::Reset);
}

static void OnIrqWakeup() {
    UsbState = SavedState; // State transition, returning to the previous state
    Usb::EventCallback(Usb::Evt::Wakeup);
}

static void OnIrqSuspend() {
    SavedState = UsbState;
    UsbState = UsbSta::Suspended;
    Usb::EventCallback(Usb::Evt::Suspend);
}

// Isochronous IN transfer failed handler
static void OnIrqIsoInFailed() {
    for(uint32_t ep=1; ep <= USB_NUM_EP_MAX; ep++) { // Exclude Ep0
        // Endpoint is ISO and is enabled -> ISOC IN transfer failed
        if(((USB->ie[ep].DIEPCTL & DIEPCTL_EPTYP_MASK) == DIEPCTL_EPTYP_ISO) and (USB->ie[ep].DIEPCTL & DIEPCTL_EPENA)) {
            // Disable endpoint
            USB->ie[ep].DIEPCTL |= (DIEPCTL_EPDIS | DIEPCTL_SNAK);
            while(USB->ie[ep].DIEPCTL & DIEPCTL_EPENA);
            TxFifo.Flush(ep); // Flush FIFO
            EpTransmittingFlag &= ~(1UL << ep);
            CallInTransferEndCallback(ep); // Prepare data for next frame
        }
    }
}

// Isochronous OUT transfer failed handler
static void OnIrqIsoOutFailed() {
    for(uint32_t ep=1; ep <= USB_NUM_EP_MAX; ep++) { // Exclude Ep0
        // Endpoint is ISO and is enabled -> ISOC OUT transfer failed
        if(((USB->oe[ep].DOEPCTL & DOEPCTL_EPTYP_MASK) == DOEPCTL_EPTYP_ISO) and (USB->oe[ep].DOEPCTL & DOEPCTL_EPENA)) {
            // Disable endpoint
            /* Core stucks here */
            /*otgp->oe[ep].DOEPCTL |= (DOEPCTL_EPDIS | DOEPCTL_SNAK);
             while (otgp->oe[ep].DOEPCTL & DOEPCTL_EPENA); */
            EpReceivingFlag &= ~(1UL << ep);
            CallOutTransferEndCallback(ep, OutEpState[ep].cnt); // Prepare transfer for next frame
        }
    }
}

// Incoming packets handler. USBFS sets this bit when there is at least one packet or status entry in the Rx FIFO
static void OnIrqRxFifoNotEmpty() {
    // Pop the event word
    uint32_t sts = USB->GRSTATP;
    // Event details
    uint32_t cnt = (sts & GRSTATP_BCNT_MASK) >> GRSTATP_BCNT_OFFSET;
    uint32_t ep  = (sts & GRSTATP_EPNUM_MASK) >> GRSTATP_EPNUM_OFFSET;
    EpState_t *osp;
    switch(sts & GRSTATP_PKTSTS_MASK) {
        case GRSTATP_SETUP_DATA:
            RxFifo.ReadToBuf(Ep0::SetupBuf, cnt, 8);
            break;
        case GRSTATP_SETUP_COMP:
            break;
        case GRSTATP_OUT_DATA:
            osp = &OutEpState[ep];
            RxFifo.ReadToBuf(&osp->pBuf[osp->cnt], cnt, osp->Sz - osp->cnt);
            osp->cnt += cnt;
            break;
        case GRSTATP_OUT_COMP:
            break;
        case GRSTATP_OUT_GLOBAL_NAK:
            break;
        default:
            break;
    }
}

static void OnIrqEpOut(uint32_t ep) {
    uint32_t epint = USB->oe[ep].DOEPINTF;
    USB->oe[ep].DOEPINTF = epint; // Clear all EP IRQ flags
    // Setup packets are handled using a specific callback
    if((ep == 0) and (epint & DOEPINT_STUP) and (USB->DOEPINTEN & DOEPMSK_STUPM)) Ep0::SetupCallback();
    // Transfer complete
    if((epint & DOEPINT_XFRC) && (USB->DOEPINTEN & DOEPMSK_XFRCM)) {
        if(ep == 0) { // EP0 requires special handling
#if USB_SEQUENCE_WORKAROUND
      /* If an OUT transaction end interrupt is processed while the state
         machine is not in an OUT state then it is ignored, this is caused
         on some devices (L4) apparently injecting spurious data complete
         words in the RX FIFO.*/
            if(!(Ep0::State == Ep0::Sta::OUT_RX or Ep0::State == Ep0::Sta::OUT_WAITING_STS)) return;
#endif
            Ep0::OutCallback();
        } // ep0
        else {
            EpReceivingFlag &= ~(1UL << ep);
            CallOutTransferEndCallback(ep, OutEpState[ep].cnt);
        }
    } // if XFRC
}

static void OnIrqEpIn(uint32_t ep) {
    uint32_t epint = USB->ie[ep].DIEPINT;
    USB->ie[ep].DIEPINT = epint; // Clear all EP IRQ flags
    // Timeouts not handled yet, not sure how to handle
    if(epint & DIEPINT_TOC) { }
    // Transfer complete
    if((epint & DIEPINT_XFRC) and (USB->DIEPINTEN & DIEPMSK_XFRCM)) {
        EpState_t *isp = &InEpState[ep];
        if(isp->Sz < isp->TotalSz) {
            /* If the transaction only covers part of the total transfer,
             * another transaction is immediately initiated to cover the remainder */
            isp->pBuf += isp->Sz;
            isp->Sz = isp->TotalSz - isp->Sz;
            isp->cnt = 0;
            Sys::LockFromIRQ();
            StartInTransfer(ep);
            Sys::UnlockFromIRQ();
        }
        else { // End on IN transfer
            EpTransmittingFlag &= ~(1UL << ep);
            if(ep == 0) Ep0::InCallback();
            else CallInTransferEndCallback(ep);
        }
    } // XFRC
    // TX FIFO empty or emptying => fill it
    if((epint & DIEPINT_TXFE) and (USB->DIEPFEINTEN & DIEPEMPMSK_INEPTXFEM(ep))) {
        TxFifo.Fill(ep);
    }
}
#endif

namespace Usb { // ===================== Namespace =============================
SetupPkt_t SetupPkt;

void Connect() {
    USB->DisconnectBus();
    Sys::SleepMilliseconds(99);
    StartCore();
    USB->ConnectBus();
}

void Disconnect() {
    StopCore();
    USB->DisconnectBus();
}

bool IsActive() { return UsbState == UsbSta::Active; }

void InitEp(uint32_t ep, const EpConfig_t *pCfg) {
    Sys::DbgAssert(ep > 0 and ep <= USB_NUM_EP_MAX, "Bad Ep indx");
    EpCfg[ep] = pCfg;

    uint32_t ctl = DIEPCTL_SD0PID | DIEPCTL_USBAEP;
    // IN and OUT common parameters
    switch(pCfg->Type) {
        case EpType::Ctrl:      ctl |= DIEPCTL_EPTYP_CTRL; break;
        case EpType::Iso:       ctl |= DIEPCTL_EPTYP_ISO;  break;
        case EpType::Bulk:      ctl |= DIEPCTL_EPTYP_BULK; break;
        case EpType::Interrupt: ctl |= DIEPCTL_EPTYP_INTR; break;
    }

    // OUT endpoint activation or deactivation
    USB->oe[ep].DOEPTSIZ = 0;
    if(pCfg->OutMaxPktSz != 0) { // OUT enabled
        USB->oe[ep].DOEPCTL = ctl | DOEPCTL_MPSIZ(pCfg->OutMaxPktSz);
        USB->DAEPINTEN |= DAINTMSK_OEPM(ep);
    }
    else { // Disable OUT functionality
        USB->oe[ep].DOEPCTL &= ~DOEPCTL_USBAEP;
        USB->DAEPINTEN &= ~DAINTMSK_OEPM(ep);
    }

    // IN endpoint activation or deactivation
    USB->ie[ep].DIEPLEN = 0;
    if(pCfg->InMaxPktSz != 0) {
        // Allocate FIFO
        uint32_t fsize = pCfg->InMaxPktSz / 4; // FIFO data is 32-bit wide
        fsize *= pCfg->InMultiplier;
        // Next is -1, as Ep0 has special register
        USB->DIEPTFLEN[ep-1] = DIEPTFLEN_IEPTXFD(fsize) | DIEPTFLEN_IEPTXRSAR(TxFifo.Allocate(fsize));
        TxFifo.Flush(ep);
        USB->ie[ep].DIEPCTL = ctl | DIEPCTL_TXFNUM(ep) | DIEPCTL_MPSIZ(pCfg->InMaxPktSz);
        USB->DAEPINTEN |= DAINTMSK_IEPM(ep);
    }
    else { // Disable IN functionality
        USB->DIEPTFLEN[ep-1] = 0x02000400; // Reset value
        TxFifo.Flush(ep);
        USB->ie[ep].DIEPCTL &= ~DIEPCTL_USBAEP;
        USB->DAEPINTEN &= ~DAINTMSK_IEPM(ep);
    }
}

void StartReceiveI(uint32_t ep, uint8_t *pBuf, uint32_t MaxSz) {
    Sys::DbgCheckClassI();
    // Set ep flag
    EpReceivingFlag |= (1UL << ep);
    // Setup transfer
    EpState_t *osp = &OutEpState[ep];
    osp->pBuf = pBuf;
    osp->Sz = MaxSz;
    osp->cnt = 0;
    StartOutTransfer(ep);
}

void StartTransmitI(uint32_t ep, uint8_t *pBuf, uint32_t Sz) {
    Sys::DbgCheckClassI();
    Sys::DbgAssert(!IsEpTransmitting(ep), "already transmitting");
    // Set ep flag
    EpTransmittingFlag |= (1UL << ep);
    // Setup transfer
    EpState_t *isp = &InEpState[ep];
    isp->pBuf = pBuf;
    isp->Sz = Sz;
    isp->cnt = 0;
    StartInTransfer(ep);
}

void StartTransmit(uint32_t ep, uint8_t *pBuf, uint32_t Sz) {
    Sys::Lock();
    StartTransmitI(ep, pBuf, Sz);
    Sys::Unlock();
}

bool IsEpTransmitting(uint32_t ep) { return EpTransmittingFlag & (1UL << ep); }
bool IsEpReceiving(uint32_t ep)    { return EpReceivingFlag    & (1UL << ep); }

} // namespace

#if 1 // ============================== IRQ ====================================
void ProcessIRQ() {
    uint32_t sts  = USB->GINTF;
    sts &= USB->GINTEN;
    USB->GINTF = sts;
    // === Process what happened ====
    if(sts & GINTF_RST) { // Reset
        OnIrqReset();
        return; // the core has been reset => do not process other flags
    }

    if(sts & GINTF_WKUPIF) { // Wake-up
        // If clocks are gated off, turn them back on (may be the case if coming out of suspend mode)
        if(USB->PWRCLKCTL & (PWRCLKCTL_SHCLK | PWRCLKCTL_SUCLK)) USB->PWRCLKCTL &= ~(PWRCLKCTL_SHCLK | PWRCLKCTL_SUCLK);
        // Clear the Remote Wake-up Signaling
        USB->DCTL &= ~DCTL_RWKUP;
        OnIrqWakeup();
    }

    // Suspend handling
    if(sts & GINTF_SP) OnIrqSuspend();

    // Enumeration done
    if(sts & GINTF_ENUMF) {
        // Full or High speed timing selection
        if((USB->DSTS & DSTS_ENUMSPD_MASK) == DSTS_ENUMSPD_HS_480)
            USB->GUSBCS = (USB->GUSBCS & ~GUSBCS_UTT_MASK) | GUSBCS_UTT(UTT_VALUE_HS);
        else
            USB->GUSBCS = (USB->GUSBCS & ~GUSBCS_UTT_MASK) | GUSBCS_UTT(UTT_VALUE_FS);
    }

#if USB_SOF_CB_EN // SOF interrupt handling
    if(sts & GINTF_SOF) Usb::SOFCallback();
#endif

    // Isochronous IN failed
    if(sts & GINTF_ISOINCIF) OnIrqIsoInFailed();
    // Isochronous OUT failed
    if(sts & GINTF_ISOONCIF) OnIrqIsoOutFailed();

    // Performing the whole FIFO emptying in the ISR, it is advised to keep this IRQ at a very low priority level
    if(sts & GINTF_RXFNEIF) OnIrqRxFifoNotEmpty();

    // IN/OUT endpoints event handling
    uint32_t src = USB->DAEPINT;
    if(sts & GINTF_OEPIF) {
        if(src & (1 << 16)) OnIrqEpOut(0);
        if(src & (1 << 17)) OnIrqEpOut(1);
        if(src & (1 << 18)) OnIrqEpOut(2);
        if(src & (1 << 19)) OnIrqEpOut(3);
#if USB_NUM_EP_MAX >= 4
        if(src & (1 << 20)) OnIrqEpOut(4);
#endif
#if USB_NUM_EP_MAX >= 5
        if(src & (1 << 21)) OnIrqEpOut(5);
#endif
#if USB_NUM_EP_MAX >= 6
        if(src & (1 << 22)) OnIrqEpOut(6);
#endif
#if USB_NUM_EP_MAX >= 7
        if(src & (1 << 23)) OnIrqEpOut(7);
#endif
#if USB_NUM_EP_MAX >= 8
        if(src & (1 << 24)) OnIrqEpOut(8);
#endif
    }
    if(sts & GINTF_IEPIF) {
        if(src & (1 << 0)) OnIrqEpIn(0);
        if(src & (1 << 1)) OnIrqEpIn(1);
        if(src & (1 << 2)) OnIrqEpIn(2);
        if(src & (1 << 3)) OnIrqEpIn(3);
#if USB_NUM_EP_MAX >= 4
        if(src & (1 << 4)) OnIrqEpIn(4);
#endif
#if USB_NUM_EP_MAX >= 5
        if(src & (1 << 5)) OnIrqEpIn(5);
#endif
#if USB_NUM_EP_MAX >= 6
        if(src & (1 << 6)) OnIrqEpIn(6);
#endif
#if USB_NUM_EP_MAX >= 7
        if(src & (1 << 7)) OnIrqEpIn(7);
#endif
#if USB_NUM_EP_MAX >= 8
        if(src & (1 << 8)) OnIrqEpIn(8);
#endif
    }
}

extern "C"
void USB_IRQ_HANDLER() {
    Sys::IrqPrologue();
    ProcessIRQ();
    Sys::IrqEpilogue();
}
#endif
