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

#include <stdio.h>
#include <unistd.h>
#include "mcp4822.h"
#include <math.h>

int main(void)
{
	printf("MCP4822 Linux Test\n");

	if(!mcp4822_initialize()) {
		printf("Failed to initialize MCP4822.\n");
		return -1;
	}

	uint16_t voltage = 0;
	const double sine_step = 2 * M_PI / 4095.0;
	double sine_val = 0.0;
	double sine_res;

	//for(;;) {
		/* channel A: sawtooth */
		//mcp4822_set_voltage(MCP_4822_CHANNEL_A, voltage);
		//++voltage;
		//if (voltage >= 4095)
		//	voltage = 0;

		/* channel B: sine */
		//sine_res = sin(sine_val);
		//sine_res += 1.0;
		//mcp4822_set_voltage(MCP_4822_CHANNEL_B, (uint16_t)((4095.0  / 2.0) * sine_res));


		//sine_val += sine_step;
		//if(sine_val >= (2 * M_PI))
		//	sine_val = 0.0;

		//usleep(1000);
	//}

	while(1) for(int i = 0; i < 4096; i++){
	printf("%i\n", i);
	mcp4822_set_voltage(MCP_4822_CHANNEL_A, i);
	mcp4822_set_voltage(MCP_4822_CHANNEL_B, i);
	usleep(10000);
	}
	mcp4822_deinitialize();
	return 0;
}

