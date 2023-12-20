#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <linux/kernel.h>
#include <sys/syscall.h>


#define SYS_CALL_REVERSE 449
#define SYS_CALL_PLUS 450
#define SYS_CALL_MINUS 451

#define MAX_BUFFER_SIZE 256

#define TYPE_OPERATOR_INVALID -1
#define TYPE_OPERATOR_NONE 0
#define TYPE_OPERATOR_PLUS 1
#define TYPE_OPERATOR_MINUS 2


void Clear_Space(char *origin, char *input);
int Check_Operator(char *input);
void Split_Input(char *input, int *first, int *second, int operatorType);


int main(void)
{
    long int result = 0;

    int isWrong = 0;
    int isSpace = 0;

    int operatorType = TYPE_OPERATOR_NONE;

    int first = 0;
    int second = 0;
    int computeResult = 0;

    char origin[MAX_BUFFER_SIZE];
    char input[MAX_BUFFER_SIZE];
    char reverse[MAX_BUFFER_SIZE];


    while(1)
    {
        // 문자열 배열 초기화
        memset(origin, '\0', MAX_BUFFER_SIZE);
        memset(input, '\0', MAX_BUFFER_SIZE);
        memset(reverse, '\0', MAX_BUFFER_SIZE);

        // 입력 스트림 받기
        printf("Input: ");
        fgets(origin, MAX_BUFFER_SIZE, stdin);
        origin[strlen(origin) - 1] = '\0';

        // 입력된 것이 없을 경우 종료
        if(origin[0] == '\0')
        {
            break;
        }

        // 입력된 스트림 중에서 공백(스페이스)을 모두 제거
        Clear_Space(origin, input);

        // 연산자 기호 앞과 뒤에 아무것도 없을 경우 예외처리
        if(input[0] == '+' || input[0] == '-' || input[strlen(input) - 1] == '+' || input[strlen(input) - 1] == '-')
        {
            printf("Wrong Input!\n");

            continue;
        }

        // 0 이상 9 이하의 숫자와 +, - 연산 기호 이외의 입력이 들어온 경우 예외처리
        isWrong = 0;
        isSpace = 0;
        for(int i = 0; i < strlen(input); i++)
        {
            if(input[i] == '\0')
            {
                break;
            }

            if((input[i] < '0' || input[i] > '9') && (input[i] != '+' && input[i] != '-'))
            {
                isWrong = 1;
            }
        }

        // 정상적인 입력인 경우
        if(isWrong == 0)
        {
            // 연산자 기호 종류 확인
            operatorType = Check_Operator(input);

            if(operatorType == TYPE_OPERATOR_INVALID)
            {
                // 다수의 연산자 기호가 입력된 경우 예외처리

                printf("Wrong Input!\n");
            }
            else if(operatorType == TYPE_OPERATOR_NONE)
            {
                // 입력된 연산자 기호가 없어 역순으로 출력하는 경우

                // 입력을 역순으로 출력하는 sys_reverse_print 시스템콜 호출
                result = syscall(SYS_CALL_REVERSE, input, reverse);

                // 시스템콜 호출이 성공한 경우에 결과 출력
                if(result == 0)
                {
                    printf("Output: %s\n", reverse);
                }
            }
            else if(operatorType == TYPE_OPERATOR_PLUS)
            {
                // - 연산자가 입력되어 + 연산을 하는 경우

                // 입력 스트림에서 피연산자 2개와 연산자 기호 1개를 분리
                Split_Input(input, &first, &second, operatorType);

                // 피연산자 2개를 덧셈 연산하는 sys_compute_plus 시스템콜 호출
                result = syscall(SYS_CALL_PLUS, first, second, &computeResult);

                // 시스템콜 호출이 성공한 경우에 결과 출력
                if(result == 0)
                {
                    printf("Output: %d\n", computeResult);
                }
            }
            else if(operatorType == TYPE_OPERATOR_MINUS)
            {
                // + 연산자가 입력되어 - 연산을 하는 경우

                // 입력 스트림에서 피연산자 2개와 연산자 기호 1개를 분리
                Split_Input(input, &first, &second, operatorType);

                // 피연산자 2개를 뺄셈 연산하는 sys_compute_minus 시스템콜 호출
                result = syscall(SYS_CALL_MINUS, first, second, &computeResult);

                // 시스템콜 호출이 성공한 경우에 결과 출력
                if(result == 0)
                {
                    printf("Output: %d\n", computeResult);
                }
            }
        }

        // 비정상적인 입력인 경우 예외처리
        else
        {
            printf("Wrong Input!\n");
        }
    }


    return 0;
}


void Clear_Space(char *origin, char *input)
{
    int idx = 0;


    // 입력 스트림 전체를 확인하면서 공백 문자를 제거
    for(int i = 0; i < strlen(origin); i++)
    {
        if(origin[i] != ' ')
        {
            input[idx] = origin[i];

            idx++;
        }
    }


    return;
}


int Check_Operator(char *input)
{
    int operatorType = TYPE_OPERATOR_NONE;

    int cnt = 0;


    for(int i = 0; i < strlen(input); i++)
    {
        // 입력 스트림에 존재하는 연산자 기호 종류가 무엇인지 검사
        if(input[i] == '+')
        {
            operatorType = TYPE_OPERATOR_MINUS;

            cnt++;
        }
        else if(input[i] == '-')
        {
            operatorType = TYPE_OPERATOR_PLUS;

            cnt++;
        }
    }

    // 2개 이상의 연산자 기호가 존재하는지 검사
    if(cnt > 1)
    {
        operatorType = TYPE_OPERATOR_INVALID;
    }


    return operatorType;
}


void Split_Input(char *input, int *first, int *second, int operatorType)
{
    char *ptr;

    
    // 연산자 기호 종류에 따라 첫번째 피연산자 분리
    if(operatorType == TYPE_OPERATOR_PLUS)
    {
        ptr = strtok(input, "-");
        *first = atoi(ptr);
    }
    else if(operatorType == TYPE_OPERATOR_MINUS)
    {
        ptr = strtok(input, "+");
        *first = atoi(ptr);
    }

    // 연산지 기호 뒤에 있는 두번째 피연산자 분리
    ptr = strtok(NULL, "");
    *second = atoi(ptr);

    
    return;
}