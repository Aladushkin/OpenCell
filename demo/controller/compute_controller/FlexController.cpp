#include "FlexController.h"

// необходимо объ€вить вне класса, иначе Flex не может выполнить преобразование во внутреннюю ErrorCallback функцию
void ErrorCallback(NvFlexErrorSeverity, const char* msg, const char* file, int line) {

	printf("Flex: %s - %s:%d\n", msg, file, line);
}