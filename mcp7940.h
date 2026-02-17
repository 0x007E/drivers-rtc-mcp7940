/**
 * @file mcp7940.h
 * @brief Header file with declarations and macros for driving an mcp7940.
 * 
 * This file provides function prototypes, type definitions, and constants for communication with an mcp7940 rtc chip.
 * 
 * @author g.raf
 * @date 2026-02-13
 * @version 1.0 Release
 * @copyright
 * Copyright (c) 2026 g.raf
 * Released under the GPLv3 License. (see LICENSE in repository)
 * 
 * @note This file is part of a larger project and subject to the license specified in the repository. For updates and the complete revision history, see the GitHub repository.
 * 
 * @see https://github.com/0x007e/drivers-rtc-mcp7940 "MCP7940 RTC driver library"
 */

#ifndef MCP7940_H_
#define MCP7940_H_
    
    #ifndef MCP7940_HAL_PLATFORM
        /**
         * @def MCP7940_HAL_PLATFORM
         * @brief Sets the target platform for the MCP7940 hardware abstraction layer (HAL), e.g., avr or avr0
         * 
         * @details
         * Define this macro with the name of the target platform to select the corresponding platform-specific HAL implementation (such as TWI communication functions) for the MCP7940 RTC driver. Common values are avr (classic AVR architecture) or avr0 (AVR0/1 series).
         * 
         * @note Set this macro as a global compiler symbol to ensure that the correct HAL implementation is used across the entire project.
        */
        #define MCP7940_HAL_PLATFORM avr0
    #endif

    #ifndef MCP7940_ADDRESS
        /**
         * @def MCP7940_ADDRESS
         * @brief Defines the TWI/I2C address of the MCP7940 device.
         *
         * @details
         * This macro specifies the 7-bit I2C slave address used to communicate with the MCP7940 device on the TWI/I2C bus. The value can be overridden by defining `MCP7940_ADDRESS` before including this header if the hardware configuration uses a different address.
         *
         * @note By default, `MCP7940_ADDRESS` is set to `0x6F`.
         */
        #define MCP7940_ADDRESS 0x6F
    #endif

    #ifndef MCP7940_USE_EXTOSC
        /** 
         * @def MCP7940_USE_EXTOSC
         * @brief Selects external clock input mode for the MCP7940 RTC device.
         *
         * This macro enables operation of the MCP7940 using an external 32.768 kHz clock signal applied to the X1 pin instead of the crystal oscillator. When this macro is defined, the driver configures the device to use the external clock input; when it is not defined, the MCP7940 is assumed to run from a standard external 32.768 kHz crystal connected to the oscillator pins (X1/X2).
         *
         * @note Define `MCP7940_USE_EXTOSC` in the project configuration if the hardware provides a clock signal source on X1. Leave it undefined (default) when a crystal is used so that the driver configures the oscillator accordingly.
         */
        //#define MCP7940_USE_EXTOSC

        #ifdef _DOXYGEN_    // Used for documentation, can be ignored
            #define MCP7940_USE_EXTOSC
        #endif
    #endif

    #ifndef MCP7940_BATTERY_BACKUP_EN
        /** 
         * @def MCP7940_BATTERY_BACKUP_EN
         * @brief Enables battery backup operation and power-fail time-stamping in the MCP7940 RTC.
         *
         *  When this macro is defined, the driver enables the VBAT battery backup feature of the MCP7940 by setting the VBATEN bit in the RTC weekday register. This allows the device to automatically switch to the VBAT supply on main power loss, maintain timekeeping and RTC/SRAM contents, and record power-down and power-up timestamps in the dedicated PWRDN and PWRUP registers.
         *
         *  If this macro is not defined, the VBAT pin is treated as unused (or must be tied to GND), and the battery backup and power-fail logging features are disabled to minimize leakage current.
         *
         *  @note Define `MCP7940_BATTERY_BACKUP_EN` in the project configuration if the hardware provides a backup battery on the VBAT pin and power-fail time-stamping is desired. Leave it undefined (default) when no battery backup is available or required.
         */
        //#define MCP7940_BATTERY_BACKUP_EN

        #ifdef _DOXYGEN_    // Used for documentation, can be ignored
            #define MCP7940_BATTERY_BACKUP_EN
        #endif
    #endif


    #ifndef MCP7940_MFP_MODE
        #define MCP7940_MFP_MODE_OUTPUT      0x00
        #define MCP7940_MFP_MODE_SQUARE_WAVE 0x01
        #define MCP7940_MFP_MODE_ALARM       0x02

        /**
         * @def MCP7940_MFP_MODE
         * @brief Selects the operating mode of the MCP7940 multi-function pin (MFP).
         *
         *  This macro specifies how the MFP pin of the MCP7940 RTC is used by the driver. Depending on the selected value, the pin can be disabled and used as a general-purpose output, configured as a square-wave (CLKOUT) output, or used as an alarm/interrupt output driven by the internal alarm logic.
         *
         * The following mode values are available:
         *  - MCP7940_MFP_MODE_OUTPUT:      Acts as a general-purpose output
         *  - MCP7940_MFP_MODE_SQUARE_WAVE: Outputs a square wave at the frequency defined below.
         *  - MCP7940_MFP_MODE_ALARM:       Is asserted by the alarm circuitry according to the defined alarm settings.
         *
         * @note Default set to MCP7940_MFP_MODE_OUTPUT so that the MFP pin can be used as a simple GPIO-like output controlled via library functions.
         */
        #define MCP7940_MFP_MODE MCP7940_MFP_MODE_OUTPUT
    #endif

    #ifndef MCP7940_SQW_CRSTRIM_EN
        /**
         * @def MCP7940_SQW_CRSTRIM_EN
         * @brief Enables coarse trimming for the MCP7940 square-wave output.
         *
         * This macro is defined when coarse trimming support (`MCP7940_SQW_CRSTRIM_EN`) is enabled in the build configuration. When present, the driver may use the MCP7940 coarse trim functionality to adjust the frequency of the square-wave (CLKOUT) output in addition to any fine-trim settings.
         *
         * @warning When coarse trim mode is active and the square-wave output is enabled, the MFP pin can only generate a 64 Hz nominal square-wave signal, as the trimming is applied on each 64 Hz clock cycle.
         *
         * @note Define `MCP7940_SQW_CRSTRIM_EN` in the project configuration to enable coarse trimming support and activate `MCP7940_SQW_CRSTRIM_EN` in the driver.
         */
        //#define MCP7940_SQW_CRSTRIM_EN
    #endif

    #ifndef MCP7940_CRSTRIM_EN
        #ifndef MCP7940_MFP_SQUARE_WAVE_PRESCALER
            /**
             * @def MCP7940_MFP_SQUARE_WAVE_PRESCALER
             * @brief Selects the output frequency of the MCP7940 square-wave signal on the MFP pin.
             *
             * This macro defines the prescaler setting used to generate the square-wave (CLKOUT) output on the MFP pin when coarse trimming support (`MCP7940_CRSTRIM_EN`) is disabled. It determines the nominal frequency of the square-wave derived from the 32.768 kHz time base and is applied when the MFP pin is configured for square-wave mode.
             *
             * The following prescaler values are available:
             *  - RTC_SQWFS_32768HZ: Output frequency of 32.768 kHz.
             *  - RTC_SQWFS_8192HZ:  Output frequency of 8.192 kHz.
             *  - RTC_SQWFS_4096HZ:  Output frequency of 4.096 kHz.
             *  - RTC_SQWFS_1HZ:     Output frequency of 1 Hz.
             *
             * @note If `MCP7940_CRSTRIM_EN` is not defined and `MCP7940_MFP_SQUARE_WAVE_PRESCALER` is not explicitly set in the project configuration, it defaults to `RTC_SQWFS_1HZ`, providing a 1 Hz square-wave output on the MFP pin when square-wave mode is enabled.
             */
            #define MCP7940_MFP_SQUARE_WAVE_PRESCALER MCP7940_SQWFS_1HZ
        #endif
    #endif

    #ifndef MCP7940_MFP_ALARM_MODE
        #define MCP7940_MFP_ALARM_MODE_ALM0     (MCP7940_ALM0EN_bm)
        #define MCP7940_MFP_ALARM_MODE_ALM1     (MCP7940_ALM1EN_bm)
        #define MCP7940_MFP_ALARM_MODE_BOTH     (MCP7940_ALM1EN_bm | MCP7940_ALM0EN_bm)

        /**
         * @def MCP7940_MFP_ALARM_MODE
         * @brief Selects which RTC alarm sources can assert the MCP7940 multi-function pin (MFP).
         *
         *  This macro configures the alarm routing for the MFP pin when it is used in alarm/interrupt mode. It is defined in terms of the available alarm mode selector macros and determines whether Alarm 0, Alarm 1, or both alarms are allowed to drive the MFP pin.
         *
         * The following mode values are available:
         *  - MCP7940_MFP_ALARM_MODE_ALM0:  MFP is asserted only by Alarm 0 events.
         *  - MCP7940_MFP_ALARM_MODE_ALM1:  MFP is asserted only by Alarm 1 events.
         *  - MCP7940_MFP_ALARM_MODE_BOTH:  MFP is asserted by both Alarm 0 and Alarm 1 events.
         *
         * @note If MCP7940_MFP_ALARM_MODE is not explicitly defined in the project configuration, it defaults to MCP7940_MFP_ALARM_MODE_BOTH so that both Alarm 0 and Alarm 1 can activate the MFP 
         */
        #define MCP7940_MFP_ALARM_MODE MCP7940_MFP_ALARM_MODE_BOTH
    #endif

    #define MCP7940_MFP_ALARM_POLARITY_NORMAL 0x00
    #define MCP7940_MFP_ALARM_POLARITY_INVERTED RTC_ALMPOL_bm

    #ifndef MCP7940_MFP_ALARM1_POLARITY

        /**
         * @def MCP7940_MFP_ALARM1_POLARITY
         * @brief Configures the output polarity of the MCP7940 multi-function pin (MFP) when driven by Alarm 1 events.
         *
         * This macro determines whether the MFP pin is asserted in an active-high or active-low manner when Alarm 1 conditions occur. It is defined in terms of the available polarity selector macros and allows for flexible integration with different types of interrupt inputs or signaling requirements.
         *
         * The following polarity values are available:
         *  - MCP7940_MFP_ALARM_POLARITY_NORMAL:  MFP is active-high (asserted high on alarm events).
         *  - MCP7940_MFP_ALARM_POLARITY_INVERTED: MFP is active-low (asserted low on alarm events).
         *
         * @note If `MCP7940_MFP_ALARM_POLARITY` is not explicitly defined in the project configuration, it defaults to `MCP7940_MFP_ALARM_POLARITY_NORMAL`, meaning that the MFP pin will be active-high when driven by Alarm 1 events.
         */
        #define MCP7940_MFP_ALARM1_POLARITY MCP7940_MFP_ALARM_POLARITY_NORMAL
    #endif
    
    #ifndef MCP7940_MFP_ALARM2_POLARITY

        /**
         * @def MCP7940_MFP_ALARM2_POLARITY
         * @brief Configures the output polarity of the MCP7940 multi-function pin (MFP) when driven by Alarm 2 events.
         *
         * This macro determines whether the MFP pin is asserted in an active-high or active-low manner when Alarm 2 conditions occur. It is defined in terms of the available polarity selector macros and allows for flexible integration with different types of interrupt inputs or signaling requirements.
         *
         * The following polarity values are available:
         *  - MCP7940_MFP_ALARM_POLARITY_NORMAL:  MFP is active-high (asserted high on alarm events).
         *  - MCP7940_MFP_ALARM_POLARITY_INVERTED: MFP is active-low (asserted low on alarm events).
         *
         * @note If `MCP7940_MFP_ALARM_POLARITY` is not explicitly defined in the project configuration, it defaults to `MCP7940_MFP_ALARM_POLARITY_NORMAL`, meaning that the MFP pin will be active-high when driven by Alarm 2 events.
         */
        #define MCP7940_MFP_ALARM2_POLARITY MCP7940_MFP_ALARM_POLARITY_NORMAL
    #endif

    #ifndef MCP7940_IO_TIMEOUT_MS
        /**
         * @def MCP7940_IO_TIMEOUT_MS
         * @brief Specifies the I/O timeout for MCP7940 bus operations in milliseconds.
         *
         * This macro defines the maximum time the driver will wait for a TWI/I2C transaction with the MCP7940 device to complete before treating it as a timeout condition. It can be adjusted to match the timing requirements and performance characteristics of the target platform and bus speed.
         *
         * @note If MCP7940_IO_TIMEOUT_MS is not explicitly defined in the project configuration, it defaults to 1 ms.
         */
        #define MCP7940_IO_TIMEOUT_MS 1
    #endif
    
    #ifndef MCP7940_OSC_ENABLE_MS
        /**
         * @def MCP7940_OSC_ENABLE_MS
         * @brief Delay time in milliseconds before oscillator run check after enabling the MCP7940 oscillator.
         *
         *  Specifies the period after enabling the oscillator (ST/EXTOSC) before the OSCRUN signals that the MCP7940 oscillator is running. During this delay, the device needs time to complete the required number of clock cycles before the status bit (OSCRUN) is asserted.
         *
         *  The MCP7940 requires approximately 32 cycles of the 32.768 kHz time base before OSCRUN goes high, so MCP7940_OSC_ENABLE_MS must be at least (32 / 32768) s ≈ 0.98 ms. Choosing a slightly larger value (for example 1–2 ms) provides additional margin for crystal start-up variation.
         *
         *  @note If MCP7940_OSC_ENABLE_MS is not explicitly defined in the project configuration, it should default to a value ≥ 1 ms to guarantee that the OSCRUN bit has time to assert before the driver continues.
         */
        #define MCP7940_OSC_ENABLE_MS 1000UL
    #endif
    
    #ifndef MCP7940_RTCSEC
        /**
         * @def MCP7940_RTCSEC
         * @brief Address of the MCP7940 RTC seconds register.
         *
         * This macro defines the register address of the seconds register, which stores the current seconds value in BCD format and contains control bits related to the RTC oscillator.
         */
        #define MCP7940_RTCSEC 0x00

        #define MCP7940_ST_bm     0x80 /**< Oscillator start bit mask (ST) in the seconds register. */
        #define MCP7940_SECTEN_bm 0x70 /**< Bit mask for the tens-of-seconds BCD field in the seconds register. */
        #define MCP7940_SECTEN_bp 4    /**< Bit position of the least significant bit of the tens-of-seconds field. */
    #endif

    #ifndef MCP7940_RTCMIN
        /**
         * @def MCP7940_RTCMIN
         * @brief Address of the MCP7940 RTC minutes register.
         *
         * This macro defines the register address of the minutes register, which stores the current minutes value in BCD format.
         */
        #define MCP7940_RTCMIN 0x01

        #define MCP7940_MINTEN_bm 0x70 /**< Bit mask for the tens-of-minutes BCD field in the minutes register. */
        #define MCP7940_MINTEN_bp 4    /**< Bit position of the least significant bit of the tens-of-minutes field. */
    #endif

    #ifndef MCP7940_RTCHOUR
        /**
         * @def MCP7940_RTCHOUR
         * @brief Address of the MCP7940 RTC hours register.
         *
         * This macro defines the register address of the hours register, which stores the current hours value in BCD format and includes the hour format selection bit (12/24-hour mode).
         */
        #define MCP7940_RTCHOUR 0x02

        #define MCP7940_FORMAT_bm 0x40 /**< Bit mask for the hour format selection bit (12/24-hour mode) in the hours register. */
        #define MCP7940_HRTEN_bm  0x30 /**< Bit mask for the tens-of-hours BCD field in the hours register. */
        #define MCP7940_HRTEN_bp  4    /**< Bit position of the least significant bit of the tens-of-hours field. */
    #endif

    #ifndef MCP7940_RTCWKDAY
        /**
         * @def MCP7940_RTCWKDAY
         * @brief Address of the MCP7940 RTC weekday register.
         *
         * This macro defines the register address of the weekday register, which stores the current day of week and several status/control flags related to oscillator state, power failure detection, and battery backup.
         */
        #define MCP7940_RTCWKDAY 0x03

        #define MCP7940_OSCRUN_bm  0x20 /**< Bit mask for the OSCRUN flag indicating that the oscillator is running. */
        #define MCP7940_PWRFAIL_bm 0x10 /**< Bit mask for the PWRFAIL flag indicating that a power failure has occurred. */
        #define MCP7940_VBATEN_bm  0x08 /**< Bit mask for the VBATEN flag enabling battery-backed timekeeping. */
        #define MCP7940_WKDAY_bp   0    /**< Bit position of the least significant bit of the weekday BCD field. */
    #endif

    #ifndef MCP7940_RTCDATE
        /**
         * @def MCP7940_RTCDATE
         * @brief Address of the MCP7940 RTC date (day-of-month) register.
         *
         * This macro defines the register address of the date register, which stores the current day-of-month value in BCD format.
         */
        #define MCP7940_RTCDATE 0x04

        #define MCP7940_DATETEN_bm 0x30 /**< Bit mask for the tens-of-day BCD field in the date register. */
        #define MCP7940_DATETEN_bp 4    /**< Bit position of the least significant bit of the tens-of-day field. */
    #endif

    #ifndef MCP7940_RTCMTH
        /**
         * @def MCP7940_RTCMTH
         * @brief Address of the MCP7940 RTC month register.
         *
         * This macro defines the register address of the month register, which stores the current month value in BCD format and includes the leap-year indicator bit.
         */
        #define MCP7940_RTCMTH 0x05

        #define MCP7940_LPYR_bm   0x20 /**< Bit mask for the leap-year indicator (LPYR) in the month register. */
        #define MCP7940_MTHTEN_bm 0x10 /**< Bit mask for the tens-of-month BCD field in the month register. */
        #define MCP7940_LPYR_bp   5    /**< Bit position of the leap-year indicator (LPYR) in the month register. */
        #define MCP7940_MTHTEN_bp 4    /**< Bit position of the least significant bit of the tens-of-month field. */
    #endif

    #ifndef MCP7940_RTCYEAR
        /**
         * @def MCP7940_RTCYEAR
         * @brief Address of the MCP7940 RTC year register.
         *
         * This macro defines the register address of the year register, which stores the current year value (last two digits) in BCD format.
         */
        #define MCP7940_RTCYEAR 0x06

        #define MCP7940_YRTEN_bm 0xF0 /**< Bit mask for the tens-of-years BCD field in the year register. */
        #define MCP7940_YRTEN_bp 4    /**< Bit position of the least significant bit of the tens-of-years field. */
    #endif

    #ifndef MCP7940_CONTROL
        /**
         * @def MCP7940_CONTROL
         * @brief Address of the MCP7940 RTC control register.
         *
         * This macro defines the register address of the control register, which provides control bits for the MFP output, square-wave generation, alarm enables, oscillator source selection, and coarse trimming.
         */
        #define MCP7940_CONTROL 0x07

        #define MCP7940_OUT_bm    0x80 /**< Bit mask for the OUT control bit, which drives the MFP output level in output mode. */
        #define MCP7940_SQWEN_bm  0x40 /**< Bit mask for the SQWEN bit, enabling or disabling the square-wave output on MFP. */
        #define MCP7940_ALM1EN_bm 0x20 /**< Bit mask for the ALM1EN bit, enabling Alarm 1 to assert the MFP pin. */
        #define MCP7940_ALM0EN_bm 0x10 /**< Bit mask for the ALM0EN bit, enabling Alarm 0 to assert the MFP pin. */
        #define MCP7940_EXTOSC_bm 0x08 /**< Bit mask for the EXTOSC bit, selecting an external clock input instead of the crystal. */
        #define MCP7940_CSTRIM_bm 0x04 /**< Bit mask for the CSTRIM bit, enabling coarse trimming of the oscillator. */
        #define MCP7940_SQWFS1_bm 0x02 /**< Bit mask for the SQWFS1 bit, high bit of the square-wave frequency select field. */
        #define MCP7940_SQWFS0_bm 0x01 /**< Bit mask for the SQWFS0 bit, low bit of the square-wave frequency select field. */

        #define MCP7940_SQWFS_32768HZ (MCP7940_SQWFS1_bm | MCP7940_SQWFS0_bm)   /**< Square-wave frequency select: 32.768 kHz output. */
        #define MCP7940_SQWFS_8192HZ  MCP7940_SQWFS1_bm                             /**< Square-wave frequency select: 8.192 kHz output. */
        #define MCP7940_SQWFS_4096HZ  MCP7940_SQWFS0_bm                             /**< Square-wave frequency select: 4.096 kHz output. */
        #define MCP7940_SQWFS_1HZ     0x00                                              /**< Square-wave frequency select: 1 Hz output. */
    #endif

    #ifndef MCP7940_OSCTRIM
        /**
         * @def MCP7940_OSCTRIM
         * @brief Address of the MCP7940 oscillator trim register.
         *
         * This macro defines the register address of the oscillator trim register, which is used to apply fine or coarse trimming adjustments to the RTC oscillator frequency.
         */
        #define MCP7940_OSCTRIM 0x08

        #define MCP7940_SIGN_bm 0x80 /**< Bit mask for the SIGN bit indicating the trim direction (positive or negative adjustment). */
    #endif

    // #############################################

    #ifndef MCP7940_WEEKDAY_bp
        #define MCP7940_WEEKDAY_MONDAY_gc    0x00 /**< Encoded weekday value for Monday */
        #define MCP7940_WEEKDAY_TUESDAY_gc   0x01 /**< Encoded weekday value for Tuesday */
        #define MCP7940_WEEKDAY_WEDNESDAY_gc 0x02 /**< Encoded weekday value for Wednesday */
        #define MCP7940_WEEKDAY_THURSDAY_gc  0x03 /**< Encoded weekday value for Thursday */
        #define MCP7940_WEEKDAY_FRIDAY_gc    0x04 /**< Encoded weekday value for Friday */
        #define MCP7940_WEEKDAY_SATURDAY_gc  0x05 /**< Encoded weekday value for Saturday */
        #define MCP7940_WEEKDAY_SUNDAY_gc    0x06 /**< Encoded weekday value for Sunday */
    #endif

    #if !defined(MCP7940_ALM0SEC) && !defined(MCP7940_ALM1SEC)
        #define MCP7940_ALM_SECTEN_bp 4 /**< Bit position of the least significant bit of the tens-of-seconds BCD field in the alarm seconds registers. */
        #define MCP7940_ALM_SECONE_bp 0 /**< Bit position of the least significant bit of the ones-of-seconds BCD field in the alarm seconds registers. */

        /**
         * @def MCP7940_ALM0SEC
         * @brief Address of the MCP7940 Alarm 0 seconds register.
         *
         * This macro defines the register address of the Alarm 0 seconds register, which stores the seconds value for Alarm 0 in BCD format.
         */
        #define MCP7940_ALM0SEC 0x0A

        /**
         * @def MCP7940_ALM1SEC
         * @brief Address of the MCP7940 Alarm 1 seconds register.
         *
         * This macro defines the register address of the Alarm 1 seconds register, which stores the seconds value for Alarm 1 in BCD format.
         */
        #define MCP7940_ALM1SEC 0x11
    #endif

    #if !defined(MCP7940_ALM0MIN) && !defined(MCP7940_ALM1MIN)
        #define MCP7940_ALM_MINTEN_bp 4 /**< Bit position of the least significant bit of the tens-of-minutes BCD field in the alarm minutes registers. */
        #define MCP7940_ALM_MINONE_bp 0 /**< Bit position of the least significant bit of the ones-of-minutes BCD field in the alarm minutes registers. */

        /**
         * @def MCP7940_ALM0MIN
         * @brief Address of the MCP7940 Alarm 0 minutes register.
         *
         * This macro defines the register address of the Alarm 0 minutes register, which stores the minutes value for Alarm 0 in BCD format as part of the alarm time configuration.
         */
        #define MCP7940_ALM0MIN 0x0B

        /**
         * @def MCP7940_ALM1MIN
         * @brief Address of the MCP7940 Alarm 1 minutes register.
         *
         * This macro defines the register address of the Alarm 1 minutes register, which stores the minutes value for Alarm 1 in BCD format as part of the alarm time configuration.
         */
        #define MCP7940_ALM1MIN 0x12
    #endif

    #if !defined(MCP7940_ALM0HOUR) && !defined(MCP7940_ALM1HOUR)
        #define MCP7940_ALM_FORMAT_bm 0x40 /**< Bit mask for the hour format selection bit (12/24-hour mode) in the alarm hours registers. */
        #define MCP7940_ALM_HRTEN_bp  4    /**< Bit position of the least significant bit of the tens-of-hours BCD field in the alarm hours registers. */
        #define MCP7940_ALM_HRONE_bp  0    /**< Bit position of the least significant bit of the ones-of-hours BCD field in the alarm hours registers. */

        /**
         * @def MCP7940_ALM0HOUR
         * @brief Address of the MCP7940 Alarm 0 hours register.
         *
         * This macro defines the register address of the Alarm 0 hours register, which stores the hours value for Alarm 0 in BCD format and includes the hour format selection bit as part of the alarm time configuration.
         */
        #define MCP7940_ALM0HOUR 0x0C

        /**
         * @def MCP7940_ALM1HOUR
         * @brief Address of the MCP7940 Alarm 1 hours register.
         *
         * This macro defines the register address of the Alarm 1 hours register, which stores the hours value for Alarm 1 in BCD format and includes the hour format selection bit as part of the alarm time configuration.
         */
        #define MCP7940_ALM1HOUR 0x13
    #endif

    #if !defined(MCP7940_ALM0WKDAY) && !defined(MCP7940_ALM1WKDAY)
        #define MCP7940_ALARM_ALMPOL_bm  0x80   /**< Bit mask for the ALMPOL bit controlling the active polarity of the alarm output on MFP. */
        #define MCP7940_ALARM_ALMMSK2_bm 0x40   /**< Bit mask for the ALMMSK2 bit, part of the alarm match mask field. */
        #define MCP7940_ALARM_ALMMSK1_bm 0x20   /**< Bit mask for the ALMMSK1 bit, part of the alarm match mask field. */
        #define MCP7940_ALARM_ALMMSK0_bm 0x10   /**< Bit mask for the ALMMSK0 bit, part of the alarm match mask field. */
        #define MCP7940_ALARM_ALMIF_bm   0x08   /**< Bit mask for the ALMIF flag indicating that an alarm event has occurred. */

        #define MCP7940_ALARM_WKDAY_bp   0    /**< Bit position of the least significant bit of the weekday BCD field. */

        #define MCP7940_ALARM_ALMMSK_bp 4       /**< Bit position of the least significant bit of the alarm match mask (ALMMSK) field. */

        #define MCP7940_ALARM_SECOND_MATCH_gc (0x00<<MCP7940_ALARM_ALMMSK_bp) /**< Alarm match mode: match on seconds only. */
        #define MCP7940_ALARM_MINUTE_MATCH    (0x01<<MCP7940_ALARM_ALMMSK_bp) /**< Alarm match mode: match on minutes and seconds. */
        #define MCP7940_ALARM_HOUR_MATCH      (0x02<<MCP7940_ALARM_ALMMSK_bp) /**< Alarm match mode: match on hours, minutes, and seconds. */
        #define MCP7940_ALARM_DAY_MATCH       (0x03<<MCP7940_ALARM_ALMMSK_bp) /**< Alarm match mode: match on weekday, hours, minutes, and seconds. */
        #define MCP7940_ALARM_DATE_MATCH      (0x04<<MCP7940_ALARM_ALMMSK_bp) /**< Alarm match mode: match on date, hours, minutes, and seconds. */
        #define MCP7940_ALARM_FULL_MATCH      (0x07<<MCP7940_ALARM_ALMMSK_bp) /**< Alarm match mode: full match on all alarm time/date fields. */

        /**
         * @def MCP7940_ALM0WKDAY
         * @brief Address of the MCP7940 Alarm 0 weekday register.
         *
         * This macro defines the register address of the Alarm 0 weekday register, which stores the weekday value and alarm control bits (polarity, match mode, and interrupt flag) for Alarm 0.
         */
        #define MCP7940_ALM0WKDAY 0x0D

        /**
         * @def MCP7940_ALM1WKDAY
         * @brief Address of the MCP7940 Alarm 1 weekday register.
         *
         * This macro defines the register address of the Alarm 1 weekday register, which stores the weekday value and alarm control bits (polarity, match mode, and interrupt flag) for Alarm 1.
         */
        #define MCP7940_ALM1WKDAY 0x14
    #endif

    #if !defined(MCP7940_ALM0DATE) && !defined(MCP7940_ALM1DATE)
        #define MCP7940_ALM_DATETEN_bp 4 /**< Bit position of the least significant bit of the tens-of-day BCD field in the alarm date registers. */
        #define MCP7940_ALM_DATEONE_bp 0 /**< Bit position of the least significant bit of the ones-of-day BCD field in the alarm date registers. */

        /**
         * @def MCP7940_ALM0DATE
         * @brief Address of the MCP7940 Alarm 0 date (day-of-month) register.
         *
         * This macro defines the register address of the Alarm 0 date register, which stores the day-of-month value for Alarm 0 in BCD format as part of the alarm time/date configuration.
         */
        #define MCP7940_ALM0DATE 0x0E

        /**
         * @def MCP7940_ALM1DATE
         * @brief Address of the MCP7940 Alarm 1 date (day-of-month) register.
         *
         * This macro defines the register address of the Alarm 1 date register, which stores the day-of-month value for Alarm 1 in BCD format as part of the alarm time/date configuration.
         */
        #define MCP7940_ALM1DATE 0x15
    #endif

    #if !defined(MCP7940_ALM0MTH) && !defined(MCP7940_ALM1MTH)
        #define MCP7940_ALM_MTHTEN_bp 4 /**< Bit position of the least significant bit of the tens-of-month BCD field in the alarm month registers. */
        #define MCP7940_ALM_MTHONE_bp 0 /**< Bit position of the least significant bit of the ones-of-month BCD field in the alarm month registers. */

        /**
         * @def MCP7940_ALM0MTH
         * @brief Address of the MCP7940 Alarm 0 month register.
         *
         * This macro defines the register address of the Alarm 0 month register, which stores the month value for Alarm 0 in BCD format as part of the alarm time/date configuration.
         */
        #define MCP7940_ALM0MTH 0x0F

        /**
         * @def MCP7940_ALM1MTH
         * @brief Address of the MCP7940 Alarm 1 month register.
         *
         *  This macro defines the register address of the Alarm 1 month register,
         *  which stores the month value for Alarm 1 in BCD format as part of the
         *  alarm time/date configuration.
         */
        #define MCP7940_ALM1MTH 0x16
    #endif

    // #############################################

    #if !defined(MCP7940_PWRDNMIN) && !defined(MCP7940_PWRUPMIN)
        /**
         * @def MCP7940_PWRDNMIN
         * @brief Address of the MCP7940 power-down minutes timestamp register.
         *
         * This macro defines the register address of the minutes field in the power-down timestamp, which stores the minutes value (in BCD format) recorded when the main power supply was lost.
         */
        #define MCP7940_PWRDNMIN 0x18

        /**
         * @def MCP7940_PWRUPMIN
         * @brief Address of the MCP7940 power-up minutes timestamp register.
         *
         * This macro defines the register address of the minutes field in the power-up timestamp, which stores the minutes value (in BCD format)recorded when the main power supply was restored.
         */
        #define MCP7940_PWRUPMIN 0x1C
    #endif

    #if !defined(MCP7940_PWRDNHOUR) && !defined(MCP7940_PWRUPHOUR)
        /**
         * @def MCP7940_PWRDNHOUR
         * @brief Address of the MCP7940 power-down hours timestamp register.
         *
         * This macro defines the register address of the hours field in the power-down timestamp, which stores the hours value (in BCD format) recorded when the main power supply was lost.
         */
        #define MCP7940_PWRDNHOUR 0x19

        /**
         * @def MCP7940_PWRUPHOUR
         * @brief Address of the MCP7940 power-up hours timestamp register.
         *
         * This macro defines the register address of the hours field in the power-up timestamp, which stores the hours value (in BCD format) recorded when the main power supply was restored.
         */
        #define MCP7940_PWRUPHOUR 0x1D
    #endif

    #if !defined(MCP7940_PWRDNDATE) && !defined(MCP7940_PWRUPDATE)
        /**
         * @def MCP7940_PWRDNDATE
         * @brief Address of the MCP7940 power-down date (day-of-month) timestamp register.
         *
         * This macro defines the register address of the date field in the power-down timestamp, which stores the day-of-month value (in BCD format) recorded when the main power supply was lost.
         */
        #define MCP7940_PWRDNDATE 0x1A

        /**
         * @def MCP7940_PWRUPDATE
         * @brief Address of the MCP7940 power-up date (day-of-month) timestamp register.
         *
         * This macro defines the register address of the date field in the power-up timestamp, which stores the day-of-month value (in BCD format) recorded when the main power supply was restored.
         */
        #define MCP7940_PWRUPDATE 0x1E
    #endif

    #if !defined(MCP7940_PWRDNMTH) && !defined(MCP7940_PWRUPMTH)
        #define MCP7940_PWRWEEKDAY_bp 5 /**< Bit position of the least significant bit of the weekday field in the power-fail timestamp month registers. */

        /**
         * @def MCP7940_PWRDNMTH
         * @brief Address of the MCP7940 power-down month timestamp register.
         *
         *  This macro defines the register address of the month field in the power-down timestamp, which stores the month value (in BCD format) and weekday information recorded when the main power supply was lost.
         */
        #define MCP7940_PWRDNMTH 0x1B

        /**
         * @def MCP7940_PWRUPMTH
         * @brief Address of the MCP7940 power-up month timestamp register.
         *
         *  This macro defines the register address of the month field in the power-up timestamp, which stores the month value (in BCD format) and weekday information recorded when the main power supply was restored.
         */
        #define MCP7940_PWRUPMTH 0x1F
    #endif

    #include "../../../shared/format/time.h"

    #include "../../../utils/systick/systick.h"
    #include "../../../utils/time/validate.h"
    #include "../../../utils/macros/stringify.h"

    #include _STR(../../../hal/MCP7940_HAL_PLATFORM/twi/twi.h)
    
    /**
     * @enum MCP7940_Status_t
     * @brief Represents status and configuration flags of the MCP7940 RTC.
     *
     * @details
     * This enumeration defines bit flags derived from the MCP7940 RTC status/control register. The flags indicate whether the battery backup is enabled, whether a power-fail event has been detected, and whether the oscillator is currently running. The value MCP7940_Status_None can be used when no status flags are set or when no status information is available.
     */
    enum MCP7940_Status_t
    {
        MCP7940_Status_None               = 0x00,				/**< No status flags set */
        MCP7940_Status_Battery_Enabled    = MCP7940_VBATEN_bm,	/**< Battery backup is enabled (VBATEN bit set) */
        MCP7940_Status_Power_Fail         = MCP7940_PWRFAIL_bm, /**< Power-fail event has been detected (PWRFAIL bit set) */
        MCP7940_Status_Oscillator_Running = MCP7940_OSCRUN_bm	/**< Oscillator is running (OSCON/OSCRUN bit set) */
    };
    
    /**
     * @typedef MCP7940_Status
     * @brief Alias for enum MCP7940_Status_t representing MCP7940 RTC status flags.
     */
    typedef enum MCP7940_Status_t MCP7940_Status;

    /**
     * @enum MCP7940_Error_t
     * @brief Represents error conditions reported by the MCP7940 driver.
     *
     * @details
     * This enumeration defines generic error codes used by the MCP7940 access routines. It currently distinguishes between the absence of an error and a generic failure condition, which may cover I2C communication issues, invalid parameters, or unexpected device responses. Additional error codes can be added as the driver implementation is extended.
     */
    enum MCP7940_Error_t
    {
        MCP7940_Error_None = 0, /**< No error occurred, operation completed successfully */
        MCP7940_Error_Fail      /**< A generic failure occurred during an MCP7940 operation */
    };
    /**
     * @typedef MCP7940_Error
     * @brief Alias for enum MCP7940_Error_t representing MCP7940 driver error codes.
     */
    typedef enum MCP7940_Error_t MCP7940_Error;

    /**
     * @enum MCP7940_Mode_t
     * @brief Selects enabled or disabled mode for MCP7940 features.
     *
     * @details
     * This enumeration provides simple enable/disable selections used by MCP7940 driver functions, for example when configuring the oscillator, alarms, or battery backup-related options. It is intended as a generic mode selector to make function calls more readable than using raw boolean or integer values.
     */
    enum MCP7940_Mode_t
    {
        MCP7940_Mode_Disable = 0, /**< Feature or function is disabled */
        MCP7940_Mode_Enable       /**< Feature or function is enabled */
    };
    /**
     * @typedef MCP7940_Mode
     * @brief Alias for enum MCP7940_Mode_t representing MCP7940 enable/disable modes.
     */
    typedef enum MCP7940_Mode_t MCP7940_Mode;

    /**
     * @enum MCP7940_Trim_t
     * @brief Selects the trim direction for MCP7940 oscillator calibration.
     *
     * @details
     * This enumeration specifies whether the MCP7940 digital trimming logic should add or subtract clock cycles when writing to the oscillator trim register. Adding clocks is typically used to correct for a slow-running oscillator, while subtracting clocks compensates for a fast-running oscillator, as described in the MCP7940 datasheet and trimming guides.
     */
    enum MCP7940_Trim_t
    {
        MCP7940_Trim_Substract = 0, /**< Subtract clock cycles to correct for a fast-running clock */
        MCP7940_Trim_Add            /**< Add clock cycles to correct for a slow-running clock */
    };
    /**
     * @typedef MCP7940_Trim
     * @brief Alias for enum MCP7940_Trim_t representing MCP7940 trim direction selection.
     */
    typedef enum MCP7940_Trim_t MCP7940_Trim;

    /**
     * @enum MCP7940_LeapYear_t
     * @brief Indicates whether the MCP7940 calendar year is a leap year.
     *
     * @details
     * This enumeration corresponds to the leap year indication used by the MCP7940 RTC calendar logic. It can be used to represent or interpret the device's internal leap year status bit, which affects how the device handles the length of February in its automatic datekeeping.
     */
    enum MCP7940_LeapYear_t
    {
        MCP7940_LeapYear_False = 0, /**< Year is not a leap year (LPYR bit cleared) */
        MCP7940_LeapYear_True       /**< Year is a leap year (LPYR bit set) */
    };
    /**
     * @typedef MCP7940_LeapYear
     * @brief Alias for enum MCP7940_LeapYear_t representing MCP7940 leap year status.
     */
    typedef enum MCP7940_LeapYear_t MCP7940_LeapYear;

    /**
     * @enum MCP7940_Register_t
     * @brief Selects MCP7940 time-stamp register blocks for access operations.
     *
     * @details
     * This enumeration identifies the different time-related register sets within the MCP7940 device that can be read from or written to by the driver. It allows higher-level code to specify whether the current time/calendar registers, the power-down time-stamp registers, or the power-up time-stamp registers should be addressed in an access operation.
     */
    enum MCP7940_Register_t
    {
        MCP7940_Register_Current_Time   = 0, /**< Current time and calendar register block */
        MCP7940_Register_Power_Down_Time,    /**< Power-down time-stamp register block*/
        MCP7940_Register_Power_Up_Time       /**< Power-up time-stamp register block */
    };

    /**
     * @typedef MCP7940_Register
     * @brief Alias for enum MCP7940_Register_t representing MCP7940 register block selections.
     */
    typedef enum MCP7940_Register_t MCP7940_Register;


                 void mcp7940_init(void);
        MCP7940_Error mcp7940_trimming(MCP7940_Trim mode, unsigned char value);
                 void mcp7940_oscillator(MCP7940_Mode mode);
       MCP7940_Status mcp7940_status(void);

    #if MCP7940_MFP_MODE == MCP7940_MFP_MODE_OUTPUT
                 void mcp7940_mfp_output(MCP7940_Mode output);
    #endif

          const char* mcp7940_weekday_string(unsigned char day);
        unsigned char mcp7940_weekday(MCP7940_Register data);
    
                 void mcp7940_time(FORMAT_Time *time, MCP7940_Register reg);
                 void mcp7940_date(FORMAT_Date *date, MCP7940_Register reg);
                 void mcp7940_datetime(FORMAT_DateTime *datetime, MCP7940_Register reg);

     MCP7940_LeapYear mcp7940_leapyear(void);

        MCP7940_Error mcp7940_setweekday(unsigned char weekday);
        MCP7940_Error mcp7940_settime(const FORMAT_Time *time);
        MCP7940_Error mcp7940_setdate(const FORMAT_Date *date);
        MCP7940_Error mcp7940_setdatetime(const FORMAT_DateTime *datetime);

#endif /* MCP7940_H_ */