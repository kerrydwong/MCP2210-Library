/* 
 * File:   mcp2210test.cpp
 * Author: root
 *
 * Created on September 24, 2012, 11:47 AM
 */

#include <cstdlib>
#include "mcp2210.h"

using namespace std;

/*
 * 
 */
int main(int argc, char** argv) {
    byte buf[64];
    byte rBuf[64];
    int r = 0;

    hid_device *handle;

    handle = InitHID();

    SPITransferSettingsDef def1 = GetSPITransferSettings(handle);
    ChipSettingsDef def2 = GetChipSettings(handle);
    USBKeyParametersDef def3 = GetUSBKeyParameters(handle);
    USBManufacturerProductNameDef def4 = GetUSBManufacturerProductName(handle, CMDSUB_USB_PRODUCT_NAME);

    for (int i = 0; i < def4.USBStringDescriptorLength; i += 2)
        printf("%c", def4.ManufacturerProductName[i]);
    printf("\n");

    byte c;

    r = WriteEEPROM(handle, 0, 'T');
    r = ReadEEPROM(handle, 0, &c);

    r = RequestSPIBusRelease(handle, 1);

    ChipStatusDef def5 = GetChipStatus(handle);
    def5 = CancelCurrentSPITransfer(handle);

    GPPinDef def6 = GetGPIOCurrentPinDirection(handle);
    def6.GP[0].GPIODirection = 0;
    r = SetGPIOCurrentPinDirection(handle, def6);
    def6 = GetGPIOCurrentPinValue(handle);
 
    while (1)
    {
        def6.GP[0].GPIOOutput = 1-def6.GP[0].GPIOOutput;
        r = SetGPIOCurrentPinVal(handle, def6);
    }

    ReleaseHID(handle);

    return 0;
    
}

