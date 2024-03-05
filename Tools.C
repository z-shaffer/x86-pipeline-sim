#include <cstdint>
#include "Tools.h"
#include <cstdlib>
#include <string>

/** 
 * builds a 64-bit long out of an array of 8 bytes
 *
 * for example, suppose bytes[0] == 0x12
 *              and     bytes[1] == 0x34
 *              and     bytes[2] == 0x56
 *              and     bytes[3] == 0x78
 *              and     bytes[4] == 0x9a
 *              and     bytes[5] == 0xbc
 *              and     bytes[6] == 0xde
 *              and     bytes[7] == 0xf0
 * then buildLong(bytes) returns 0xf0debc9a78563412
 *
 * @param array of 8 bytes
 * @return uint64_t where the low order byte is bytes[0] and
 *         the high order byte is bytes[7]
*/
uint64_t Tools::buildLong(uint8_t bytes[LONGSIZE])
{
	uint64_t ret = 0;
	ret += bytes[7];
	ret = ret << 8;
	ret += bytes[6];
	ret = ret << 8;
	ret += bytes[5];
	ret = ret << 8;  
	ret += bytes[4];
	ret = ret << 8;  
	ret += bytes[3];
	ret = ret << 8;  
	ret += bytes[2];
	ret = ret << 8;  
	ret += bytes[1];
	ret = ret << 8;  
	ret += bytes[0]; 
	return ret;	
}

/** 
 * accepts as input an uint64_t and returns the designated byte
 * within the uint64_t; returns 0 if the indicated byte number
 * is out of range 
 *
 * for example, getByte(0x1122334455667788, 7) returns 0x11
 *              getByte(0x1122334455667788, 1) returns 0x77
 *              getByte(0x1122334455667788, 8) returns 0
 *
 * @param uint64_t source that is the source data
 * @param int32_t byteNum that indicates the byte to return (0 through 7)
 * @return 0 if byteNum is out of range
 *         byte 0, 1, .., or 7 of source if byteNum is within range
 *
 * RESTRICTIONS: You cannot use an if statement, the ternary operator, switch
 *               statement or any other conditional statements. 
 *               This means you need to come up with some clever method to get the
 *               code to return 0 if bytenum is out of range and
 *               the selected byte otherwise.  
*/
uint64_t Tools::getByte(uint64_t source, int32_t byteNum)
{
	uint8_t ret = 0;
	int shift = byteNum * 8;
	if(shift > 56 || shift < 0)
	{
		return 0;
	}
	ret = source >> shift;
	return ret;
}

/**
 * accepts as input an uint64_t and returns the bits low through 
 * high of the uint64_t.  bit 0 is the low order bit and bit 63
 * is the high order bit. returns 0 if the low or high bit numbers 
 * are out of range
 *
 * for example, getBits(0x8877665544332211, 0, 7) returns 0x11
 *              getBits(0x8877665544332211, 4, 11) returns 0x21
 *              getBits(0x8877665544332211, 0, 63) returns 0x8877665544332211
 *
 * @param uint64_t source that holds the bits to be grabbed and 
 *        returned
 * @param int32_t low that is the bit number of the lowest numbered
 *        bit to be returned
 * @param int32_t high that is the bit number of the highest numbered
 *        bit to be returned
 * @return an uint64_t that holds a subset of the source bits
 *         that is returned in the low order bits; 0 if low or high 
 *         is out of range
 *
 *
 * RESTRICTIONS: You can use an if statement to determine whether
 *               the low and high values are valid. The remaining statements
 *               need to be simple assignment statements. (No loops, switch,
 *               ternary operator , or other conditional statements....)
 *               The purpose of this restriction is to push you to write
 *               "clean" code.
 */
uint64_t Tools::getBits(uint64_t source, int32_t low, int32_t high)
{
	if (low < 0 || high > 63)
	{
		return 0;
	}
	uint64_t ret = 0;
	int shift1 = 63 - high;
	ret = source << shift1;
	ret = ret >> shift1;
	ret = ret >> low;
	return ret;   
}

/**
 * sets the bits of source in the range specified by the low and high
 * parameters to 1 and returns that value. returns source if the low or high
 * bit numbers are out of range
 *
 * for example, setBits(0x1122334455667788, 0, 7) returns 0x11223344556677ff
 *              setBits(0x1122334455667788, 8, 0xf) returns 0x112233445566ff88
 *              setBits(0x1122334455667788, 8, 64) returns 0x1122334455667788
 *                      note: 64 is out of range
 *
 * @param uint64_t source 
 * @param int32_t low that is the bit number of the lowest numbered
 *        bit to be set to 1
 * @param int32_t high that is the bit number of the highest numbered
 *        bit to be set to 1
 * @return an uint64_t that holds the modified source
 *
 * RESTRICTIONS: You can use an if statement to determine whether
 *               the low and high values are valid. The remaining statements
 *               need to be simple assignment statements. (No loops, switch,
 *               ternary operator, or other conditional statements ....)
 *               You can use other functions that you have written.
 *               The purpose of this restriction is to push you to write
 *               "clean" code.
 */
uint64_t Tools::setBits(uint64_t source, int32_t low, int32_t high)
{
	if (low < 0 || high > 63 || high < low)
	{
		return source;
	}
	uint64_t ret = 0;
	ret = source;
	ret = ret | 0xffffffffffffffff;
	int shift = 63 - high;
	ret = ret << shift;
	ret = ret >> shift;
	ret = ret >> low;
	ret = ret << low;
	ret = ret | source;
	return ret; 
}

/**
 * sets the bits of source in the range low to high to 0 (clears them) and
 * returns that value. returns source if the low or high
 * bit numbers are out of range
 *
 * for example, clearBits(0x1122334455667788, 0, 7) returns 0x1122334455667700
 *              clearBits(0x1122334455667788, 8, 15) returns 0x1122334455660088
 *
 * @param uint64_t source 
 * @param int32_t low that is the bit number of the lowest numbered
 *        bit to be set to 0
 * @param int32_t high that is the bit number of the highest numbered
 *        bit to be set to 0
 * @return an uint64_t that holds the modified source
 *
 * RESTRICTIONS: You can use an if statement to determine whether
 *               the low and high values are valid. The remaining statements
 *               need to be simple assignment statements. (No loops, switch,
 *               ternary operator, or other conditional statements ....)
 *               You can use other functions that you have written.
 *               The purpose of this restriction is to push you to write
 *               "clean" code.
 */
uint64_t Tools::clearBits(uint64_t source, int32_t low, int32_t high)
{
if(low < 0 || high > 63 || high < low)
{
return source;
}
	uint64_t ret = 0;
	ret = source;
	ret = ret | 0xffffffffffffffff;
	int shift = 63 - high;
	ret = ret << shift;
	ret = ret >> shift;
	ret = ret >> low;
	ret = ret << low;
	ret = ~ret;
	ret = ret & source;
   
	return ret; 
}

/**
 * copies length bits from the source to a destination and returns the
 * modified destination. If low bit number of the source or 
 * dest is out of range or the calculated source or dest high bit 
 * number is out of range, then the unmodified destination is returned.
 *
 * for example,
 *   copyBits(0x1122334455667788, 0x8877665544332211, 0, 0, 8) 
 *           returns 0x8877665544332288
 *   copyBits(0x1122334455667788, 0x8877665544332211, 0, 8, 8) 
 *           returns 0x8877665544338811
 *
 * @param uint64_t source 
 * @param uint64_t dest 
 * @param int32_t srclow that is the bit number of the lowest numbered
 *        bit of the source to be copied
 * @param int32_t destlow that is the bit number of the lowest numbered
 *        bit of the destination to be modified
 * @param int32_t length that is the number of bits to be copied
 * @return uint64_t that is the modifed dest
 *
 * RESTRICTIONS: You can use an if statement to determine whether
 *               the low and high values are valid. The remaining statements
 *               need to be simple assignment statements. (No loops, switch,
 *               ternary operator, or other conditional statements ....) 
 *               You can use other functions that you have written.
 *               The purpose of this restriction is to push you to write
 *               "clean" code.
 */
uint64_t Tools::copyBits(uint64_t source, uint64_t dest, 
                         int32_t srclow, int32_t dstlow, int32_t length)
{
	int32_t dsthigh = dstlow + length - 1;
	int32_t srchigh = srclow + length - 1;
	uint64_t ret = 0;
	if (srclow < 0 || dstlow < 0 || dsthigh > 63 || srchigh > 63)
	{
		return dest;
	}
	uint64_t source_bits = getBits(source, srclow, srchigh);
	source_bits = source_bits << dstlow;
	uint64_t dest_cleared = clearBits(dest, dstlow, dsthigh);
	ret = source_bits | dest_cleared;
	return ret;
}

/**
 * sets the bits of source identfied by the byte number to 1 and
 * returns that value. if the byte number is out of range then source
 * is returned unchanged.
 *
 * for example, setByte(0x1122334455667788, 0) returns 0x11223344556677ff
 *              setByte(0x1122334455667788, 1) returns 0x112233445566ff88
 *              setByte(0x1122334455667788, 8) returns 0x1122334455667788
 *
 * @param uint64_t source 
 * @param int32_t byteNum that indicates the number of the byte to be
 *        set to 0xff; the low order byte is byte number 0
 * @return uint64_t that is source with byte byteNum set to 0xff
 *
 * RESTRICTIONS: You cannot use an if statement, the ternary operator, switch
 *               statement or any other conditional statements. 
 *               This means you need to come up with some clever method to get the
 *               code to return unmodified source if bytenum is out of range and
 *               the modified source otherwise.  
 *
 */
uint64_t Tools::setByte(uint64_t source, int32_t byteNum)
{
	int byte = byteNum * 8;
	int byteEnd = 64 - (byte + 8);
	if (byteNum < 0 || byteNum > 7)
	{
		return source;
	}
	uint64_t ret = 0xffffffffffffffff;
	ret = ret << byteEnd;
	ret = ret >> byteEnd;
	ret = ret >> byte;
	ret = ret << byte;
	ret = ret | source;
	return ret;
}

/**
 * assumes source contains a 64 bit two's complement value and
 * returns the sign (1 or 0)
 *
 * for example, sign(0xffffffffffffffff) returns 1
 *              sign(0x0000000000000000) returns 0
 *              sign(0x8000000000000000) returns 1
 *
 * @param uint64_t source
 * @return 1 if source is negative when treated as a two's complement 
 *         value and 0 otherwise
 *
 * RESTRICTIONS: You cannot use an if statement, ternary operator, switch statement
 *               or any other conditional statement. 
 *                
 */
uint8_t Tools::sign(uint64_t source)
{
	uint64_t num = source >> 63;
	int ret = num;
	return ret;   
}

/**
 * assumes that op1 and op2 contain 64 bit two's complement values
 * and returns true if an overflow would occur if they are summed
 * and false otherwise
 *
 * for example, addOverflow(0x8000000000000000, 0x8000000000000000) returns 1
 *              addOverflow(0x7fffffffffffffff, 0x7fffffffffffffff) returns 1
 *              addOverflow(0x8000000000000000, 0x7fffffffffffffff) returns 0
 *
 * @param uint64_t op1 that is one of the operands of the addition
 * @param uint64_t op2 that is the other operand of the addition
 * @return true if op1 + op2 would result in an overflow assuming that op1
 *         and op2 contain 64-bit two's complement values
 *
 * RESTRICTIONS: You cannot use an if statement, ternary operator, switch statement
 *               or any other conditional statement.  However, you can use other
 *               functions you have written and conditional expressions. For example,
 *               return op1 != op2; 
 *               is allowed, but
 *               if (op1 != op2) return true; 
 *               is not.  The purpose of this restriction is to push you to write
 *               "clean" code. 
 */
bool Tools::addOverflow(uint64_t op1, uint64_t op2)
{
	uint64_t result = op1 + op2;
	return (op1 == 0x8000000000000000) || (op2 == 0x8000000000000000) || ((sign(op1) == sign(op2)) && 
			(sign(op1) != sign(result)));


}

/**
 * assumes that op1 and op2 contain 64 bit two's complement values
 * and returns true if an overflow would occur from op2 - op1
 * and false otherwise
 *
 * for example, subOverflow(0x8000000000000000, 0x8000000000000000) returns 0
 *              subOverflow(0x7fffffffffffffff, 0x7fffffffffffffff) returns 0
 *              subOverflow(0x8000000000000000, 0x7fffffffffffffff) returns 1
 *
 * @param uint64_t op1 that is one of the operands of the subtraction
 * @param uint64_t op2 that is the other operand of the subtraction
 * @return true if op2 - op1 would result in an overflow assuming that op1
 *         and op2 contain 64-bit two's complement values
 *
 * RESTRICTIONS: You cannot use an if statement, ternary operator, switch statement
 *               or any other conditional statement.  However, you can use other
 *               functions you have written and conditional expressions. For example,
 *               return op1 != op2; 
 *               is allowed, but
 *               if (op1 != op2) return true; 
 *               is not.  The purpose of this restriction is to push you to write
 *               "clean" code. 
 */
bool Tools::subOverflow(uint64_t op1, uint64_t op2)
{
	//Note: the result computed is op2 - op1 (not op1 - op2)
	bool two = (op2 >> 63 == 1);
	uint64_t op3 = op1 + op2;
	bool total = (op3 >> 63 == 1);
	bool diff = !(op1 == op2);
	return (diff == 1 && two == total);
}

