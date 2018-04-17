.PHONY: compile

RPC_ENDPOINT=http://localhost:8545

DOCKER_NAME=geth-rink

console: compile
	@geth attach ${RPC_ENDPOINT} --preload .compiledContract.js,launch.js

clean:
	@rm -rf .compiledContract.js

compile:
	@./compile.sh -n rinkeby

sync:
	@./geth-docker -n ${DOCKER_NAME} -d -- --config /config.toml

log:
	@docker logs --tail 500 -f ${DOCKER_NAME}
