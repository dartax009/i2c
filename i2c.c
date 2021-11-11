#include <avr/io.h>
#include <util/twi.h>
#include "i2c.h"


#ifndef __AVR_ATmega328P__
	#include <avr/iom328p.h>
#endif

void init_i2c_master ()
{
	TWBR = 0x80;		//Устанавливаем скорость
	TWCR = 0x0;

	return;
}

uint8_t start_i2c (uint8_t addr, uint8_t flag)
{
	TWCR = 0;		//Сброс контрольного регистра

	TWCR = (1<<TWINT) | (1<<TWSTA) | (1<<TWEN);			//Становимся ведущим и включаем TWI

	while( !(TWCR & (1<<TWINT)) )
		;

	if((TWSR & 0xF8) != TW_START)
		return 1;						//Не удалось захватить шину

	TWDR = (addr<<1 | flag);			//Сдвигаем адрес и устанавливаем бит приема/передачи

	TWCR = (1<<TWINT) | (1<<TWEN);

	while( !(TWCR & (1<<TWINT)) )
		;

	uint8_t twst = TW_STATUS & 0xF8;
	if( twst != TW_MT_SLA_ACK && twst != TW_MR_SLA_ACK )
		return 2;						//Оконечник не того

	return 0;
}

void stop_i2c ()
{
	TWCR = (1<<TWINT) | (1<<TWSTO) | (1<<TWEN);	//Останавливаем TWI

	return;
}

uint8_t write_i2c_8b (uint8_t data)
{
	TWDR = data;

	TWCR = (1<<TWINT) | (1<<TWEN);		//Дергаем отправку

	while ( !(TWCR & (1<<TWINT)) )		//Ждем передачи
		;

	if ( (TWSR & 0xF8) != TW_MT_DATA_ACK )		//Проверяем подствержедение операции
		return 1;

	return 0;
}

uint8_t read_i2c_8b ()
{
	TWCR = (1<<TWINT) | (1<<TWEA) | (1<<TWEN);			//Встаем на прием

	while ( !(TWCR & (1<<TWINT)) )			//Подтверждаем заверешение приема
		;

	return TWDR;		//Отдаем полученные данные наверх
}

uint8_t a_read_i2c_24b (uint32_t *data, uint8_t reg, uint8_t addr)
{
	if (start_i2c(addr, WRITE_I2C) != 0 )		//Если инициализация успешна
		return 1;		//Не удачно дернули первый раз

	if (write_i2c_8b(reg) == 1)
		return 2; 		//Не передали регистр
	stop_i2c();

	if (start_i2c(addr, READ_I2C) !=0 )
		return 3;		//Не удачно прочитали

	*data = read_i2c_8b();
	*data <<= 8;
	*data |= read_i2c_8b();
	*data <<= 8;
	*data |= read_i2c_8b();
	stop_i2c();

	return 0;
}

uint8_t a_read_i2c_8b (uint8_t *data, uint8_t reg, uint8_t addr)
{
	if (start_i2c(addr, WRITE_I2C) != 0)
		return 1;

	if (write_i2c_8b(reg) == 1)
		return 2;
	stop_i2c();

	if (start_i2c(addr, READ_I2C) != 0 )
		return 3;

	*data = read_i2c_8b();

	return 0;
}
