#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/timer.h>
#include <libopencm3/cm3/nvic.h>
#include <libopencm3/cm3/systick.h>
#include <string.h>

#include "mini-printf.h"
#include "HAL.h"

int main(void)
{
	int i;
	int slow_count = 0;
	imu_data IMU = {0,0,0,0,0};


	clock_setup();
	gpio_setup();
	tim_setup();
	systick_setup(100); //systick in microseconds
	i2c_setup();
	usart_setup();
	char strDisp[20];

	for (i=0;i<20;i++)
	{
		strDisp[i] = 0;
	}

	for (i=0;i<5000000;i++)
	{
		__asm__("nop");
	}

	i2c_write(0x3D, 0x0); //OPR_MODE register, config mode
	i2c_write(0x3E, 0x0); //PWR_MODE register, normal
	i2c_write(0x07, 0x01); //Page ID, switch to page 1
	i2c_write(0x0A, 0x3C); //change gyro range to 125dps @32Hz
	//i2c_write(0x3B, 0x01); //change ACC_unit to mg
	i2c_write(0x07, 0x00); //Page ID, switch to page 0
	i2c_write(0x3D, 0x8); //OPR_MODE register, IMU mode	

	for (i=0;i<50000;i++)
	{
		__asm__("nop");
	}

	for(;;)
	{
		if (main_tick == 1) 
		{	//main tick every 100 us
			main_tick = 0;
		


			slow_count++;
			if (slow_count > 50)
			{
				slow_count = 0;
				getIMU(&IMU);
				mini_snprintf(strDisp, 20, "%d\n", IMU.accel_utr);
				usart_print(strDisp);
				scaleIMU(&IMU);
				setLEDs(&IMU, &LEDs);
			}
		}
	}
}
