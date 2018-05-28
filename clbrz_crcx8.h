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

#ifndef CLBRZ_CRCX8_H_
#define CLBRZ_CRCX8_H_

#ifdef __cplusplus
extern "C" {
#endif // #ifdef __cplusplus

#include <stdint.h>


#define CLBRZCRCX8_USE_TABLE_FOR_CRC			// disable to remove table usage.
//#define CLBRZCRCX8_ENABLE_TABLE_GENERATION		// disable to remove the on demand table generation/print api (ENSURE UPDATE OF FIXED TABLE !!!)
//#define CLBRZCRCX8_ENABLE_CRC_TEST				// disable to remove the CRC 8/16/32 tests
//#define CLBRZCRCX8_ENABLE_CRC_SELF_TEST			// disable to remove the self test API.
//#define CLBRZCRCX8_ENABLE_CRC_SELF_RESIDUE		// disable to remove the self residue calculation API.


typedef struct _crcAlgoTypeDescriptor
{
	const char* name;
	uint8_t 	width;				// 8/16/32
	uint32_t 	polynomial;			// can actually be uint8_t/uint16_t/uint32_t
	uint32_t 	initial_value;		// can actually be uint8_t/uint16_t/uint32_t
	uint32_t 	final_xor_value;	// can actually be uint8_t/uint16_t/uint32_t
	uint8_t 	reflect_input;
	uint8_t 	reflect_output;
	uint32_t	check_value;					// CRC when run against "123456789\0"
	uint32_t	residue;						// constant value = CRC (DATA + CRC_OF_DATA)

} CLBRZCRCx8_CRCTypeDescriptor_t;

extern int clbrzcrcx8_crc_algo_list_size;
extern CLBRZCRCx8_CRCTypeDescriptor_t clbrzcrcx8_crc_algo_list[];


uint32_t clbrzcrcx8_reflect(uint32_t value, uint8_t num_bits_to_reflect);

#ifdef CLBRZCRCX8_ENABLE_TABLE_GENERATION
void clbrzcrcx8_generate_crc_table();
void clbrzcrcx8_print_crc_table();
#endif // #ifdef CLBRZCRCX8_ENABLE_TABLE_GENERATION

// set the CRC config to be used and generate table is enabled.
void clbrzcrcx8_init_crc(CLBRZCRCx8_CRCTypeDescriptor_t* crc_configuration_ptr);

// calculate CRC on chunk, CRC is carried over from previous calculation.
uint32_t clbrzcrcx8_calculate_crc_chunk(uint8_t* byte_data, int32_t data_len);

// reset CRC for fresh calculation, CRC configuration is unchanged.
uint32_t clbrzcrcx8_reset_crc_chunk();

// return final CRC value, call after all chunks are processed. applies reflect_out and final_xor.
uint32_t clbrzcrcx8_finalize_crc();

#ifdef CLBRZCRCX8_ENABLE_CRC_SELF_TEST
// run the self-test -> calculates CRC of string: "123456789" and should be equal to check_value
int clbrzcrcx8_self_test();
#endif // CLBRZCRCX8_ENABLE_CRC_SELF_TEST

#ifdef CLBRZCRCX8_ENABLE_CRC_SELF_RESIDUE
// runs the CRC over a few data points, and calculates residue, which should be the same for any data point.
uint32_t clbrzcrcx8_calculate_self_residue();
#endif // CLBRZCRCX8_ENABLE_CRC_SELF_RESIDUE

#ifdef CLBRZCRCX8_ENABLE_CRC_TEST
int clbrzcrcx8_test();
#endif // #ifdef CLBRZCRCX8_ENABLE_CRC_TEST


#ifdef __cplusplus
}
#endif // #ifdef __cplusplus

#endif /* CLBRZ_CRCX8_H_ */
