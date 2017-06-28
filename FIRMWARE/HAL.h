#ifndef HAL_H_
#define HAL_H_

#define PORT_X0_LED		GPIOA
#define PORT_X1_LED		GPIOB
#define PORT_X2_LED		GPIOA
#define PORT_X3_LED		GPIOB
#define PORT_Y0_LED		GPIOB
#define PORT_Y1_LED		GPIOA
#define PORT_Y2_LED		GPIOA

#define PIN_X0_LED		GPIO8 //TIM2_CH1
#define PIN_X1_LED		GPIO7 //TIM2_CH4
#define PIN_X2_LED		GPIO1 //TIM2_CH2
#define PIN_X3_LED		GPIO6 //TIM2_CH3
#define PIN_Y0_LED		GPIO0
#define PIN_Y1_LED		GPIO3
#define PIN_Y2_LED		GPIO4

#define PORT_IDENTIFY	GPIOB
#define PIN_IDENTIFY	GPIO5

#define PORT_AXON1_EX	GPIOB
#define PORT_AXON1_IN	GPIOB
#define PORT_AXON2_EX	GPIOA
#define PORT_AXON2_IN	GPIOA
#define PORT_AXON3_EX	GPIOA
#define PORT_AXON3_IN	GPIOA
#define PORT_AXON4_EX	GPIOA
#define PORT_AXON4_IN	GPIOB
#define PORT_AXON5_EX	GPIOC
#define PORT_AXON5_IN	GPIOC

#define PIN_AXON1_EX	GPIO4
#define PIN_AXON1_IN	GPIO3
#define PIN_AXON2_EX	GPIO6
#define PIN_AXON2_IN	GPIO5
#define PIN_AXON3_EX	GPIO2
#define PIN_AXON3_IN	GPIO0
#define PIN_AXON4_EX	GPIO1
#define PIN_AXON4_IN	GPIO7
#define PIN_AXON5_EX	GPIO15
#define PIN_AXON5_IN	GPIO14

#define PORT_I2C_SDA	GPIOA
#define PORT_I2C_SCL	GPIOA

#define PIN_I2C_SDA		GPIO10
#define PIN_I2C_SCL		GPIO9

#define PORT_USART		GPIOA
#define PIN_USART_TX	GPIO2

extern volatile uint8_t tick;
extern volatile uint8_t main_tick;

static const uint16_t gamma_lookup[1024];

typedef struct
{
	uint16_t	LED_AC_LOW;		 	// Anterior Canal, 0-1023
	uint16_t	LED_AC_HIGH;
	uint16_t	LED_LC_LOW;			// Lateral Canal, 0-1023
	uint16_t	LED_LC_HIGH;
	uint16_t	LED_PC_LOW;			// Posterior Canal, 0-1023
	uint16_t	LED_PC_HIGH;
	uint16_t	LED_UT_LOW;			// Utricle, 0-1023
	uint16_t	LED_UT_HIGH;
	uint16_t	LED_SA_LOW;			// Saccule, 0-1023
	uint16_t	LED_SA_HIGH;
	uint16_t	LED_LEFT;		
	uint16_t	LED_RIGHT;
	uint8_t		current_y;			// Y0, Y1, Y2
} led_array;

extern volatile led_array LEDs;

typedef struct
{
	int16_t		accel_utr;
	int16_t		accel_sac;
	int16_t		gyro_ant;
	int16_t		gyro_lat;
	int16_t		gyro_pos;
} imu_data;

void usart_setup(void);
void usart_send(uint8_t word);
void usart_print(char *msg);
int _write(int file, char *ptr, int len);
void systick_setup(int xms);
void clock_setup(void);
void gpio_setup(void);
void tim_setup(void);
void i2c_setup(void);
void i2c_write(uint8_t address, uint8_t val);
uint8_t i2c_read(uint8_t address);
uint16_t get_gamma(uint16_t val);
void updateLEDs(led_array *leds);
void getIMU(imu_data *imu);
void scaleIMU(imu_data *imu);
void setLEDs(imu_data *imu, led_array *leds);

#endif
