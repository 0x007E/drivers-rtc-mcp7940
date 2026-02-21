[![Version: 1.0 Release](https://img.shields.io/badge/Version-1.0%20Release-green.svg)](https://github.com/0x007e/drivers-rtc-mcp7940) ![Build](https://github.com/0x007e/drivers-rtc-mcp7940/actions/workflows/release.yml/badge.svg) [![License GPLv3](https://img.shields.io/badge/License-GPLv3-lightgrey)](https://www.gnu.org/licenses/gpl-3.0.html)

# `MCP7940 RTC Driver`

[![Ask DeepWiki](https://deepwiki.com/badge.svg)](https://deepwiki.com/0x007E/drivers-rtc-mcp7940)

This hardware abstracted driver can be used to interact with an [MCP7940](#additional-information) over `TWI`/`I2C`. The hardware layer is fully abstract an can be switched between different plattforms. The `TWI`/`I2C` library has to impelement the [twi.h](https://0x007e.github.io/drivers-rtc-mcp7940/twi_8c.html)-header used in this repository.

## File Structure

![File Structure](https://0x007e.github.io/drivers-rtc-mcp7940/mcp7940_8c__incl.png)

```
drivers/
└── rtc/
    └── mcp7940/
        ├── mcp7940.c
        └── mcp7940.h

hal/
├── common/
|   ├── defines/
|   |   └── TWI_defines.h
|   └── enums/
|       └── TWI_enums.h
└── avr0/
    └── twi/
        ├── twi.c
        └── twi.h

core_types/
├── format/
|   └── time.h
└── return/
    └── status.h

utils/
├── macros/
|   └── stringify.h
└── systick/
    ├── systick.c
    └── systick.h
```

> The plattform `avr0` can completely be exchanged with any other hardware abstraction library.

## Downloads

The library can be downloaded (`zip` or `tar`), cloned or used as submodule in a project.

| Type      | File               | Description              |
|:---------:|:------------------:|:-------------------------|
| Library   | [zip](https://github.com/0x007E/drivers-rtc-mcp7940/releases/latest/download/library.zip) / [tar](https://github.com/0x007E/drivers-rtc-mcp7940/releases/latest/download/library.tar.gz) | MCP7940 rtc library including all required libraries (including `hal-avr0-twi`). |

### Using with `git clone`

```sh
mkdir -p ./core_types/
git clone https://github.com/0x007E/core_types-format.git ./core_types
mv ./core_types/core_types-format ./core_types/format
git clone https://github.com/0x007E/core_types-return.git ./core_types
mv ./core_types/core_types-return ./core_types/return

mkdir -p ./drivers/rtc/
git clone https://github.com/0x007E/drivers-rtc-mcp7940.git ./drivers/rtc
mv ./drivers/rtc/drivers-rtc-mcp7940 ./drivers/rtc/mcp7940

mkdir -p ./hal/
git clone https://github.com/0x007E/hal-common.git ./hal
mv ./hal/hal-common ./hal/common

# !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
# Hardware abstraction layer of TWI (Must fit the used plattform)
mkdir -p ./hal/avr0/
git clone https://github.com/0x007E/hal-avr0-twi.git ./hal/avr0
mv ./hal/avr0/hal-avr0-twi ./hal/avr0/twi
# !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

mkdir -p ./utils/
git clone https://github.com/0x007E/utils-macros.git  ./utils
git clone https://github.com/0x007E/utils-systick.git ./utils
git clone https://github.com/0x007E/utils-time.git    ./utils
mv ./utils/utils-macros  ./utils/macros
mv ./utils/utils-systick ./utils/systick
mv ./utils/utils-time    ./utils/time
```

### Using as `git submodule`

```sh
git submodule add https://github.com/0x007E/core_types-format.git         core_types/format
git submodule add https://github.com/0x007E/core_types-return.git         core_types/return

git submodule add https://github.com/0x007E/drivers-rtc-mcp7940.git   drivers/rtc/mcp7940
git submodule add https://github.com/0x007E/hal-common.git            hal/common

# !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
# Hardware abstraction layer of TWI (Must fit the used plattform)
git submodule add https://github.com/0x007E/hal-avr0-twi.git          hal/avr0/twi
# !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

git submodule add https://github.com/0x007E/utils-macros.git          utils/macros
git submodule add https://github.com/0x007E/utils-systick.git         utils/systick
git submodule add https://github.com/0x007E/utils-time.git            utils/time
```

## Programming

```c
#include "./hal/avr0/twi/twi.h"
#include "./utils/systick/systick.h"
#include "./drivers/rtc/mcp7940/mcp7940.h"

ISR(...)
{
    systick_tick();
}

void systick_timer_wait_ms(unsigned int ms)
{
    systick_timer_wait(ms);
}

int main(void)
{	
    systick_init();
    twi_init();

    // Only works with #define MCP7940_BATTERY_BACKUP_EN enabled!!!
    if(mcp7940_status() & MCP7940_Status_Power_Fail)
    {
        FORMAT_DateTime power_down;
        mcp7940_datetime(&power_down, MCP7940_Register_Power_Down_Time);

        // Do something with the time

        FORMAT_DateTime power_up;
        mcp7940_datetime(&power_up, MCP7940_Register_Power_Up_Time);

        // Do something with the time
    }
    
    mcp7940_trimming(MCP7940_Trim_Add, 0x01);
    
    // Set date and time
    // For data integrity the time and date should be set before the clock starts running.
    FORMAT_Date d = { 17, 2, 26 };
	mcp7940_setdate(&d);
	
	FORMAT_Time t = { 13, 37, 0 };
	mcp7940_settime(&t);
	
	FORMAT_DateTime dt = {
		{ 17, 2, 26},
		{ 13, 37, 0 }
	};
	mcp7940_setdatetime(&dt);

    // Set weekday
	mcp7940_setweekday(MCP7940_WEEKDAY_TUESDAY_gc);

    mcp7940_init();
    
    // Blocking method to check if oscillator is running
    while(!(mcp7940_status() & MCP7940_Status_Oscillator_Running))
	{
		asm volatile("NOP");
	}

    // Non-blocking method to check if oscillator is running
    systick_timer_wait_ms(MCP7940_OSC_ENABLE_MS);
    
    // Check if Oscillator is Running
    if(!(mcp7940_status() & MCP7940_Status_Oscillator_Running))
    {
        // Error -> Oscillator is not running!
    }

    // Fetch time and date from RTC
    FORMAT_Time time;
    mcp7940_time(&time, MCP7940_Register_Current_Time);
    
    FORMAT_Date date;
    mcp7940_date(&date, MCP7940_Register_Current_Time);
    
    FORMAT_DateTime datetime;
    mcp7940_datetime(&datetime, MCP7940_Register_Current_Time);

    // Fetch weekday from RTC
	unsigned char wkday = mcp7940_weekday(MCP7940_Register_Current_Time);

    // Weekday as string
    mcp7940_weekday_string(mcp7940_weekday(MCP7940_Register_Current_Time));

    // Check if current year is a leap year
    if(mcp7940_leapyear() == MCP7940_LeapYear_True)
    {
        
    }
```

# Additional Information

| Type       | Link               | Description              |
|:----------:|:------------------:|:-------------------------|
| MCP7940 | [pdf](https://ww1.microchip.com/downloads/en/devicedoc/20005010f.pdf) | Battery-Backed TWI/I2C Real-Time Clock/Calendar with SRAM |

---

R. GAECHTER