# ������ ���� �� ��������� ��� ������� � CW

CW:=C:\Program Files\Metrowerks\CodeWarrior
TOOLS_DIR:=$(CW)\bin

#----------------------------------------------------------------
.PHONY: compile link clean distribute

# ���������� ������ �� SRCS_DIR
compile: 

# ������ ���� ���������� �������
link:
	@make -C ..

clean:
	-@rm -fr bins

distribute:
	@$(DISTRIBUTIV) palmos5.prc armc.bin


.SUFFIXES:
