CC=g++

CXXFLAGS += -c -I/usr/local/opt/curl/include
LDFLAGS += -L/usr/local/opt/curl/lib -lcurl

all: sample_scraper.out

sample_scraper.out: https.o sample_scraper.o
	$(CC) $(LDFLAGS) $^ -o $@

%.o: %.cpp
	$(CC) $(CXXFLAGS) $^ -o $@
	
