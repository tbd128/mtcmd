# mtcmd
Magnetic tape device controller for MacOS X

This is a command-line utility tool for MacOS X to manipulate magnetic tape device connected to SCSI interface.

# Installation
Just type 'make' to build it from source code.
(XCode environment is required to build the source code.)

Please copy the built 'mtcmd' to anywhere you like.

For more information, please see "5. Compile from Source" in the 
[Manual.txt](Manual.txt).


# Usage
mtcmd [option...] [cmd]

# Update History
- 1.0 (March 19, 2002)	: 1st release
- 1.0.1 (March 24, 2002)	: Minor update
- 1.1   (May 07, 2002)	: Added 'bsf', 'setopt', '-c' and '-dt'
- 1.1.1 (Jun 15, 2002)	: Minor update
- 1.1.2 (Jul 28, 2004)	: Removed "#include <IOKit/cdb/IOSCSILib.h>"
- 1.1.3 (Jan 04, 2023)	: Minor update

# Copyright
Copyright © 2002-2023 tbd128 All Rights Reserved.

# License
This project is licensed under the MIT License, see the [LICENSE.txt](LICENSE.txt) file for details
