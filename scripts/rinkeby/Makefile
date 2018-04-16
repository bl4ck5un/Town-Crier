.PHONY: compile

RPC_ENDPOINT=http://localhost:8545

console: compile
	@geth attach ${RPC_ENDPOINT} --preload .compiledContract.js,launch.js

clean:
	@rm -rf .compiledContract.js

compile:
	@./compile.sh
