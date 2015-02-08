/******************************************************************************
*	kernel.ld
*	 by Alex Chadwick
*
*	A linker script for generation of raspberry pi kernel images.
******************************************************************************/

SECTIONS {
	/*
	* First and formost we need the .init section, containing the IVT.
	*/
	.init 0x0000 : {
		*(.init)
	}
	
	/* 
	* We allow room for the ATAGs and the stack and then start our code at
	* 0x8000.
	*/
	.text 0x8000 : {
		*(.text)
	}
	
	/* 
	* Next we put the data.
	*/
	.data : {
		*(.data)
	}

	/* 
	* Next we put the rodata.
	*/
	.rodata : {
		*(.rodata)
	}

	/* 
	* Next we put the bss.
	*/
	.bss : {
		*(.bss)
	}	
}
