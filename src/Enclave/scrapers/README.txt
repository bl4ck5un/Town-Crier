The Scraper interface has one function called handle_event that is responsible for handling any requested event.  To add a Scraper class, simply make it a subclass of the Scraper interface and provide an implementation of for handle_request.
To add a new scraper:
	1) Under /src/Common/Constants.h add a new Macro for the event
	2) Handle the even in /src/Enclave/event_handler.cpp
	3) Add the new scraper object to the /src/Enclave/scrapers folder and make sure that it is a subclass of the Scraper.h interface
	4) Add unit tests for this scraper in the /src/Enclave/test folder
	5) Compile and Run

	