#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/timer.h>
#include <libopencm3/cm3/nvic.h>
#include <libopencm3/cm3/systick.h>
#include <string.h>

#include "HAL.h"

volatile uint8_t tick = 0;
volatile uint8_t main_tick = 0;
volatile led_array LEDs = {0,0,0,0,0,0,0,0,0,0,0};

void sys_tick_handler(void)
{
	// Switch TIM21 ISR to this
}

void systick_setup(int xms) 
{
	systick_set_clocksource(STK_CSR_CLKSOURCE_EXT);
	STK_CVR = 0;
	systick_set_reload(2000 * xms);
	systick_counter_enable();
	systick_interrupt_enable();
}

void clock_setup(void)
{
	/*	Set clock source to 16 MHz High-Speed Internal oscillator and turn it on */
	rcc_set_sysclk_source(RCC_HSI16);
	rcc_osc_on(RCC_HSI16);
}

void gpio_setup(void)
{
	/*	Enable GPIO clocks */
	rcc_periph_clock_enable(RCC_GPIOA);
	rcc_periph_clock_enable(RCC_GPIOB);
	rcc_periph_clock_enable(RCC_GPIOC);

	/*	Set up LED matrix
		X pins on TIM2 PWM outputs
		Y pins on .. other GPIOs */
	gpio_mode_setup(PORT_X0_LED, GPIO_MODE_AF, GPIO_PUPD_NONE, PIN_X0_LED);
	gpio_mode_setup(PORT_X1_LED, GPIO_MODE_AF, GPIO_PUPD_NONE, PIN_X1_LED);
	gpio_mode_setup(PORT_X2_LED, GPIO_MODE_AF, GPIO_PUPD_NONE, PIN_X2_LED);
	gpio_mode_setup(PORT_X3_LED, GPIO_MODE_AF, GPIO_PUPD_NONE, PIN_X3_LED);
	gpio_mode_setup(PORT_Y0_LED, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, PIN_Y0_LED);
	gpio_mode_setup(PORT_Y1_LED, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, PIN_Y1_LED);
	gpio_mode_setup(PORT_Y2_LED, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, PIN_Y2_LED);
	gpio_set_output_options(PORT_X0_LED, GPIO_OTYPE_PP, GPIO_OSPEED_HIGH, PIN_X0_LED);	
	gpio_set_output_options(PORT_X1_LED, GPIO_OTYPE_PP, GPIO_OSPEED_HIGH, PIN_X1_LED);	
	gpio_set_output_options(PORT_X2_LED, GPIO_OTYPE_PP, GPIO_OSPEED_HIGH, PIN_X2_LED);	
	gpio_set_output_options(PORT_X3_LED, GPIO_OTYPE_PP, GPIO_OSPEED_HIGH, PIN_X3_LED);	
	gpio_set_af(PORT_X0_LED, GPIO_AF5, PIN_X0_LED);
	gpio_set_af(PORT_X1_LED, GPIO_AF5, PIN_X1_LED);
	gpio_set_af(PORT_X2_LED, GPIO_AF2, PIN_X2_LED);
	gpio_set_af(PORT_X3_LED, GPIO_AF5, PIN_X3_LED);


	/*	Set up button input */
	gpio_mode_setup(PORT_IDENTIFY, GPIO_MODE_INPUT, GPIO_PUPD_PULLDOWN, PIN_IDENTIFY);
}

void tim_setup(void)
{
	/* 	Enable and reset TIM2 clock */
	rcc_periph_clock_enable(RCC_TIM2);
	timer_reset(TIM2);

	/* 	Set up TIM2 mode to no clock divider ratio, edge alignment, and up direction */
	timer_set_mode(TIM2, TIM_CR1_CKD_CK_INT, TIM_CR1_CMS_EDGE, TIM_CR1_DIR_UP);

	/*	Set prescaler to 0: 16 MHz clock */
	timer_set_prescaler(TIM2, 0);
	timer_set_period(TIM2, 1023);

	/* 	Set TIM2 Output Compare mode to PWM1 on channels 1,2,3, and 4 */ 
	timer_set_oc_mode(TIM2, TIM_OC1, TIM_OCM_PWM1);
	timer_set_oc_mode(TIM2, TIM_OC2, TIM_OCM_PWM1);
	timer_set_oc_mode(TIM2, TIM_OC3, TIM_OCM_PWM1);
	timer_set_oc_mode(TIM2, TIM_OC4, TIM_OCM_PWM1);

	/* 	Set starting output compare values */
	timer_set_oc_value(TIM2, TIM_OC1, 0);
	timer_set_oc_value(TIM2, TIM_OC2, 0);
	timer_set_oc_value(TIM2, TIM_OC3, 0);
	timer_set_oc_value(TIM2, TIM_OC4, 0);

	/* 	Enable outputs */ 
	timer_enable_oc_output(TIM2, TIM_OC1);
	timer_enable_oc_output(TIM2, TIM_OC2);
	timer_enable_oc_output(TIM2, TIM_OC3);
	timer_enable_oc_output(TIM2, TIM_OC4);
	
	/*	Enable counter */
	timer_enable_counter(TIM2);

    // Enable TIM2 interrupts (600 us)
    timer_enable_irq(TIM2, TIM_DIER_UIE);

    // setup TIM21

    MMIO32((RCC_BASE) + 0x34) |= (1<<2); //Enable TIM21
    MMIO32((RCC_BASE) + 0x24) |= (1<<2); //Set reset bit, TIM21
    MMIO32((RCC_BASE) + 0x24) &= ~(1<<2); //Clear reset bit, TIM21

    /*    TIM21 control register 1 (TIMx_CR1): */
    MMIO32((TIM21_BASE) + 0x00) &= ~((1<<5) | (1<<6)); //Edge-aligned (default setting)
    MMIO32((TIM21_BASE) + 0x00) &= ~((1<<9) | (1<<10)); //No clock division (default setting)
    MMIO32((TIM21_BASE) + 0x00) &= ~(1<<4); //Up direction (default setting)
           
    /*    TIM21 interrupt enable register (TIMx_DIER): */
    MMIO32((TIM21_BASE) + 0x0C) |= (1<<0); //Enable update interrupts

    /*    TIM21 prescaler (TIMx_PSC): */
    MMIO32((TIM21_BASE) + 0x28) = 7; //prescaler = clk/8 (see datasheet, they add one for convenience)

    /*    TIM21 auto-reload register (TIMx_ARR): */
    MMIO32((TIM21_BASE) + 0x2C) = 200; //100 us interrupts (with clk/8 prescaler)
   
    /*    Enable TIM21 counter: */
    MMIO32((TIM21_BASE) + 0x00) |= (1<<0);

    nvic_enable_irq(NVIC_TIM21_IRQ);
    nvic_set_priority(NVIC_TIM21_IRQ, 1);
}

void usart_setup(void)
{
	gpio_mode_setup(PORT_USART, GPIO_MODE_AF, GPIO_PUPD_NONE, PIN_USART_TX);
	gpio_set_af(PORT_USART, GPIO_AF4, PIN_USART_TX);

	rcc_periph_clock_enable(RCC_USART2);
	
	// USART_BRR: set baud rate
	MMIO32((USART2_BASE) + 0x0C) = 139; // 16 MHz / 115200 baud
	
	// default 8-bit character length
	// default 1 stop bit
	// default no parity
	// default no flow control

	// USART_CR1: enable USART2
	MMIO32((USART2_BASE) + 0x00) |= (1<<0);
}

void usart_send(uint8_t word)
{
	while ((MMIO32((USART2_BASE) + 0x1C) & (1<<7)) == 0); // ensure transmit register is empty

		MMIO32((USART2_BASE) + 0x00) |= (1<<3); // enable transmit
		MMIO32((USART2_BASE) + 0x28) = (word & 0xFF); // write data to USART_TDR
}

void usart_print(char *msg)
{
	int len = strlen(msg);
	int i;
	for (i=0;i<len;i++)
	{
		usart_send((uint8_t)*msg++);
	}
}



void i2c_setup(void)
{
/*	BNO055 on PA10 (SDA) and PA9 (SCL)
	I2C1 via AF1 */
	rcc_periph_clock_enable(RCC_I2C1);
	gpio_mode_setup(PORT_I2C_SDA, GPIO_MODE_AF, GPIO_PUPD_NONE, PIN_I2C_SDA); 
	gpio_mode_setup(PORT_I2C_SCL, GPIO_MODE_AF, GPIO_PUPD_NONE, PIN_I2C_SCL); 
	gpio_set_af(PORT_I2C_SDA, GPIO_AF1, PIN_I2C_SDA);
	gpio_set_af(PORT_I2C_SCL, GPIO_AF1, PIN_I2C_SCL);
	rcc_periph_reset_pulse(RST_I2C1);

/*	Manual I2C register setup. Note that the system defaults to 7-bit addressing,
	clock stretching, and analog noise filtering (so these don't need to be configured). 
	Timer settings are taken from the STM32L0 reference manual Table 98 for 100 kHz
	standard mode with F(i2cclk) = 16 MHz. See also I2C initialization flowchart (figure 178). */

/*	Make sure the I2C peripheral is disabled by clearing PE */
	MMIO32(I2C1_BASE + 0x00) &= ~(1<<0);
	
/*	I2C control register 1: I2C_CR1 */
	MMIO32(I2C1_BASE + 0x00) |= (0x3<<8); //set DNF (digital filter) to 3 clock cycles 

/*	I2C timer register: I2C_TIMINGR */
	MMIO32(I2C1_BASE + 0x10) |= (3<<28); //set PRESC to 3
	MMIO32(I2C1_BASE + 0x10) |= (0x4 << 20); //set SCLDEL to 0x4
	MMIO32(I2C1_BASE + 0x10) |= (0x2 << 16); //set SDADEL to 0x2
	MMIO32(I2C1_BASE + 0x10) |= (0xF << 8); //set SCLH to 0xF
	MMIO32(I2C1_BASE + 0x10) |= (0x13 << 0); //set SCLL to 0x13

/*	Enable the I2C peripheral by setting PE */		
	MMIO32(I2C1_BASE + 0x00) |= (1<<0);
}

void i2c_write(uint8_t address, uint8_t val)
{
	MMIO32(I2C1_BASE + 0x04) |= (0x28 << 1); //set slave address to 0x28 (BNO055)
	MMIO32(I2C1_BASE + 0x04) &= ~(1<<10); //RW_WRN clear = request a WRITE transfer
	MMIO32(I2C1_BASE + 0x04) |= (1<<25); //turn on auto-end
	MMIO32(I2C1_BASE + 0x04) &= ~(0xFF<<16); //clear NBYTE register
	MMIO32(I2C1_BASE + 0x04) |= (2<<16); //send two bytes
	MMIO32(I2C1_BASE + 0x04) |= (1<<13); //send START command
	while((MMIO32(I2C1_BASE + 0x18) & (1<<1)) == 0)
	{
	}
	MMIO32(I2C1_BASE + 0x28) = address;
	while((MMIO32(I2C1_BASE + 0x18) & (1<<1)) == 0)
	{
	}
	MMIO32(I2C1_BASE + 0x28) = val;
}

uint8_t i2c_read(uint8_t address)
{	
	while(MMIO32(I2C1_BASE + 0x18) & (1<<15))
	{
	} //checks BUSY bit of interrupt/status register (I2C_ISR)
	while(MMIO32(I2C1_BASE + 0x04) & (1<<13))
	{
	} //checks for START condition
	
	MMIO32(I2C1_BASE + 0x04) |= (0x28 << 1); //set slave address to 0x28 (BNO055)
	MMIO32(I2C1_BASE + 0x04) &= ~(1<<10); //RW_WRN clear = request a WRITE transfer
	MMIO32(I2C1_BASE + 0x04) |= (1<<25); //turn on auto-end
	MMIO32(I2C1_BASE + 0x04) &= ~(0xFF<<16); //clear NBYTE register
	MMIO32(I2C1_BASE + 0x04) |= (1<<16); //send one byte
	MMIO32(I2C1_BASE + 0x04) |= (1<<13); //send START command
	while((MMIO32(I2C1_BASE + 0x18) & (1<<1)) == 0)
	{
	}
	MMIO32(I2C1_BASE + 0x28) = address;

	MMIO32(I2C1_BASE + 0x04) |= (0x28 << 1); //set slave address to 0x28 (BNO055)
	MMIO32(I2C1_BASE + 0x04) |= (1<<10); //RW_WRN set = request a READ transfer
	MMIO32(I2C1_BASE + 0x04) |= (1<<25); //turn on auto-end
	MMIO32(I2C1_BASE + 0x04) &= ~(0xFF<<16); //clear NBYTE register
	MMIO32(I2C1_BASE + 0x04) |= (1<<16); //send one byte
	MMIO32(I2C1_BASE + 0x04) |= (1<<13); //send START command
	while((MMIO32(I2C1_BASE + 0x18) & (1<<2)) == 1)
	{
	}
	return MMIO32(I2C1_BASE + 0x24) & 0xFF;
}

uint16_t get_gamma(uint16_t val)
{
	if (val > 1023) 
	{
		return 9600;
	}
	else
	{

		return gamma_lookup[val];
	}
}

void updateLEDs(led_array *leds)
{
	gpio_clear(PORT_Y0_LED, PIN_Y0_LED);	
	gpio_clear(PORT_Y1_LED, PIN_Y1_LED);
	gpio_clear(PORT_Y2_LED, PIN_Y2_LED);
	timer_set_oc_value(TIM2, TIM_OC1, 0);		//X0
	timer_set_oc_value(TIM2, TIM_OC2, 0);		//X2
	timer_set_oc_value(TIM2, TIM_OC3, 0);		//X3
	timer_set_oc_value(TIM2, TIM_OC4, 0);		//X1

	switch(leds->current_y)
	{
		case 0:
			gpio_set(PORT_Y0_LED, PIN_Y0_LED);
			timer_set_oc_value(TIM2, TIM_OC1, get_gamma(leds->LED_RIGHT));		//X0
			timer_set_oc_value(TIM2, TIM_OC2, get_gamma(leds->LED_SA_HIGH));	//X2
			timer_set_oc_value(TIM2, TIM_OC3, get_gamma(leds->LED_SA_LOW));		//X3
			timer_set_oc_value(TIM2, TIM_OC4, get_gamma(leds->LED_LEFT));		//X1
			leds->current_y = 1;
			break;
		case 1:
			gpio_set(PORT_Y1_LED, PIN_Y1_LED);
			timer_set_oc_value(TIM2, TIM_OC1, get_gamma(leds->LED_UT_HIGH));	//X0
			timer_set_oc_value(TIM2, TIM_OC2, get_gamma(leds->LED_PC_HIGH));	//X2
			timer_set_oc_value(TIM2, TIM_OC3, get_gamma(leds->LED_PC_LOW));		//X3
			timer_set_oc_value(TIM2, TIM_OC4, get_gamma(leds->LED_UT_LOW));		//X1
			leds->current_y = 2;
			break;
		case 2:
			gpio_set(PORT_Y2_LED, PIN_Y2_LED);
			timer_set_oc_value(TIM2, TIM_OC1, get_gamma(leds->LED_AC_LOW));		//X0
			timer_set_oc_value(TIM2, TIM_OC2, get_gamma(leds->LED_LC_LOW));		//X2
			timer_set_oc_value(TIM2, TIM_OC3, get_gamma(leds->LED_LC_HIGH));	//X3
			timer_set_oc_value(TIM2, TIM_OC4, get_gamma(leds->LED_AC_HIGH));	//X1
			leds->current_y = 0;
			break;
	}
}

void tim21_isr(void)
{
    /*
        TIM21 is the communication clock. 
        Each interrupt is one-bit read and one-bit write of gpios.
        Interrupts occur every 100 us.
    */

    if (++tick >= 2){
        main_tick = 1;
        tick = 0;
    }

	updateLEDs(&LEDs);

//  readInputs();
//  write();

	MMIO32((TIM21_BASE) + 0x10) &= ~(1<<0); //clear the interrupt register
}

void getIMU(imu_data *imu)
{
	imu->accel_utr = (int16_t)i2c_read(0x2B) | ((int16_t)i2c_read(0x2A) << 8);
	imu->accel_sac = (int16_t)i2c_read(0x29) | ((int16_t)i2c_read(0x28) << 8);
	imu->gyro_ant = (int16_t)i2c_read(0x17) | ((int16_t)i2c_read(0x16) << 8);
	imu->gyro_lat = (int16_t)i2c_read(0x15) | ((int16_t)i2c_read(0x14) << 8);
	imu->gyro_pos = (int16_t)i2c_read(0x19) | ((int16_t)i2c_read(0x18) << 8);
}

void scaleIMU(imu_data *imu)
{
	imu->gyro_ant = imu->gyro_ant / 16;
	if (((imu->gyro_ant) >= -16) && ((imu->gyro_ant) <= 15))
	{
		imu->gyro_ant = 0;
	}

	imu->gyro_lat = imu->gyro_lat / 16;
	if (((imu->gyro_lat) >= -16) && ((imu->gyro_lat) <= 15))
	{
		imu->gyro_lat = 0;
	}

	imu->gyro_pos = imu->gyro_pos / 16;
	if (((imu->gyro_pos) >= -16) && ((imu->gyro_pos) <= 15))
	{
		imu->gyro_pos = 0;
	}
}

void setLEDs(imu_data *imu, led_array *leds)
{
	if (imu->gyro_ant > 0) 
	{
		leds->LED_AC_LOW = 0;
		leds->LED_AC_HIGH = imu->gyro_ant;
	}
	else
	{
		leds->LED_AC_LOW = (-1) * imu->gyro_ant;
		leds->LED_AC_HIGH = 0;
	}

	if (imu->gyro_lat > 0) 
	{
		leds->LED_LC_LOW = 0;
		leds->LED_LC_HIGH = imu->gyro_lat;
	}
	else
	{
		leds->LED_LC_LOW = (-1) * imu->gyro_lat;
		leds->LED_LC_HIGH = 0;
	}
	
	if (imu->gyro_pos > 0) 
	{
		leds->LED_PC_LOW = 0;
		leds->LED_PC_HIGH = imu->gyro_pos;
	}
	else
	{
		leds->LED_PC_LOW = (-1) * imu->gyro_pos;
		leds->LED_PC_HIGH = 0;
	}
}

static const uint16_t gamma_lookup[1024] = {
	/*	Gamma = 2.8, input range = 0-1023, output range = 0-1023 */
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,
    1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,
    1,  1,  1,  1,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,
    2,  2,  2,  2,  2,  2,  2,  2,  3,  3,  3,  3,  3,  3,  3,  3,
    3,  3,  3,  3,  3,  3,  3,  4,  4,  4,  4,  4,  4,  4,  4,  4,
    4,  4,  4,  4,  5,  5,  5,  5,  5,  5,  5,  5,  5,  5,  5,  6,
    6,  6,  6,  6,  6,  6,  6,  6,  7,  7,  7,  7,  7,  7,  7,  7,
    7,  8,  8,  8,  8,  8,  8,  8,  8,  9,  9,  9,  9,  9,  9,  9,
    9, 10, 10, 10, 10, 10, 10, 10, 11, 11, 11, 11, 11, 11, 12, 12,
   12, 12, 12, 12, 12, 13, 13, 13, 13, 13, 13, 14, 14, 14, 14, 14,
   15, 15, 15, 15, 15, 15, 16, 16, 16, 16, 16, 17, 17, 17, 17, 17,
   18, 18, 18, 18, 18, 19, 19, 19, 19, 20, 20, 20, 20, 20, 21, 21,
   21, 21, 22, 22, 22, 22, 23, 23, 23, 23, 24, 24, 24, 24, 25, 25,
   25, 25, 26, 26, 26, 26, 27, 27, 27, 27, 28, 28, 28, 29, 29, 29,
   29, 30, 30, 30, 31, 31, 31, 31, 32, 32, 32, 33, 33, 33, 34, 34,
   34, 35, 35, 35, 35, 36, 36, 36, 37, 37, 37, 38, 38, 38, 39, 39,
   40, 40, 40, 41, 41, 41, 42, 42, 42, 43, 43, 43, 44, 44, 45, 45,
   45, 46, 46, 46, 47, 47, 48, 48, 48, 49, 49, 50, 50, 50, 51, 51,
   52, 52, 52, 53, 53, 54, 54, 55, 55, 55, 56, 56, 57, 57, 58, 58,
   58, 59, 59, 60, 60, 61, 61, 62, 62, 63, 63, 63, 64, 64, 65, 65,
   66, 66, 67, 67, 68, 68, 69, 69, 70, 70, 71, 71, 72, 72, 73, 73,
   74, 74, 75, 75, 76, 76, 77, 77, 78, 79, 79, 80, 80, 81, 81, 82,
   82, 83, 83, 84, 85, 85, 86, 86, 87, 87, 88, 89, 89, 90, 90, 91,
   92, 92, 93, 93, 94, 95, 95, 96, 96, 97, 98, 98, 99, 99,100,101,
  101,102,103,103,104,105,105,106,106,107,108,108,109,110,110,111,
  112,112,113,114,115,115,116,117,117,118,119,119,120,121,122,122,
  123,124,124,125,126,127,127,128,129,130,130,131,132,132,133,134,
  135,136,136,137,138,139,139,140,141,142,143,143,144,145,146,146,
  147,148,149,150,151,151,152,153,154,155,155,156,157,158,159,160,
  161,161,162,163,164,165,166,167,167,168,169,170,171,172,173,174,
  175,175,176,177,178,179,180,181,182,183,184,185,186,186,187,188,
  189,190,191,192,193,194,195,196,197,198,199,200,201,202,203,204,
  205,206,207,208,209,210,211,212,213,214,215,216,217,218,219,220,
  221,222,223,224,225,226,228,229,230,231,232,233,234,235,236,237,
  238,239,241,242,243,244,245,246,247,248,249,251,252,253,254,255,
  256,257,259,260,261,262,263,264,266,267,268,269,270,272,273,274,
  275,276,278,279,280,281,282,284,285,286,287,289,290,291,292,294,
  295,296,297,299,300,301,302,304,305,306,308,309,310,311,313,314,
  315,317,318,319,321,322,323,325,326,327,329,330,331,333,334,336,
  337,338,340,341,342,344,345,347,348,349,351,352,354,355,356,358,
  359,361,362,364,365,366,368,369,371,372,374,375,377,378,380,381,
  383,384,386,387,389,390,392,393,395,396,398,399,401,402,404,405,
  407,408,410,412,413,415,416,418,419,421,423,424,426,427,429,431,
  432,434,435,437,439,440,442,444,445,447,448,450,452,453,455,457,
  458,460,462,463,465,467,468,470,472,474,475,477,479,480,482,484,
  486,487,489,491,493,494,496,498,500,501,503,505,507,509,510,512,
  514,516,518,519,521,523,525,527,528,530,532,534,536,538,539,541,
  543,545,547,549,551,553,554,556,558,560,562,564,566,568,570,572,
  574,575,577,579,581,583,585,587,589,591,593,595,597,599,601,603,
  605,607,609,611,613,615,617,619,621,623,625,627,629,631,633,635,
  637,640,642,644,646,648,650,652,654,656,658,660,663,665,667,669,
  671,673,675,678,680,682,684,686,688,690,693,695,697,699,701,704,
  706,708,710,712,715,717,719,721,724,726,728,730,733,735,737,739,
  742,744,746,749,751,753,755,758,760,762,765,767,769,772,774,776,
  779,781,783,786,788,790,793,795,798,800,802,805,807,810,812,814,
  817,819,822,824,827,829,831,834,836,839,841,844,846,849,851,854,
  856,859,861,864,866,869,871,874,876,879,881,884,887,889,892,894,
  897,899,902,905,907,910,912,915,918,920,923,925,928,931,933,936,
  939,941,944,947,949,952,955,957,960,963,965,968,971,973,976,979,
  982,984,987,990,992,995,998,1001,1004,1006,1009,1012,1015,1017,1020,1023 };
