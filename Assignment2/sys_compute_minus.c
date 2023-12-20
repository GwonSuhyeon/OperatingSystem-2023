#include <linux/kernel.h>
#include <linux/syscalls.h>
#include <asm/uaccess.h>


asmlinkage long sys_compute_minus(int input1, int input2, int *result)
{
    int value = 0;

    int copyResult = 0;


	// 입력 값 2개를 빼기
    value = input1 - input2;

	// 커널모드에서의 연산 결과 값을 유저모드의 메모리로 복사
	copyResult = copy_to_user(result, &value, sizeof(int));
	if(copyResult != 0)
	{
		return -1;
	}
	
	return 0;
}

SYSCALL_DEFINE3(compute_minus, int, input1, int, input2, int *, result)
{
    return sys_compute_minus(input1, input2, result);
}