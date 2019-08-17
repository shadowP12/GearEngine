#ifndef FILE_SYSTEM_H
#define FILE_SYSTEM_H
#include <fstream>
#include <iostream>
#include <vector>
#include <string>

bool readFile(const std::string& dstFileName, std::string& outData)
{
	std::istream* stream = &std::cin;
	std::ifstream inputFile;

	inputFile.open(dstFileName, std::ios_base::binary);
	stream = &inputFile;
	if (inputFile.fail())
	{
		std::cout << "error: cannot open input file: '" << dstFileName << std::endl;
		return false;
	}
	outData = std::string((std::istreambuf_iterator<char>(*stream)), std::istreambuf_iterator<char>());
	return true;
}

#endif