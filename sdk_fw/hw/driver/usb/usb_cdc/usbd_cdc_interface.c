/*
 * Copyright (c) 2015 - 2016, Freescale Semiconductor, Inc.
 * Copyright 2016 - 2017 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */


#include "bsp.h"

#include "usb_device_config.h"
#include "usb.h"
#include "usb_device.h"
#include "usb_device_class.h"
#include "usb_device_cdc_acm.h"
#include "usb_device_ch9.h"
#include "usb_device_descriptor.h"

#include "fsl_device_registers.h"
#include "usb_phy.h"

#include "usbd_cdc_interface.h"
#include "usb_composite.h"

#include "reset.h"


#define APP_RX_DATA_SIZE  2048
#define APP_TX_DATA_SIZE  2048




/*******************************************************************************
* Definitions
******************************************************************************/

/*******************************************************************************
 * Prototypes
 ******************************************************************************/
void BOARD_InitHardware(void);
void USB_DeviceClockInit(void);
void USB_DeviceIsrEnable(void);
#if USB_DEVICE_CONFIG_USE_TASK
void USB_DeviceTaskFn(void *deviceHandle);
#endif


uint32_t CDC_Itf_GetBaud(void);
uint32_t CDC_Itf_TxAvailable( void );
uint32_t CDC_Itf_RxAvailable( void );
int32_t  CDC_Itf_Write( uint8_t *p_buf, uint32_t length );
uint8_t  CDC_Itf_Getch( void );
uint8_t  CDC_Itf_Read( void );
uint32_t CDC_Itf_TxBufLengh( void );
uint8_t  CDC_Itf_TxRead( void );
bool CDC_Itf_IsConnected(void);
void CDC_Itf_Flush( void );
void CDC_Itf_TxISR(void);



void BOARD_DbgConsole_Deinit(void);
void BOARD_DbgConsole_Init(void);
usb_status_t USB_DeviceCdcVcomCallback(class_handle_t handle, uint32_t event, void *param);
usb_status_t USB_DeviceCallback(usb_device_handle handle, uint32_t event, void *param);

/*******************************************************************************
* Variables
******************************************************************************/
extern usb_device_endpoint_struct_t g_cdcVcomDicEndpoints[];
extern usb_device_class_struct_t g_UsbDeviceCdcVcomConfig;
/* Data structure of virtual com device */

volatile static usb_device_composite_struct_t *g_deviceComposite;




/* Line coding of cdc device */
USB_DMA_INIT_DATA_ALIGN(USB_DATA_ALIGN_SIZE) static uint8_t s_lineCoding[LINE_CODING_SIZE] =
{
    /* E.g. 0x00,0xC2,0x01,0x00 : 0x0001C200 is 115200 bits per second */
    (LINE_CODING_DTERATE >> 0U) & 0x000000FFU,
    (LINE_CODING_DTERATE >> 8U) & 0x000000FFU,
    (LINE_CODING_DTERATE >> 16U) & 0x000000FFU,
    (LINE_CODING_DTERATE >> 24U) & 0x000000FFU,
    LINE_CODING_CHARFORMAT,
    LINE_CODING_PARITYTYPE,
    LINE_CODING_DATABITS
};


/* Abstract state of cdc device */
USB_DMA_INIT_DATA_ALIGN(USB_DATA_ALIGN_SIZE) static uint8_t s_abstractState[COMM_FEATURE_DATA_SIZE] = {(STATUS_ABSTRACT_STATE >> 0U) & 0x00FFU,
                                                          (STATUS_ABSTRACT_STATE >> 8U) & 0x00FFU};

/* Country code of cdc device */
USB_DMA_INIT_DATA_ALIGN(USB_DATA_ALIGN_SIZE) static uint8_t s_countryCode[COMM_FEATURE_DATA_SIZE] = {(COUNTRY_SETTING >> 0U) & 0x00FFU,
                                                        (COUNTRY_SETTING >> 8U) & 0x00FFU};

/* CDC ACM information */
USB_DMA_NONINIT_DATA_ALIGN(USB_DATA_ALIGN_SIZE) static usb_cdc_acm_info_t s_usbCdcAcmInfo;
/* Data buffer for receiving and sending*/
USB_DMA_NONINIT_DATA_ALIGN(USB_DATA_ALIGN_SIZE) static uint8_t s_currRecvBuf[APP_RX_DATA_SIZE];
USB_DMA_NONINIT_DATA_ALIGN(USB_DATA_ALIGN_SIZE) static uint8_t s_currSendBuf[APP_TX_DATA_SIZE];
volatile static uint32_t s_recvSize = 0;
volatile static uint32_t s_sendSize = 0;



#if defined(FSL_FEATURE_USB_KHCI_KEEP_ALIVE_ENABLED) && (FSL_FEATURE_USB_KHCI_KEEP_ALIVE_ENABLED > 0U) && \
    defined(USB_DEVICE_CONFIG_KEEP_ALIVE_MODE) && (USB_DEVICE_CONFIG_KEEP_ALIVE_MODE > 0U) &&             \
    defined(FSL_FEATURE_USB_KHCI_USB_RAM) && (FSL_FEATURE_USB_KHCI_USB_RAM > 0U)
volatile static uint8_t s_waitForDataReceive = 0;
volatile static uint8_t s_comOpen = 0;
#endif
/*******************************************************************************
* Code
******************************************************************************/



volatile bool usb_rx_full = false;

volatile uint8_t  rxd_buffer[APP_RX_DATA_SIZE];
volatile uint32_t rxd_length    = 0;
volatile uint32_t rxd_BufPtrIn  = 0;
volatile uint32_t rxd_BufPtrOut = 0;

volatile uint8_t  txd_buffer[APP_TX_DATA_SIZE];
volatile uint32_t txd_length    = 0;
volatile uint32_t txd_BufPtrIn  = 0;
volatile uint32_t txd_BufPtrOut = 0;



#define USB_CDC_RESET_MODE_TO_BOOT      1
#define USB_CDC_RESET_MODE_TO_JUMP      2


volatile uint8_t  cdc_reset_mode = 0;
volatile uint32_t cdc_reset_delay_cnt = 0;






void USB_SoF_IRQHandler(void)
{
  uint32_t rx_buf_length;


  rx_buf_length = APP_RX_DATA_SIZE - CDC_Itf_RxAvailable() - 1;

  // 수신버퍼가 USB 전송 패킷 이상 남았을때만 수신하도록 함.
  if (usb_rx_full == true)
  {
    if (rx_buf_length >= g_cdcVcomDicEndpoints[0].maxPacketSize)
    {
      USB_DeviceCdcAcmRecv(g_deviceComposite->cdcVcom.cdcAcmHandle, USB_CDC_VCOM_DIC_BULK_OUT_ENDPOINT, s_currRecvBuf, rx_buf_length);
      usb_rx_full = false;
    }
  }

  CDC_Itf_TxISR();

  if (cdc_reset_delay_cnt > 0)
  {
    cdc_reset_delay_cnt--;

    if (cdc_reset_delay_cnt == 0)
    {
      resetRunSoftReset();
    }
  }
}

void CDC_Itf_TxISR(void)
{
  uint32_t buffsize;
  usb_device_cdc_acm_struct_t *cdcAcmHandle;
  cdcAcmHandle = (usb_device_cdc_acm_struct_t *)g_deviceComposite->cdcVcom.cdcAcmHandle;


  if (g_deviceComposite->cdcVcom.attach != 1 || g_deviceComposite->cdcVcom.startTransactions != 1) return;
  if(cdcAcmHandle->bulkIn.isBusy == 1) return;

  buffsize = CDC_Itf_TxBufLengh();

  if (buffsize == 0) return;

  for (int i=0; i<buffsize; i++)
  {
    s_currSendBuf[i] = CDC_Itf_TxRead();
  }


  USB_DeviceCdcAcmSend(g_deviceComposite->cdcVcom.cdcAcmHandle, USB_CDC_VCOM_DIC_BULK_IN_ENDPOINT, s_currSendBuf, buffsize);
}


/*!
 * @brief CDC class specific callback function.
 *
 * This function handles the CDC class specific requests.
 *
 * @param handle          The CDC ACM class handle.
 * @param event           The CDC ACM class event type.
 * @param param           The parameter of the class specific request.
 *
 * @return A USB error code or kStatus_USB_Success.
 */
usb_status_t USB_DeviceCdcVcomCallback(class_handle_t handle, uint32_t event, void *param)
{
    uint32_t i;
    uint32_t len;
    uint8_t *uartBitmap;
    usb_device_cdc_acm_request_param_struct_t *acmReqParam;
    usb_device_endpoint_callback_message_struct_t *epCbParam;
    usb_status_t error = kStatus_USB_Error;
    usb_cdc_acm_info_t *acmInfo = &s_usbCdcAcmInfo;
    acmReqParam = (usb_device_cdc_acm_request_param_struct_t *)param;
    epCbParam = (usb_device_endpoint_callback_message_struct_t *)param;


    switch (event)
    {
        case kUSB_DeviceCdcEventSendResponse:
        {
            if ((epCbParam->length != 0) && (!(epCbParam->length % g_cdcVcomDicEndpoints[0].maxPacketSize)))
            {
                /* If the last packet is the size of endpoint, then send also zero-ended packet,
                 ** meaning that we want to inform the host that we do not have any additional
                 ** data, so it can flush the output.
                 */
                error = USB_DeviceCdcAcmSend(handle, USB_CDC_VCOM_DIC_BULK_IN_ENDPOINT, NULL, 0);
            }
            else if ((1 == g_deviceComposite->cdcVcom.attach) && (1 == g_deviceComposite->cdcVcom.startTransactions))
            {
                if ((epCbParam->buffer != NULL) || ((epCbParam->buffer == NULL) && (epCbParam->length == 0)))
                {
                    /* User: add your own code for send complete event */
                }
            }
            else
            {
            }
        }
        break;
        case kUSB_DeviceCdcEventRecvResponse:
        {
            if ((1 == g_deviceComposite->cdcVcom.attach) && (1 == g_deviceComposite->cdcVcom.startTransactions))
            {
              if (epCbParam->length > APP_RX_DATA_SIZE)
              {
                break;
              }
#if 0
              if (cdc_reset_mode == USB_CDC_RESET_MODE_TO_BOOT)
              {
                resetSetBootMode(RESET_MODE_TO_BOOT);
                cdc_reset_delay_cnt = 100;
              }
              if (cdc_reset_mode == USB_CDC_RESET_MODE_TO_JUMP)
              {
                resetSetBootMode(RESET_MODE_TO_JUMP);
                cdc_reset_delay_cnt = 100;
              }
#endif
#ifdef __APP_MODE
              cdc_reset_delay_cnt = 100;
#endif
                s_recvSize = epCbParam->length;


                for( i=0; i<epCbParam->length; i++ )
                {
                  rxd_buffer[rxd_BufPtrIn] = s_currRecvBuf[i];

                  rxd_BufPtrIn++;

                  /* To avoid buffer overflow */
                  if(rxd_BufPtrIn == APP_RX_DATA_SIZE)
                  {
                    rxd_BufPtrIn = 0;
                  }
                }

                uint32_t rx_buf_len;

                rx_buf_len = APP_RX_DATA_SIZE - CDC_Itf_RxAvailable() - 1;


                if (rx_buf_len >= g_cdcVcomDicEndpoints[0].maxPacketSize)
                {
                  /* Schedule buffer for next receive event */
                  error = USB_DeviceCdcAcmRecv(handle, USB_CDC_VCOM_DIC_BULK_OUT_ENDPOINT, s_currRecvBuf, rx_buf_len);
                }
                else
                {
                  usb_rx_full = true;
                }
            }
        }
        break;
        case kUSB_DeviceCdcEventSerialStateNotif:
            ((usb_device_cdc_acm_struct_t *)handle)->hasSentState = 0;
            error = kStatus_USB_Success;
            break;
        case kUSB_DeviceCdcEventSendEncapsulatedCommand:
            break;
        case kUSB_DeviceCdcEventGetEncapsulatedResponse:
            break;
        case kUSB_DeviceCdcEventSetCommFeature:
            if (USB_DEVICE_CDC_FEATURE_ABSTRACT_STATE == acmReqParam->setupValue)
            {
                if (1 == acmReqParam->isSetup)
                {
                    *(acmReqParam->buffer) = s_abstractState;
                }
                else
                {
                    *(acmReqParam->length) = 0;
                }
            }
            else if (USB_DEVICE_CDC_FEATURE_COUNTRY_SETTING == acmReqParam->setupValue)
            {
                if (1 == acmReqParam->isSetup)
                {
                    *(acmReqParam->buffer) = s_countryCode;
                }
                else
                {
                    *(acmReqParam->length) = 0;
                }
            }
            else
            {
            }
            error = kStatus_USB_Success;
            break;
        case kUSB_DeviceCdcEventGetCommFeature:
            if (USB_DEVICE_CDC_FEATURE_ABSTRACT_STATE == acmReqParam->setupValue)
            {
                *(acmReqParam->buffer) = s_abstractState;
                *(acmReqParam->length) = COMM_FEATURE_DATA_SIZE;
            }
            else if (USB_DEVICE_CDC_FEATURE_COUNTRY_SETTING == acmReqParam->setupValue)
            {
                *(acmReqParam->buffer) = s_countryCode;
                *(acmReqParam->length) = COMM_FEATURE_DATA_SIZE;
            }
            else
            {
            }
            error = kStatus_USB_Success;
            break;
        case kUSB_DeviceCdcEventClearCommFeature:
            break;
        case kUSB_DeviceCdcEventGetLineCoding:
            *(acmReqParam->buffer) = s_lineCoding;
            *(acmReqParam->length) = LINE_CODING_SIZE;
            error = kStatus_USB_Success;
            break;
        case kUSB_DeviceCdcEventSetLineCoding:
        {
            if (1 == acmReqParam->isSetup)
            {
              uint32_t baud;

              *(acmReqParam->buffer) = s_lineCoding;

              baud  = s_lineCoding[0];
              baud |= s_lineCoding[1]<<8;
              baud |= s_lineCoding[2]<<16;
              baud |= s_lineCoding[3]<<24;


              if (baud == 1200)
              {
                cdc_reset_mode = USB_CDC_RESET_MODE_TO_BOOT;
              }
              if (baud == 1201)
              {
                cdc_reset_mode = USB_CDC_RESET_MODE_TO_JUMP;
              }

              if (1 == g_deviceComposite->cdcVcom.attach)
              {
                g_deviceComposite->cdcVcom.startTransactions = 1;
              }
            }
            else
            {
                *(acmReqParam->length) = 0;
            }
        }
            error = kStatus_USB_Success;
            break;
        case kUSB_DeviceCdcEventSetControlLineState:
        {
            s_usbCdcAcmInfo.dteStatus = acmReqParam->setupValue;
            /* activate/deactivate Tx carrier */
            if (acmInfo->dteStatus & USB_DEVICE_CDC_CONTROL_SIG_BITMAP_CARRIER_ACTIVATION)
            {
                acmInfo->uartState |= USB_DEVICE_CDC_UART_STATE_TX_CARRIER;
            }
            else
            {
                acmInfo->uartState &= (uint16_t)~USB_DEVICE_CDC_UART_STATE_TX_CARRIER;
            }

            /* activate carrier and DTE. Com port of terminal tool running on PC is open now */
            if (acmInfo->dteStatus & USB_DEVICE_CDC_CONTROL_SIG_BITMAP_DTE_PRESENCE)
            {
                acmInfo->uartState |= USB_DEVICE_CDC_UART_STATE_RX_CARRIER;
            }
            /* Com port of terminal tool running on PC is closed now */
            else
            {
                acmInfo->uartState &= (uint16_t)~USB_DEVICE_CDC_UART_STATE_RX_CARRIER;
            }

            /* Indicates to DCE if DTE is present or not */
            acmInfo->dtePresent = (acmInfo->dteStatus & USB_DEVICE_CDC_CONTROL_SIG_BITMAP_DTE_PRESENCE) ? 1 : 0;

            /* Initialize the serial state buffer */
            acmInfo->serialStateBuf[0] = NOTIF_REQUEST_TYPE;                /* bmRequestType */
            acmInfo->serialStateBuf[1] = USB_DEVICE_CDC_NOTIF_SERIAL_STATE; /* bNotification */
            acmInfo->serialStateBuf[2] = 0x00;                              /* wValue */
            acmInfo->serialStateBuf[3] = 0x00;
            acmInfo->serialStateBuf[4] = 0x00; /* wIndex */
            acmInfo->serialStateBuf[5] = 0x00;
            acmInfo->serialStateBuf[6] = UART_BITMAP_SIZE; /* wLength */
            acmInfo->serialStateBuf[7] = 0x00;
            /* Notify to host the line state */
            acmInfo->serialStateBuf[4] = acmReqParam->interfaceIndex;
            /* Lower byte of UART BITMAP */
            uartBitmap = (uint8_t *)&acmInfo->serialStateBuf[NOTIF_PACKET_SIZE + UART_BITMAP_SIZE - 2];
            uartBitmap[0] = acmInfo->uartState & 0xFFu;
            uartBitmap[1] = (acmInfo->uartState >> 8) & 0xFFu;
            len = (uint32_t)(NOTIF_PACKET_SIZE + UART_BITMAP_SIZE);
            if (0 == ((usb_device_cdc_acm_struct_t *)handle)->hasSentState)
            {
                error = USB_DeviceCdcAcmSend(handle, USB_CDC_VCOM_CIC_INTERRUPT_IN_ENDPOINT, acmInfo->serialStateBuf, len);
                if (kStatus_USB_Success != error)
                {
                    //usb_echo("kUSB_DeviceCdcEventSetControlLineState error!");
                }
                ((usb_device_cdc_acm_struct_t *)handle)->hasSentState = 1;
            }

            /* Update status */
            if (acmInfo->dteStatus & USB_DEVICE_CDC_CONTROL_SIG_BITMAP_CARRIER_ACTIVATION)
            {
                /*  To do: CARRIER_ACTIVATED */
            }
            else
            {
                /* To do: CARRIER_DEACTIVATED */
            }
            if (acmInfo->dteStatus & USB_DEVICE_CDC_CONTROL_SIG_BITMAP_DTE_PRESENCE)
            {
                /* DTE_ACTIVATED */
                if (1 == g_deviceComposite->cdcVcom.attach)
                {
                  g_deviceComposite->cdcVcom.startTransactions = 1;
#if defined(FSL_FEATURE_USB_KHCI_KEEP_ALIVE_ENABLED) && (FSL_FEATURE_USB_KHCI_KEEP_ALIVE_ENABLED > 0U) && \
    defined(USB_DEVICE_CONFIG_KEEP_ALIVE_MODE) && (USB_DEVICE_CONFIG_KEEP_ALIVE_MODE > 0U) &&             \
    defined(FSL_FEATURE_USB_KHCI_USB_RAM) && (FSL_FEATURE_USB_KHCI_USB_RAM > 0U)
                    s_waitForDataReceive = 1;
                    USB0->INTEN &= ~USB_INTEN_SOFTOKEN_MASK;
                    s_comOpen = 1;
                    usb_echo("USB_APP_CDC_DTE_ACTIVATED\r\n");
#endif
                }
            }
            else
            {
                /* DTE_DEACTIVATED */
                if (1 == g_deviceComposite->cdcVcom.attach)
                {
                  g_deviceComposite->cdcVcom.startTransactions = 0;
                }
            }
        }
        break;
        case kUSB_DeviceCdcEventSendBreak:
            break;
        default:
            break;
    }

    return error;
}



/*!
 * @brief Virtual COM device set configuration function.
 *
 * This function sets configuration for CDC class.
 *
 * @param handle The CDC ACM class handle.
 * @param configure The CDC ACM class configure index.
 *
 * @return A USB error code or kStatus_USB_Success.
 */
usb_status_t USB_DeviceCdcVcomSetConfigure(class_handle_t handle, uint8_t configure)
{
    if (USB_COMPOSITE_CONFIGURE_INDEX == configure)
    {
        g_deviceComposite->cdcVcom.attach = 1;
        /* Schedule buffer for receive */
        USB_DeviceCdcAcmRecv(g_deviceComposite->cdcVcom.cdcAcmHandle, USB_CDC_VCOM_DIC_BULK_OUT_ENDPOINT, s_currRecvBuf,
                             g_cdcVcomDicEndpoints[0].maxPacketSize);
    }
    return kStatus_USB_Success;
}


/*!
 * @brief Virtual COM device initialization function.
 *
 * This function initializes the device with the composite device class information.
 *
 * @param deviceComposite The pointer to the composite device structure.
 *
 * @return A USB error code or kStatus_USB_Success.
 */
usb_status_t USB_DeviceCdcVcomInit(usb_device_composite_struct_t *deviceComposite)
{
    g_deviceComposite = deviceComposite;
    return kStatus_USB_Success;
}


#if 0

/*!
 * @brief USB device callback function.
 *
 * This function handles the usb device specific requests.
 *
 * @param handle          The USB device handle.
 * @param event           The USB device event type.
 * @param param           The parameter of the device specific request.
 *
 * @return A USB error code or kStatus_USB_Success.
 */
usb_status_t USB_DeviceCallback(usb_device_handle handle, uint32_t event, void *param)
{
    usb_status_t error = kStatus_USB_Error;
    uint16_t *temp16 = (uint16_t *)param;
    uint8_t *temp8 = (uint8_t *)param;

    switch (event)
    {
        case kUSB_DeviceEventBusReset:
        {
            s_cdcVcom.attach = 0;
            s_cdcVcom.currentConfiguration = 0U;
#if (defined(USB_DEVICE_CONFIG_EHCI) && (USB_DEVICE_CONFIG_EHCI > 0U)) || \
    (defined(USB_DEVICE_CONFIG_LPCIP3511HS) && (USB_DEVICE_CONFIG_LPCIP3511HS > 0U))
            /* Get USB speed to configure the device, including max packet size and interval of the endpoints. */
            if (kStatus_USB_Success == USB_DeviceClassGetSpeed(CONTROLLER_ID, &s_cdcVcom.speed))
            {
                USB_DeviceSetSpeed(handle, s_cdcVcom.speed);
            }
#endif
        }
        break;
        case kUSB_DeviceEventSetConfiguration:
            if (0U ==(*temp8))
            {
                s_cdcVcom.attach = 0;
                s_cdcVcom.currentConfiguration = 0U;
            }
            else if (USB_CDC_VCOM_CONFIGURE_INDEX == (*temp8))
            {
                s_cdcVcom.attach = 1;
                s_cdcVcom.currentConfiguration = *temp8;
                /* Schedule buffer for receive */
                USB_DeviceCdcAcmRecv(s_cdcVcom.cdcAcmHandle, USB_CDC_VCOM_BULK_OUT_ENDPOINT, s_currRecvBuf,
                                     g_UsbDeviceCdcVcomDicEndpoints[0].maxPacketSize);
            }
            else
            {
                error = kStatus_USB_InvalidRequest;
            }
            break;
        case kUSB_DeviceEventSetInterface:
            if (s_cdcVcom.attach)
            {
                uint8_t interface = (uint8_t)((*temp16 & 0xFF00U) >> 0x08U);
                uint8_t alternateSetting = (uint8_t)(*temp16 & 0x00FFU);
                if (interface < USB_CDC_VCOM_INTERFACE_COUNT)
                {
                    s_cdcVcom.currentInterfaceAlternateSetting[interface] = alternateSetting;
                }
            }
            break;
        case kUSB_DeviceEventGetConfiguration:
            break;
        case kUSB_DeviceEventGetInterface:
            break;
        case kUSB_DeviceEventGetDeviceDescriptor:
            if (param)
            {
                error = USB_DeviceGetDeviceDescriptor(handle, (usb_device_get_device_descriptor_struct_t *)param);
            }
            break;
        case kUSB_DeviceEventGetConfigurationDescriptor:
            if (param)
            {
                error = USB_DeviceGetConfigurationDescriptor(handle,
                                                             (usb_device_get_configuration_descriptor_struct_t *)param);
            }
            break;
        case kUSB_DeviceEventGetStringDescriptor:
            if (param)
            {
                /* Get device string descriptor request */
                error = USB_DeviceGetStringDescriptor(handle, (usb_device_get_string_descriptor_struct_t *)param);
            }
            break;
        default:
            break;
    }

    return error;
}

#endif


void CDC_Itf_Init(void)
{
  rxd_length      = 0;
  rxd_BufPtrIn    = 0;
  rxd_BufPtrOut   = 0;

  txd_length      = 0;
  txd_BufPtrIn    = 0;
  txd_BufPtrOut   = 0;
}

uint32_t CDC_Itf_GetBaud(void)
{
  uint32_t ret;


 ret = s_lineCoding[0] | (s_lineCoding[1]<<8) | (s_lineCoding[2]<<16) | (s_lineCoding[3]<<24);


  return ret;
}

uint32_t CDC_Itf_TxAvailable( void )
{
  return DATA_BUFF_SIZE;
}

uint32_t CDC_Itf_RxAvailable( void )
{
  uint32_t length = 0;


  if( rxd_BufPtrIn != rxd_BufPtrOut )
  {
    length = (APP_RX_DATA_SIZE + rxd_BufPtrIn - rxd_BufPtrOut) % APP_RX_DATA_SIZE;
  }

  return length;
}

uint32_t CDC_Itf_TxBufLengh( void )
{
  uint32_t length = 0;


  length = (APP_TX_DATA_SIZE + txd_BufPtrIn - txd_BufPtrOut) % APP_TX_DATA_SIZE;

  return length;
}

int32_t  CDC_Itf_Write( uint8_t *p_buf, uint32_t length )
{
  uint32_t i;
  uint32_t ptr_index;


  if (g_deviceComposite->cdcVcom.attach != 1 || g_deviceComposite->cdcVcom.startTransactions != 1)
  {
    return -1;
  }

  if (length >= CDC_Itf_TxAvailable())
  {
    return 0;
  }

  ptr_index = txd_BufPtrIn;


  for (i=0; i<length; i++)
  {

    txd_buffer[ptr_index] = p_buf[i];

    ptr_index++;

    /* To avoid buffer overflow */
    if(ptr_index == APP_TX_DATA_SIZE)
    {
      ptr_index = 0;
    }
  }
  txd_BufPtrIn = ptr_index;



  return length;
}

uint8_t  CDC_Itf_Getch( void )
{
  while(1)
  {
    if( CDC_Itf_RxAvailable() ) break;
  }

  return CDC_Itf_Read();
}

uint8_t  CDC_Itf_Read( void )
{
  uint8_t ch = 0;
  uint32_t buffptr;


  if( rxd_BufPtrIn == rxd_BufPtrOut ) return 0;


  buffptr = rxd_BufPtrOut;

  ch = rxd_buffer[buffptr];

  rxd_BufPtrOut += 1;
  if (rxd_BufPtrOut >= APP_RX_DATA_SIZE)
  {
    rxd_BufPtrOut = 0;
  }


  return ch;
}

uint8_t CDC_Itf_TxRead( void )
{
  uint8_t ch = 0;
  uint32_t buffptr;


  buffptr = txd_BufPtrOut;

  ch = txd_buffer[buffptr];


  txd_BufPtrOut += 1;
  if (txd_BufPtrOut >= APP_TX_DATA_SIZE)
  {
    txd_BufPtrOut = 0;
  }

  return ch;
}


bool CDC_Itf_IsConnected(void)
{

  if (g_deviceComposite->cdcVcom.attach != 1)
  {
    return false;
  }

  if (g_deviceComposite->cdcVcom.currentConfiguration == 0)
  {
    return false;
  }

  return true;
}

void CDC_Itf_Flush( void )
{
  rxd_BufPtrOut = 0;
  rxd_BufPtrIn  = 0;
}

