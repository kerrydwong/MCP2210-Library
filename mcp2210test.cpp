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

#include "mcp2210.h"

using namespace std;

/**
 * Test the TC77 temperature sensor
 */
void TestTC77(hid_device* handle) {
    ChipSettingsDef chipDef;

    //set GPIO pins to be CS
    chipDef = GetChipSettings(handle);

    for (int i = 0; i < 9; i++) {
        chipDef.GP[i].PinDesignation = GP_PIN_DESIGNATION_CS;
        chipDef.GP[i].GPIODirection = GPIO_DIRECTION_OUTPUT;
        chipDef.GP[i].GPIOOutput = 1;
    }
    int r = SetChipSettings(handle, chipDef);

    //configure SPI
    SPITransferSettingsDef def;
    def = GetSPITransferSettings(handle);

    //chip select is GP7
    def.ActiveChipSelectValue = 0xff7f;
    def.IdleChipSelectValue = 0xffff;
    def.BitRate = 6000000l;
    def.BytesPerSPITransfer = 2;

    r = SetSPITransferSettings(handle, def);

    if (r != 0) {
        printf("Errror setting SPI parameters.\n");
        return;
    }

    byte buf[2];

    SPIDataTransferStatusDef def1 = SPISendReceive(handle, buf, 2);

    int tempVal = 0;
    int sign = def1.DataReceived[0] & 0x80;

    //13 bit 2's complement left aligned (last three bits are all 1's)
    if (sign == 0)
        tempVal = (def1.DataReceived[0] << 8 | def1.DataReceived[1]) >> 3;
    else
        (((def1.DataReceived[0] & 0x7f) << 8 | def1.DataReceived[1]) >> 3) - 4096;

    float tempC = (float) tempVal * 0.0625;
    float tempF = 1.8 * (float) tempC + 32.0f;

    printf("temp (C) = %3.1f\n", tempC);
    printf("temp (F) = %3.1f\n", tempF);
}

/**
 * Test MCP23S08 (parameters used are for the evaluation board)
 */
void TestMCP23S08(hid_device* handle) {
    ChipSettingsDef chipDef;

    //set GPIO pins to be CS
    chipDef = GetChipSettings(handle);

    for (int i = 0; i < 9; i++) {
        chipDef.GP[i].PinDesignation = GP_PIN_DESIGNATION_CS;
        chipDef.GP[i].GPIODirection = GPIO_DIRECTION_OUTPUT;
        chipDef.GP[i].GPIOOutput = 1;
    }
    int r = SetChipSettings(handle, chipDef);

    //configure SPI
    SPITransferSettingsDef def;
    def = GetSPITransferSettings(handle);

    //chip select is GP4
    def.ActiveChipSelectValue = 0xffef;
    def.IdleChipSelectValue = 0xffff;
    def.BitRate = 6000000l;
    def.BytesPerSPITransfer = 3;

    r = SetSPITransferSettings(handle, def);

    if (r != 0) {
        printf("Errror setting SPI parameters.\n");
        return;
    }

    byte spiCmdBuffer[3];

    spiCmdBuffer[0] = 0x40; //device address is 01000A1A0, write
    spiCmdBuffer[1] = 0x00; //write to IODIR register,
    spiCmdBuffer[2] = 0x00; //set all outputs to low

    SPIDataTransferStatusDef def1;

    def1 = SPISendReceive(handle, spiCmdBuffer, 3);

    spiCmdBuffer[0] = 0x40;
    spiCmdBuffer[1] = 0x0a; //write to output latches

    for (int k = 0; k < 10; k++) {
        //lights up LED0 through LED7 one by one
        for (int i = 0; i < 8; i++) {
            spiCmdBuffer[2] = 1 << i;
            SPIDataTransferStatusDef def2 = SPISendReceive(handle, spiCmdBuffer, 3);
            usleep(20000ul);
        }
        //lights up LED7 through LD0 one by one
        for (int i = 0; i < 8; i++) {
            spiCmdBuffer[2] = 0x80 >> i;
            SPIDataTransferStatusDef def2 = SPISendReceive(handle, spiCmdBuffer, 3);
            usleep(20000ul);
        }
    }
}

/**
 * Test GPIO 
 * Generate a rectangular wave on GP0
 */
void TestGPIO(hid_device* handle) {
    ChipSettingsDef chipDef;

    /**
     * Set GPIO to be output
     */
    chipDef = GetChipSettings(handle);

    for (int i = 0; i < 9; i++) {
        chipDef.GP[i].PinDesignation = GP_PIN_DESIGNATION_GPIO;
        chipDef.GP[i].GPIODirection = GPIO_DIRECTION_OUTPUT;
        chipDef.GP[i].GPIOOutput = 0;
    }

    int r0 = SetChipSettings(handle, chipDef);

    /**
     * Configure GPIO0 direction to output
     */
    GPPinDef def = GetGPIOPinDirection(handle);
    def.GP[0].GPIODirection = GPIO_DIRECTION_OUTPUT;
    def.GP[0].PinDesignation = GP_PIN_DESIGNATION_GPIO;

    int r = SetGPIOPinDirection(handle, def);

    ///< Generate a rectangular wave by toggling GP0.
    while (1) {
        def.GP[0].GPIOOutput = 1 - def.GP[0].GPIOOutput;
        r = SetGPIOPinVal(handle, def);
    }
}

/**
 * Running a few tests, you can see the values returned by setting
 * break points and run through the debugger.
 */
void TestMisc(hid_device* handle) {
    SPITransferSettingsDef def1 = GetSPITransferSettings(handle);
    ChipSettingsDef def2 = GetChipSettings(handle);
    USBKeyParametersDef def3 = GetUSBKeyParameters(handle);
    ManufacturerProductNameDef def4 = GetManufacturerProductName(handle, CMDSUB_USB_PRODUCT_NAME);

    for (int i = 0; i < def4.USBStringDescriptorLength; i += 2)
        printf("%c", def4.ManufacturerProductName[i]);
    printf("\n");

    byte c;

    int r = WriteEEPROM(handle, 0, 'T');
    r = ReadEEPROM(handle, 0, &c);

    r = RequestSPIBusRelease(handle, 1);

    ChipStatusDef def5 = GetChipStatus(handle);
    def5 = CancelSPITransfer(handle);

}

int main(int argc, char** argv) {
    hid_device *handle;

    /**
     * initializing the MCP2210 device.
     */
    handle = InitMCP2210();

    if (handle == 0) {
        printf("ERROR opening device. Try using sudo.\n");
        exit(-1);
    }

    //TestGPIO(handle);
    //TestMCP23S08(handle);
    TestTC77(handle);

    /**
     * release the handle
     */
    ReleaseMCP2210(handle);

    return 0;
}