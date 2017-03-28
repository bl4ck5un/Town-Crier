The Scraper interface has one function called handle_event that is responsible for handling any requested event.  To add a Scraper class, simply make it a subclass of the Scraper interface and provide an implementation of for handle_request.
To add a new scraper:
	1) Under /src/Common/Constants.h add a new Macro for the event
	2) Handle the even in /src/Enclave/event_handler.cpp
	3) Add the new scraper object to the /src/Enclave/scrapers folder and make sure that it is a subclass of the Scraper.h interface
	4) Add unit tests for this scraper in the /src/Enclave/test folder
	5) Compile and 


As of March 28, Town Crier supports 4 different events.  Each of these contains a handler class that returns a desired value.  We describe the structure of the input data below:
	SteamTrade:
		0x00 - 0x40 Enc API key
		0x40 - 0x60 buyer ID
		0x60 - 0x80 wait time
		0x80 - 0xa0 item len 
		0xa0 - 0xc0 
	
	StockTicker:
		/* The data is structured as follows (Feel free to change if there is a better way to structure it *:
		0x00 - 0x20 Symbol (i.e GOOG, APPL, etc)
		0x20 - 0x28 Month
		0x28 - 0x30 Day
		0x30 - 0x40 Year
	
	FlightInsurance:
       	0x00 - 0x20 string flight_number
       	0x20 - 0x40 uint64 unix_epoch
 