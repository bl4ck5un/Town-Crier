.PHONY: console

ganache:
	docker run --rm \
		-v $PWD:/cwd \
 		-p 8545:8545 \
 		trufflesuite/ganache-cli \
 		--deterministic \
 		--networkId 1827391 \
 		--account_keys_path /cwd/keys

console:
	@./geth init /cwd/genesis.json
	@./geth --preload /cwd/CompiledContracts-solc-v0.4.26.js,/cwd/testing_scripts.js console

clean:
	@./geth removedb

doc:
	@./geth --help