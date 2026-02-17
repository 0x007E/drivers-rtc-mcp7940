/**
 * @file mcp7940.c
 *
 * @brief Implementation of the MCP7940 driver functions.
 *
 * This file contains the implementation of functions to initialize the MCP7940 device, write and read data from/to rtc. It utilizes TWI/I2C communication to interact with the MCP7940 hardware.
 *
 * @author g.raf
 * @date 2026-01-25
 * @version 1.0 Release
 * @copyright
 * Copyright (c) 2026 g.raf
 * Released under the GPLv3 License. (see LICENSE in repository)
 * 
 * @note This file is part of a larger project and subject to the license specified in the repository. For updates and the complete revision history, see the GitHub repository.
 * 
 * @see https://github.com/0x007e/drivers-rtc-mcp7940 "MCP7940 RTC driver library"
 */

#include "mcp7940.h"

const char weeksdays[][4] = {
    "MON",
    "TUE",
    "WED",
    "THU",
    "FRI",
    "SAT",
    "SUN",
    "???"
};

/**
 * @brief Returns a three-letter weekday abbreviation for a given day value.
 *
 * @param day Encoded weekday value in the MCP7940 format (1–7 expected),
 * where:
 * - 1 maps to `"MON"`
 * - 2 maps to `"TUE"`
 * - 3 maps to `"WED"`
 * - 4 maps to `"THU"`
 * - 5 maps to `"FRI"`
 * - 6 maps to `"SAT"`
 * - 7 maps to `"SUN"`
 * Any other value is wrapped into this range and may yield the fallback `"???"`.
 *
 * @return Pointer to a constant, null-terminated string containing the three-letter weekday abbreviation corresponding to @p day.
 *
 * @details
 * This function converts the MCP7940 weekday encoding into a human-readable three-character abbreviation by masking and normalizing the input value into the index range of the internal @c weeksdays lookup table. The returned pointer refers to a static string and remains valid for the lifetime of the program.
 */
const char* mcp7940_weekday_string(unsigned char day)
{
    return weeksdays[(0x07 & (day - 1))];
}

static void mcp7940_write(unsigned char address, unsigned char data)
{
    twi_start();
    twi_address(MCP7940_ADDRESS, TWI_WRITE);
    twi_set(address);
    twi_set(data);
    twi_stop();
    
    systick_timer_wait_ms(MCP7940_IO_TIMEOUT_MS);
}

static unsigned char mcp7940_read(unsigned char address)
{
    unsigned char temp;
    
    twi_start();
    twi_address(MCP7940_ADDRESS, TWI_WRITE);
    twi_set(address);
    twi_address(MCP7940_ADDRESS, TWI_READ);
    twi_get(&temp, TWI_NACK);
    twi_stop();
    
    systick_timer_wait_ms(MCP7940_IO_TIMEOUT_MS);
    
    return temp;
}

static unsigned char mcp7940_tobinary(unsigned char value, unsigned char mask)
{
    return (((mask & value)>>4) * 10UL + (0x0F & value));
}

static unsigned char mcp7940_data(unsigned char address, unsigned char mask)
{
    return mcp7940_tobinary(mcp7940_read(address), mask);
}

static void mcp7940_battery(MCP7940_Mode mode)
{
    unsigned char temp = mcp7940_read(MCP7940_RTCWKDAY);
    
    if(mode == MCP7940_Mode_Enable)
    {
        mcp7940_write(MCP7940_RTCWKDAY, (MCP7940_VBATEN_bm | temp));
        return;
    }
    mcp7940_write(MCP7940_RTCWKDAY, ((~MCP7940_VBATEN_bm) & temp));
}

/**
 * @brief Initializes the MCP7940 RTC with battery backup, MFP mode, and oscillator settings.
 *
 * @details
 * This function configures the MCP7940 device according to the compile-time configuration macros. It first enables or disables the battery backup feature using mcp7940_battery() depending on MCP7940_BATTERY_BACKUP_EN. It then reads the current CONTROL register, preserves the EXTOSC bit, and updates control flags related to coarse trimming (MCP7940_CSTRIM_bm), square-wave output and prescaler (MCP7940_SQWEN_bm and MCP7940_MFP_SQUARE_WAVE_PRESCALER) or alarm mode (MCP7940_MFP_ALARM_MODE), depending on MCP7940_SQW_CRSTRIM_EN and MCP7940_MFP_MODE. Finally, it enables the RTC oscillator via mcp7940_oscillator(), allowing the device to begin timekeeping.
 */
void mcp7940_init(void)
{
    #ifdef MCP7940_BATTERY_BACKUP_EN
        mcp7940_battery(MCP7940_Mode_Enable);
    #else
        mcp7940_battery(MCP7940_Mode_Disable);
    #endif

    unsigned char temp = mcp7940_read(MCP7940_CONTROL) & MCP7940_EXTOSC_bm;
    
    mcp7940_write(MCP7940_CONTROL, (temp
    #ifdef MCP7940_SQW_CRSTRIM_EN
        | MCP7940_CSTRIM_bm
    #endif

    #if MCP7940_MFP_MODE == MCP7940_MFP_MODE_SQUARE_WAVE
        | MCP7940_SQWEN_bm

        #ifndef MCP7940_SQW_CRSTRIM_EN
            | MCP7940_MFP_SQUARE_WAVE_PRESCALER
        #endif
    #elif MCP7940_MFP_MODE == MCP7940_MFP_MODE_ALARM
        | MCP7940_MFP_ALARM_MODE
    #endif
    ));
    
    mcp7940_oscillator(MCP7940_Mode_Enable);
}

/**
 * @brief Configures the MCP7940 oscillator trimming value and verifies the write.
 *
 * @param mode Selects the trim direction, using a value from ::MCP7940_Trim:
 * - `MCP7940_Trim_Substract` to subtract clock cycles (correct a fast clock).
 * - `MCP7940_Trim_Add` to add clock cycles (correct a slow clock).
 *
 * @param value Raw trim magnitude to apply to the OSCTRIM register. Only the lower 7 bits (bit 6:0) are used as the trim value; bit 7 is set or cleared internally by this function according to @p mode to encode the sign of the adjustment. A value of 0 disables digital trimming.
 *
 * @return Returns one of the following error codes:
 * - `MCP7940_Error_None` if the OSCTRIM register was written successfully and a subsequent readback matches the programmed value.
 * - `MCP7940_Error_Fail` if the readback of OSCTRIM does not match, indicating a possible I2C communication or device error.
 *
 * @details
 * This function masks @p value to seven bits, applies the sign bit according to the selected trim @p mode, and writes the resulting byte to the MCP7940 OSCTRIM register to adjust the RTC oscillator frequency. It then reads back the OSCTRIM register and compares it to the written value to confirm that the configuration was accepted by the device.
 */
MCP7940_Error mcp7940_trimming(MCP7940_Trim mode, unsigned char value)
{
    value = (value & 0x7F);

    if(mode == MCP7940_Trim_Add)
    {
        value |= 0x80;
    }

    mcp7940_write(MCP7940_OSCTRIM, value);
    
    if(mcp7940_read(MCP7940_OSCTRIM) == value)
    {
        return MCP7940_Error_None;
    }
    return MCP7940_Error_Fail;
}

/**
 * @brief Enables or disables the MCP7940 RTC oscillator or external clock input.
 *
 * @param mode Selects the desired oscillator mode, using a value from ::MCP7940_Mode:
 * - `MCP7940_Mode_Enable` to start the oscillator or enable the external clock.
 * - `MCP7940_Mode_Disable` to stop the oscillator or disable the external clock.
 *
 * @details
 * Depending on the compile-time configuration, this function controls either the external oscillator input (MCP7940_USE_EXTOSC defined) by setting or clearing the EXTOSC bit in the CONTROL register, or the internal RTC oscillator by setting or clearing the ST (start oscillator) bit in the seconds register RTCSEC. The corresponding register is read first and then updated with the appropriate bit set or cleared while preserving the other bits.
 */
void mcp7940_oscillator(MCP7940_Mode mode)
{
    #ifdef MCP7940_USE_EXTOSC
        unsigned char temp = mcp7940_read(MCP7940_CONTROL);

        if(mode == MCP7940_Mode_Enable)
        {
            mcp7940_write(MCP7940_CONTROL, (MCP7940_EXTOSC_bm | temp));
            return;
        }
        mcp7940_write(MCP7940_CONTROL, ((~MCP7940_EXTOSC_bm) & temp));
    #else
        unsigned char temp = mcp7940_read(MCP7940_RTCSEC);

        if(mode == MCP7940_Mode_Enable)
        {
            mcp7940_write(MCP7940_RTCSEC, (MCP7940_ST_bm | temp));
            return;
        }
        mcp7940_write(MCP7940_RTCSEC, ((~MCP7940_ST_bm) & temp));
    #endif
}

/**
 * @brief Reads and returns the current MCP7940 status flags from the weekday register.
 *
 * @return A ::MCP7940_Status value containing a bitwise combination of:
 * - `MCP7940_Status_Oscillator_Running` if the OSCRUN bit is set, indicating that the oscillator is currently running.
 * - `MCP7940_Status_Power_Fail` if the PWRFAIL bit is set, indicating that a power-fail event has been logged and the corresponding time stamps are available.
 * - `MCP7940_Status_Battery_Enabled` if the VBATEN bit is set, indicating that battery backup mode is enabled.
 * 
 * If none of these bits are set, the function returns `MCP7940_Status_None`.
 *
 * @details
 * This function reads the RTCWKDAY register of the MCP7940 and masks out the OSCRUN, PWRFAIL, and VBATEN bits to construct an ::MCP7940_Status value. The resulting status can be used by higher-level code to determine whether the RTC oscillator is running, whether a power failure has occurred, and whether battery backup is configured.
 */
MCP7940_Status mcp7940_status(void)
{
    return (mcp7940_read(MCP7940_RTCWKDAY) & (MCP7940_OSCRUN_bm | MCP7940_PWRFAIL_bm | MCP7940_VBATEN_bm));
}

#if MCP7940_MFP_MODE == MCP7940_MFP_MODE_OUTPUT
    /**
     * @brief Controls the MCP7940 MFP pin when configured as a general-purpose output.
     *
     * @param output Desired output mode for the MFP pin, using a value from ::MCP7940_Mode:
     * - `MCP7940_Mode_Enable` drives the MFP pin active by setting the OUT bit in the CONTROL register.
     * - `MCP7940_Mode_Disable` releases the MFP pin (open-drain inactive state) by clearing the OUT bit.
     *
     * @details
     * This function is available only when @c MCP7940_MFP_MODE is set to @c MCP7940_MFP_MODE_OUTPUT at compile time. In this mode, the MFP pin behaves as an open-drain general-purpose output controlled by the OUT bit in the CONTROL register. The function reads the current CONTROL value, sets or clears the OUT bit according to @p output, and writes the updated value back, preserving all other control bits.
     */
    void mcp7940_mfp_output(MCP7940_Mode output)
    {   
        unsigned char temp = mcp7940_read(MCP7940_CONTROL);
        
        if(output)
        {
            mcp7940_write(MCP7940_CONTROL, (MCP7940_OUT_bm | temp));
            return;
        }
        mcp7940_write(MCP7940_CONTROL, ((~MCP7940_OUT_bm) & temp));
    }
#endif

static unsigned char mcp7940_hour(MCP7940_Register data)
{
    unsigned char temp;

    switch (data)
    {
        case MCP7940_Register_Power_Down_Time:
            temp = MCP7940_PWRDNHOUR;
        break;
        case MCP7940_Register_Power_Up_Time:
            temp = MCP7940_PWRUPHOUR;
        break;
        default:
            temp = MCP7940_RTCHOUR;
        break;
    }
    return mcp7940_data(temp, MCP7940_HRTEN_bm);
}

static unsigned char mcp7940_minute(MCP7940_Register data)
{
    unsigned char temp;

    switch (data)
    {
        case MCP7940_Register_Power_Down_Time:
            temp = MCP7940_PWRDNMIN;
        break;
        case MCP7940_Register_Power_Up_Time:
            temp = MCP7940_PWRUPMIN;
        break;
        default:
            temp = MCP7940_RTCMIN;
        break;
    }
    return mcp7940_data(temp, MCP7940_MINTEN_bm);
}

static unsigned char mcp7940_second(void)
{
    return mcp7940_data(MCP7940_RTCSEC, MCP7940_SECTEN_bm);
}

/**
 * @brief Reads the weekday value from the MCP7940 for the selected timestamp register set.
 *
 * @param data Selects which MCP7940 register block to read the weekday from, using a value from ::MCP7940_Register:
 * - `MCP7940_Register_Current_Time` to read the current weekday from RTCWKDAY.
 * - `MCP7940_Register_Power_Down_Time` to read the power-down weekday from PWRDNMTH.
 * - `MCP7940_Register_Power_Up_Time` to read the power-up weekday from PWRUPMTH.
 *
 * @return An unsigned 3-bit weekday value in the range 0–7, where the underlying MCP7940 encoding uses 1–7 for the day-of-week and 0 may be returned if the register contents are not yet initialized.
 *
 * @details
 * This function extracts the weekday field from the appropriate MCP7940 register depending on @p data. For the power-down and power-up timestamp registers, the weekday bits are located in the upper three bits of the month register (PWRDNMTH or PWRUPMTH) and are right-shifted by @c MCP7940_PWRWEEKDAY_bp after masking. For the current time, the weekday is read directly from the RTCWKDAY register and masked with 0x07 to return only the weekday bits.
 */
unsigned char mcp7940_weekday(MCP7940_Register data)
{
    switch (data)
    {
        case MCP7940_Register_Power_Down_Time:
            return ((0xE0 & mcp7940_read(MCP7940_PWRDNMTH)) >> MCP7940_PWRWEEKDAY_bp);
        case MCP7940_Register_Power_Up_Time:
            return ((0xE0 & mcp7940_read(MCP7940_PWRUPMTH)) >> MCP7940_PWRWEEKDAY_bp);
        default:
            return (0x07 & mcp7940_read(MCP7940_RTCWKDAY));
    }
}

static unsigned char mcp7940_day(MCP7940_Register data)
{
    unsigned char temp;

    switch (data)
    {
        case MCP7940_Register_Power_Down_Time:
            temp = MCP7940_PWRDNDATE;
        break;
        case MCP7940_Register_Power_Up_Time:
            temp = MCP7940_PWRUPDATE;
        break;
        default:
            temp = MCP7940_RTCDATE;
        break;
    }
    return mcp7940_data(temp, MCP7940_DATETEN_bm);
}

static unsigned char mcp7940_month(MCP7940_Register data)
{
    unsigned char temp;

    switch (data)
    {
        case MCP7940_Register_Power_Down_Time:
            temp = MCP7940_PWRDNMTH;
        break;
        case MCP7940_Register_Power_Up_Time:
            temp = MCP7940_PWRUPMTH;
        break;
        default:
            temp = MCP7940_RTCMTH;
        break;
    }
    return mcp7940_data(temp, MCP7940_MTHTEN_bm);
}

static unsigned char mcp7940_year(void)
{
    return mcp7940_data(MCP7940_RTCYEAR, MCP7940_YRTEN_bm);
}

/**
 * @brief Reads hour, minute, and second fields from the MCP7940 into a FORMAT_Time structure.
 *
 * @param time Pointer to a ::FORMAT_Time structure that will be populated with time information from the MCP7940 registers. On return:
 * - @c time->hour contains the decoded hour value.
 * - @c time->minute contains the decoded minute value.
 * - @c time->second contains the decoded second value for current time, or 0 for power-up/power-down timestamp selections.
 *
 * @param reg Selects which MCP7940 register set to read, using a value from ::MCP7940_Register:
 * - `MCP7940_Register_Current_Time` to read the live time registers.
 * - `MCP7940_Register_Power_Down_Time` to read the power-down timestamp.
 * - `MCP7940_Register_Power_Up_Time` to read the power-up timestamp.
 *
 * @details
 * This function uses helper routines (mcp7940_hour(), mcp7940_minute(), and optionally mcp7940_second()) to extract the hour and minute from the selected MCP7940 time/timestamp registers and write them into @p time. For the current time selection, seconds are also read via mcp7940_second(). For power-down and power-up timestamp registers, the seconds field is not read and @c time->second is set to 0, since those registers do not store a separate seconds value in this implementation.
 */
void mcp7940_time(FORMAT_Time *time, MCP7940_Register reg)
{
    time->hour = mcp7940_hour(reg);
    time->minute = mcp7940_minute(reg);

    if(reg == MCP7940_Register_Current_Time)
    {
        time->second = mcp7940_second();
        return;
    }
    time->second = 0;
}


/**
 * @brief Reads day, month, and year fields from the MCP7940 into a FORMAT_Date structure.
 *
 * @param date Pointer to a ::FORMAT_Date structure that will be populated with calendar information from the MCP7940 registers. On return:
 * - @c date->day contains the decoded day-of-month value.
 * - @c date->month contains the decoded month value.
 * - @c date->year contains the decoded year value for current time, or 0 for power-up/power-down timestamp selections.
 *
 * @param reg Selects which MCP7940 register set to read, using a value from ::MCP7940_Register:
 * - `MCP7940_Register_Current_Time` to read the live calendar date.
 * - `MCP7940_Register_Power_Down_Time` to read the power-down timestamp date.
 * - `MCP7940_Register_Power_Up_Time` to read the power-up timestamp date.
 *
 * @details
 * This function uses helper routines (mcp7940_day(), mcp7940_month(), and optionally mcp7940_year()) to extract the calendar date fields from the selected MCP7940 time or timestamp registers and write them into @p date. For the current time selection, the year field is read from the device via mcp7940_year(). For power-down and power-up timestamp registers, the year field is not read and @c date->year is set to 0, as this implementation does not associate a stored year with those timestamp records.
 */
void mcp7940_date(FORMAT_Date *date, MCP7940_Register reg)
{
    date->day = mcp7940_day(reg);
    date->month = mcp7940_month(reg);

    if(reg == MCP7940_Register_Current_Time)
    {
        date->year = mcp7940_year();
        return;
    }
    date->year = 0;
}

/**
 * @brief Reads both time and date from the MCP7940 into a FORMAT_DateTime structure.
 *
 * @param datetime Pointer to a ::FORMAT_DateTime structure that will be filled with the time and date fields obtained from the MCP7940 registers corresponding to @p reg. On return, @c datetime->time and @c datetime->date contain the decoded values.
 *
 * @param reg Selects which MCP7940 register set to read, using a value from ::MCP7940_Register:
 * - `MCP7940_Register_Current_Time` to read the live date and time.
 * - `MCP7940_Register_Power_Down_Time` to read the power-down timestamp.
 * - `MCP7940_Register_Power_Up_Time` to read the power-up timestamp.
 *
 * @details
 * This function is a convenience wrapper that combines mcp7940_time() and mcp7940_date() to populate a ::FORMAT_DateTime instance in a single call. It first reads the time portion into @c datetime->time, then reads the date portion into @c datetime->date, using the same @p reg selection for both operations to keep time and date information consistent.
 */
void mcp7940_datetime(FORMAT_DateTime *datetime, MCP7940_Register reg)
{
    mcp7940_time(&datetime->time, reg);
    mcp7940_date(&datetime->date, reg);
}

/**
 * @brief Returns the current leap year status from the MCP7940 device.
 *
 * @return A ::MCP7940_LeapYear value derived from the MCP7940 leap-year field:
 * - `MCP7940_LeapYear_False` if the device indicates a non-leap year.
 * - `MCP7940_LeapYear_True` if the device indicates a leap year.
 *
 * @details
 * This function reads the leap-year indicator bits from the MCP7940 and masks and shifts them into the ::MCP7940_LeapYear enumeration domain. The returned value reflects the RTC’s internal leap-year status, which influences how February 29 is handled in the device’s calendar logic.
 */
MCP7940_LeapYear mcp7940_leapyear(void)
{
    return ((0x07 & mcp7940_read(MCP7940_LPYR_bm))>>MCP7940_LPYR_bp);
}

/**
 * @brief Sets the MCP7940 weekday field in the RTCWKDAY register.
 *
 * @param weekday Zero-based weekday index to be written to the device. Valid values are:
 * - 0 for the first day of the week (mapped to device value 1),
 * - 1 for the second day of the week (mapped to device value 2),
 * - ...
 * - 6 for the seventh day of the week (mapped to device value 7).
 * 
 * Values greater than or equal to 7 are rejected.
 *
 * @return Returns one of the following error codes:
 * - `MCP7940_Error_None` if @p weekday is within range and the RTCWKDAY register was updated.
 * - `MCP7940_Error_Fail` if @p weekday is out of range (>= 7) and the register is not modified.
 *
 * @details
 * This function programs the weekday field of the MCP7940 RTCWKDAY register. The MCP7940 encodes weekday values in the range 1–7, so the provided zero-based @p weekday is incremented by 1 and masked with 0x07 before being written. The existing upper bits of RTCWKDAY (such as VBATEN, PWRFAIL, and OSCRUN) are preserved by masking with 0xF8 and OR-ing in the new weekday value.
 */
MCP7940_Error mcp7940_setweekday(unsigned char weekday)
{
    if(weekday >= 7)
    {
        return MCP7940_Error_Fail;
    }
    
    unsigned char temp = mcp7940_read(MCP7940_RTCWKDAY);
    mcp7940_write(MCP7940_RTCWKDAY, ((0xF8 & temp) | (0x07 & (weekday + 1))));
    
    return MCP7940_Error_None;
}

static unsigned char mcp7940_tobcd(unsigned char value)
{
    return ((value / 10)<<4) | (value%10);
}

static void mcp7940_setdata(
    unsigned char address1,
    unsigned char value1,
    unsigned char address2,
    unsigned char value2,
    unsigned char address3,
    unsigned char value3)
{
    mcp7940_write(address1, mcp7940_tobcd(value1));
    mcp7940_write(address2, mcp7940_tobcd(value2));
    mcp7940_write(address3, mcp7940_tobcd(value3));
}

/**
 * @brief Sets the current time of the MCP7940 RTC from a FORMAT_Time structure.
 *
 * @param time Pointer to a ::FORMAT_Time structure containing the time to be set. The following fields are used:
 * - @c time->hour   (expected range: 0–23)
 * - @c time->minute (expected range: 0–59)
 * - @c time->second (expected range: 0–59)
 *
 * @return Returns one of the following error codes:
 * - `MCP7940_Error_None` if the supplied time is valid and the MCP7940 registers were updated successfully.
 * - `MCP7940_Error_Fail` if the supplied time is invalid according to validate_time() and no write is attempted.
 *
 * @details
 * This function first validates the @p time fields using validate_time(). If validation fails, it returns `MCP7940_Error_Fail` immediately. Otherwise, it writes the hour, minute, and second values to the MCP7940 RTCHOUR, RTCMIN, and RTCSEC registers using mcp7940_setdata(), then enables the RTC oscillator via mcp7940_oscillator() so that timekeeping starts or continues from the new value. The implementation assumes that conversion to the device’s register format (for example BCD) is handled inside mcp7940_setdata().
 */
MCP7940_Error mcp7940_settime(const FORMAT_Time *time)
{
    if(validate_time(time) != RETURN_Valid)
    {
        return MCP7940_Error_Fail;
    }
    
    mcp7940_setdata(
        MCP7940_RTCHOUR, time->hour,
        MCP7940_RTCMIN,  time->minute,
        MCP7940_RTCSEC,  time->second
    );
    mcp7940_oscillator(MCP7940_Mode_Enable);
    return MCP7940_Error_None;
}

/**
 * @brief Sets the current calendar date of the MCP7940 RTC from a FORMAT_Date structure.
 *
 * @param date Pointer to a ::FORMAT_Date structure containing the date to be set. The following fields are used:
 * - @c date->day   (expected range: 1–31; only a basic range check is performed)
 * - @c date->month (expected range: 1–12)
 * - @c date->year  (expected range: 0–99, interpreted as an application-defined offset)
 *
 * @return Returns one of the following error codes:
 * - `MCP7940_Error_None` if the supplied date is valid and the MCP7940 date registers were updated successfully.
 * - `MCP7940_Error_Fail` if the supplied date is invalid according to validate_date() and no write is attempted.
 *
 * @details
 * This function first validates the @p date fields using validate_date(). If validation fails, it returns `MCP7940_Error_Fail` immediately. Otherwise, it writes the day, month, and year values to the MCP7940 RTCDATE, RTCMTH, and RTCYEAR registers via mcp7940_setdata(). The implementation assumes that any required conversion to the device’s register format (for example, BCD encoding and leap-year bit handling) is performed inside mcp7940_setdata() or the lower-level I2C access functions.
 */
MCP7940_Error mcp7940_setdate(const FORMAT_Date *date)
{
    if(validate_date(date) != RETURN_Valid)
    {
        return MCP7940_Error_Fail;
    }
    
    mcp7940_setdata(
        MCP7940_RTCDATE, date->day,
        MCP7940_RTCMTH,  date->month,
        MCP7940_RTCYEAR, date->year
    );
    return MCP7940_Error_None;
}

/**
 * @brief Sets the current MCP7940 date and time from a FORMAT_DateTime structure.
 *
 * @param datetime Pointer to a ::FORMAT_DateTime structure containing both time and date components to be written to the MCP7940. The following subfields are used:
 * - @c datetime->time.hour, @c datetime->time.minute, @c datetime->time.second
 * - @c datetime->date.day,  @c datetime->date.month,  @c datetime->date.year
 *
 * @return A combined ::MCP7940_Error value obtained by OR-ing the results of:
 * - mcp7940_settime() for the time component.
 * - mcp7940_setdate() for the date component.
 * If both calls succeed, the function returns `MCP7940_Error_None`; if either fails, the resulting value will include `MCP7940_Error_Fail`.
 *
 * @details
 * This function is a convenience wrapper that programs both the current time and calendar date of the MCP7940 in one call. It first calls mcp7940_settime() with @c datetime->time and then mcp7940_setdate() with @c datetime->date. The individual error codes are bitwise OR-combined and returned to the caller, allowing detection of failures in either step.
 */
MCP7940_Error mcp7940_setdatetime(const FORMAT_DateTime *datetime)
{
    MCP7940_Error error = MCP7940_Error_None;

    error |= mcp7940_settime(&datetime->time);
    error |= mcp7940_setdate(&datetime->date);

    return error;
}