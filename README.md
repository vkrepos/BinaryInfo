# BinaryInfo - File Properties relevant to software developers

Windows Shell File Properties dialog by default shows properties like Size, Created/Modified/Accessed timestamps, Read-Only flag, etc.
As a software developer I often more interested in properties like:
* Actual build timestamp (the 'Created timestamp' might lie)
* Build configuration (Release or Debug?)
* Bitness (32-bit? 64-bit? AnyCPU?)
* Version of Visual Studio used to build the image
* Target .NET Framework version (for .NET binaries)
* etc

The idea of the project was to display all these information in a quick and concise way.
All these information already could be extracted with various powerfull PE viewers. 
But these tools often present too much information most of which is not relevant or requires decoding and proper interpretation.