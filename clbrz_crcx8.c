/*
 ============================================================================

 ██████╗██████╗  ██████╗██╗  ██╗ █████╗
██╔════╝██╔══██╗██╔════╝╚██╗██╔╝██╔══██╗
██║     ██████╔╝██║      ╚███╔╝ ╚█████╔╝
██║     ██╔══██╗██║      ██╔██╗ ██╔══██╗
╚██████╗██║  ██║╚██████╗██╔╝ ██╗╚█████╔╝
 ╚═════╝╚═╝  ╚═╝ ╚═════╝╚═╝  ╚═╝ ╚════╝

	Author      : clbrz
	Version     : v1.3

	This is free and unencumbered software released into the public domain.

    Anyone is free to copy, modify, publish, use, compile, sell, or
    distribute this software, either in source code form or as a compiled
    binary, for any purpose, commercial or non-commercial, and by any
    means.

    In jurisdictions that recognize copyright laws, the author or authors
    of this software dedicate any and all copyright interest in the
    software to the public domain. We make this dedication for the benefit
    of the public at large and to the detriment of our heirs and
    successors. We intend this dedication to be an overt act of
    relinquishment in perpetuity of all present and future rights to this
    software under copyright law.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
    EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
    MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
    IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR
    OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
    ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
    OTHER DEALINGS IN THE SOFTWARE.

    For more information, please refer to <http://unlicense.org/>
    
	Description : We wanted a self-understood, preferably self-implemented crc code, that we can use freely.
				 After a lot of hand-wringing and hair pulling, we finally understood the general
				 class of CRC algorithms for various widths, and the implementation level tricks as well.
				 REFERENCES:
				 >> theory, background:
					 http://www.zlib.net/crc_v3.txt (Ross Williams)
				 >> practical, implementation oriented:
					 http://www.sunshine2k.de/articles/coding/crc/understanding_crc.html (Bastian Molkenthin)
					 check out the references section specially.
				 >> handbook/reference for current CRC algos:
					 http://reveng.sourceforge.net/crc-catalogue/ (Greg Cook)
				 >> some more explanations:
					 http://www.alterawiki.com/wiki/Practical_CRC_Usage
				 Thanks to these guys, I finally have a working and clearly understood crc implementation.
				 the goal is to target any crc-8/crc-16/crc-32 algorithms out there, leaving out the exotic crc widths (12/24/31...)
				 and to ensure that all the common options can be changed on the go, to keep it flexible.
				 this is all public domain code.

	credits: http://patorjk.com/software/taag/#p=display&h=0&v=1&f=ANSI%20Shadow&t=crcx8

	usage :
	(1) decide if you want to use TABLE -> yes if you don't know, leave the CLBRZCRCX8_USE_TABLE_FOR_CRC defined
	(3) create the CRCTypeDescriptor_t instance, with the parameters: name width poly init xorout refin refout check_value
	name is an optional string to describe this crc configuration
	check_value is optional, and is the result of the CRC algo running over the string: "123456789\0"
	if valid check_value is passed in, then crc_self_test() can be used to check if this algo is ok.
	(2) call init_crc() with the CRCTypeDescriptor_t instance address.
	(3) call calculate_crc_chunk() as many times on the data stream, each call returns an "intermediate crc" if you would like to use this
	(4) call finalize_crc(), returns the calculated crc.

	To repeat crc on another set of data with the same crc config:
	(1) call reset_crc()
	(2) follow steps (3) and (4) as above

 ============================================================================
 */


#include "clbrz_crcx8.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>




#ifdef CLBRZCRCX8_USE_TABLE_FOR_CRC

#ifdef CLBRZCRCX8_ENABLE_TABLE_GENERATION

uint32_t clbrzcrcx8_crc_table [256] = {0};

#else

uint32_t clbrzcrcx8_crc_table[256] =
{
0x00000000, 0x04c11db7, 0x09823b6e, 0x0d4326d9, 0x130476dc, 0x17c56b6b, 0x1a864db2, 0x1e475005,
0x2608edb8, 0x22c9f00f, 0x2f8ad6d6, 0x2b4bcb61, 0x350c9b64, 0x31cd86d3, 0x3c8ea00a, 0x384fbdbd,
0x4c11db70, 0x48d0c6c7, 0x4593e01e, 0x4152fda9, 0x5f15adac, 0x5bd4b01b, 0x569796c2, 0x52568b75,
0x6a1936c8, 0x6ed82b7f, 0x639b0da6, 0x675a1011, 0x791d4014, 0x7ddc5da3, 0x709f7b7a, 0x745e66cd,
0x9823b6e0, 0x9ce2ab57, 0x91a18d8e, 0x95609039, 0x8b27c03c, 0x8fe6dd8b, 0x82a5fb52, 0x8664e6e5,
0xbe2b5b58, 0xbaea46ef, 0xb7a96036, 0xb3687d81, 0xad2f2d84, 0xa9ee3033, 0xa4ad16ea, 0xa06c0b5d,
0xd4326d90, 0xd0f37027, 0xddb056fe, 0xd9714b49, 0xc7361b4c, 0xc3f706fb, 0xceb42022, 0xca753d95,
0xf23a8028, 0xf6fb9d9f, 0xfbb8bb46, 0xff79a6f1, 0xe13ef6f4, 0xe5ffeb43, 0xe8bccd9a, 0xec7dd02d,
0x34867077, 0x30476dc0, 0x3d044b19, 0x39c556ae, 0x278206ab, 0x23431b1c, 0x2e003dc5, 0x2ac12072,
0x128e9dcf, 0x164f8078, 0x1b0ca6a1, 0x1fcdbb16, 0x018aeb13, 0x054bf6a4, 0x0808d07d, 0x0cc9cdca,
0x7897ab07, 0x7c56b6b0, 0x71159069, 0x75d48dde, 0x6b93dddb, 0x6f52c06c, 0x6211e6b5, 0x66d0fb02,
0x5e9f46bf, 0x5a5e5b08, 0x571d7dd1, 0x53dc6066, 0x4d9b3063, 0x495a2dd4, 0x44190b0d, 0x40d816ba,
0xaca5c697, 0xa864db20, 0xa527fdf9, 0xa1e6e04e, 0xbfa1b04b, 0xbb60adfc, 0xb6238b25, 0xb2e29692,
0x8aad2b2f, 0x8e6c3698, 0x832f1041, 0x87ee0df6, 0x99a95df3, 0x9d684044, 0x902b669d, 0x94ea7b2a,
0xe0b41de7, 0xe4750050, 0xe9362689, 0xedf73b3e, 0xf3b06b3b, 0xf771768c, 0xfa325055, 0xfef34de2,
0xc6bcf05f, 0xc27dede8, 0xcf3ecb31, 0xcbffd686, 0xd5b88683, 0xd1799b34, 0xdc3abded, 0xd8fba05a,
0x690ce0ee, 0x6dcdfd59, 0x608edb80, 0x644fc637, 0x7a089632, 0x7ec98b85, 0x738aad5c, 0x774bb0eb,
0x4f040d56, 0x4bc510e1, 0x46863638, 0x42472b8f, 0x5c007b8a, 0x58c1663d, 0x558240e4, 0x51435d53,
0x251d3b9e, 0x21dc2629, 0x2c9f00f0, 0x285e1d47, 0x36194d42, 0x32d850f5, 0x3f9b762c, 0x3b5a6b9b,
0x0315d626, 0x07d4cb91, 0x0a97ed48, 0x0e56f0ff, 0x1011a0fa, 0x14d0bd4d, 0x19939b94, 0x1d528623,
0xf12f560e, 0xf5ee4bb9, 0xf8ad6d60, 0xfc6c70d7, 0xe22b20d2, 0xe6ea3d65, 0xeba91bbc, 0xef68060b,
0xd727bbb6, 0xd3e6a601, 0xdea580d8, 0xda649d6f, 0xc423cd6a, 0xc0e2d0dd, 0xcda1f604, 0xc960ebb3,
0xbd3e8d7e, 0xb9ff90c9, 0xb4bcb610, 0xb07daba7, 0xae3afba2, 0xaafbe615, 0xa7b8c0cc, 0xa379dd7b,
0x9b3660c6, 0x9ff77d71, 0x92b45ba8, 0x9675461f, 0x8832161a, 0x8cf30bad, 0x81b02d74, 0x857130c3,
0x5d8a9099, 0x594b8d2e, 0x5408abf7, 0x50c9b640, 0x4e8ee645, 0x4a4ffbf2, 0x470cdd2b, 0x43cdc09c,
0x7b827d21, 0x7f436096, 0x7200464f, 0x76c15bf8, 0x68860bfd, 0x6c47164a, 0x61043093, 0x65c52d24,
0x119b4be9, 0x155a565e, 0x18197087, 0x1cd86d30, 0x029f3d35, 0x065e2082, 0x0b1d065b, 0x0fdc1bec,
0x3793a651, 0x3352bbe6, 0x3e119d3f, 0x3ad08088, 0x2497d08d, 0x2056cd3a, 0x2d15ebe3, 0x29d4f654,
0xc5a92679, 0xc1683bce, 0xcc2b1d17, 0xc8ea00a0, 0xd6ad50a5, 0xd26c4d12, 0xdf2f6bcb, 0xdbee767c,
0xe3a1cbc1, 0xe760d676, 0xea23f0af, 0xeee2ed18, 0xf0a5bd1d, 0xf464a0aa, 0xf9278673, 0xfde69bc4,
0x89b8fd09, 0x8d79e0be, 0x803ac667, 0x84fbdbd0, 0x9abc8bd5, 0x9e7d9662, 0x933eb0bb, 0x97ffad0c,
0xafb010b1, 0xab710d06, 0xa6322bdf, 0xa2f33668, 0xbcb4666d, 0xb8757bda, 0xb5365d03, 0xb1f740b4,
};


#endif // #ifdef CLBRZCRCX8_ENABLE_TABLE_GENERATION

#endif // #ifdef CLBRZCRCX8_USE_TABLE_FOR_CRC


#define TOPBIT(width)	 		(1UL << (width-1UL))

// this portable mask stuff was stolen from: http://www.zlib.net/crc_v3.txt, Ross Williams, avoid for e.g << 32 on a 32-bit machine !!
// the original, naive stuff is commented below, followed by the magic way to do it right.
//#define CRC_MASK(width)			((1 << (width)) - 1)
#define CRC_MASK(width)			((((1UL<<(width-1))-1UL)<<1UL)|1UL)

#define BITMASK(X) 				(1UL << (X))



// stolen from:  http://www.zlib.net/crc_v3.txt, Ross Williams.
// Returns the value with the bottom b [0,n] bits reflected.
// Example: reflect(0x3e23L,3) == 0x3e26
uint32_t clbrzcrcx8_reflect(uint32_t value, uint8_t num_bits_to_reflect)
{
	int bit_index;
	uint32_t temp_value = value;
	for (bit_index = 0; bit_index < num_bits_to_reflect; bit_index++)
	{
		if (temp_value & (uint32_t)1)
		{
			value |=  BITMASK((num_bits_to_reflect-1)-bit_index);
		}
		else
		{
			value &= ~BITMASK((num_bits_to_reflect-1)-bit_index);
		}

		temp_value >>= 1;
	}

	return value;
}


// INTERNAL FUNCTIONS
#ifdef CLBRZCRCX8_ENABLE_TABLE_GENERATION
void _clbrzcrcx8_generate_crc_table(uint32_t generator_polynomial, uint8_t crc_width)
{
	uint16_t byte_value;
	uint8_t bit_index;
	uint32_t crc_value;

	// iterate over all byte values 0 - 255
	for (byte_value = 0; byte_value < 256; byte_value++)
	{
		// calculate the CRC-8 value for current byte

		crc_value = byte_value << (crc_width - 8); // move byte into MSB of 32Bit CRC

		for (bit_index = 0; bit_index < 8; bit_index++)
		{
			if ((crc_value & TOPBIT(crc_width)) != 0)
			{
				crc_value = (crc_value << 1) ^ generator_polynomial;
			}
			else
			{
				crc_value <<= 1;
			}
		}
		/* store CRC value in lookup table */
		clbrzcrcx8_crc_table[byte_value] = (crc_value & CRC_MASK(crc_width));
	}
}
#endif // #ifdef CLBRZCRCX8_ENABLE_TABLE_GENERATION


#ifdef CLBRZCRCX8_ENABLE_TABLE_GENERATION
void _clbrzcrcx8_print_crc_table(uint8_t crc_width)
{
	uint16_t byte_value;

	printf("clbrzcrcx8_crc_table[256] = \n{\n");

	// iterate over all byte values 0 - 255
	for (byte_value = 0; byte_value < 256; byte_value+=8)
	{
		// 4 bits = 1 hex, use format specifier * for variable based substitution
		// https://stackoverflow.com/questions/5932214/printf-string-variable-length-item
		printf("0x%0*x, 0x%0*x, 0x%0*x, 0x%0*x, 0x%0*x, 0x%0*x, 0x%0*x, 0x%0*x,\n",
															crc_width/4,clbrzcrcx8_crc_table[byte_value],
															crc_width/4,clbrzcrcx8_crc_table[byte_value+1],
															crc_width/4,clbrzcrcx8_crc_table[byte_value+2],
															crc_width/4,clbrzcrcx8_crc_table[byte_value+3],
															crc_width/4,clbrzcrcx8_crc_table[byte_value+4],
															crc_width/4,clbrzcrcx8_crc_table[byte_value+5],
															crc_width/4,clbrzcrcx8_crc_table[byte_value+6],
															crc_width/4,clbrzcrcx8_crc_table[byte_value+7]);
	}

	printf("};\n\n");
}
#endif // #ifdef CLBRZCRCX8_ENABLE_TABLE_GENERATION



// internal variables to keep track of chunked crc configuration
// default config - CRC-32 : width=32 poly=0x04c11db7 init=0xffffffff refin=true refout=true xorout=0xffffffff check=0xcbf43926 name="CRC-32"

struct crc_info
{
	CLBRZCRCx8_CRCTypeDescriptor_t crc_configuration;
	uint32_t calculated_crc;
};

// keep the currently configured crc and its state in memory.
static
struct crc_info current_crc_info =
				{
					{
						"default",		// optional here, can be any string
						32,
						0x04c11db7,
						0xffffffff,
						0xffffffff,
						1,
						1,
						0xcbf43926,		// optional here : check
						0x00000000,		// optional here : residue
					},
					0xffffffff
				};


void clbrzcrcx8_init_crc(CLBRZCRCx8_CRCTypeDescriptor_t* crc_configuration_ptr)
{
	current_crc_info.crc_configuration = (*crc_configuration_ptr);

	current_crc_info.calculated_crc =
			current_crc_info.crc_configuration.initial_value
			&
			CRC_MASK(current_crc_info.crc_configuration.width);

#ifdef CLBRZCRCX8_ENABLE_TABLE_GENERATION
	clbrzcrcx8_generate_crc_table();
#endif // CLBRZCRCX8_USE_TABLE_FOR_CRC
}


uint32_t clbrzcrcx8_calculate_crc_chunk(uint8_t* byte_data, int32_t data_len)
{
	int32_t byte_data_index;
#ifndef CLBRZCRCX8_USE_TABLE_FOR_CRC
	int32_t bit_index;
#endif // #ifdef CLBRZCRCX8_USE_TABLE_FOR_CRC

	// start from previous CRC value
	uint32_t calculated_crc =
			current_crc_info.calculated_crc
			&
			CRC_MASK(current_crc_info.crc_configuration.width);

	for(byte_data_index = 0; byte_data_index < data_len; byte_data_index++) // for each byte of data:
	{
		// xor in the next input byte, **at the MSB**
		if(current_crc_info.crc_configuration.reflect_input == 1)
		{
			calculated_crc ^= clbrzcrcx8_reflect(byte_data[byte_data_index],8) << (current_crc_info.crc_configuration.width-8);
			calculated_crc = calculated_crc & CRC_MASK(current_crc_info.crc_configuration.width);
		}
		else
		{
			calculated_crc ^= byte_data[byte_data_index] << (current_crc_info.crc_configuration.width-8);
			calculated_crc = calculated_crc & CRC_MASK(current_crc_info.crc_configuration.width);
		}

#ifdef CLBRZCRCX8_USE_TABLE_FOR_CRC

		// http://www.sunshine2k.de/articles/coding/crc/understanding_crc.html
		// (1)The important point is here that after xoring the current byte into the MSB of the intermediate CRC,
		// the MSB is the index into the lookup table, so take ONLY MSB for lookup table index... this explains the crc_value >> (WIDTH-8)
		// used for the lookup table.
		// (2) now, as we are getting the value corresponding to MSB from the lookup table, drop the MSB from the crc
		// i.e. shift the crc left, dropping the msb, then XOR this crc/remainder with the lookuptable value.
		calculated_crc = (calculated_crc << 8) ^ (clbrzcrcx8_crc_table[calculated_crc >> (current_crc_info.crc_configuration.width-8) ]);
		calculated_crc = calculated_crc & CRC_MASK(current_crc_info.crc_configuration.width);

#else


		// calculate crc by iterating over each bit of current byte and applying polynomial.
		for (bit_index = 0; bit_index < 8; bit_index++)
		{
			// if the MSbit is 1, left-shift and apply the polynomial, else just left-shift
			if ((calculated_crc & TOPBIT(current_crc_info.crc_configuration.width)) != 0)
			{
				calculated_crc = ( ( (calculated_crc << 1) ^ current_crc_info.crc_configuration.polynomial ) & CRC_MASK(current_crc_info.crc_configuration.width) );
			}
			else
			{
				calculated_crc <<= 1;
			}
		}


#endif // CLBRZCRCX8_USE_TABLE_FOR_CRC


		// at this point, we have the calculated crc upto the current byte.
	}
	current_crc_info.calculated_crc  =	calculated_crc;

	return current_crc_info.calculated_crc;
}


uint32_t clbrzcrcx8_reset_crc_chunk()
{
	current_crc_info.calculated_crc =
			current_crc_info.crc_configuration.initial_value
			&
			CRC_MASK(current_crc_info.crc_configuration.width);


	return current_crc_info.calculated_crc;
}


uint32_t clbrzcrcx8_finalize_crc()
{
	uint32_t calculated_crc =
			current_crc_info.calculated_crc
			&
			CRC_MASK(current_crc_info.crc_configuration.width);

	// xor with final_xor_value:
	if(current_crc_info.crc_configuration.reflect_output == 1)
	{
		calculated_crc =  clbrzcrcx8_reflect(calculated_crc,current_crc_info.crc_configuration.width) & CRC_MASK(current_crc_info.crc_configuration.width);
		calculated_crc ^= current_crc_info.crc_configuration.final_xor_value;
	}
	else
	{
		calculated_crc ^= current_crc_info.crc_configuration.final_xor_value;
	}
	current_crc_info.calculated_crc  =	calculated_crc;

	return current_crc_info.calculated_crc;
}


#ifdef CLBRZCRCX8_ENABLE_TABLE_GENERATION
void clbrzcrcx8_generate_crc_table()
{
	_clbrzcrcx8_generate_crc_table(current_crc_info.crc_configuration.polynomial,
						current_crc_info.crc_configuration.width);
}

void clbrzcrcx8_print_crc_table()
{
	_clbrzcrcx8_print_crc_table(current_crc_info.crc_configuration.width);
}
#endif // #ifdef CLBRZCRCX8_ENABLE_TABLE_GENERATION


#ifdef CLBRZCRCX8_ENABLE_CRC_SELF_RESIDUE
uint32_t clbrzcrcx8_calculate_self_residue()
{
	// WORK IN PROGRESS, does not work as expected with refin=refout=true!
	uint32_t calculated_crc;
	//uint32_t residue[3];
	uint8_t _data [14];
	uint8_t crc_width_bytes = current_crc_info.crc_configuration.width/8;

	memcpy(_data, (uint8_t*)"123456789", 9);
	clbrzcrcx8_reset_crc_chunk();
	calculated_crc = clbrzcrcx8_calculate_crc_chunk(_data,9);
	calculated_crc = clbrzcrcx8_finalize_crc();
	clbrzcrcx8_reset_crc_chunk();
	if(current_crc_info.crc_configuration.reflect_input == 0 &&
			current_crc_info.crc_configuration.reflect_output == 0 )
	{
		//calculated_crc = 0;
	}
	memcpy(_data + 9, (uint8_t*)&calculated_crc,  crc_width_bytes);
	calculated_crc = clbrzcrcx8_calculate_crc_chunk(_data,9 + crc_width_bytes);
	calculated_crc = clbrzcrcx8_finalize_crc();
	clbrzcrcx8_reset_crc_chunk();

	printf("residue: 0x%08x\n\n",calculated_crc);



	memcpy(_data, (uint8_t*)"987654321", 9);
	clbrzcrcx8_reset_crc_chunk();
	calculated_crc = clbrzcrcx8_calculate_crc_chunk(_data,9);
	calculated_crc = clbrzcrcx8_finalize_crc();
	clbrzcrcx8_reset_crc_chunk();
	if(current_crc_info.crc_configuration.reflect_input == 0 &&
			current_crc_info.crc_configuration.reflect_output == 0 )
	{
		//calculated_crc = 0;
	}
	memcpy(_data + 9, (uint8_t*)&calculated_crc,  crc_width_bytes);
	calculated_crc = clbrzcrcx8_calculate_crc_chunk(_data,9 + crc_width_bytes);
	calculated_crc = clbrzcrcx8_finalize_crc();
	clbrzcrcx8_reset_crc_chunk();

	printf("residue: 0x%08x\n\n",calculated_crc);



	memcpy(_data, (uint8_t*)"123123123", 9);
	clbrzcrcx8_reset_crc_chunk();
	calculated_crc = clbrzcrcx8_calculate_crc_chunk(_data,9);
	calculated_crc = clbrzcrcx8_finalize_crc();
	clbrzcrcx8_reset_crc_chunk();
	if(current_crc_info.crc_configuration.reflect_input == 0 &&
			current_crc_info.crc_configuration.reflect_output == 0 )
	{
		//calculated_crc = 0;
	}
	memcpy(_data + 9, (uint8_t*)&calculated_crc,  crc_width_bytes);
	calculated_crc = clbrzcrcx8_calculate_crc_chunk(_data,9 + crc_width_bytes);
	calculated_crc = clbrzcrcx8_finalize_crc();
	clbrzcrcx8_reset_crc_chunk();

	printf("residue: 0x%08x\n\n",calculated_crc);



	memcpy(_data, (uint8_t*)"123123123123", 12);
	clbrzcrcx8_reset_crc_chunk();
	calculated_crc = clbrzcrcx8_calculate_crc_chunk(_data,12);
	calculated_crc = clbrzcrcx8_finalize_crc();
	clbrzcrcx8_reset_crc_chunk();
	if(current_crc_info.crc_configuration.reflect_input == 0 &&
			current_crc_info.crc_configuration.reflect_output == 0 )
	{
		//calculated_crc = 0;
	}
	memcpy(_data + 12, (uint8_t*)&calculated_crc,  crc_width_bytes);
	calculated_crc = clbrzcrcx8_calculate_crc_chunk(_data,12 + crc_width_bytes);
	calculated_crc = clbrzcrcx8_finalize_crc();
	clbrzcrcx8_reset_crc_chunk();

	printf("residue: 0x%08x\n\n",calculated_crc);

	//NOTE: residue is reflected in our algo compared to reveng catalogue.

	return 0;
}
#endif // #ifdef CLBRZCRCX8_ENABLE_CRC_SELF_RESIDUE


#ifdef CLBRZCRCX8_ENABLE_CRC_SELF_TEST
// init_crc() must be called with valid configuration and valid check value before this !
int clbrzcrcx8_self_test()
{
	uint32_t calculated_crc;
	// assumed that the user has set proper configuration using init_crc, including the check value.
	// the CRC algorithm is run over the string : "123456789\0" and calculated crc should be the check value.
	// if not, then the crc implementation has a problem with the particular configuration or the check value is wrong.
	// this is more of a sanity test to check that the crc works ok, and not meant to be used for anything else.
	clbrzcrcx8_reset_crc_chunk();
	calculated_crc = clbrzcrcx8_calculate_crc_chunk((uint8_t*)"123456789",9);
	calculated_crc = clbrzcrcx8_finalize_crc();
	clbrzcrcx8_reset_crc_chunk();

	if(calculated_crc != current_crc_info.crc_configuration.check_value)
	{
		//printf("\n%-20s : self-test failed.\n", current_crc_info.crc_configuration.crc_algo_name);
		return 0; // failed self test
	}

	//printf("\n%-20s : self-test ok.\n", current_crc_info.crc_configuration.crc_algo_name);
	return 1; // ok
}
#endif // #ifdef CLBRZCRCX8_ENABLE_CRC_SELF_TEST


#ifdef CLBRZCRCX8_ENABLE_CRC_TEST
int clbrzcrcx8_check_crc_8_algo()
{
	uint8_t calculated_crc;
	CLBRZCRCx8_CRCTypeDescriptor_t crc_configuration;

	// CRC-8 : width=8 poly=0x07 init=0x00 refin=false refout=false xorout=0x00 check=0xf4 name="CRC-8"
	crc_configuration.name = "CRC-8";					// optional
	crc_configuration.width = 8;
	crc_configuration.polynomial = 0x07;
	crc_configuration.initial_value = 0x00;
	crc_configuration.final_xor_value = 0x00;
	crc_configuration.reflect_input = 0;
	crc_configuration.reflect_output = 0;
	crc_configuration.check_value = 0xf4;						// optional, check = CRC of string "123456789\0"
	crc_configuration.residue = 0x00;							// optional, residue = CRC (DATA + CRC)
	clbrzcrcx8_init_crc(&crc_configuration);

	calculated_crc = (uint8_t)clbrzcrcx8_calculate_crc_chunk((uint8_t*)"123456789",9);
	calculated_crc = (uint8_t)clbrzcrcx8_finalize_crc();
	//print_crc_table();

	if(0xf4 != calculated_crc)
	{
		printf ("CRC-8 reference check failed!\n\n");
		return -1;
	}
	else
	{
		printf ("CRC-8 check passed.\n\n");
	}


	// CRC-8/CDMA2000 : width=8 poly=0x9b init=0xff refin=false refout=false xorout=0x00 check=0xda name="CRC-8/CDMA2000"
	crc_configuration.name = "CRC-8/CDMA2000";					// optional
	crc_configuration.width = 8;
	crc_configuration.polynomial = 0x9b;
	crc_configuration.initial_value = 0xff;
	crc_configuration.final_xor_value = 0x00;
	crc_configuration.reflect_input = 0;
	crc_configuration.reflect_output = 0;
	crc_configuration.check_value = 0xda;								// optional, check = CRC of string "123456789\0"
	crc_configuration.residue = 0x00;							// optional, residue = CRC (DATA + CRC)
	clbrzcrcx8_init_crc(&crc_configuration);
	calculated_crc = (uint8_t)clbrzcrcx8_calculate_crc_chunk((uint8_t*)"123456789",9);
	calculated_crc = (uint8_t)clbrzcrcx8_finalize_crc();

	if(0xda != calculated_crc)
	{
		printf ("CRC-8/CDMA2000 reference check failed!\n\n");
		return -1;
	}
	else
	{
		printf ("CRC-8/CDMA2000 check passed.\n\n");
	}


	// CRC-8/ITU : width=8 poly=0x07 init=0x00 refin=false refout=false xorout=0x55 check=0xa1 name="CRC-8/ITU"
	crc_configuration.name = "CRC-8/ITU";						// optional
	crc_configuration.width = 8;
	crc_configuration.polynomial = 0x07;
	crc_configuration.initial_value = 0x00;
	crc_configuration.final_xor_value = 0x55;
	crc_configuration.reflect_input = 0;
	crc_configuration.reflect_output = 0;
	crc_configuration.check_value = 0xa1;								// optional, check = CRC of string "123456789\0"
	crc_configuration.residue = 0x00;									// optional, residue = CRC (DATA + CRC)
	clbrzcrcx8_init_crc(&crc_configuration);
	calculated_crc = (uint8_t)clbrzcrcx8_calculate_crc_chunk((uint8_t*)"123456789",9);
	calculated_crc = (uint8_t)clbrzcrcx8_finalize_crc();

	if(0xa1 != calculated_crc)
	{
		printf ("CRC-8/ITU reference check failed!\n\n");
		return -1;
	}
	else
	{
		printf ("CRC-8/ITU check passed.\n\n");
	}


	// CRC-8/DARC : width=8 poly=0x39 init=0x00 refin=true refout=true xorout=0x00 check=0x15 name="CRC-8/DARC"
	crc_configuration.name = "CRC-8/DARC";						// optional
	crc_configuration.width = 8;
	crc_configuration.polynomial = 0x39;
	crc_configuration.initial_value = 0x00;
	crc_configuration.final_xor_value = 0x00;
	crc_configuration.reflect_input = 1;
	crc_configuration.reflect_output = 1;
	crc_configuration.check_value = 0x15;								// optional, check = CRC of string "123456789\0"
	crc_configuration.residue = 0x00;							// optional, residue = CRC (DATA + CRC)
	clbrzcrcx8_init_crc(&crc_configuration);
	clbrzcrcx8_calculate_crc_chunk((uint8_t*)"123456789",9);
	calculated_crc = (uint8_t)clbrzcrcx8_finalize_crc();

	if(0x15 != calculated_crc)
	{
		printf ("CRC-8/DARC reference check failed!\n\n");
		return -1;
	}
	else
	{
		printf ("CRC-8/DARC check passed.\n\n");
	}


	return 1; // ok.
}


int clbrzcrcx8_check_crc_16_algo()
{
	uint16_t calculated_crc;
	CLBRZCRCx8_CRCTypeDescriptor_t crc_configuration;

	// CRC-16/AUG-CCITT : width=16 poly=0x1021 init=0x1d0f refin=false refout=false xorout=0x0000 check=0xe5cc name="CRC-16/AUG-CCITT"
	crc_configuration.name = "CRC-16/AUG-CCITT";			// optional
	crc_configuration.width = 16;
	crc_configuration.polynomial = 0x1021;
	crc_configuration.initial_value = 0x1d0f;
	crc_configuration.final_xor_value = 0x0000;
	crc_configuration.reflect_input = 0;
	crc_configuration.reflect_output = 0;
	crc_configuration.check_value = 0xe5cc;							// optional, check = CRC of string "123456789\0"
	crc_configuration.residue = 0x0000;								// optional, residue = CRC (DATA + CRC)
	clbrzcrcx8_init_crc(&crc_configuration);
	calculated_crc = (uint16_t)clbrzcrcx8_calculate_crc_chunk((uint8_t*)"123456789",9);
	calculated_crc = (uint16_t)clbrzcrcx8_finalize_crc();
	//print_crc_table();

	if(0xe5cc != calculated_crc)
	{
		printf ("CRC-16/AUG-CCITT reference check failed!\n\n");
		return -1;
	}
	else
	{
		printf ("CRC-16/AUG-CCITT check passed.\n\n");
	}


	// CRC-16/CCITT-FALSE: width=16 poly=0x1021 init=0xffff refin=false refout=false xorout=0x0000 check=0x29b1 name="CRC-16/CCITT-FALSE"
	crc_configuration.name = "CRC-16/CCITT-FALSE";			// optional
	crc_configuration.width = 16;
	crc_configuration.polynomial = 0x1021;
	crc_configuration.initial_value = 0xffff;
	crc_configuration.final_xor_value = 0x0000;
	crc_configuration.reflect_input = 0;
	crc_configuration.reflect_output = 0;
	crc_configuration.check_value = 0x29b1;							// optional, check = CRC of string "123456789\0"
	crc_configuration.residue = 0x0000;								// optional, residue = CRC (DATA + CRC)
	clbrzcrcx8_init_crc(&crc_configuration);
	calculated_crc = (uint16_t)clbrzcrcx8_calculate_crc_chunk((uint8_t*)"123456789",9);
	calculated_crc = (uint16_t)clbrzcrcx8_finalize_crc();

	if(0x29b1 != calculated_crc)
	{
		printf ("CRC-16/CCITT-FALSE reference check failed!\n\n");
		return -1;
	}
	else
	{
		printf ("CRC-16/CCITT-FALSE check passed.\n\n");
	}


	// CRC-16/ARC : width=16 poly=0x8005 init=0x0000 refin=true refout=true xorout=0x0000 check=0xbb3d name="ARC"
	crc_configuration.name = "CRC-16/ARC";					// optional
	crc_configuration.width = 16;
	crc_configuration.polynomial = 0x8005;
	crc_configuration.initial_value = 0x0000;
	crc_configuration.final_xor_value = 0x0000;
	crc_configuration.reflect_input = 1;
	crc_configuration.reflect_output = 1;
	crc_configuration.check_value = 0xbb3d;							// optional, check = CRC of string "123456789\0"
	crc_configuration.residue = 0x0000;								// optional, residue = CRC (DATA + CRC)
	clbrzcrcx8_init_crc(&crc_configuration);
	calculated_crc = (uint16_t)clbrzcrcx8_calculate_crc_chunk((uint8_t*)"123456789",9);
	calculated_crc = (uint16_t)clbrzcrcx8_finalize_crc();

	if(0xbb3d != calculated_crc)
	{
		printf ("CRC-16/ARC reference check failed!\n\n");
		return -1;
	}
	else
	{
		printf ("CRC-16/ARC check passed.\n\n");
	}


	return 1; // ok.
}


int clbrzcrcx8_check_crc_32_algo()
{
	uint32_t calculated_crc;
	CLBRZCRCx8_CRCTypeDescriptor_t crc_configuration;

	// CRC-32/BZIP2 : width=32 poly=0x04c11db7 init=0xffffffff refin=false refout=false xorout=0xffffffff check=0xfc891918 name="CRC-32/BZIP2"
	crc_configuration.name = "CRC-32/BZIP2";					// optional
	crc_configuration.width = 32;
	crc_configuration.polynomial = 0x04c11db7;
	crc_configuration.initial_value = 0xffffffff;
	crc_configuration.final_xor_value = 0xffffffff;
	crc_configuration.reflect_input = 0;
	crc_configuration.reflect_output = 0;
	crc_configuration.check_value = 0xfc891918;							// optional, check = CRC of string "123456789\0"
	crc_configuration.residue = 0x00000000;								// optional, residue = CRC (DATA + CRC)
	clbrzcrcx8_init_crc(&crc_configuration);
	calculated_crc = (uint32_t)clbrzcrcx8_calculate_crc_chunk((uint8_t*)"123456789",9);
	calculated_crc = (uint32_t)clbrzcrcx8_finalize_crc();

	if(0xfc891918 != calculated_crc)
	{
		printf ("CRC-32/BZIP2 reference check failed!\n\n");
		return -1;
	}
	else
	{
		printf ("CRC-32/BZIP2 check passed.\n\n");
	}

	// CRC-32 : width=32 poly=0x04c11db7 init=0xffffffff refin=true refout=true xorout=0xffffffff check=0xcbf43926 name="CRC-32"
	crc_configuration.name = "CRC-32";					// optional
	crc_configuration.width = 32;
	crc_configuration.polynomial = 0x04c11db7;
	crc_configuration.initial_value = 0xffffffff;
	crc_configuration.final_xor_value = 0xffffffff;
	crc_configuration.reflect_input = 1;
	crc_configuration.reflect_output = 1;
	crc_configuration.check_value = 0xcbf43926;						// optional, check = CRC of string "123456789\0"
	crc_configuration.residue = 0x00000000;							// optional, residue = CRC (DATA + CRC)
	clbrzcrcx8_init_crc(&crc_configuration);
	calculated_crc = (uint32_t)clbrzcrcx8_calculate_crc_chunk((uint8_t*)"123456789",9);
	calculated_crc = (uint32_t)clbrzcrcx8_finalize_crc();
	//print_crc_table();

	if(0xcbf43926 != calculated_crc)
	{
		printf ("CRC-32 reference check failed!\n\n");
		return -1;
	}
	else
	{
		printf ("CRC-32 check passed.\n\n");
	}


	// CRC-32C : width=32 poly=0x1edc6f41 init=0xffffffff refin=true refout=true xorout=0xffffffff check=0xe3069283 name="CRC-32C"
	crc_configuration.name = "CRC-32C";					// optional
	crc_configuration.width = 32;
	crc_configuration.polynomial = 0x1edc6f41;
	crc_configuration.initial_value = 0xffffffff;
	crc_configuration.final_xor_value = 0xffffffff;
	crc_configuration.reflect_input = 1;
	crc_configuration.reflect_output = 1;
	crc_configuration.check_value = 0xe3069283;						// optional, check = CRC of string "123456789\0"
	crc_configuration.residue = 0x00000000;							// optional, residue = CRC (DATA + CRC)
	clbrzcrcx8_init_crc(&crc_configuration);
	calculated_crc = (uint32_t)clbrzcrcx8_calculate_crc_chunk((uint8_t*)"123456789",9);
	calculated_crc = (uint32_t)clbrzcrcx8_finalize_crc();

	if(0xe3069283 != calculated_crc)
	{
		printf ("CRC-32C reference check failed!\n\n");
		return -1;
	}
	else
	{
		printf ("CRC-32C check passed.\n\n");
	}


	return 1; // ok.
}


int clbrzcrcx8_test()
{
	puts("\ncrickey! test crc algo for 8,16,32 bit crcs >>\n"); // prints crickey!


	// self-check:
	printf("\n---------------------------------------\n");
	if( clbrzcrcx8_check_crc_8_algo() == 1)
	{
		printf(">> CRC-8 ok. <<\n");
	}
	else
	{
		printf(">> CRC-8 test failed. <<\n");
		return -1;
	}
	printf("---------------------------------------\n\n");

	printf("\n---------------------------------------\n");
	if( clbrzcrcx8_check_crc_16_algo() == 1)
	{
		printf(">> CRC-16 ok. <<\n");
	}
	else
	{
		printf(">> CRC-16 test failed. <<\n");
		return -1;
	}
	printf("---------------------------------------\n\n");

	printf("\n---------------------------------------\n");
	if( clbrzcrcx8_check_crc_32_algo() == 1)
	{
		printf(">> CRC-32 ok. <<\n");
	}
	else
	{
		printf(">> CRC-32 test failed. <<\n");
		return -1;
	}
	printf("---------------------------------------\n\n");


	return 1;
}
#endif // #ifdef CLBRZCRCX8_ENABLE_CRC_TEST

// TODO add further configurations from <reveng>
CLBRZCRCx8_CRCTypeDescriptor_t clbrzcrcx8_crc_algo_list[] =
{
//		  name						width	poly		init		xor			refin	refout	check
		{ "CRC-8",					8, 		0x00000007, 0x00000000, 0x00000000, 0, 		0, 		0x000000f4 },
		{ "CRC-8/AUTOSAR",			8, 		0x0000002f, 0x000000ff, 0x000000ff, 0, 		0, 		0x000000df },
		{ "CRC-8/BLUETOOTH",		8, 		0x000000a7, 0x00000000, 0x00000000, 1, 		1, 		0x00000026 },
		{ "CRC-16/CCITT",	  		16, 	0x00001021, 0x00000000, 0x00000000, 1, 		1, 		0x00002189 },
		{ "CRC-16/ARC",	  			16,		0x00008005, 0x00000000, 0x00000000, 1, 		1, 		0x0000bb3d },
		{ "CRC-24/OPENPGP",	  		24,		0x00864cfb, 0x00b704ce, 0x00000000, 0, 		0, 		0x0021cf02 },
		{ "CRC-24/BLE",	  			24,		0x0000065b, 0x00555555, 0x00000000, 1, 		1, 		0x00c25a56 },
		{ "CRC-24/FLEXRAY-B",	  	24,		0x005d6dcb, 0x00abcdef, 0x00000000, 0, 		0, 		0x001f23b8 },
		{ "CRC-32",	  				32,		0x04c11db7, 0xffffffff, 0xffffffff, 1, 		1, 		0xcbf43926 },
		{ "CRC-32C",	  			32,		0x1edc6f41, 0xffffffff, 0xffffffff, 1, 		1, 		0xe3069283 },
		{ "CRC-32/AUTOSAR",	  		32,		0xf4acfb13, 0xffffffff, 0xffffffff, 1, 		1, 		0x1697d06a },
		{ "CRC-32/BZIP2",	  		32,		0x04c11db7, 0xffffffff, 0xffffffff, 0, 		0, 		0xfc891918 },

};

int clbrzcrcx8_crc_algo_list_size = sizeof(clbrzcrcx8_crc_algo_list)/sizeof(CLBRZCRCx8_CRCTypeDescriptor_t);


#ifdef CLBRZCRCX8_ENABLE_CRC_TEST

int main()
{
	clbrzcrcx8_test();

	int i = 0;

	printf("\n%-5s \t %-16s \t %-5s \t %-10s \t %-10s \t %-10s \t %-6s \t %-6s \t %-10s \n\n",
			"index",
			"name",
			"width",
			"poly",
			"init",
			"xor",
			"refin",
			"refout",
			"check");
	for (i = 0; i < clbrzcrcx8_crc_algo_list_size; i++)
	{
		printf("%-5d \t %-16s \t %-5d \t 0x%08x \t 0x%08x \t 0x%08x \t %-6d \t %-6d \t 0x%08x\n",
				i,
				clbrzcrcx8_crc_algo_list[i].name,
				clbrzcrcx8_crc_algo_list[i].width,
				clbrzcrcx8_crc_algo_list[i].polynomial,
				clbrzcrcx8_crc_algo_list[i].initial_value,
				clbrzcrcx8_crc_algo_list[i].final_xor_value,
				clbrzcrcx8_crc_algo_list[i].reflect_input,
				clbrzcrcx8_crc_algo_list[i].reflect_output,
				clbrzcrcx8_crc_algo_list[i].check_value);


		// enable the below code to test ALL the CRC algorithms added in the list!
#ifdef CLBRZCRCX8_ENABLE_CRC_SELF_TEST
		clbrzcrcx8_init_crc(&clbrzcrcx8_crc_algo_list[i]);
		if(clbrzcrcx8_self_test() == 1)
		{
			printf("self-check : PASS\n\n");
		}
		else
		{
			printf("self-check : FAIL\n\n");
		}
#endif // #ifdef CLBRZCRCX8_ENABLE_CRC_SELF_TEST


#ifdef CLBRZCRCX8_ENABLE_CRC_SELF_RESIDUE
		clbrzcrcx8_calculate_self_residue();
#endif // #ifdef CLBRZCRCX8_ENABLE_CRC_SELF_RESIDUE
	}

	return 0;
}

#endif // #ifdef CLBRZCRCX8_ENABLE_CRC_TEST
