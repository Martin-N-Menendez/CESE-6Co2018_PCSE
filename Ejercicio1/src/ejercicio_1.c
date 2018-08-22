/* Copyright 2016, Eric Pernia.
 * All rights reserved.
 *
 * This file is part sAPI library for microcontrollers.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its
 *    contributors may be used to endorse or promote products derived from this
 *    software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 */

/*
 * Date: 2016-04-26
 */

/*==================[inclusions]=============================================*/

//#include "rtc.h"
//#include "adc_dac.h"   // <= own header (optional)
//#include "sd_spi.h"   // <= own header (optional)
#include "sapi.h"     // <= sAPI header
#include "ff.h"       // <= Biblioteca FAT FS

/*==================[macros and definitions]=================================*/

#define FILENAME "hola.txt"

/*==================[internal data declaration]==============================*/

static FATFS fs;           // <-- FatFs work area needed for each volume
static FIL fp;             // <-- File object needed for each open file

/*==================[internal functions declaration]=========================*/

/*==================[internal data definition]===============================*/

/*==================[external data definition]===============================*/
static char uartBuff[10]; /* Buffer */

/*==================[internal functions definition]==========================*/

/*==================[external functions definition]==========================*/
// FUNCION que se ejecuta cada vezque ocurre un Tick
void diskTickHook( void *ptr );
void disk_timerproc(void);

void diskTickHook( void *ptr ){
	disk_timerproc();   // Disk timer process
	return;
}

/**
 * C++ version 0.4 char* style "itoa":
 * Written by Lukás Chmela
 * Released under GPLv3.

 */
char* itoa(int value, char* result, int base) {
	// check that the base if valid
	if (base < 2 || base > 36) { *result = '\0'; return result; }

	char* ptr = result, *ptr1 = result, tmp_char;
	int tmp_value;

	do {
		tmp_value = value;
		value /= base;
		*ptr++ = "zyxwvutsrqponmlkjihgfedcba9876543210123456789abcdefghijklmnopqrstuvwxyz" [35 + (tmp_value - value * base)];
	} while ( value );

	// Apply negative sign
	if (tmp_value < 0) *ptr++ = '-';
	*ptr-- = '\0';
	while(ptr1 < ptr) {
		tmp_char = *ptr;
		*ptr--= *ptr1;
		*ptr1++ = tmp_char;
	}
	return result;
}


/* Enviar fecha y hora en formato "DD/MM/YYYY, HH:MM:SS" */
void showDateAndTime( rtc_t * rtc ){
	/* Conversion de entero a ascii con base decimal */
	itoa( (int) (rtc->mday), (char*)uartBuff, 10 ); /* 10 significa decimal */
	/* Envio el dia */
	if( (rtc->mday)<10 )
		uartWriteByte( UART_USB, '0' );
	uartWriteString( UART_USB, uartBuff );
	uartWriteByte( UART_USB, '/' );

	/* Conversion de entero a ascii con base decimal */
	itoa( (int) (rtc->month), (char*)uartBuff, 10 ); /* 10 significa decimal */
	/* Envio el mes */
	if( (rtc->month)<10 )
		uartWriteByte( UART_USB, '0' );
	uartWriteString( UART_USB, uartBuff );
	uartWriteByte( UART_USB, '/' );

	/* Conversion de entero a ascii con base decimal */
	itoa( (int) (rtc->year), (char*)uartBuff, 10 ); /* 10 significa decimal */
	/* Envio el año */
	if( (rtc->year)<10 )
		uartWriteByte( UART_USB, '0' );
	uartWriteString( UART_USB, uartBuff );


	uartWriteString( UART_USB, ", ");


	/* Conversion de entero a ascii con base decimal */
	itoa( (int) (rtc->hour), (char*)uartBuff, 10 ); /* 10 significa decimal */
	/* Envio la hora */
	if( (rtc->hour)<10 )
		uartWriteByte( UART_USB, '0' );
	uartWriteString( UART_USB, uartBuff );
	uartWriteByte( UART_USB, ':' );

	/* Conversion de entero a ascii con base decimal */
	itoa( (int) (rtc->min), (char*)uartBuff, 10 ); /* 10 significa decimal */
	/* Envio los minutos */
	// uartBuff[2] = 0;    /* NULL */
	if( (rtc->min)<10 )
		uartWriteByte( UART_USB, '0' );
	uartWriteString( UART_USB, uartBuff );
	uartWriteByte( UART_USB, ':' );

	/* Conversion de entero a ascii con base decimal */
	itoa( (int) (rtc->sec), (char*)uartBuff, 10 ); /* 10 significa decimal */
	/* Envio los segundos */
	if( (rtc->sec)<10 )
		uartWriteByte( UART_USB, '0' );
	uartWriteString( UART_USB, uartBuff );


	/* Envio un 'enter' */
	//uartWriteString( UART_USB, "\r\n");
}

/* FUNCION PRINCIPAL, PUNTO DE ENTRADA AL PROGRAMA LUEGO DE RESET. */
int main(void){

	/* ------------- INICIALIZACIONES ------------- */
	boardConfig(); /* Inicializar la placa */
	uartConfig( UART_USB, 115200 ); /* Inicializar UART_USB a 115200 baudios */

	/* Inicializar AnalogIO */
	adcConfig( ADC_ENABLE ); /* ADC */
	dacConfig( DAC_ENABLE ); /* DAC */

	spiConfig( SPI0 );  // SPI configuration

	/* Estructura RTC */
	rtc_t rtc;

	rtc.year = 2016;
	rtc.month = 7;
	rtc.mday = 3;
	rtc.wday = 1;
	rtc.hour = 13;
	rtc.min = 17;
	rtc.sec= 0;

	bool_t val = 0;
	// uint8_t i = 0;

	val = rtcConfig( &rtc ); /* Inicializar RTC */
	delay(2000);

	val = rtcWrite( &rtc );   /* Establecer fecha y hora */

	bool_t ledState1 = OFF; /* Configuración de estado inicial del Led */
	uint32_t i = 0;  /* Contador */

	uint16_t muestra = 0; /* Variable para almacenar el valor leido del ADC CH1 */

	delay_t delay1; /* Variables de delays no bloqueantes */
	delay_t delay2; /* Variables de delays no bloqueantes */

	delayConfig( &delay1, 500 ); /* Inicializar Retardo no bloqueante con tiempo en ms */
	delayConfig( &delay2, 200 ); /* Inicializar Retardo no bloqueante con tiempo en ms */

	// Inicializar el conteo de Ticks con resolucion de 10ms,
	// con tickHook diskTickHook
	//tickConfig( 10 );
	//tickCallbackSet( diskTickHook, NULL );

	// ------ PROGRAMA QUE ESCRIBE EN LA SD -------

	UINT nbytes;

	// Give a work area to the default drive
	if( f_mount( &fs, "", 0 ) != FR_OK ){
		// If this fails, it means that the function could
		// not register a file system object.
		// Check whether the SD card is correctly connected
	}

	// Create/open a file, then write a string and close it

	uint8_t j=0;

	for( j=0; j<5; j++ ){

		if( f_open( &fp, FILENAME, FA_WRITE | FA_OPEN_APPEND ) == FR_OK ){
			f_write( &fp, "Hola mundo\r\n", 12, &nbytes );

			f_close(&fp);

			if( nbytes == 12 ){
				// Turn ON LEDG if the write operation was successful
				gpioWrite( LEDG, ON );
			}
		} else{
			// Turn ON LEDR if the write operation was fail
			gpioWrite( LEDR, ON );
		}
	}

	while(1) {  /* ------------- REPETIR POR SIEMPRE ------------- */


		if ( delayRead( &delay1 ) ){ /* delayRead retorna TRUE cuando se cumple el tiempo de retardo */

			val = rtcRead( &rtc );  /* Leer fecha y hora */
			muestra = adcRead( CH1 ); /* Leo la Entrada Analogica AI0 - ADC0 CH1 */


			showDateAndTime( &rtc );  /* Mostrar fecha y hora en formato "DD/MM/YYYY, HH:MM:SS" */
			uartWriteString( UART_USB, " | " ); /* Envío la primer parte del mnesaje a la Uart */

			itoa( muestra, uartBuff, 10 ); /* Conversión de muestra entera a ascii con base decimal */

			uartWriteString( UART_USB, uartBuff ); /* Enviar muestra y Enter */
			uartWriteString( UART_USB, ";\r\n" );

			dacWrite( DAC, muestra ); /* Escribo la muestra en la Salida AnalogicaAO - DAC */
		}


		if ( delayRead( &delay2 ) ){ /* delayRead retorna TRUE cuando se cumple el tiempo de retardo */
			ledState1 = !ledState1;
			gpioWrite( LED1, ledState1 );

			i++;
			if( i == 20 ) /* Si pasaron 20 delays le aumento el tiempo */
				delayWrite( &delay2, 1000 );
		}
		//sleepUntilNextInterrupt();
	}

	/* NO DEBE LLEGAR NUNCA AQUI*/
	return 0 ;
}

/*==================[end of file]============================================*/
