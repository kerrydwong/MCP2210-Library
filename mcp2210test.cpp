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

#include <cstdlib>
#include "mcp2210.h"

using namespace std;

int main(int argc, char** argv) {
    byte buf[64];
    byte rBuf[64];
    int r = 0;

    hid_device *handle;

    handle = InitHID();

    SPITransferSettingsDef def1 = GetSPITransferSettings(handle);
    ChipSettingsDef def2 = GetChipSettings(handle);
    USBKeyParametersDef def3 = GetUSBKeyParameters(handle);
    ManufacturerProductNameDef def4 = GetManufacturerProductName(handle, CMDSUB_USB_PRODUCT_NAME);

    for (int i = 0; i < def4.USBStringDescriptorLength; i += 2)
        printf("%c", def4.ManufacturerProductName[i]);
    printf("\n");

    byte c;

    r = WriteEEPROM(handle, 0, 'T');
    r = ReadEEPROM(handle, 0, &c);

    r = RequestSPIBusRelease(handle, 1);

    ChipStatusDef def5 = GetChipStatus(handle);
    def5 = CancelSPITransfer(handle);

    GPPinDef def6 = GetGPIOPinDirection(handle);
    def6.GP[0].GPIODirection = 0;
    r = SetGPIOPinDirection(handle, def6);
    def6 = GetGPIOPinValue(handle);
 
    ///< Generate a square wave by toggling GP0.
    while (1)
    {
        def6.GP[0].GPIOOutput = 1-def6.GP[0].GPIOOutput;
        r = SetGPIOPinVal(handle, def6);
    }

    ReleaseHID(handle);

    return 0;
    
}

