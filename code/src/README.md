PREREQUISITE
----------------


- libjson-rpc-cpp: https://github.com/cinemast/libjson-rpc-cpp, which can be
  installed via a package manager on some platform (e.g. Arch and Ubuntu) or compiled
  from the source code. To compile, you need to install these dependencies
  (tested on Debian 8):

    ```
    sudo apt-get install libmicrohttpd-dev libmicrohttpd10 \ 
        libjsoncpp-dev libjsoncpp0 libargtable2-0 libargtable2-dev \
        libcurl4-openssl-dev catch
    ```

- sqlite3: `sudo apt-get install libsqlite3-0 libsqlite3-dev`
- http-parser: https://github.com/nodejs/http-parser. No package available yet.
  Can only be installed by compilcation.

