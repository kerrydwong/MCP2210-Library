MCP2210-Library
===============

This is a C/C++ library for Microchip's USB-to-SPI protocol converter chip <a href="http://ww1.microchip.com/downloads/en/DeviceDoc/22288A.pdf">MCP2210</a>. It works under Linux.

The library uses <a href="https://github.com/signal11/hidapi">signal11's hidapi</a>. You can find the library documentations in the doc directory.

<ul>
<li>
<a href="http://www.kerrywong.com/2012/09/27/mcp2210-library/">MCP2210 Library</a>
</li>
</ul>

Note, the compiled code needs root access to the device and need to be run using "sudo". Alternatively, please refer to signal11's instructions on how to change the device rules.

More examples will be available on my <a href="http://www.kerrywong.com">blog</a>.
