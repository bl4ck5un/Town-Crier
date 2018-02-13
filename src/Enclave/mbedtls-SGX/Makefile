all:
	@make -C trusted
	@mkdir -p lib
	@echo "Installing.."
	@mv -f trusted/*.a lib
	@echo "Done."
clean:
	@rm -f lib/*.a
	@make -C trusted/mbedtls-2.2.1/ clean
	@make -C trusted clean
