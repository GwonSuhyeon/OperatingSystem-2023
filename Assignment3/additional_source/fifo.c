#define _GNU_SOURCE


#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sched.h>


#define MAX_ARRAY_ROW_SIZE 100 // 배열 최대 크기
#define MAX_ARRAY_COLUMN_SIZE 100 // 배열 최대 크기

#define MAX_CHILD_PROCESS_CNT 21 // 생성할 최대 자식 프로세스 개수


int Set_Cpu_Affinity(); // cpu core 1개로 제한하기 위한 함수


int main(void)
{
    pid_t pid;
    pid_t childPid[MAX_CHILD_PROCESS_CNT];
    pid_t waited;

    int status;

    int cpuSetResult = 0;

    int count = 0;
    int result[MAX_ARRAY_ROW_SIZE][MAX_ARRAY_COLUMN_SIZE];
    int A[MAX_ARRAY_ROW_SIZE][MAX_ARRAY_COLUMN_SIZE];
    int B[MAX_ARRAY_ROW_SIZE][MAX_ARRAY_COLUMN_SIZE];


    // 사용할 core 개수 1개로 제한
    cpuSetResult = Set_Cpu_Affinity();
    if(cpuSetResult == -1)
    {
        return -1;
    }


    // 배열 연산에 사용할 배열 초기화
    for(int i = 0; i < MAX_ARRAY_ROW_SIZE; i++)
    {
        for(int k = 0; k < MAX_ARRAY_COLUMN_SIZE; k++)
        {
            A[i][k] = i;
            B[i][k] = k;
            result[i][k] = 0;
        }
    }


    // 자식 프로세스 21개 생성
    for(int i = 0; i < MAX_CHILD_PROCESS_CNT; i++)
    {
        // 자식 프로세스 생성
        pid = fork();
        if(pid == -1)
        {
            printf("child process create error\n");

            return -1;
        }
        else if(pid > 0)
        {
            // 부모 프로세스에서는 생성된 자식 프로세스의 pid를 저장
            childPid[i] = pid;
        }
        else if(pid == 0)
        {
            break;
        }
    }

    // 부모 프로세스인 경우
    if(pid > 0)
    {
        // 부모 프로세스가 생성한 자식 프로세스들의 종료를 기다림
        for(int i = 0; i < MAX_CHILD_PROCESS_CNT; i++)
        {
            waited = waitpid(childPid[i], &status, 0);
        }

        // 종료된 자식 프로세스들의 pid를 출력
        for(int i = 0; i < MAX_CHILD_PROCESS_CNT; i++)
        {
            printf("PID: %d\n", childPid[i]);
        }
    }

    // 생성된 자식 프로세스인 경우
    else if(pid == 0)
    {
        // 배열 연산
        count = 0;
        while(count < 100)
        {
            for(int k = 0; k < 100; k++)
            {
                for(int i = 0; i < 100; i++)
                {
                    for(int j = 0; j < 100; j++)
                    {
                        result[k][j] += A[k][i] * B[i][j];
                    }
                }
            }

            count++;
        }
    }


    return 0;
}


int Set_Cpu_Affinity()
{
    cpu_set_t cpuMask; // cpu 마스크

    int usingCore = 0; // 사용할 core 번호


    CPU_ZERO(&cpuMask); // cpu 마스크 초기화
    CPU_SET(usingCore, &cpuMask); // cpu 마스크에 사용할 core를 추가

    // getpid() 대신 0을 사용해도 됨
    // 0을 사용하면 현재 동작 중인 프로세스를 가리킨다
    // 부모 프로세스에 cpu affinity를 설정하면 fork로 생성되는 자식 프로세스도
    // 부모 프로세스와 동일한 cpu affinity를 가진다
    if(sched_setaffinity(getpid(), sizeof(cpuMask), &cpuMask) == -1)
    {
        printf("sched_setaffinity error\n");

        return -1;
    }


    return 0;
}