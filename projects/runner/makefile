.PHONY: distribute win32 wince

# для отладки
win32:
	@make -s PROJECT=runner -C ../.. PLATFORM=$@ BUILD_TYPE=rebuild
	@echo Run game
	@win32.exe

# для отладки
wince:
	@make -s PROJECT=runner -C ../.. PLATFORM=$@ BUILD_TYPE=build

# создание дистрибутива
distribute:
	@$(DISTRIBUTIV) readme.html land/*.zip land/*/*.ogg land/*/*/*.ogg theme/*.zip theme/*/*.ogg theme/*/*/*.ogg interface/*.zip map/*.zip docs/* /docs/*/*
