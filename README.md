MCP2210-Library
===============

This is an open source C/C++ library for Microchip's USB-to-SPI protocol converter chip <a href="http://ww1.microchip.com/downloads/en/DeviceDoc/22288A.pdf">MCP2210</a>. It works under Linux. This library is released under <a href="http://www.apache.org/licenses/LICENSE-2.0">Apache License, Version 2.0</a>

The library uses <a href="https://github.com/signal11/hidapi">signal11's hidapi</a>. You can find the library documentations in the doc directory, or you can view online <a href="http://www.kerrywong.com/mcp2210-library-reference/">here</a>. Some code examples are listed below: 

<ul>
<li>
<a href="http://www.kerrywong.com/2012/09/27/mcp2210-library/">MCP2210 Library -- Setup and GPIO</a>
</li>
<li>
<a href="http://www.kerrywong.com/2012/10/01/mcp2210-library-spi-example-using-mcp23s08/">SPI Example Using MCP23S08</a>
</li>
<li>
<a href="http://www.kerrywong.com/2012/10/10/mcp2210-library-spi-example-using-tc77/">SPI Example Using TC77</a>
</li>
<li>
<a href="http://www.kerrywong.com/2012/10/15/mcp2210-library-spi-eeprom/">SPI EEPROM Example</a>
</li>
<li>
<a href="http://www.kerrywong.com/2012/10/22/mcp2210-library-mcp3204-spi-adc/">MCP3204 SPI ADC Example</a>
</li>
</ul>

Note, the compiled code needs root access to the device and need to be run using "sudo". You may need to add the provided udev rule (99-hid.rules) to the /etc/udev/rules.d directory. 
