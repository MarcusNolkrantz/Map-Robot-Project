/*
 * sensor.c
 *
 * Created: 2019-11-07 13:24:20
 *  Author: marno874, felli675, edwjo109, matlj387
 */ 



#include "sensor.h"


#define LEFT_SENSOR 0
#define RIGHT_SENSOR 1
#define TRANSMIT_DELAY 200


//Fields
int overflow_count = 0;
uint16_t prev_time = 0;
static struct sensor_data data;
static struct sensor_state state;

/*-------Interupt handlers--------------------
---------------------------------------------*/
ISR(TIMER1_OVF_vect){
	overflow_count += 1;
}


ISR(PCINT1_vect){
	static bool rising_edge = true;
	if(rising_edge) usart_transmit_competition();
	rising_edge = !rising_edge;	
}


/*-------Functions----------------------------
---------------------------------------------*/
void init_timer(){
	TCCR1A = 0x00;
	TCCR1B = (1<<CS10) | (1<<CS12);  // Timer mode with 1024 prescaling.
	TIMSK1 = (1 << TOIE1) ;   // Enable timer1 overflow interrupt(TOIE1).	
}


void read_gyro(float *rotation) {
	i2c_start(GYRO_SAD + I2C_WRITE);
	i2c_write(GYRO_REGS);
	i2c_rep_start(GYRO_SAD+I2C_READ);
	
	uint8_t gyro_lo = i2c_readAck();
	uint8_t gyro_hi = i2c_readNak();
	i2c_stop();
	
	int16_t gyro = gyro_hi;
	gyro <<= 8;
	gyro |= gyro_lo;
	
	*rotation = (float)gyro;
}


void calc_offset(float *gyro, int n) {
	float sum_gyro = 0;
	
	for (int i = 0; i < n; i++) {
		read_gyro(gyro);
		sum_gyro += *gyro;
	}
	
	*gyro = sum_gyro / n;
}


void init_btn(){
	PCICR |= (1 << PCIE1);
	PCMSK1 |= (1 << PORTB0);
}

void 
init_i2c(){
    i2c_init();
	write_to_reg(GYRO_SAD, 0x0F, 0x20);
	write_to_reg(GYRO_SAD, 0x00, 0x23);
}

void hw_init(){
	init_usart();
	init_i2c();
	adc_init();
	init_btn();
	sei();        
	init_timer();
	sensor_init();
}


void update_angle(float offset, float *angle){
	float gyro_rotation;

	// read data
	read_gyro(&gyro_rotation);
	
	//Calculate dt
	uint16_t temp_dt = overflow_count * 65536 + TCNT1 - prev_time;
	prev_time = TCNT1;
	float dt = (PRESCALE * temp_dt) / F_CPU;
	overflow_count = 0;
	
	// integrate angle velocity
	*angle += (gyro_rotation - offset) * GYRO_SENSITIVITY_250DPS * dt;
}


uint16_t get_dist(int n){
	if(n == 0){
		uint16_t right_sensor_voltage = adc_read(0);
		return voltage_to_dist(right_sensor_voltage);
	} else{
		uint16_t left_sensor_voltage = adc_read(1);
		return voltage_to_dist(left_sensor_voltage);	
	}
}


void sensor_tick(float offset, float angle, struct sensor_data* d){
	
	uint16_t right_dist = get_dist(0);
	uint16_t left_dist = get_dist(1);
	//angle = (int)angle;	//skips two decimals, just uses integers
	int16_t gyro_angle = (int16_t)angle;	
	if(right_dist != d->right_distance || left_dist != d->left_distance || gyro_angle != d->gyro_angle){
		d->right_distance = right_dist;
		d->left_distance = left_dist;
		d->gyro_angle = gyro_angle;
		
		send_data(d);
	}	
}


void send_data(struct sensor_data* d){
	/*Data is sent in the following order every time:
		Header
		Angle(gyro)
		Right distance
		Left distance
	*/
	//Header
	cli();
	usart_transmit(MEASUREMENT);
	sei();
	_delay_us(TRANSMIT_DELAY);
	//Data
	uint8_t g_hi = (int8_t)(d->gyro_angle >> 8);
	uint8_t g_lo = (int8_t)(d->gyro_angle);
	cli();
	usart_transmit(g_hi);
	sei();
	_delay_us(TRANSMIT_DELAY);
	cli();
	usart_transmit(g_lo);
	sei();
	_delay_us(TRANSMIT_DELAY);
	
	uint8_t r_hi = (uint8_t)(d->right_distance >> 8);
	uint8_t r_lo = (uint8_t)(d->right_distance);
	cli();
	usart_transmit(r_hi);
	sei();
	_delay_us(TRANSMIT_DELAY);
	cli();
	usart_transmit(r_lo);
	sei();
	_delay_us(TRANSMIT_DELAY);
	uint8_t l_hi = (uint8_t)(d->left_distance >> 8);
	uint8_t l_lo = (uint8_t)(d->left_distance);	
	cli();
	usart_transmit(l_hi);
	sei();
	_delay_us(TRANSMIT_DELAY);
	cli();
	usart_transmit(l_lo);
	sei();
	_delay_us(TRANSMIT_DELAY);
}


void sensor_init()
{
	state.identified = false;

	while(!state.identified)
	{
		usart_transmit(SENSOR_ID);
		_delay_ms(100);
	}
}


void sensor_identified()
{
	state.identified = true;
}


int main()
{
	hw_init();
	// calculate mean offsets
	float offset_gyro;
	calc_offset(&offset_gyro, 5000);
	//static struct sensor_data data;
	static float angle = 0;
	while(1) {
		update_angle(offset_gyro, &angle);
		_delay_ms(5);
		sensor_tick(offset_gyro, angle, &data);	
	}
	
	return 0;
}

