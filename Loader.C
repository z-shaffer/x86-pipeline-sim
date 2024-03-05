#include <iostream>
#include <fstream>
#include <string.h>
#include <ctype.h>
#include <stdint.h>
#include "Loader.h"
#include "Memory.h"

#define ADDRBEGIN 2
#define ADDREND 4
#define DATABEGIN 7
#define COMMENT 28

/* 
 * Loader
 * opens up the file named in argv[0] and loads the
 * contents into memory. If the file is able to be loaded,
 * then loaded is set to true.
 */
//This method is complete and does not need to be modified.  
Loader::Loader(int argc, char * argv[])
{
	std::ifstream inf;  //input file stream for reading from file
	int lineNumber = 1;
	lastAddress = -1;
	loaded = false;

	//if no file name given or filename badly formed, return without loading
	if(argc < 2 || badFile(argv[1])) return;  
	inf.open(argv[1]);

	//if file can't be opened, return without loading
	if(!inf.is_open()) return;  
  
	std::string line;
	while (getline(inf, line))
	{
		if (hasErrors(line))
		{
			std::cout << "Error on line " << std::dec << lineNumber 
					<< ": " << line << std::endl;
			return;
		}
		if (hasAddress(line) && hasData(line)) loadLine(line);
		lineNumber++;
	}
	loaded = true;
}

/*
 * hasAddress
 * returns true if the line passed in has an address on it.
 * A line that has an address has a '0' in column 0.
 * It is assumed that the address has already been checked to
 * make sure it is properly formed.
 *
 * @param line - a string containing a line of valid input from 
 *               a .yo file
 * @return true, if the line has an address on it
 *         false, otherwise
 */
bool Loader::hasAddress(std::string line)
{
	if (line.at(0) != '0')
	{
		return 0;
	}
	return 1;
}

/*
 * hasData
 * returns true if the line passed in has data on it.
 * A line that has data does not contain a space
 * at index DATABEGIN.
 * It is assumed that the data has already been checked to
 * make sure it is properly formed.
 *
 * @param line - a string containing a line of valid input from 
 *               a .yo file
 * @return true, if the line has data in it
 *         false, otherwise
 */
bool Loader::hasData(std::string line)
{
	if (line.at(DATABEGIN) == ' ')
	{
		return 0;
	}
	return 1;
}

/*
 * hasComment
 * returns true if line is at least COMMENT in length and 
 * line has a | at index COMMENT.
 * 
 * @param line - a string containing a line from a .yo file
 * @return true, if the line is long enough and has a | in index COMMENT
 *         false, otherwise
 */
bool Loader::hasComment(std::string line)
{
	if (line.length() >= COMMENT && line.at(COMMENT) == '|')
	{
		return 1;
	}
	return 0;
}

/*
 * loadLine
 * The line that is passed in contains an address and data.
 * This method loads that data into memory byte by byte
 * using the Memory::getInstance->putByte method.
 *
 * @param line - a string containing a line of valid input from 
 *               a .yo file. The line contains an address and
 *               a variable number of bytes of data (at least one)
 */
void Loader::loadLine(std::string line)
{
	int32_t addr = convert(line, ADDRBEGIN, ADDREND);
	bool boo = 0;
        for (int i = DATABEGIN; isxdigit(line.at(i)); i += 2)
        {
                Memory::getInstance()->putByte(convert(line, i, 2), addr, boo);
		lastAddress = addr;
                addr++;
        }

}

/*
 * convert
 * takes "len" characters from the line starting at character "start"
 * and converts them to a number, assuming they represent hex characters.
 * For example, if len is 2 and line[start] is '1' and 
 * line[start + 1] is 'a' then this function returns 26.
 * This function assumes that the line is long enough to hold the desired
 * characters and that the characters represent hex values.
 *
 * @param line - string of characters
 * @param start - starting index in line
 * @param len - represents the number of characters to retrieve
 */
int32_t Loader::convert(std::string line, int32_t start, int32_t len)
{	
	return stoul(line.substr(start, len), NULL, 16);
}

/*
 * hasErrors
 * Returns true if the line file has errors in it and false
 * otherwise.
 *
 * @param line - a string that contains a line from a .yo file
 * @return true, if the line has errors 
 *         false, otherwise
 */
bool Loader::hasErrors(std::string line)
{
	if (isSpaces(line, 0, COMMENT - 1) && line.at(COMMENT) == '|')
	{
		return 0;	
	}
	else if (isSpaces(line, 0, COMMENT - 1) && line.at(COMMENT) != '|')
	{
		return 1;
	}
	if (hasComment(line) == 0)
	{
		return 1;
	}

	if (hasAddress(line) == 0)
	{
		return !isSpaces(line, 0, COMMENT - 1);
	}

	if (errorAddr(line) == 1)
	{
		return 1;
	}

	int32_t numDBytes = 0;
	int i = 0;
	while (line.at(i + ADDREND) != ' ')
	{
		numDBytes += 2;
		i += 2;
	}

	if (errorData(line, numDBytes) == 1)
	{
		return 1;
	}

	int32_t start = 2;
	int32_t len = 3;
	int32_t addrConv = convert(line, start, len);
	
	if (addrConv <= lastAddress)
	{
		return 1;
	}

	if (addrConv + numDBytes > MEMSIZE)
	{
		return 1;
	}

	return 0;
}

/*
 * errorData
 * Called when the line contains data. It returns true if the data
 * in the line is invalid. 
 *
 * Valid data consists of characters in the range
 * '0' .. '9','a' ... 'f', and 'A' .. 'F' (valid hex digits). 
 * The data digits start at index DATABEGIN.
 * The hex digits come in pairs, thus there must be an even number of them.
 * In addition, the characters after the last hex digit up to the
 * '|' character at index COMMENT must be spaces. 
 * If these conditions are met, errorData returns false, else errorData
 * returns true.
 *
 * @param line - input line from the .yo file
 * @return numDBytes is set to the number of data bytes on the line
 */
bool Loader::errorData(std::string line, int32_t & numDBytes)
{
	//Hint: use isxdigit and isSpaces
	int i = DATABEGIN;
	for (i = DATABEGIN; isxdigit(line.at(i)); i++) { 
	}
	if ((i - DATABEGIN) % 2 == 1)
	{
		return 1;
	}

	numDBytes = ((i - DATABEGIN) / 2);

	if (!isSpaces(line, i, COMMENT - 1) || line.at(COMMENT) != '|')
	{
		return 1;    
	}
	return 0;}

/*
 * errorAddr
 * This function is called when the line contains an address in order
 * to check whether the address is properly formed.  An address must be of
 * this format: 0xHHH: where HHH are valid hex digits.
 * 
 * @param line - input line from a .yo input file
 * @return true if the address is not properly formed and false otherwise
 */
bool Loader::errorAddr(std::string line)
{
	for (int i = ADDRBEGIN; i <= ADDREND; i++)
	{
		if (!isxdigit(line.at(i)))
		{
			return 1;	
		}
	}
	if (line.at(1) != 'x' || line.at(5) != ':' || line.at(6) != ' ')
	{
		return 1;
	}
	return 0;}

/* 
 * isSpaces
 * This function checks that characters in the line starting at 
 * index start and ending at index end are all spaces.
 * This can be used to check for errors
 *
 * @param line - string containing a line from a .yo file
 * @param start - starting index
 * @param end - ending index
 * @return true, if the characters in index from start to end are spaces
 *         false, otherwise
 */
bool Loader::isSpaces(std::string line, int32_t start, int32_t end)
{
	for (int i = start; i <= end; i++)
	{
		if (line.at(i) != ' ')
		{
			return 0;
		}
	} 
	return 1;
}

/*
 * isLoaded
 * getter for the private loaded data member
 */
bool Loader::isLoaded()
{
	return loaded;  
}

/*
 * badFile
 * returns true if the name of the file passed in is an improperly 
 * formed .yo filename. A properly formed .yo file name is at least
 * four characters in length and ends with a .yo extension.
 *
 * @return true - if the filename is improperly formed
 *         false - otherwise
 */
bool Loader::badFile(std::string filename)
{
	if (filename.length() <  4)
	{
		printf("File is not valid\n");
		return 1;
	}
	else if (filename.find(".yo") < 0)
	{
		printf("File doesn't end with a .yo extension\n");
		return 1;		
	}
	else
	{
		return 0;
	}
}
