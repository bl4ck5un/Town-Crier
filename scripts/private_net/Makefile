console: compile
	./geth init /cwd/genesis.json
	./geth --preload /cwd/.compiledContract.js,/cwd/unit_tests.js console

clean:
	./geth removedb
	@rm -rf .compiledContract.js

doc:
	./geth --help

compile:
	@./compile.sh
