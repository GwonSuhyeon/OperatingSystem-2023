#include <linux/kernel.h>
#include <linux/syscalls.h>
#include <asm/uaccess.h>


asmlinkage long sys_reverse_print(char *input, char *result)
{
	int copyResult = 0;
	int reverseIdx = 0;
	int inputIdx = 0;
	int i = 0;

	char kernelInput[256] = "\0";
	char reverse[256] = "\0";

	
	// 유저모드에서 파라미터로 넘겨준 입력 스트림의 메모리 위치를 커널모드의 메모리로 복사
	copyResult = copy_from_user(kernelInput, input, 256);
	if(copyResult != 0)
	{
		return -1;
	}

	
	// 입력 스트림의 문자열 크기를 확인
	inputIdx = 0;
	while(1)
	{
		if(kernelInput[inputIdx] == '\0')
		{
			break;
		}

		inputIdx++;
	}

	// 입력 스트림을 역순으로 변환
	reverseIdx = 0;
	i = 0;
	for(i = inputIdx - 1; i >= 0; i--)
	{
		reverse[reverseIdx] = kernelInput[i];
		reverseIdx++;
	}

	// 커널모드에서의 역순으로 변환된 문자열의 메모리 위치를 유저모드의 메모리로 복사
	copyResult = copy_to_user(result, reverse, 256);
	if(copyResult != 0)
	{
		return -1;
	}

	
	return 0;
}

SYSCALL_DEFINE2(reverse_print, char *, input, char *, result)
{
	return sys_reverse_print(input, result);
}