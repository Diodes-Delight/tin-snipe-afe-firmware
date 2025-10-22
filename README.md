# tin-snipe-afe-firmware
Zephyr firmware for the AFE

# Building

Requirements
----

Setup Toolchain as per Zephyr [Getting Started Guide](https://docs.zephyrproject.org/latest/develop/getting_started/index.html).
The SDK (Zephyr itself) will be pulled in by west specific to this workspace.

For working with the FDRM-MCXA156 board you will need to install the Linkserver tool from NXP: https://www.nxp.com/design/design-center/software/development-software/mcuxpresso-software-and-tools-/linkserver-for-microcontrollers:LINKERSERVER
Alternatively use an external debugger to flash the board. J-Link being the least painful option.

Setup west workspace
----
```bash
cd application
west init -l
west update
```
Build the application
----
```bash
west build -b frdm_mcxa156
```

Optionally do pristine build to purge any cached devicetree stuff
```bash
west build -p -b frdm_mcxa156
```

Flash the application
----
```bash
west flash
```