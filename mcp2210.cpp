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

using namespace std;

#include "mcp2210.h"

int SendUSBCmd(hid_device *handle, byte *cmdBuf, byte *responseBuf) {
    int r = 0;
    r = hid_write(handle, cmdBuf, 64);
    if (r < 0) return ERROR_UNABLE_TO_WRITE_TO_DEVICE;

    //when the hid device is configured as synchronous, the first 
    //hid_read returns the desired results. and the while() loop
    //is skipped.
    //
    //when the hid device is configured as asynchronous, the first
    //hid_read may or may not succeed, depending on the latency
    //of the attached device. When no data is returned, r = 0 and
    //the while loop keeps polling the returned data until it is 
    //received.
    r = 0;
    r = hid_read(handle, responseBuf, 64);
    if (r < 0) return ERROR_UNABLE_TO_READ_FROM_DEVICE;

    while (r == 0) {
        r = hid_read(handle, responseBuf, 64);
        if (r < 0) return ERROR_UNABLE_TO_READ_FROM_DEVICE;
        usleep(1000);
    }

    return responseBuf[1];
}

SPITransferSettingsDef GetSPITransferSettings(hid_device *handle, bool isVolatile) {
    SPITransferSettingsDef def;

    byte cmd[COMMAND_BUFFER_LENGTH];
    byte rsp[RESPONSE_BUFFER_LENGTH];

    memset(cmd, 0x0, COMMAND_BUFFER_LENGTH);
    memset(rsp, 0x0, RESPONSE_BUFFER_LENGTH);

    if (isVolatile) {
        cmd[0] = CMD_GET_SPI_SETTING;
    } else {
        cmd[0] = CMD_GET_NVRAM_PARAM;
        cmd[1] = CMDSUB_SPI_POWERUP_XFER_SETTINGS;
    }

    int r = SendUSBCmd(handle, cmd, rsp);

    def.ErrorCode = r;

    if (r == 0) {
        def.BitRate = rsp[7] << 24 | rsp[6] << 16 | rsp[5] << 8 | rsp[4];
        def.IdleChipSelectValue = (rsp[9] & 0x1) << 8 | rsp[8];
        def.ActiveChipSelectValue = (rsp[11] & 0x1) << 8 | rsp[10];
        def.CSToDataDelay = rsp[13] << 8 | rsp[12];
        def.LastDataByteToCSDelay = rsp[15] << 8 | rsp[14];
        def.SubsequentDataByteDelay = rsp[17] << 8 | rsp [16];
        def.BytesPerSPITransfer = rsp [19] << 8 | rsp [18];
        def.SPIMode = rsp[20];
    }

    return def;
}

int SetSPITransferSettings(hid_device *handle, SPITransferSettingsDef def, bool isVolatile) {
    byte cmd[COMMAND_BUFFER_LENGTH];
    byte rsp[RESPONSE_BUFFER_LENGTH];

    memset(cmd, 0x0, COMMAND_BUFFER_LENGTH);
    memset(rsp, 0x0, RESPONSE_BUFFER_LENGTH);

    if (isVolatile) {
        cmd[0] = CMD_SET_SPI_SETTING;
    } else {
        cmd[0] = CMD_SET_NVRAM_PARAM;
        cmd[1] = CMDSUB_SPI_POWERUP_XFER_SETTINGS;
    }

    cmd[4] = def.BitRate & 0xfful;
    cmd[5] = (def.BitRate & 0xff00ul) >> 8;
    cmd[6] = (def.BitRate & 0xff0000ul) >> 16;
    cmd[7] = (def.BitRate & 0xff000000ul) >> 24;

    cmd[8] = def.IdleChipSelectValue & 0xff;
    cmd[9] = (def.IdleChipSelectValue & 0x100) >> 8;

    cmd[10] = def.ActiveChipSelectValue & 0xff;
    cmd[11] = (def.ActiveChipSelectValue & 0x100) >> 8;

    cmd[12] = def.CSToDataDelay & 0xff;
    cmd[13] = (def.CSToDataDelay & 0xff00) >> 8;

    cmd[14] = def.LastDataByteToCSDelay & 0xff;
    cmd[15] = (def.LastDataByteToCSDelay & 0xff00) >> 8;

    cmd[16] = def.SubsequentDataByteDelay & 0xff;
    cmd[17] = (def.SubsequentDataByteDelay & 0xff00) >> 8;

    cmd[18] = def.BytesPerSPITransfer & 0xff;
    cmd[19] = (def.BytesPerSPITransfer & 0xff00) >> 8;

    cmd[20] = def.SPIMode;

    return SendUSBCmd(handle, cmd, rsp);
}

ChipSettingsDef GetChipSettings(hid_device *handle, bool isVolatile) {
    ChipSettingsDef def;

    byte cmd[COMMAND_BUFFER_LENGTH];
    byte rsp[RESPONSE_BUFFER_LENGTH];

    memset(cmd, 0x0, COMMAND_BUFFER_LENGTH);
    memset(rsp, 0x0, RESPONSE_BUFFER_LENGTH);

    if (isVolatile) {
        cmd[0] = CMD_GET_GPIO_SETTING;
    } else {
        cmd[0] = CMD_GET_NVRAM_PARAM;
        cmd[1] = CMDSUB_POWERUP_CHIP_SETTINGS;
    }

    int r = SendUSBCmd(handle, cmd, rsp);

    def.ErrorCode = r;

    if (r == 0) {
        for (int i = 0; i < 8; i++) {
            def.GP[i].PinDesignation = rsp[i + 4] & 0x3;
            def.GP[i].GPIOOutput = (rsp[13] >> i) & 0x1;
            def.GP[i].GPIODirection = (rsp[15] >> i) & 0x1;
        }

        def.GP[8].PinDesignation = rsp[12] & 0x3;
        def.GP[8].GPIOOutput = rsp[14] & 0x1;
        def.GP[8].GPIODirection = rsp[16] & 0x1;

        def.RemoteWakeUpEnabled = (rsp[17] & 0x10) >> 4;
        def.DedicatedFunctionInterruptPinMode = (rsp[17] & 0xE) >> 1;
        def.SPIBusReleaseMode = rsp[17] & 0x1;
        def.NVRamChipParamAccessControl = rsp[18];

        if (def.NVRamChipParamAccessControl == CHIP_SETTINGS_PROTECTED_BY_PWD) {
            for (int i = 0; i < 8; i++) {
                def.password[i] = rsp[19 + i];
            }
        }
    }

    return def;
}

int SetChipSettings(hid_device *handle, ChipSettingsDef def, bool isVolatile) {
    byte cmd[COMMAND_BUFFER_LENGTH];
    byte rsp[RESPONSE_BUFFER_LENGTH];

    memset(cmd, 0x0, COMMAND_BUFFER_LENGTH);
    memset(rsp, 0x0, RESPONSE_BUFFER_LENGTH);

    if (isVolatile) {
        cmd[0] = CMD_SET_GPIO_SETTING;
    } else {
        cmd[0] = CMD_SET_NVRAM_PARAM;
        cmd[1] = CMDSUB_POWERUP_CHIP_SETTINGS;
    }

    cmd[13] = 0;
    cmd[15] = 0;

    for (int i = 0; i < 8; i++) {
        cmd[i + 4] = def.GP[i].PinDesignation;
        cmd[13] |= def.GP[i].GPIOOutput << i;
        cmd[15] |= def.GP[i].GPIODirection << i;
    }

    cmd[12] = def.GP[8].PinDesignation;
    cmd[14] = def.GP[8].GPIOOutput;
    cmd[16] = def.GP[8].GPIODirection;
    cmd[17] = def.RemoteWakeUpEnabled << 4 | def.DedicatedFunctionInterruptPinMode << 1 | def.SPIBusReleaseMode;
    cmd[18] = def.NVRamChipParamAccessControl;

    if (def.NVRamChipParamAccessControl == CHIP_SETTINGS_PROTECTED_BY_PWD) {
        for (int i = 0; i < 8; i++)
            cmd[19 + i] = def.password[0];
    } //if not password protected, the default is 0.

    return SendUSBCmd(handle, cmd, rsp);
}

USBKeyParametersDef GetUSBKeyParameters(hid_device *handle) {
    USBKeyParametersDef def;

    byte cmd[COMMAND_BUFFER_LENGTH];
    byte rsp[RESPONSE_BUFFER_LENGTH];

    memset(cmd, 0x0, COMMAND_BUFFER_LENGTH);
    memset(rsp, 0x0, RESPONSE_BUFFER_LENGTH);

    cmd[0] = CMD_GET_NVRAM_PARAM;
    cmd[1] = CMDSUB_USB_KEY_PARAMETERS;

    int r = SendUSBCmd(handle, cmd, rsp);

    def.ErrorCode = r;

    if (r == 0) {
        def.VID = rsp[13] << 8 | rsp[12];
        def.PID = rsp[15] << 8 | rsp[14];
        def.HostPowered = (rsp[29] & 0x80) >> 7;
        def.SelfPowered = (rsp[29] & 0x40) >> 6;
        def.RemoteWakeupCapable = (rsp[29] & 0x20) >> 5;
        def.RequestedCurrentAmountFromHost = rsp[30] * 2;
    }

    return def;
}

int SetUSBKeyParameters(hid_device *handle, USBKeyParametersDef def) {
    byte cmd[COMMAND_BUFFER_LENGTH];
    byte rsp[RESPONSE_BUFFER_LENGTH];

    memset(cmd, 0x0, COMMAND_BUFFER_LENGTH);
    memset(rsp, 0x0, RESPONSE_BUFFER_LENGTH);

    cmd[0] = CMD_GET_NVRAM_PARAM;
    cmd[1] = CMDSUB_USB_KEY_PARAMETERS;

    cmd[4] = def.VID & 0xff;
    cmd[5] = (def.VID & 0xff00) >> 8;

    cmd[6] = def.PID & 0xff;
    cmd[7] = (def.PID & 0xff00) >> 8;

    cmd[8] = def.HostPowered << 7 | def.SelfPowered << 6;
    cmd[9] = def.RequestedCurrentAmountFromHost;

    return SendUSBCmd(handle, cmd, rsp);
    ;
}

ManufacturerProductNameDef GetManufacturerProductName(hid_device *handle, unsigned int subCmdCode) {
    ManufacturerProductNameDef def;

    byte cmd[COMMAND_BUFFER_LENGTH];
    byte rsp[RESPONSE_BUFFER_LENGTH];

    memset(cmd, 0x0, COMMAND_BUFFER_LENGTH);
    memset(rsp, 0x0, RESPONSE_BUFFER_LENGTH);

    cmd[0] = CMD_GET_NVRAM_PARAM;
    cmd[1] = subCmdCode;

    int r = SendUSBCmd(handle, cmd, rsp);

    def.ErrorCode = r;

    if (r == 0) {
        def.USBStringDescriptorLength = rsp[4];
        def.USBStringDescriptorID = rsp[5];

        for (int i = 0; i < def.USBStringDescriptorLength; i++)
            def.ManufacturerProductName[i] = rsp[6 + i];
    }

    return def;
}

int SetManufacturerProductName(hid_device *handle, unsigned int subCmdCode, ManufacturerProductNameDef def) {
    byte cmd[COMMAND_BUFFER_LENGTH];
    byte rsp[RESPONSE_BUFFER_LENGTH];

    memset(cmd, 0x0, COMMAND_BUFFER_LENGTH);
    memset(rsp, 0x0, RESPONSE_BUFFER_LENGTH);

    cmd[0] = CMD_SET_NVRAM_PARAM;
    cmd[1] = subCmdCode;

    cmd[4] = def.USBStringDescriptorLength;
    cmd[5] = 0x3;

    for (int i = 0; i < def.USBStringDescriptorLength; i++)
        cmd[6 + i] = def.ManufacturerProductName[i];

    return SendUSBCmd(handle, cmd, rsp);
}

int SendAccessPassword(hid_device *handle, char *pwd, int pwdLength) {
    byte cmd[COMMAND_BUFFER_LENGTH];
    byte rsp[RESPONSE_BUFFER_LENGTH];

    memset(cmd, 0x0, COMMAND_BUFFER_LENGTH);
    memset(rsp, 0x0, RESPONSE_BUFFER_LENGTH);

    cmd[0] = CMD_SEND_PASSWORD;

    for (int i = 0; i < pwdLength; i++)
        cmd[4 + i] = pwd[i];

    return SendUSBCmd(handle, cmd, rsp);
}

int ReadEEPROM(hid_device *handle, byte addr, byte* val) {
    byte cmd[COMMAND_BUFFER_LENGTH];
    byte rsp[RESPONSE_BUFFER_LENGTH];

    memset(cmd, 0x0, COMMAND_BUFFER_LENGTH);
    memset(rsp, 0x0, RESPONSE_BUFFER_LENGTH);

    cmd[0] = CMD_READ_EEPROM_MEM;
    cmd[1] = addr;

    int r = SendUSBCmd(handle, cmd, rsp);

    if (r == 0) {
        *val = rsp[3];
        return 0;
    } else {
        return ERROR_UNABLE_TO_READ_FROM_DEVICE;
    }
}

int WriteEEPROM(hid_device *handle, byte addr, byte val) {
    byte cmd[COMMAND_BUFFER_LENGTH];
    byte rsp[RESPONSE_BUFFER_LENGTH];

    memset(cmd, 0x0, COMMAND_BUFFER_LENGTH);
    memset(rsp, 0x0, RESPONSE_BUFFER_LENGTH);

    cmd[0] = CMD_WRITE_EEPROM_MEM;
    cmd[1] = addr;
    cmd[2] = val;

    return SendUSBCmd(handle, cmd, rsp);
}

int RequestSPIBusRelease(hid_device *handle, byte val) {
    byte cmd[COMMAND_BUFFER_LENGTH];
    byte rsp[RESPONSE_BUFFER_LENGTH];

    memset(cmd, 0x0, COMMAND_BUFFER_LENGTH);
    memset(rsp, 0x0, RESPONSE_BUFFER_LENGTH);

    cmd[0] = CMD_SPI_BUS_RELEASE_REQ;
    cmd[1] = val;

    return SendUSBCmd(handle, cmd, rsp);
}

ChipStatusDef GetChipStatus(hid_device *handle) {
    ChipStatusDef def;

    byte cmd[COMMAND_BUFFER_LENGTH];
    byte rsp[RESPONSE_BUFFER_LENGTH];

    memset(cmd, 0x0, COMMAND_BUFFER_LENGTH);
    memset(rsp, 0x0, RESPONSE_BUFFER_LENGTH);

    cmd[0] = CMD_GET_CHIP_STATUS;

    int r = SendUSBCmd(handle, cmd, rsp);

    def.ErrorCode = r;

    if (r == 0) {
        def.SPIBusReleaseExtReqStat = rsp[2];
        def.SPIBusCurrentOwner = rsp[3];
        def.AttemptedPWDAccesses = rsp[4];
        def.PasswordGuessed = rsp[5];
    }

    return def;
}

ChipStatusDef CancelSPITransfer(hid_device *handle) {
    ChipStatusDef def;

    byte cmd[COMMAND_BUFFER_LENGTH];
    byte rsp[RESPONSE_BUFFER_LENGTH];

    memset(cmd, 0x0, COMMAND_BUFFER_LENGTH);
    memset(rsp, 0x0, RESPONSE_BUFFER_LENGTH);

    cmd[0] = CMD_SPI_CANCEL;

    int r = SendUSBCmd(handle, cmd, rsp);

    def.ErrorCode = r;

    if (r == 0) {
        def.SPIBusReleaseExtReqStat = rsp[2];
        def.SPIBusCurrentOwner = rsp[3];
        def.AttemptedPWDAccesses = rsp[4];
        def.PasswordGuessed = rsp[5];
    }

    return def;
}

SPIDataTransferStatusDef SPIDataTransfer(hid_device *handle, byte* data, int length) {
    SPIDataTransferStatusDef def;

    byte cmd[COMMAND_BUFFER_LENGTH];
    byte rsp[RESPONSE_BUFFER_LENGTH];

    memset(cmd, 0x0, COMMAND_BUFFER_LENGTH);
    memset(rsp, 0x0, RESPONSE_BUFFER_LENGTH);

    cmd[0] = CMD_SPI_TRANSFER;
    cmd[1] = length;

    for (int i = 0; i < length; i++) cmd[i + 4] = data[i];

    int r = SendUSBCmd(handle, cmd, rsp);

    def.ErrorCode = r;

    if (r == 0) {
        def.NumberOfBytesReceived = rsp[2];
        def.SPIEngineStatus = rsp[3];

        for (int i = 0; i < def.NumberOfBytesReceived; i++)
            def.DataReceived[i] = rsp[i + 4];
    }

    return def;
}

SPIDataTransferStatusDef SPISendReceive(hid_device *handle, byte* data, int cmdBufferLength, int dataLength) {
    SPIDataTransferStatusDef def = SPIDataTransfer(handle, data, cmdBufferLength);

    while (def.SPIEngineStatus == SPI_STATUS_STARTED_NO_DATA_TO_RECEIVE || def.SPIEngineStatus == SPI_STATUS_SUCCESSFUL) {
        if (dataLength > 0)
            def = SPIDataTransfer(handle, data, dataLength);
        else
            def = SPIDataTransfer(handle, data, cmdBufferLength);
    }

    return def;
}

ExternalInterruptPinStatusDef GetNumOfEventsFromInterruptPin(hid_device *handle, byte resetCounter) {
    ExternalInterruptPinStatusDef def;

    byte cmd[COMMAND_BUFFER_LENGTH];
    byte rsp[RESPONSE_BUFFER_LENGTH];

    memset(cmd, 0x0, COMMAND_BUFFER_LENGTH);
    memset(rsp, 0x0, RESPONSE_BUFFER_LENGTH);

    cmd[0] = CMD_GET_NUM_EVENTS_FROM_INT_PIN;
    cmd[1] = resetCounter;

    int r = SendUSBCmd(handle, cmd, rsp);

    def.ErrorCode = r;

    if (r == 0)
        def.InterruptEventCounter = rsp[5] << 8 | rsp[4];

    return def;
}

GPPinDef GetGPIOPinDirection(hid_device *handle) {
    GPPinDef def;

    byte cmd[COMMAND_BUFFER_LENGTH];
    byte rsp[RESPONSE_BUFFER_LENGTH];

    memset(cmd, 0x0, COMMAND_BUFFER_LENGTH);
    memset(rsp, 0x0, RESPONSE_BUFFER_LENGTH);

    cmd[0] = CMD_GET_GPIO_PIN_DIR;

    int r = SendUSBCmd(handle, cmd, rsp);

    def.ErrorCode = r;

    if (r == 0) {
        for (int i = 0; i < 8; i++)
            def.GP[i].GPIODirection = (rsp[4] >> i) & 0x1;

        def.GP[8].GPIODirection = rsp[5] & 0x1;
    }

    return def;
}

int SetGPIOPinDirection(hid_device *handle, GPPinDef def) {
    byte cmd[COMMAND_BUFFER_LENGTH];
    byte rsp[RESPONSE_BUFFER_LENGTH];

    memset(cmd, 0x0, COMMAND_BUFFER_LENGTH);
    memset(rsp, 0x0, RESPONSE_BUFFER_LENGTH);

    cmd[0] = CMD_SET_GPIO_PIN_DIR;

    cmd[4] = 0;
    for (int i = 0; i < 8; i++)
        cmd[4] |= def.GP[i].GPIODirection << i;

    cmd[5] = def.GP[8].GPIODirection;

    return SendUSBCmd(handle, cmd, rsp);
}

GPPinDef GetGPIOPinValue(hid_device *handle) {
    GPPinDef def;

    byte cmd[COMMAND_BUFFER_LENGTH];
    byte rsp[RESPONSE_BUFFER_LENGTH];

    memset(cmd, 0x0, COMMAND_BUFFER_LENGTH);
    memset(rsp, 0x0, RESPONSE_BUFFER_LENGTH);

    cmd[0] = CMD_GET_GPIO_PIN_VAL;

    int r = SendUSBCmd(handle, cmd, rsp);

    def.ErrorCode = r;

    if (r == 0) {
        for (int i = 0; i < 8; i++)
            def.GP[i].GPIOOutput = (rsp[4] >> i) & 0x1;

        def.GP[8].GPIOOutput = rsp[5] & 0x1;
    }

    return def;
}

int SetGPIOPinVal(hid_device *handle, GPPinDef def) {
    byte cmd[COMMAND_BUFFER_LENGTH];
    byte rsp[RESPONSE_BUFFER_LENGTH];

    memset(cmd, 0x0, COMMAND_BUFFER_LENGTH);
    memset(rsp, 0x0, RESPONSE_BUFFER_LENGTH);

    cmd[0] = CMD_SET_GPIO_PIN_VAL;

    cmd[4] = 0;
    for (int i = 0; i < 8; i++)
        cmd[4] |= def.GP[i].GPIOOutput << i;

    cmd[5] = def.GP[8].GPIOOutput;


    return SendUSBCmd(handle, cmd, rsp);
}

hid_device_info* EnumerateMCP2210() {
    return hid_enumerate(MCP2210_VID, MCP2210_PID);
}

hid_device* InitMCP2210(unsigned short vid, unsigned short pid, wchar_t* serialNumber) {
    return hid_open(vid, pid, serialNumber);    
}

hid_device* InitMCP2210(wchar_t* serialNumber) {
    return hid_open(MCP2210_VID, MCP2210_PID, serialNumber);
}

hid_device* InitMCP2210() {
    return hid_open(MCP2210_VID, MCP2210_PID, NULL);
}

void ReleaseMCP2210(hid_device *handle) {
    hid_close(handle);
    hid_exit();
}