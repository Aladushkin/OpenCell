#include "FlexController.h"

// ���������� �������� ��� ������, ����� Flex �� ����� ��������� �������������� �� ���������� ErrorCallback �������
void ErrorCallback(NvFlexErrorSeverity, const char* msg, const char* file, int line) {

	printf("Flex: %s - %s:%d\n", msg, file, line);
}