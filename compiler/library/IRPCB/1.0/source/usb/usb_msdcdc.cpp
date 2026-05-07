/*
 * usb_cdc.cpp
 *
 *  Created on: 03 ����. 2015 �.
 *      Author: Kreyl
 */

#include "descriptors_msdcdc.h"
#include "usb_msdcdc.h"
#include "board.h"
#include "gd_lib.h"
#include "mem_msd_glue.h"
#include "usb.h"
#include "scsi.h"
#include "MsgQ.h"
#include "EvtMsgIDs.h"

UsbMsdCdc usb_msd_cdc;

static uint8_t sbyte;

#define CDC_OUT_BUF_SZ  EP_CDC_BULK_SZ

#if 1 // ============ Mass Storage constants, types, variables =================
// Enum for the Mass Storage class specific control requests that can be issued by the USB bus host
enum MS_ClassRequests_t {
    // Mass Storage class-specific request to retrieve the total number of Logical Units (drives) in the SCSI device.
    MS_REQ_GetMaxLUN = 0xFE,
    // Mass Storage class-specific request to reset the Mass Storage interface, ready for the next command.
    MS_REQ_MassStorageReset = 0xFF,
};

#pragma pack(push, 1)
// Mass Storage Class Command Block Wrapper
struct MS_CommandBlockWrapper_t {
    uint32_t Signature;         // Command block signature, must be MS_CBW_SIGNATURE to indicate a valid Command Block
    uint32_t Tag;               // Unique command ID value, to associate a command block wrapper with its command status wrapper
    uint32_t DataTransferLen;   // Length of the optional data portion of the issued command, in bytes
    uint8_t  Flags;             // Command block flags, indicating command data direction
    uint8_t  LUN;               // Logical Unit number this command is issued to
    uint8_t  SCSICmdLen;        // Length of the issued SCSI command within the SCSI command data array
    uint8_t  SCSICmdData[16];   // Issued SCSI command in the Command Block
};
#define MS_CMD_SZ   sizeof(MS_CommandBlockWrapper_t)

// Mass Storage Class Command Status Wrapper
struct MS_CommandStatusWrapper_t {
    uint32_t Signature;          // Status block signature, must be \ref MS_CSW_SIGNATURE to indicate a valid Command Status
    uint32_t Tag;                // Unique command ID value, to associate a command block wrapper with its command status wrapper
    uint32_t DataTransferResidue;// Number of bytes of data not processed in the SCSI command
    uint8_t  Status;             // Status code of the issued command - a value from the MS_CommandStatusCodes_t enum
};
#pragma pack(pop)

void MSDStartReceiveHdrI();
void OnMSDDataOut(uint32_t Sz);
void OnMSDDataIn();
static bool isay_is_ready = true;
static Thread *PMsdThd = nullptr;
#endif

// CDC Reception buffers and methods
namespace CdcOutQ {
static enum class TransferAction { SendEvt, WakeThd } action = TransferAction::SendEvt;

static uint8_t buf1[CDC_OUT_BUF_SZ], buf2[CDC_OUT_BUF_SZ], *pbuf_w = buf1;
static Buf_t buf_to_parse;
static Thread *pwaiting_thd = nullptr;

// OUT transfer end callback
void OnTransferEnd(uint32_t Sz) {
    Sys::LockFromIRQ();
    // Save what received
    buf_to_parse.ptr = pbuf_w;
    buf_to_parse.sz = Sz;
    // Switch buffers and start new reception
    pbuf_w = (pbuf_w == buf1)? buf2 : buf1;
    Usb::StartReceiveI(EP_CDC_DATA, pbuf_w, CDC_OUT_BUF_SZ);
    // Take necessary action
    if(action == TransferAction::SendEvt) evt_q_main.SendNowOrExitI(EvtMsg(EvtId::UsbCdcDataRcvd));
    else Sys::WakeI(&pwaiting_thd, retv::Ok);
    Sys::UnlockFromIRQ();
}

} // namespace

// Transmission buffers: several buffers of EP_BULK_SZ
static BufQ_t<uint8_t, EP_CDC_BULK_SZ, USB_TXBUF_CNT> cdc_input_buf;

// IN transfer end callback
void CdcOnBulkInTransferEnd() {
    // Unlock the buffer just sent to allow it to be written again
    cdc_input_buf.UnlockBuf();
    // Start tx if buf is not empty
    if(Usb::IsActive() and !cdc_input_buf.IsEmpty()) {
        BufType_t<uint8_t> buf = cdc_input_buf.GetAndLockBuf();
        Sys::LockFromIRQ();
        Usb::StartTransmitI(EP_CDC_DATA, buf.ptr, buf.sz);
        Sys::UnlockFromIRQ();
    }
}

#if 1 // ====================== CDC Line Coding related ========================
#define CDC_SET_LINE_CODING         0x20U
#define CDC_GET_LINE_CODING         0x21U
#define CDC_SET_CONTROL_LINE_STATE  0x22U

#define LC_STOP_1                   0U
#define LC_STOP_1P5                 1U
#define LC_STOP_2                   2U

#define LC_PARITY_NONE              0U
#define LC_PARITY_ODD               1U
#define LC_PARITY_EVEN              2U
#define LC_PARITY_MARK              3U
#define LC_PARITY_SPACE             4U

// Line Coding
#define CDC_LINECODING_SZ   7UL
static union CDCLinecoding_t {
    struct {
        uint32_t dwDTERate = 115200;
        uint8_t bCharFormat = LC_STOP_1;
        uint8_t bParityType = LC_PARITY_NONE;
        uint8_t bDataBits = 8;
    };
    uint8_t Buf8[CDC_LINECODING_SZ];
} linecoding;
#endif

#if 1 // ====================== Endpoints config ===============================
// InMultiplier determines the space allocated for the TXFIFO as multiples of the packet size
// ==== EP1 ==== both IN and OUT
static const Usb::EpConfig_t CdcEpBulkCfg = {
        .OutTransferEndCallback = CdcOutQ::OnTransferEnd,
        .InTransferEndCallback = CdcOnBulkInTransferEnd,
        .Type = Usb::EpType::Bulk,
        .OutMaxPktSz = EP_CDC_BULK_SZ,
        .InMaxPktSz = EP_CDC_BULK_SZ,
        .InMultiplier = 2
};

// ==== EP2 ==== Interrupt, IN only. Actually not used.
static const Usb::EpConfig_t CdcEpInterruptCfg = {
        .OutTransferEndCallback = nullptr,
        .InTransferEndCallback = nullptr,
        .Type = Usb::EpType::Interrupt,
        .OutMaxPktSz = 0, // IN only
        .InMaxPktSz = EP_INTERRUPT_SZ,
        .InMultiplier = 1
};

// ==== EP3 ==== both IN and OUT
static const Usb::EpConfig_t MsdEpCfg = {
        .OutTransferEndCallback = OnMSDDataOut,
        .InTransferEndCallback  = OnMSDDataIn,
        .Type = Usb::EpType::Bulk,
        .OutMaxPktSz = EP_MSD_BULK_SZ,
        .InMaxPktSz = EP_MSD_BULK_SZ,
        .InMultiplier = 2
};
#endif

#if 1 // ============================ Events ===================================
// Setup request callback: process class-related requests
Usb::StpReqCbRpl_t Usb::SetupReqHookCallback() {
    Usb::StpReqCbRpl_t reply; // NotFound by default
    if(Usb::SetupPkt.Type == USB_REQTYPE_CLASS) {
        // === CDC handler ===
        if(Usb::SetupPkt.wIndex == 1) {
            switch(Usb::SetupPkt.bRequest) {
                case CDC_GET_LINE_CODING: // Send linecoding
                    reply.Retval = retv::Ok;
                    reply.Buf = linecoding.Buf8;
                    reply.Sz = CDC_LINECODING_SZ;
                    break;
                case CDC_SET_LINE_CODING: // Receive data into linecoding
                    reply.Retval = retv::Ok;
                    reply.Buf= linecoding.Buf8;
                    reply.Sz = CDC_LINECODING_SZ;
                    break;
                case CDC_SET_CONTROL_LINE_STATE: // Nothing to do, there are no control lines
                    reply.Retval = retv::Ok;
                    reply.Sz = 0; // Receive nothing
                    break;
                default: break;
            } // switch
        } // if windex == 1

        // === MSD handler ===
        else {
            // GetMaxLun
            if(Usb::SetupPkt.Direction == USB_REQDIR_DEV2HOST and
                    Usb::SetupPkt.bRequest == MS_REQ_GetMaxLUN and
                    Usb::SetupPkt.wLength == 1) {
//                PrintfI("MS_REQ_GetMaxLUN\r");
                sbyte = 0;  // Maximum LUN ID
                reply.Retval = retv::Ok;
                reply.Buf = &sbyte;
                reply.Sz = 1;
            }
            // Reset
            else if(Usb::SetupPkt.Direction == USB_REQDIR_HOST2DEV and
                    Usb::SetupPkt.bRequest == MS_REQ_MassStorageReset and
                    Usb::SetupPkt.wLength == 0) {
//                PrintfI("MS_REQ_MassStorageReset\r");
                // TODO: remove Stall condition
                reply.Retval = retv::Ok; // Acknowledge reception
            }
        } // if MSD
    } // if class
    return reply;
}

// this callback is invoked from an ISR so I-Class functions must be used
void Usb::EventCallback(Usb::Evt event) {
    switch(event) {
        case Usb::Evt::Reset:
            return;
        case Usb::Evt::Address:
            return;
        case Usb::Evt::Configured:
            Sys::LockFromIRQ();
            // ==== CDC ====
            Usb::InitEp(EP_CDC_DATA,   &CdcEpBulkCfg);
            Usb::InitEp(EP_CDC_INTERRUPT, &CdcEpInterruptCfg);
            // Reset queues
            CdcOutQ::pbuf_w = (CdcOutQ::pbuf_w == CdcOutQ::buf1)? CdcOutQ::buf2 : CdcOutQ::buf1;
            // Start reception. In this case, transaction size is limited to EP max size
            Usb::StartReceiveI(EP_CDC_DATA, CdcOutQ::pbuf_w, CDC_OUT_BUF_SZ);
            // ==== MSD ====
            Usb::InitEp(EP_MSD_DATA,   &MsdEpCfg);
            MSDStartReceiveHdrI();
            isay_is_ready = true;
            // ==== Sys ====
            evt_q_main.SendNowOrExitI(EvtMsg(EvtId::UsbReady)); // Inform main thread
            Sys::UnlockFromIRQ();
            return;
        case Usb::Evt::Suspend:
        case Usb::Evt::Wakeup:
        case Usb::Evt::Stalled:
        case Usb::Evt::Unconfigured:
            return;
    } // switch
}
#endif // Events

#if 1 // =========================== CDC =======================================
retv UsbMsdCdc::IPutChar(char c) {
    if(!Usb::IsActive()) return retv::Disconnected;
    Sys::Lock();
    retv r = cdc_input_buf.Put(c);
    if(cdc_input_buf.IsFullBufPresent() and !Usb::IsEpTransmitting(EP_CDC_DATA)) { // New buffer is full
        BufType_t<uint8_t> buf = cdc_input_buf.GetAndLockBuf();
        Usb::StartTransmitI(EP_CDC_DATA, buf.ptr, buf.sz);
    }
    Sys::Unlock();
    return r;
}

void UsbMsdCdc::IStartTransmissionIfNotYet() {
    // Start tx if it has not already started and if buf is not empty.
    if(Usb::IsActive() and !Usb::IsEpTransmitting(EP_CDC_DATA) and !cdc_input_buf.IsEmpty()) {
        Sys::Lock();
        BufType_t<uint8_t> buf = cdc_input_buf.GetAndLockBuf();
        Usb::StartTransmitI(EP_CDC_DATA, buf.ptr, buf.sz);
        Sys::Unlock();
    }
}

retv UsbMsdCdc::TryParseRxBuff() {
    while(CdcOutQ::buf_to_parse.sz) {
        CdcOutQ::buf_to_parse.sz--;
        if(cmd.PutChar(*CdcOutQ::buf_to_parse.ptr++) == pdrNewCmd) return retv::Ok;
    }
    return retv::Fail;
}

// Send '>' and receive what follows
retv UsbMsdCdc::ReceiveBinaryToBuf(uint8_t *ptr, uint32_t Len, uint32_t Timeout_ms) {
    CdcOutQ::action = CdcOutQ::TransferAction::WakeThd; // Do not send evt to main q on buf reception
    if(IPutChar('>') != retv::Ok) return retv::Fail;
    IStartTransmissionIfNotYet();
    // Wait for data to be received
    Sys::Lock();
    systime_t Start = Sys::GetSysTimeX();
    systime_t TimeLeft, Timeout_st = TIME_MS2I(Timeout_ms);
    while(Len != 0) {
        // Calculate time left to wait
        systime_t Elapsed = Sys::TimeElapsedSince(Start);
        if(Elapsed > Timeout_st) break;
        TimeLeft = Timeout_st - Elapsed;
        // Wait data
        CdcOutQ::pwaiting_thd = Sys::GetSelfThd();
        if(Sys::SleepS(TimeLeft) == retv::Timeout) break; // Timeout occured
        // Will be here after successful reception; put data to buffer
        if(CdcOutQ::buf_to_parse.sz > Len) CdcOutQ::buf_to_parse.sz = Len; // Flush too large data
        memcpy(ptr, CdcOutQ::buf_to_parse.ptr, CdcOutQ::buf_to_parse.sz);
        Len -= CdcOutQ::buf_to_parse.sz;
        ptr += CdcOutQ::buf_to_parse.sz;
    }
    CdcOutQ::action = CdcOutQ::TransferAction::SendEvt; // Return to normal life
    Sys::Unlock();
    return (Len == 0)? retv::Ok : retv::Fail; // Check if everything was received
}

// Wait '>' and then transmit buffer
retv UsbMsdCdc::TransmitBinaryFromBuf(uint8_t *ptr, uint32_t Len, uint32_t Timeout_ms) {
    if(Usb::IsEpTransmitting(EP_CDC_DATA)) return retv::Busy;
    retv r = retv::Timeout;
    Sys::Lock();
    CdcOutQ::action = CdcOutQ::TransferAction::WakeThd; // Do not send evt to main q on buf reception
    systime_t Start = Sys::GetSysTimeX();
    systime_t TimeLeft, Timeout_st = TIME_MS2I(Timeout_ms);
    // Wait '>'
    while(r == retv::Timeout) {
        // Calculate time left to wait
        systime_t Elapsed = Sys::TimeElapsedSince(Start);
        if(Elapsed > Timeout_st) break;
        TimeLeft = Timeout_st - Elapsed;
        // Wait data
        CdcOutQ::pwaiting_thd = Sys::GetSelfThd();
        if(Sys::SleepS(TimeLeft) == retv::Timeout) break; // Timeout occured
        // Will be here after successful reception; check if '>' present
        for(uint32_t i=0; i<CdcOutQ::buf_to_parse.sz; i++) {
            if(CdcOutQ::buf_to_parse.ptr[i] == '>') {
                // Found
                r = retv::Ok;
                break;
            }
        } // for
    }
    // Will be here after either timeout or successful '>' reception
    CdcOutQ::action = CdcOutQ::TransferAction::SendEvt; // Return to normal life
    if(r == retv::Ok) { // Transmit data
        if(Usb::IsActive()) Usb::StartTransmitI(EP_CDC_DATA, ptr, Len);
        else r = retv::Disconnected;
    }
    Sys::Unlock();
    return r;
}
#endif

#if 1 // ============================= MSD =====================================
//#define DBG_PRINT_CMD   TRUE

static MS_CommandBlockWrapper_t cmd_block;
static MS_CommandStatusWrapper_t cmd_status;
static SCSI_RequestSenseResponse_t SenseData;
static SCSI_ReadCapacity10Response_t ReadCapacity10Response;
static SCSI_ReadFormatCapacitiesResponse_t ReadFormatCapacitiesResponse;
static uint32_t Buf32[(MSD_DATABUF_SZ/4)];

static void SCSICmdHandler();
// Scsi commands
static void CmdTestUnitReady();
static retv CmdStartStopUnit();
static retv CmdInquiry();
static retv CmdRequestSense();
static retv CmdReadCapacity10();
static retv CmdSendDiagnostic();
static retv CmdReadFormatCapacities();
static retv CmdRead10();
static retv CmdWrite10();
static retv CmdModeSense6();

// ==== Thread ====
static THD_WORKSPACE(waMsdThd, 256);
__attribute__((noreturn)) static void MsdThd() {
    while(true) {
        Sys::Lock();
        PMsdThd = Sys::GetSelfThd();
        retv r = Sys::SleepS(TIME_INFINITE); // Wait forever until new data is received
        Sys::Unlock();
        if(r == retv::Ok) SCSICmdHandler(); // New header received
        Sys::Lock();
        MSDStartReceiveHdrI();
        Sys::Unlock();
    }
}

// Receive header
void MSDStartReceiveHdrI() {
    Usb::StartReceiveI(EP_MSD_DATA, (uint8_t*)&cmd_block, MS_CMD_SZ);
}

void OnMSDDataOut(uint32_t Sz) {
    Sys::LockFromIRQ();
    if(PMsdThd and PMsdThd->state == ThdState::Sleeping) Sys::WakeI(&PMsdThd, retv::Ok);
    Sys::UnlockFromIRQ();
}

void OnMSDDataIn() {
    Sys::LockFromIRQ();
    if(PMsdThd and PMsdThd->state == ThdState::Sleeping) Sys::WakeI(&PMsdThd, retv::Ok);
    Sys::UnlockFromIRQ();
}

void TransmitBuf(uint32_t *ptr, uint32_t Len) {
    Sys::Lock();
    PMsdThd = Sys::GetSelfThd();
    Usb::StartTransmitI(EP_MSD_DATA, (uint8_t*)ptr, Len);
    Sys::SleepS(TIME_INFINITE); // Wait forever until data is transmitted
    Sys::Unlock();
}

retv ReceiveToBuf(uint32_t *ptr, uint32_t Len) {
    Sys::Lock();
    PMsdThd = Sys::GetSelfThd();
    Usb::StartReceiveI(EP_MSD_DATA, (uint8_t*)ptr, Len);
    retv r = Sys::SleepS(TIME_INFINITE); // Wait forever until data is received
    Sys::Unlock();
    return r;
}
#endif

void UsbMsdCdc::Init() {
    // Variables
    SenseData.ResponseCode = 0x70;
    SenseData.AddSenseLen = 0x0A;
    // MSD Thread
    Sys::CreateThd(waMsdThd, sizeof(waMsdThd), NORMALPRIO, MsdThd);
}

void UsbMsdCdc::Reset() {
    // Wake thread if sleeping
    Sys::Lock();
    if(PMsdThd and PMsdThd->state == ThdState::Sleeping) Sys::WakeI(&PMsdThd, retv::Reset);
    Sys::Unlock();
}

void UsbMsdCdc::Connect()    { Usb::Connect(); }
void UsbMsdCdc::Disconnect() { Usb::Disconnect(); }
bool UsbMsdCdc::IsActive()   { return Usb::IsActive(); }

#if 1 // =========================== SCSI ======================================
//#define DBG_PRINT_CMD   TRUE
void SCSICmdHandler() {
//    Printf("Sgn=%X; Tag=%X; Len=%u; Flags=%X; LUN=%u; SLen=%u; SCmd=%A\r", cmd_block.Signature, cmd_block.Tag, cmd_block.DataTransferLen, cmd_block.Flags, cmd_block.LUN, cmd_block.SCSICmdLen, cmd_block.SCSICmdData, cmd_block.SCSICmdLen, ' ');
//    Printf("SCmd=%A\r", cmd_block.SCSICmdData, cmd_block.SCSICmdLen, ' ');
    retv cmd_rslt = retv::Fail;
    switch(cmd_block.SCSICmdData[0]) {
        case 0x00: CmdTestUnitReady(); return; // Will report inside
        case 0x03: cmd_rslt = CmdRequestSense(); break;
        case 0x12: cmd_rslt = CmdInquiry(); break;
        case 0x1A: cmd_rslt = CmdModeSense6(); break;
        case 0x1B: cmd_rslt = CmdStartStopUnit(); break;
        case 0x1D: cmd_rslt = CmdSendDiagnostic(); break;
        case 0x23: cmd_rslt = CmdReadFormatCapacities(); break;
        case 0x25: cmd_rslt = CmdReadCapacity10(); break;
        case 0x28: cmd_rslt = CmdRead10(); break;
        case 0x2A: cmd_rslt = CmdWrite10(); break;
        // These commands should just succeed, no handling required
        case 0x1E: // Prevent/Allow Medium Removal
        case 0x2F: // Verify10
        case 0x35: // SynchronizeCache10
            cmd_rslt = retv::Ok;
            cmd_block.DataTransferLen = 0;
            break;
        // Not implemented
        case 0x15: // ModeSelect6
        case 0x55: // ModeSelect10
        case 0x5A: // ModeSense10
        case 0xA0: // ReportLUNs
        default:
            Printf("MSCmd %X not supported\r", cmd_block.SCSICmdData[0]);
            // Update the SENSE key to reflect the invalid command
            SenseData.SenseKey = SCSI_SENSE_KEY_ILLEGAL_REQUEST;
            SenseData.AdditionalSenseCode = SCSI_ASENSE_INVALID_COMMAND;
            SenseData.AdditionalSenseQualifier = SCSI_ASENSEQ_NO_QUALIFIER;
            break;
    } // switch
    // Update Sense if command was successfully processed
    if(cmd_rslt == retv::Ok) {
        SenseData.SenseKey = SCSI_SENSE_KEY_GOOD;
        SenseData.AdditionalSenseCode = SCSI_ASENSE_NO_ADDITIONAL_INFORMATION;
        SenseData.AdditionalSenseQualifier = SCSI_ASENSEQ_NO_QUALIFIER;
    }

    // Send status
    cmd_status.Signature = MS_CSW_SIGNATURE;
    cmd_status.Tag = cmd_block.Tag;
    if(cmd_rslt == retv::Ok) {
        cmd_status.Status = SCSI_STATUS_OK;
        cmd_status.DataTransferResidue = cmd_block.DataTransferLen; // DataTransferLen decreased by cmd handler
    }
    else {
        cmd_status.Status = SCSI_STATUS_CHECK_CONDITION;
        cmd_status.DataTransferResidue = 0;    // 0 or requested length?
    }
    TransmitBuf((uint32_t*)&cmd_status, sizeof(MS_CommandStatusWrapper_t));
}

void CmdTestUnitReady() {
#if DBG_PRINT_CMD
    Printf("CmdTestReady (Rdy: %u)\r", isay_is_ready);
#endif
    cmd_block.DataTransferLen = 0;
    cmd_status.Signature = MS_CSW_SIGNATURE;
    cmd_status.Tag = cmd_block.Tag;
    cmd_status.DataTransferResidue = cmd_block.DataTransferLen;
    if(isay_is_ready) {
        cmd_status.Status = SCSI_STATUS_OK;
        SenseData.SenseKey = SCSI_SENSE_KEY_GOOD;
        SenseData.AdditionalSenseCode = SCSI_ASENSE_NO_ADDITIONAL_INFORMATION;
        SenseData.AdditionalSenseQualifier = SCSI_ASENSEQ_NO_QUALIFIER;
    }
    else {
        cmd_status.Status = SCSI_STATUS_CHECK_CONDITION;
        SenseData.SenseKey = SCSI_SENSE_KEY_NOT_READY;
        SenseData.AdditionalSenseCode = SCSI_ASENSE_MEDIUM_NOT_PRESENT;
        SenseData.AdditionalSenseQualifier = SCSI_ASENSEQ_NO_QUALIFIER;
    }
    TransmitBuf((uint32_t*)&cmd_status, sizeof(MS_CommandStatusWrapper_t));
}

retv CmdStartStopUnit() {
#if DBG_PRINT_CMD
    Printf("CmdStartStopUnit [4]=%02X\r", cmd_block.SCSICmdData[4]);
#endif
    if((cmd_block.SCSICmdData[4] & 0x03) == 0x02) {  // Eject
        isay_is_ready = false;
    }
    else if((cmd_block.SCSICmdData[4] & 0x03) == 0x03) {  // Load
        isay_is_ready = true;
    }
    return retv::Ok;
}

retv CmdInquiry() {
#if DBG_PRINT_CMD
    Printf("CmdInquiry %u\r", cmd_block.SCSICmdData[1] & 0x01);
#endif
    uint16_t requested_length =  Convert::BuildU16(cmd_block.SCSICmdData[4], cmd_block.SCSICmdData[3]);
    uint16_t bytes_to_transfer;
    if(cmd_block.SCSICmdData[1] & 0x01) { // Evpd is set
        bytes_to_transfer = MIN_(requested_length, PAGE0_INQUIRY_DATA_SZ);
        TransmitBuf((uint32_t*)&Page00InquiryData, bytes_to_transfer);
    }
    else {
        // Transmit InquiryData
        bytes_to_transfer = MIN_(requested_length, sizeof(SCSI_InquiryResponse_t));
        TransmitBuf((uint32_t*)&InquiryData, bytes_to_transfer);
    }
    // Succeed the command and update the bytes transferred counter
    cmd_block.DataTransferLen -= bytes_to_transfer;
    return retv::Ok;
}

retv CmdRequestSense() {
#if DBG_PRINT_CMD
    Printf("CmdRequestSense\r");
#endif
    uint16_t requested_length = cmd_block.SCSICmdData[4];
    uint16_t bytes_to_transfer = MIN_(requested_length, sizeof(SenseData));
    // Transmit SenceData
    TransmitBuf((uint32_t*)&SenseData, bytes_to_transfer);
    // Succeed the command and update the bytes transferred counter
    cmd_block.DataTransferLen -= bytes_to_transfer;
    return retv::Ok;
}

retv CmdReadCapacity10() {
#if DBG_PRINT_CMD
    Printf("CmdReadCapacity10\r");
#endif
    ReadCapacity10Response.LastBlockAddr = __REV(MsdMem::block_cnt - 1);
    ReadCapacity10Response.BlockSize = __REV(MsdMem::block_sz);
    // Transmit SenceData
    TransmitBuf((uint32_t*)&ReadCapacity10Response, sizeof(ReadCapacity10Response));
    // Succeed the command and update the bytes transferred counter
    cmd_block.DataTransferLen -= sizeof(ReadCapacity10Response);
    return retv::Ok;
}

retv CmdSendDiagnostic() {
    Printf("CmdSendDiagnostic\r");
    return retv::CmdUnknown;
}

retv CmdReadFormatCapacities() {
#if DBG_PRINT_CMD
    Printf("CmdReadFormatCapacities\r");
#endif
    ReadFormatCapacitiesResponse.Length = 0x08;
    ReadFormatCapacitiesResponse.NumberOfBlocks = __REV(MsdMem::block_cnt);
    // 01b Unformatted Media - Maximum formattable capacity for this cartridge
    // 10b Formatted Media - Current media capacity
    // 11b No Cartridge in Drive - Maximum formattable capacity
    ReadFormatCapacitiesResponse.DescCode = 0x02;
    ReadFormatCapacitiesResponse.BlockSize[0] = (uint8_t)(MsdMem::block_sz >> 16);
    ReadFormatCapacitiesResponse.BlockSize[1] = (uint8_t)(MsdMem::block_sz >> 8);
    ReadFormatCapacitiesResponse.BlockSize[2] = (uint8_t)(MsdMem::block_sz);
    // Transmit Data
    TransmitBuf((uint32_t*)&ReadFormatCapacitiesResponse, sizeof(ReadFormatCapacitiesResponse));
    // Succeed the command and update the bytes transferred counter
    cmd_block.DataTransferLen -= sizeof(ReadFormatCapacitiesResponse);
    return retv::Ok;
}

struct AddrLen { uint32_t addr, len; };
using RetvValAL = RetvVal<AddrLen>;

RetvValAL PrepareAddrAndLen() {
    RetvValAL r;
    r->addr = Convert::BuildU132(cmd_block.SCSICmdData[5], cmd_block.SCSICmdData[4], cmd_block.SCSICmdData[3], cmd_block.SCSICmdData[2]);
    r->len  = Convert::BuildU16(cmd_block.SCSICmdData[8], cmd_block.SCSICmdData[7]);
    // Check block addr
    if((r->addr + r->len) > MsdMem::block_cnt) {
        Printf("Out Of Range: addr %u, len %u\r", r->addr, r->len);
        SenseData.SenseKey = SCSI_SENSE_KEY_ILLEGAL_REQUEST;
        SenseData.AdditionalSenseCode = SCSI_ASENSE_LOGICAL_BLOCK_ADDRESS_OUT_OF_RANGE;
        SenseData.AdditionalSenseQualifier = SCSI_ASENSEQ_NO_QUALIFIER;
        r.rslt = retv::Fail;
    }
    // Check cases 4, 5: (Hi != Dn); and 3, 11, 13: (Hn, Ho != Do)
    if(cmd_block.DataTransferLen != r->len * MsdMem::block_sz) {
        Printf("Wrong length\r");
        SenseData.SenseKey = SCSI_SENSE_KEY_ILLEGAL_REQUEST;
        SenseData.AdditionalSenseCode = SCSI_ASENSE_INVALID_COMMAND;
        SenseData.AdditionalSenseQualifier = SCSI_ASENSEQ_NO_QUALIFIER;
        r.rslt = retv::Fail;
    }
    return r; // Ok by default
}

retv CmdRead10() {
#if DBG_PRINT_CMD
    Printf("CmdRead10\r");
#endif
    RetvValAL al = PrepareAddrAndLen();
    if(al.NotOk()) return retv::Fail;
    // Send data
    uint32_t BlocksToRead, BytesToSend; // Intermediate values
    while(al->len != 0) {
        BlocksToRead = MIN_(MSD_DATABUF_SZ / MsdMem::block_sz, al->len);
        BytesToSend = BlocksToRead * MsdMem::block_sz;
        if(MsdMem::Read(al->addr, (uint8_t*)Buf32, BlocksToRead) == retv::Ok) {
            TransmitBuf(Buf32, BytesToSend);
            cmd_block.DataTransferLen -= BytesToSend;
            al->len  -= BlocksToRead;
            al->addr += BlocksToRead;
        }
        else {
            Printf("Rd fail\r");
            // TODO: handle read error
            return retv::Fail;
        }
    } // while
    return retv::Ok;
}

retv CmdWrite10() {
#if DBG_PRINT_CMD
    Printf("CmdWrite10\r");
#endif
#if MSD_READ_ONLY
    SenseData.SenseKey = SCSI_SENSE_KEY_DATA_PROTECT;
    SenseData.AdditionalSenseCode = SCSI_ASENSE_WRITE_PROTECTED;
    SenseData.AdditionalSenseQualifier = SCSI_ASENSEQ_NO_QUALIFIER;
    return retvFail;
#else
    // Check case 8: Hi != Do
    if(cmd_block.Flags & 0x80) {
        SenseData.SenseKey = SCSI_SENSE_KEY_ILLEGAL_REQUEST;
        SenseData.AdditionalSenseCode = SCSI_ASENSE_INVALID_COMMAND;
        return retv::Fail;
    }
    // TODO: Check if ready
    if(false) {
        SenseData.SenseKey = SCSI_SENSE_KEY_NOT_READY;
        SenseData.AdditionalSenseCode = SCSI_ASENSE_MEDIUM_NOT_PRESENT;
        return retv::Fail;
    }
    // Get transaction size
    RetvValAL al = PrepareAddrAndLen();
    if(al.NotOk()) return retv::Fail;
//    Printf("Addr=%u; Len=%u\r", BlockAddress, TotalBlocks);
    uint32_t BlocksToWrite, BytesToReceive;
    retv Rslt = retv::Ok;

    while(al->len != 0) {
        // Fill Buf
        BytesToReceive = MIN_(MSD_DATABUF_SZ, al->len * MsdMem::block_sz);
        BlocksToWrite  = BytesToReceive / MsdMem::block_sz;
        if(ReceiveToBuf(Buf32, BytesToReceive) != retv::Ok) {
            Printf("Rcv fail\r");
            return retv::Fail;
        }
        // Write Buf to memory
        Rslt = MsdMem::Write(al->addr, (uint8_t*)Buf32, BlocksToWrite);
        if(Rslt != retv::Ok) return retv::Fail;
        cmd_block.DataTransferLen -= BytesToReceive;
        al->len -= BlocksToWrite;
        al->addr += BlocksToWrite;
    } // while
    return retv::Ok;
#endif
}

retv CmdModeSense6() {
#if DBG_PRINT_CMD
    Printf("CmdModeSense6\r");
#endif
    uint16_t requested_length = cmd_block.SCSICmdData[4];
    uint16_t bytes_to_transfer = MIN_(requested_length, MODE_SENSE6_DATA_SZ);
    TransmitBuf((uint32_t*)&Mode_Sense6_data, bytes_to_transfer);
    // Succeed the command and update the bytes transferred counter
    cmd_block.DataTransferLen -= bytes_to_transfer;
    return retv::Ok;
}
#endif
