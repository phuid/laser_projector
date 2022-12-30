/*==========================================================================================================*\
 * MIT License
 *
 * Copyright (c) 2022 Pawel Kusinski
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:

 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.

 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
\*==========================================================================================================*/

#include "mcp4822.h"
#include <stdint.h>
#include <stdbool.h>
#include <stdint.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/ioctl.h>
#include <linux/spi/spidev.h>
#include <math.h>

#define MCP4822_VREF_MV                ((uint16_t)2048u)
#define MCP4822_MAX_OUTPUT_MV_NO_GAIN  ((uint16_t)2047u)
#define MCP4822_MAX_OUTPUT_MV          ((uint16_t)4095u)
#define MCP4822_CHANNEL_BIT            ((uint8_t)0x80)
#define MCP4822_GAIN_BIT               ((uint8_t)0x20)
#define MCP4822_SHDN_BIT               ((uint8_t)0x10)

static const char *device = "/dev/spidev0.1";
static bool mcp4822_is_initialized;
static uint32_t mode;
static uint8_t bits = 8;
static uint16_t delay;
static uint32_t speed = 500000;
static int mcp4822_fd;

#define ARRAY_SIZE(a) (sizeof(a) / sizeof((a)[0]))

uint8_t mcp4888_tx_buffer[] = {
	0x00, 0x00
};

uint8_t ddefault_rx[ARRAY_SIZE(mcp4888_tx_buffer)] = {0, };


static void transfer(int fd, uint8_t const *tx, uint8_t const *rx, size_t len)
{
	int ret;
	struct spi_ioc_transfer tr = {
		.tx_buf = (unsigned long)tx,
		.rx_buf = (unsigned long)rx,
		.len = len,
		.speed_hz = speed,
		.delay_usecs = delay,
		.bits_per_word = bits,
	};

	ret = ioctl(mcp4822_fd, SPI_IOC_MESSAGE(1), &tr);
	
	if (ret < 1)
		printf("can't send spi message\n");
}

bool mcp4822_initialize(void)
{
	int ret = 0;
	mcp4822_fd = open(device, O_RDWR);

	if (mcp4822_fd < 0) {
		printf("1");
		return false;
	}

	ret = ioctl(mcp4822_fd, SPI_IOC_WR_MODE, &mode);

	if (-1 == ret) {
		printf("2");
		return false;
	}

	ret = ioctl(mcp4822_fd, SPI_IOC_RD_MODE, &mode);

	if (-1 == ret) {
		printf("3");
		return false;
	}

	ret = ioctl(mcp4822_fd, SPI_IOC_WR_BITS_PER_WORD, &bits);

	if (-1 == ret) {
		printf("4");
		return false;
	}

	ret = ioctl(mcp4822_fd, SPI_IOC_RD_BITS_PER_WORD, &bits);

	if (-1 == ret) {
		printf("5");
		return false;
	}

	ret = ioctl(mcp4822_fd, SPI_IOC_WR_MAX_SPEED_HZ, &speed);

	if (-1 == ret) {
		printf("6");
		return false;
	}

	ret = ioctl(mcp4822_fd, SPI_IOC_RD_MAX_SPEED_HZ, &speed);

	if (-1 == ret) {
		printf("7");
		return false;
	}

	mcp4822_is_initialized = true;
	return true;
}

bool mcp4822_set_voltage(mcp4822_channel_t channel, uint16_t value_mV)
{
	if (!mcp4822_is_initialized)
		return false;

	if(value_mV > MCP4822_MAX_OUTPUT_MV)
		return false;

	memset(mcp4888_tx_buffer, 0x00, 2);

	if (MCP_4822_CHANNEL_A == channel)
		mcp4888_tx_buffer[0] &= ~MCP4822_CHANNEL_BIT;
	else
		mcp4888_tx_buffer[0] |= MCP4822_CHANNEL_BIT;

	if (value_mV <= MCP4822_MAX_OUTPUT_MV_NO_GAIN)
		mcp4888_tx_buffer[0] |= MCP4822_GAIN_BIT;
	else
		mcp4888_tx_buffer[0] &= ~MCP4822_GAIN_BIT;

	mcp4888_tx_buffer[0] |= MCP4822_SHDN_BIT;

	uint16_t dac_input_data = 0x0000;

	if (value_mV < MCP4822_MAX_OUTPUT_MV_NO_GAIN)
		dac_input_data = value_mV * 2;
	else
		dac_input_data = value_mV;

	mcp4888_tx_buffer[0] |= ((dac_input_data & 0x0F00) >> 8);
	mcp4888_tx_buffer[1] |= (dac_input_data & 0x00FF);

	transfer(mcp4822_fd, mcp4888_tx_buffer, ddefault_rx, sizeof(mcp4888_tx_buffer));
	return true;
}

void mcp4822_deinitialize(void)
{
	close(mcp4822_fd);
	mcp4822_is_initialized = false;
}
