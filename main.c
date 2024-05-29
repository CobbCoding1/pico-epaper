#include "EPD_Test.h"   // Examples
#include "run_File.h"

#include "led.h"
#include "waveshare_PCF85063.h" // RTC
#include "DEV_Config.h"

#include <time.h>

extern const char *fileList;
extern char pathName[];
int horizontal = 0;

#define enChargingRtc 1

/*
Mode 0: Automatically get pic folder names and sort them
Mode 1: Automatically get pic folder names but not sorted
Mode 2: pic folder name is not automatically obtained, users need to create fileList.txt file and write the picture name in TF card by themselves
*/
#define Mode 1


float measureVBAT(void)
{
    float Voltage=0.0;
    const float conversion_factor = 3.3f / (1 << 12);
    uint16_t result = adc_read();
    Voltage = result * conversion_factor * 3;
    printf("Raw value: 0x%03x, voltage: %f V\n", result, Voltage);
    return Voltage;
}

void chargeState_callback() 
{
    if(DEV_Digital_Read(VBUS)) {
        if(!DEV_Digital_Read(CHARGE_STATE)) {  // is charging
            ledCharging();
        }
        else {  // charge complete
            ledCharged();
        }
    }
}

void run_display(Time_data Time, Time_data alarmTime, char hasCard, int isRtc)
{
    if(hasCard) {
        setFilePath();
        EPD_7in3f_display_BMP(pathName, measureVBAT());   // display bmp
    }
    else {
        EPD_7in3f_display(measureVBAT());
    }

    PCF85063_clear_alarm_flag();    // clear RTC alarm flag
    rtcRunAlarm(Time, alarmTime);  // RTC run alarm
}

int detect_double_press(UWORD pin, UWORD debounce_time, UWORD timeout) {
    static UWORD last_press_time = 0;
    static UWORD press_count = 0;

    UWORD current_time = to_ms_since_boot(get_absolute_time());

    if (DEV_Digital_Read(pin) == 0) {  // Button is pressed
        if (current_time - last_press_time > debounce_time) {
            press_count++;
            last_press_time = current_time;
        }
    }

    if (press_count == 2) {
        if (current_time - last_press_time <= timeout) {
            press_count = 0;  // Reset press count after detecting double press
            return 2;  // Double press detected
        } else {
            press_count = 1;  // Consider the current press as the first press of the next sequence
        }
    }

    if (current_time - last_press_time > timeout) {
	if(press_count == 1) {
		press_count = 0;
		return 1;
	}
	press_count = 0;
    }

    return 0;  // No double press detected
}

int main(void)
{
    Time_data Time = {2024-2000, 3, 31, 0, 0, 0};
    Time_data alarmTime = Time;
    // alarmTime.seconds += 10;
    // alarmTime.minutes += 30;
    alarmTime.hours += 1;
    char isCard = 0;

    UWORD debounce_time = 50;
    UWORD double_press_timeout = 500;
  
    printf("Init...\r\n");
    if(DEV_Module_Init() != 0) {  // DEV init
        return -1;
    }
    
    watchdog_enable(8*1000, 1);    // 8s
    DEV_Delay_ms(1000);
    PCF85063_init();    // RTC init
    rtcRunAlarm(Time, alarmTime);  // RTC run alarm
    gpio_set_irq_enabled_with_callback(CHARGE_STATE, GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, true, chargeState_callback);

    if(measureVBAT() < 3.1) {   // battery power is low
        printf("low power ...\r\n");
        PCF85063_alarm_Time_Disable();
        ledLowPower();  // LED flash for Low power
        powerOff(); // BAT off
        return 0;
    }
    else {
        printf("work ...\r\n");
        ledPowerOn();
    }

    if(!sdTest()) 
    {
        isCard = 1;
        if(Mode == 0)
        {
            sdScanDir(horizontal);
            file_sort();
        }
        if(Mode == 1)
        {
            sdScanDir(horizontal);
        }
        if(Mode == 2)
        {
            file_cat();
        }
        
    }
    else 
    {
        isCard = 0;
    }

    if(!DEV_Digital_Read(VBUS)) {    // no charge state
	horizontal = !horizontal;
	sdScanDir(horizontal);
        run_display(Time, alarmTime, isCard, 0);
    }
    else {  // charge state
        chargeState_callback();
        while(DEV_Digital_Read(VBUS)) {
	    int count = 0;
            measureVBAT();
            
            if(!DEV_Digital_Read(RTC_INT)) {    // RTC interrupt trigger
                printf("rtc interrupt\r\n");
                run_display(Time, alarmTime, isCard, 1);
            }

/*
check_button_again:
	    if (detect_double_press(BAT_STATE, debounce_time, double_press_timeout) == 2) {
		horizontal = !horizontal;
		sdScanDir(horizontal);
		run_display(Time, alarmTime, isCard, 0);
	    } else if(detect_double_press(BAT_STATE, debounce_time, double_press_timeout) == 1) {
		run_display(Time, alarmTime, isCard, 0);
	    }
*/

            if(!DEV_Digital_Read(BAT_STATE)) {  // KEY pressed
                printf("key interrupt\r\n");
		run_display(Time, alarmTime, isCard, 0);
	    }


            if(!DEV_Digital_Read(30)) {  // RUN KEY pressed
                printf("key interrupt\r\n");
		horizontal = !horizontal;
		sdScanDir(horizontal);
		file_sort();
		run_display(Time, alarmTime, isCard, 0);
	    }

/*
            if(!DEV_Digital_Read(BAT_STATE)) {  // KEY pressed
		count++;
                printf("key interrupt\r\n");
		if(count == 2) {
			horizontal = !horizontal;
			sdScanDir(horizontal);
			run_display(Time, alarmTime, isCard, 0);
		} else {
		    DEV_Delay_ms(500);
		    goto check_button_again;
		}
            }
	    if(count == 1)
		run_display(Time, alarmTime, isCard, 0);
*/
            DEV_Delay_ms(200);
        }
    }
    
    printf("power off ...\r\n");
    powerOff(); // BAT off

    return 0;
}
