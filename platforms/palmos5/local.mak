# пуста€ пока не разберусь как сделать в CW

CW:=C:\Program Files\Metrowerks\CodeWarrior
TOOLS_DIR:=$(CW)\bin

#----------------------------------------------------------------
.PHONY: compile link clean distribute

# компил€ци€ файлов из SRCS_DIR
compile: 

# сборка всех полученных модулей
link:
	@make -C ..

clean:
	-@rm -fr bins

distribute:
	@$(DISTRIBUTIV) palmos5.prc armc.bin


.SUFFIXES:
