//  _    ___      __              __________      _     __
// | |  / (_)____/ /___  ______ _/ / ____/ /_  __(_)___/ /____
// | | / / / ___/ __/ / / / __ `/ / /_  / / / / / / __  / ___/
// | |/ / / /  / /_/ /_/ / /_/ / / __/ / / /_/ / / /_/ (__  )
// |___/_/_/   \__/\__,_/\__,_/_/_/   /_/\__,_/_/\__,_/____/
//
#ifndef UBFILEINPUTBINARY_H
#define UBFILEINPUTBINARY_H

#include <fstream>
#include <iostream>
#include <string>

#include <basics/utilities/UbException.h>
#include <basics/utilities/UbFileInput.h>

/*=========================================================================*/
/*  UbFileInputBinary                                                      */
/*                                                                         */
/**
...
<BR><BR>
@author <A HREF="mailto:muffmolch@gmx.de">S. Freudiger</A>
@version 1.0 - 23.11.04
*/ 

/*
usage: ...
*/

class UbFileInputBinary : public UbFileInput
{                               
public:
   UbFileInputBinary() : UbFileInput() {  }
   UbFileInputBinary(std::string filename);
	
	bool        open(std::string filename);

   void	      skipLine();					   // Springt zur naechsten Zeile
	void        readLine();		 
   std::string readStringLine();				
   std::size_t readSize_t();				
   int		   readInteger();				   // Liest einen Int-Wert ein
	double	   readDouble();				   // Liest einen double-Wert ein
	float 	   readFloat();				   // Liest einen float-Wert ein
	bool  	   readBool();				      // Liest einen bool-Wert ein
   char        readChar();                // Liest einen char-Wert ein
   std::string	readString();				   // Liest ein Wort ein
	std::string	readLineTill(char stop);	// Liest gesamte Zeile ein bis zu einem bestimmten Zeichen
	std::string	parseString();	// Liest 

   bool        containsString(const std::string& var);
   void        setPosAfterLineWithString(const std::string& var);
   int		   readIntegerAfterString(const std::string& var);
   double	   readDoubleAfterString(const std::string& var);
   bool        readBoolAfterString(const std::string& var);
   std::string readStringAfterString(const std::string& var);

   FILETYPE getFileType() { return BINARY; }

   template< typename T >
   friend inline UbFileInputBinary& operator>>(UbFileInputBinary& file, T& data) 
   {
      file.infile.read((char*)&data,sizeof(T));
      return file;
   }
};

#endif


