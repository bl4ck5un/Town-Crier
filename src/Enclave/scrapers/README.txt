The Scraper interface has one function called handle_event that is responsible for handling any requested event.  To add a Scraper class, simply make it a subclass of the Scraper interface and provide an implementation of for handle_request


To add a new scraper:
	1) Under /src/Common/Constants.h add a new Macro for the event
	2) Handle the even in /src/Enclave/event_handler.cpp
	3) Add the new scraper object to the /src/Enclave/scrapers folder and make sure that it is a subclass of the Scraper.h interface
	4) Add unit tests for this scraper in the /src/Enclave/test folder
	5) Compile and test

 
** List of Events that town crier currently supports (Updated 4/9/2017) **
Each of these contains a handle class that returns a desired value.  We describe the structure of the input data below:
	SteamTrade:
		Input:
			0x00 - 0x40 Enc API key
			0x40 - 0x60 buyer ID
			0x60 - 0x80 wait time
			0x80 - 0xa0 item len 
			0xa0 - 0xc0 
		Output:
			Returns 1 if the trade has been found, 0 otherwise

	StockTicker:
		Input:
			0x00 - 0x20 Symbol (i.e GOOG, APPL, etc)
			0x20 - 0x40 ascii Month 
			0x40 - 0x60 ascii Day
			0x60 - 0x80 ascii Year
		Output:
			32 bytes - Closing Price

	
	FlightInsurance:
		Input:
	       	0x00 - 0x20 string flight_number
	       	0x20 - 0x40 uint64 unix_epoch
 		Output:
 			32 byte- delay time in epoch time 
 			Note: If the flight has not departed yet then returns 0
 			
 	UPSTracking:
 		Input:
 			0x00 - 0x20 string tracking number
 		Output:
 			32 byte integer, indicating the status of the Package, the ints are,
 			   -1  - ERROR
				0  - PACKAGE_NOT_FOUND,
				1  - ORDER_PROCESSED,
				2  - SHIPPED,
				3  - IN_TRANSIT,
				4  - OUT_FOR_DELIVERY,
				5  - DELIVERED,

	CoinMarketScraper:
		Description: Gives the current exchange rate in dollars for a given crypto-currency
		Input:
			0x00 - 0x20 string cryptocurrency
		Output:
			32 bit integer indicating the currentn exchange rate of the package


