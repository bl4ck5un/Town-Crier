update:
	docker-compose -f docker-compose.debug.yml pull
	docker-compose -f docker-compose.prerelease.yml pull
	docker-compose -f docker-compose.release.yml pull

up-release:
	docker-compose -f docker-compose.release.yml up -d

up-prerelease:
	docker-compose -f docker-compose.prerelease.yml up -d

up-debug:
	docker-compose -f docker-compose.debug.yml up

log-release:
	docker-compose -f docker-compose.release.yml logs -f --tail=500

log-prerelease:
	docker-compose -f docker-compose.prerelease.yml logs -f --tail=500

attach-release:
	docker-compose -f docker-compose.release.yml exec tc-core bash

attach-prerelease:
	docker-compose -f docker-compose.prerelease.yml exec tc-core bash
