/**
 *    Copyright 2012, Kerry D. Wong
 * 
 *      http://www.kerrywong.com
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 */

#ifndef MCP2210_H
#define	MCP2210_H

#include <cstdlib>
#include <stdio.h>
#include <wchar.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>

#include "hidapi.h"

typedef uint8_t byte;

#define MCP2210_VID 0x04d8
#define MCP2210_PID 0x00de

/**
 * GPIO pin directions
 */
#define GPIO_DIRECTION_OUTPUT 0
#define GPIO_DIRECTION_INPUT 1

/** 
 * Response bit value for 
 * CMD_SET_NVRAM_PARAM 0x60, CMD_GET_NVRAM_PARAM 0x61
 * CMDSUB_POWERUP_CHIP_SETTINGS 0x20
 */
#define GP_PIN_DESIGNATION_GPIO 0x0
#define GP_PIN_DESIGNATION_CS 0x1
#define GP_PIN_DESIGNATION_DEDICATED 0x2
#define REMOTE_WAKEUP_DISABLED 0x0
#define REMOTE_WAVEUP_ENABLED 0x1
#define COUNT_HIGH_PULSES 0x4
#define COUNT_LOW_PULSES 0x3
#define COUNT_RISING_EDGES 0x2
#define COUNT_FALLING_EDGES 0x1
#define NO_INTERRUPT_COUNTING 0x0
#define CHIP_SETTINGS_NOT_PROTECTED 0x0
#define CHIP_SETTINGS_PROTECTED_BY_PWD 0x40
#define CHIP_SETTINGS_LOCKED 0x80

#define USB_STRING_DESCRIPTOR_ID 0x03

#define CMD_UNSPOORTED 0x0    
#define CMD_GET_CHIP_STATUS 0x10
#define CMD_SPI_CANCEL 0x11
#define CMD_GET_NUM_EVENTS_FROM_INT_PIN 0x12
#define CMD_GET_GPIO_SETTING 0x20
#define CMD_SET_GPIO_SETTING 0x21
#define CMD_SET_GPIO_PIN_VAL 0x30
#define CMD_GET_GPIO_PIN_VAL 0x31
#define CMD_SET_GPIO_PIN_DIR 0x32
#define CMD_GET_GPIO_PIN_DIR 0x33
#define CMD_SET_SPI_SETTING 0x40
#define CMD_GET_SPI_SETTING 0x41    
#define CMD_SPI_TRANSFER 0x42
#define CMD_READ_EEPROM_MEM 0x50
#define CMD_WRITE_EEPROM_MEM 0x51  
#define CMD_SET_NVRAM_PARAM 0x60
#define CMD_GET_NVRAM_PARAM 0x61
#define CMD_SEND_PASSWORD 0x70
#define CMD_SPI_BUS_RELEASE_REQ 0x80

#define CMDSUB_SPI_POWERUP_XFER_SETTINGS 0x10
#define CMDSUB_POWERUP_CHIP_SETTINGS 0x20
#define CMDSUB_USB_KEY_PARAMETERS 0x30
#define CMDSUB_USB_PRODUCT_NAME 0x40
#define CMDSUB_USB_MANUFACTURER_NAME 0x50

#define OPERATION_SUCCESSFUL 0
#define ERROR_UNABLE_TO_OPEN_DEVICE -1
#define ERROR_UNABLE_TO_WRITE_TO_DEVICE -2
#define ERROR_UNABLE_TO_READ_FROM_DEVICE -3
#define ERROR_INVALID_DEVICE_HANDLE -99

#define COMMAND_BUFFER_LENGTH 64
#define RESPONSE_BUFFER_LENGTH 64

#define SPI_STATUS_FINISHED_NO_DATA_TO_SEND 0x10
#define SPI_STATUS_STARTED_NO_DATA_TO_RECEIVE 0x20
#define SPI_STATUS_SUCCESSFUL 0x30

/**
 * General purpose pin definition
 */
struct GPPin {
    /**
     * The designation of the pin 
     * 0x00: GPIO 
     * 0x01: Chip Selects 
     * 0x02: Dedicated Function Pin
     */
    unsigned int PinDesignation;

    /**
     * The output value of the GPIO pin
     * 0: Low
     * 1: High
     */
    unsigned int GPIOOutput;

    /**
     * GPIO pin direction
     * 0: Output
     * 1: Input
     */
    unsigned int GPIODirection;
};

/**
 * SPI transfer settings definition
 */
struct SPITransferSettingsDef {
    /**
     * Bit Rate (bps)
     */
    unsigned long BitRate;

    /**
     * MSB                             LSB
     * CS8 CS7 CS6 CS5 CS4 CS3 CS2 CS1 CS0
     */
    unsigned int IdleChipSelectValue;

    /**
     * MSB                             LSB
     * CS8 CS7 CS6 CS5 CS4 CS3 CS2 CS1 CS0
     */
    unsigned int ActiveChipSelectValue;

    /**
     * Chip select to data delay
     * delay = value x 100 ns
     */
    unsigned int CSToDataDelay;

    /**
     * Last data byte to chip select (de-asserted) delay
     * delay = value x 100 ns
     */
    unsigned int LastDataByteToCSDelay;

    /**
     * Delay between subsequent data bytes
     * delay = value x 100 ns
     */
    unsigned int SubsequentDataByteDelay;

    /**
     * Number of bytes to be transfered per SPI transaction
     */
    unsigned int BytesPerSPITransfer;

    /**
     * SPI Mode
     * 0x00: SPI mode 0
     * 0x01: SPI mode 1
     * 0x02; SPI mode 2
     * 0x03: SPI mode 3
     */
    unsigned int SPIMode;

    /**
     * The error code returned
     */
    int ErrorCode;
};

/**
 * Chip settings definition
 */
struct ChipSettingsDef {
    /**
     * General purpose I/O pin 0...8
     */
    GPPin GP[9];
    
    /**
     * Remote wake-up enabled/disabled
     * 0 - Disabled
     * 1 - Enabled
     */
    unsigned int RemoteWakeUpEnabled;
    
    /**
     * Dedicated function mode
     * 
     * B100 - count high pulses
     * B011 - count low pulses
     * B010 - count rising edges
     * B001 - count falling edges
     * B000 - no interrupt counting
     */
    unsigned int DedicatedFunctionInterruptPinMode;
    
    /**
     * SPI bus release enable
     * 
     * 0 - SPI bus is released between transfers
     * 1 - SPI bus is not released by the MCP2210 between transfers
     */
    unsigned int SPIBusReleaseMode;
    
    /**
     * NVRAM chip parameters access control
     * 
     * 0x00 - Chip settings not protected
     * 0x01 - Chip settings protected by password access
     * 0x80 - Chip settings permanently locked
     */
    unsigned int NVRamChipParamAccessControl;
    
    /**
     * New password charecters 0..7
     * 
     * When the password does not need to be changed, the fields must be filled with 0.
     */
    char password[8];
    int ErrorCode;
};

/**
 * USB power-up key parameters definition
 */
struct USBKeyParametersDef {
    /**
     * Vendor ID
     */
    unsigned int VID;

    /**
     * Product ID
     */
    unsigned int PID;

    /**
     * Whether it is host powered
     * 0: No
     * 1: Yes
     */
    unsigned int HostPowered;

    /**
     * Whether it is self powered
     * 0: No
     * 1: Yes
     */
    unsigned int SelfPowered;

    /**
     * Whether supports remote wake-up
     * 0: No
     * 1: Yes
     */
    unsigned int RemoteWakeupCapable;

    /**
     * Requested current amount from USB Host
     * current = value x 2 mA
     */
    unsigned int RequestedCurrentAmountFromHost;

    /**
     * The error code returned
     */
    int ErrorCode;
};

/**
 * USB manufacturer name/product name definition
 */
struct ManufacturerProductNameDef {
    /**
     * Total USB string descriptor length
     * 
     * Note, the string is in unicode and 
     * the total length is the length of the string descriptor plus 2 bytes
     */
    unsigned int USBStringDescriptorLength;

    /**
     * USB string descriptor ID
     * Fixed value: 0x03
     */
    unsigned int USBStringDescriptorID;

    /**
     * The manufacture name/product name
     */
    char ManufacturerProductName[59];

    /**
     * The error code returned
     */
    int ErrorCode;
};

/**
 * MCP2210 status definition
 */
struct ChipStatusDef {
    /**
     * SPI bus release external request status
     * 
     * 0x01: no external request for SPI bus release
     * 0x00: pending external request for SPI bus release
     */
    unsigned int SPIBusReleaseExtReqStat;

    /**
     * SPI bus current owner
     * 
     * 0x00: no owner
     * 0x01: USB bridge
     * 0x02: External master
     */
    unsigned int SPIBusCurrentOwner;

    /**
     * Attempted password accesses
     * informs the USB host on how many times the NVRAM password was tried
     */
    unsigned int AttemptedPWDAccesses;

    /**
     * Whether the password was guessed
     * 
     * 0x00: password not guessed
     * 0x01: password guessed
     */
    unsigned int PasswordGuessed;

    /**
     * The error code returned
     */
    int ErrorCode;
};

/**
 * SPI data transfer status definition
 */
struct SPIDataTransferStatusDef {
    /**
     * Number of bytes received
     */
    unsigned int NumberOfBytesReceived;

    /**
     * SPI transfer engine status
     * 
     * 0x10: SPI transfer finished, no data to send
     * 0x20: SPI transfer started, no data to receive
     * 0x30: SPI data accepted, command completed successfully
     *
     */
    unsigned int SPIEngineStatus;

    /**
     * The buffer contains received data. 
     * Actual amount of data is determined by NumberOfBytesReceived.
     */
    unsigned char DataReceived[60];

    /**
     * The error code returned
     */
    int ErrorCode;
};

/**
 * External interrupt pin (GP6) status definition
 */
struct ExternalInterruptPinStatusDef {
    /**
     * interrupt event counter value
     */
    unsigned int InterruptEventCounter;

    /**
     * The error code returned
     */
    int ErrorCode;
};

/**
 * General purpose pins definition
 */
struct GPPinDef {
    /**
     * GP0 through GP8
     * @see GPPin
     */
    GPPin GP[9];

    /**
     * The error code returned
     */
    int ErrorCode;
};

/**
 * Enumerate the connected MCP2210's
 * 
 * @return 
 *      a pointer to hid_device_info
 */
hid_device_info* EnumerateMCP2210();

/**
 * Initialize MCP2210
 *      Vendor ID:  0x04d8
 *      Product ID: 0x00de
 * 
 * @return 
 *      The handle to the MCP2210 device
 */
hid_device* InitMCP2210();

/**
 * Initialize MCP2210 (if more than one is present)
 *      Vendor ID:  0x04d8
 *      Product ID: 0x00de
 *
 *      @param serialNumber
 *              The serial number of the MCP2210 device
 *      
 * @return 
 *      The handle to the MCP2210 device
 */
hid_device* InitMCP2210(wchar_t* serialNumber);

/**
 * Initialize MCP2210 (using all parameters)
 * @param vid
 *      Vender ID
 * @param pid
 *      Product ID
 * @param serialNumber
 *      Serial number
 * @return 
 *      The handle to the MCP2210 device
 */
hid_device* InitMCP2210(unsigned short vid, unsigned short pid, wchar_t* serialNumber);

/**
 * Release the device handle and close the device
 * 
 * @param handle
 *      The handle to the MCP2210 device
 */
void ReleaseMCP2210(hid_device *handle);

/**
 * Send a USB command
 * 
 * @param handle
 *      The handle to the MCP2210 device
 * @param cmdBuf
 *      command buffer (64 bytes), unused/reserved entries must be filled with zero.
 * @param responseBuf
 *      the buffer (64 bytes) that contains the response.
 * @return 
 *      0:    Operation was successful
 *      <0:  Other device errors (see error codes)
 */
int SendUSBCmd(hid_device *handle, byte *cmdBuf, byte *responseBuf);

/**
 * Get SPI power-up transfer settings
 * @param handle
 *      The handle to the MCP2210 device
 * @param isVolatile (default true)
 *      Whether to make change permanently or in memory only.
 *      
 *      ture: in memory only.
 *      false: changes are written to NVRAM 
 * @return 
 *       @see SPITransferSettingsDef
 */
SPITransferSettingsDef GetSPITransferSettings(hid_device *handle, bool isVolatile = true);

/**
 * Set SPI power-up transfer settings
 * @param handle
 *      The handle to the MCP2210 device
 * @param def
 *      @see SPITransferSettingsDef
 * @param isVolatile (default true)
 *      Whether to make change permanently or in memory only.
 *      
 *      ture: in memory only.
 *      false: changes are written to NVRAM  
 * @return 
 *      0:    Operation was successful
 *      0xFB: The provided password does not match the one stored in the chip 
 *            or the settings are permanently locked.
 *      <0:  Other device errors (see error codes)
 */
int SetSPITransferSettings(hid_device *handle, SPITransferSettingsDef def, bool isVolatile = true);

/**
 * Get power-up settings
 * 
 * @param handle
 *      The handle to the MCP2210 device
 * @param isVolatile (default true)
 *      Whether to make change permanently or in memory only. 
 * @return 
 *      @see ChipSettingsDef
 */
ChipSettingsDef GetChipSettings(hid_device *handle, bool isVolatile = true);

/**
 * Set chip settings power-up default
 * 
 * @param handle
 *      The handle to the MCP2210 device
 * @param def
 *      @see ChipSettingDef
 * @param isVolatile (default true)
 *      Whether to make change permanently or in memory only.
 *      
 *      ture: in memory only.
 *      false: changes are written to NVRAM
 * @return 
 *      0:    Operation was successful
 *      0xFB: The provided password does not match the one stored in the chip 
 *            or the settings are permanently locked.
 *      <0:  Other device errors (see error codes)
 */
int SetChipSettings(hid_device *handle, ChipSettingsDef def, bool isVolatile = true);

/**
 * Get USB power-up key parameters
 * 
 * @param handle
 *      The handle to the MCP2210 device
 * @return 
 *      @see USBKeyParametersDef
 */
USBKeyParametersDef GetUSBKeyParameters(hid_device *handle);

/**
 * Set USB power-up key parameters
 * 
 * @param handle
 *      The handle to the MCP2210 device
 * @param def
 *      @see USBKeyParametersDef
 * @return 
 *      0:    Operation was successful
 *      0xFB: The provided password does not match the one stored in the chip 
 *            or the settings are permanently locked.
 *      <0:  Other device errors (see error codes) 
 */
int SetUSBKeyParameters(hid_device *handle, USBKeyParametersDef def);

/**
 * Set USB manufacturer name or product name
 * @param handle
 *      The handle to the MCP2210 device
 * @param subCmdCode
 *      The sub command code which determines whether the action
 *      is to set manufacturer name or product name:
 *      
 *      CMDSUB_USB_MANUFACTURER_NAME: set manufacturer name
 *      CMDSUB_USB_PRODUCT_NAME: set product name
 * @return 
 *      @see USBManufacturerProductNameDef
 */
ManufacturerProductNameDef GetManufacturerProductName(hid_device *handle, unsigned int subCmdCode);

/**
 * Set USB manufacturer name or product name
 * 
 * @param handle
 *      The handle to the MCP2210 device
 * @param subCmdCode
 *      The sub command code which determines whether the action
 *      is to set manufacturer name or product name:
 *      
 *      CMDSUB_USB_MANUFACTURER_NAME: set manufacturer name
 *      CMDSUB_USB_PRODUCT_NAME: set product name
 * @param def
 *      @see USBManufacturerProductNameDef
 * @return 
 *      0:    Operation was successful
 *      0xFB: The provided password does not match the one stored in the chip 
 *            or the settings are permanently locked.
 *      <0:  Other device errors (see error codes) 
 */
int SetManufacturerProductName(hid_device *handle, unsigned int subCmdCode, ManufacturerProductNameDef def);

/**
 * Send access password
 * 
 * @param handle
 *      The handle to the MCP2210 device
 * @param pwd
 *      password
 * @param pwdLength
 *      password length (<=8)
 * @return 
 *      0:    Operation was successful
 *      0xFB: Chip conditional access is on, the password does not match
 *              and the number of attempts is above the accepted 
 *              threshold of 5. The Access Password mechanism is
 *              temporarily blocked and no further password will be
 *              accepted until the next power-up.
 *      0xFC: Access rejected
 *      0XFD: Chip conditional access is on, the password does not match 
 *              and the number of attempts is less than the accepted 
 *              threshold of 5.
 */
int SendAccessPassword(hid_device *handle, char *pwd, int pwdLength);

/**
 * Read EEPROM Memory
 * 
 * @param handle
 *      The handle to the MCP2210 device
 * @param addr
 *      The address to be accessed (0-255)
 * @param val
 *      The byte read at the specified address
 * @return 
 *      0:    Operation was successful
 *      <0:  Other device errors (see error codes) 
 */
int ReadEEPROM(hid_device *handle, byte addr, byte* val);

/**
 * Write EEPROM Memory
 * 
 * @param handle
 *      The handle to the MCP2210 device
 * @param addr
 *      The address to be accessed (0-255)
 * @param val
 *      The byte to be written to the specified address.
 * @return 
 *      0:    Operation was successful
 *      0xFA: EEPROM Write failure
 *      0xFB: EEPROM is password protected or permanently locked.
 *      <0:  Other device errors (see error codes) 
 */
int WriteEEPROM(hid_device *handle, byte addr, byte val);

/**
 * Request SPI bus release
 * 
 * @param handle
 *      The handle to the MCP2210 device
 * @param val
 *      The value of the SPI bus release ACK pin (only if GP7 is assigned to this 
 *      dedicated function)
 * @return 
 *      0:    Operation was successful
 *      0xF8: SPI transfer in process (BUS not released)
 *      <0:  Other device errors (see error codes) 
 */
int RequestSPIBusRelease(hid_device *handle, byte val);

/**
 * Get MCP2210 status
 * 
 * @param handle
 *      The handle to the MCP2210 device
 * @return 
 *      @see ChipStatusDef
 */
ChipStatusDef GetChipStatus(hid_device *handle);

/**
 * Cancel the current SPI transfer
 * 
 * @param handle
 *      The handle to the MCP2210 device
 * @return 
 *      @see ChipStatusDef
 */
ChipStatusDef CancelSPITransfer(hid_device *handle);

/**
 * SPI Data Transfer
 * 
 * @param handle
 *      The handle to the MCP2210 device
 * @param data
 *      a pointer to the data array to be transfered
 * @param length
 *      number of bytes to be transfered
 * @return 
 *      @see SPIDataTransferStatusDef
 *      ErrorCode meaning:
 *      0xF7:   SPI bus not available
 *      0xF8:   SPI transfer in progress (cannot accept any data for the moment)
 */
SPIDataTransferStatusDef SPIDataTransfer(hid_device *handle, byte* data, int length);

/**
 * Send data and wait till results are received from the SPI bus.
 * 
 * Note: this method differers from SPIDataTransfer in that it also waits till
 *       the transfer is finished. In SPIDataTransfer, the SPIEngineStatus in the
 *       returned data structure tells the status of the SPI engine, and the call
 *       returns after the data is sent, regardless of whether the data has been
 *       received.
 * 
 * @param handle
 *      The handle to the MCP2210 device
 * @param data
 *      a pointer to the data array to be transfered
 * @param cmdBufferLength
 *      number of command bytes to be transfered
 * @param dataLength (optional)
 *      number of data elements to be returned
 * 
 *      if this parameter is not supplied the default length is set to 
 *      be the same as the command buffer length
 * @return 
 *      @see SPIDataTransferStatusDef
 *      ErrorCode meaning:
 *      0xF7:   SPI bus not available
 *      0xF8:   SPI transfer in progress (cannot accept any data for the moment)
 */
SPIDataTransferStatusDef SPISendReceive(hid_device *handle, byte* data, int cmdBufferLength, int dataLength = -1);

/**
 * Get the current number of events from the interrupt pin
 * 
 * Note, GP6 must be configured to have its dedicated function active.
 * 
 * @param handle
 *      The handle to the MCP2210 device
 * @param resetCounter
 *      0x0: reads, then resets the event counter
 *      >0: the event counter is read, however the counter is not reset
 * @return 
 *      0:    Operation was successful
 *      <0:  Other device errors (see error codes) 
 */
ExternalInterruptPinStatusDef GetNumOfEventsFromInterruptPin(hid_device *handle, byte resetCounter);

/**
 * Get GPIO current pin direction from volatile memory
 * 
 * @param handle
 *      The handle to the MCP2210 device
 * @return 
 *      @see GPPinDef
 *      note, only the direction member variable is affected.
 */
GPPinDef GetGPIOPinDirection(hid_device *handle);

/**
 * Set GPIO current pin direction in volatile memory
 * 
 * @param handle
 *      The handle to the MCP2210 device
 * @param def
 *      @see GPPinDef
 *      note, only the direction member variable is used.
 * @return 
 *      0:    Operation was successful
 *      <0:  Other device errors (see error codes) 
 */
int SetGPIOPinDirection(hid_device *handle, GPPinDef def);

/**
 * Get GPIO current pin value 
 * 
 * @param handle
 *      The handle to the MCP2210 device
 * @return 
 *      @see GPPinDef
 *      note, only the value member variable is affected.
 */
GPPinDef GetGPIOPinValue(hid_device *handle);

/**
 * Set GPIO current pin value
 * @param handle
 *      The handle to the MCP2210 device
 * @param def
 *      @see GPPinDef
 *      note, only the value member variable is used.
 * @return 
 *      0:    Operation was successful
 *      <0:  Other device errors (see error codes) 
 */
int SetGPIOPinVal(hid_device *handle, GPPinDef def);

#endif