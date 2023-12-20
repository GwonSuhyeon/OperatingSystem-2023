#define _GNU_SOURCE


#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <time.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <sys/time.h>

#include <sched.h>
#include <linux/sched.h>
#include <sys/syscall.h>


// 적용 스케줄러 종류
#define POLICY_CFS_DEFAULT 1
#define POLICY_CFS_NICE 2
#define POLICY_RT_FIFO 3
#define POLICY_RT_RR 4

#define MAX_ARRAY_ROW_SIZE 100 // 배열 최대 크기
#define MAX_ARRAY_COLUMN_SIZE 100 // 배열 최대 크기

#define MAX_TIME_BUFFER_SIZE 10 // 시간 문자열 버퍼 크기

#define MAX_CHILD_PROCESS_CNT 21 // 생성할 최대 자식 프로세스 개수

#define SHARED_MEMORY_SIZE 8 // shared memory 크기 (byte)

#define PATH_RT_RR_TIMESLICE "/proc/sys/kernel/sched_rr_timeslice_ms" // 라운드 로빈 스케줄러 time quantum 지정 파일 위치


// 스케줄러 attribute 구조체
struct sched_attr {
	__u32 size;

	__u32 sched_policy;
	__u64 sched_flags;

	/* SCHED_NORMAL, SCHED_BATCH */
	__s32 sched_nice;

	/* SCHED_FIFO, SCHED_RR */
	__u32 sched_priority;

	/* SCHED_DEADLINE */
	__u64 sched_runtime;
	__u64 sched_deadline;
	__u64 sched_period;

	/* Utilization hints */
	__u32 sched_util_min;
	__u32 sched_util_max;

};


int Set_Cpu_Affinity(); // cpu core 1개로 제한하기 위한 함수

void Print_Result(int policy, double totalTime, int timeQuantum); // 배열 연산 종료 후 결과 출력하기 위한 함수


int main(void)
{
    struct sched_attr attr;

    struct timespec startTimeSpec;
    struct timespec endTimeSpec;
    struct timespec elapsedTimeSpec;

    struct tm startTM;
    struct tm endTM;

    pid_t pid;
    pid_t childPid[MAX_CHILD_PROCESS_CNT];
    pid_t waited;
    
    char startTimeBuffer[MAX_TIME_BUFFER_SIZE];
    char endTimeBuffer[MAX_TIME_BUFFER_SIZE];

    char timeQuantumBuffer[5];

    int fd;

    int policy = 0;

    int timeQuantum = 0;

    int status;

    int writeResult = 0;
    int cpuSetResult = 0;
    int schedResult = 0;

    int count = 0;
    int result[MAX_ARRAY_ROW_SIZE][MAX_ARRAY_COLUMN_SIZE];
    int A[MAX_ARRAY_ROW_SIZE][MAX_ARRAY_COLUMN_SIZE];
    int B[MAX_ARRAY_ROW_SIZE][MAX_ARRAY_COLUMN_SIZE];

    double elapsedTime;
    double totalTime;

    double *shared_memory;
    int shared_memory_id;


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


    // shared memory 생성
    shared_memory_id = shm_open("/my_shared_memory", O_CREAT | O_RDWR, 0777);
    if(shared_memory_id == -1)
    {
        printf("shared memory open fail\n");
        
        return -1;
    }

    // shared memory 설정
    if(ftruncate(shared_memory_id, SHARED_MEMORY_SIZE) == -1)
    {
        printf("shared memory truncate fail\n");
        
        return -1;
    }

    // shared memory mapping
    shared_memory = mmap(NULL, SHARED_MEMORY_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, shared_memory_id, 0);
    if(shared_memory == MAP_FAILED)
    {
        printf("shared memory mapping fail\n");

        return -1;
    }


    // 스케줄러 attribute 초기화
    memset(&attr, 0, sizeof(attr));
    attr.size = sizeof(attr);


    // 적용할 스케줄러 정책 선택
    printf("1. CFS_DEFAULT\n");
    printf("2. CFS_NICE\n");
    printf("3. RT_FIFO\n");
    printf("4. RT_RR\n");
    printf("Input the Scheduling policy to apply: ");

    scanf("%d", &policy);


    switch(policy)
    {
        // CFS 선택 시 policy 변경 안함.
        case POLICY_CFS_DEFAULT:
        {
            // attr.sched_policy = SCHED_NORMAL;
        }
        break;

        // CFS 선택 시 policy 변경 안함.
        case POLICY_CFS_NICE:
        {
            // attr.sched_policy = SCHED_NORMAL;
        }
        break;

        // RT_FIFO 선택 시 policy는 FIFO로 변경
        // rt_priority는 95로 임의 변경
        case POLICY_RT_FIFO:
        {
            attr.sched_policy = SCHED_FIFO;
            attr.sched_priority = 95;
        }
        break;

        // RT_RR 선택 시 time quantum 지정함.
        case POLICY_RT_RR:
        {
            printf("1. 10 ms\n");
            printf("2. 100 ms\n");
            printf("3. 1000 ms\n");
            printf("Input the Time Quantum to apply: ");

            scanf("%d", &timeQuantum);

            if(timeQuantum < 1 || timeQuantum > 3)
            {
                printf("Invalid input\n");

                return -1;
            }

            // time quantum 10ms로 선택
            if(timeQuantum == 1)
            {
                timeQuantum = 10;

                sprintf(timeQuantumBuffer, "%d", timeQuantum);
            }

            // time quantum 100ms로 선택
            else if(timeQuantum == 2)
            {
                timeQuantum = 100;

                sprintf(timeQuantumBuffer, "%d", timeQuantum);
            }

            // time quantum 1000ms로 선택
            else if(timeQuantum == 3)
            {
                timeQuantum = 1000;

                sprintf(timeQuantumBuffer, "%d", timeQuantum);
            }
            
            // 라운드 로빈의 time quantum을 지정하는 sched_rr_timeslice_ms 파일을 오픈
            fd = open(PATH_RT_RR_TIMESLICE, O_WRONLY | O_TRUNC);
            if(fd == -1)
            {
                printf("sched_rr_timeslice_ms file open error\n");
                
                return -1;
            }

            // 오픈한 sched_rr_timeslice_ms 파일에 선택된 time quantum 값으로 변경
            writeResult = write(fd, timeQuantumBuffer, strlen(timeQuantumBuffer));
            if(writeResult == -1)
            {
                printf("sched_rr_timeslice file write error\n");

                if(fd >= 0)
                {
                    close(fd);
                }

                return -1;
            }

            if(fd >= 0)
            {
                close(fd);
            }
            
            // policy를 라운드 로빈으로 변경하고
            // rt_priority를 95로 임의 변경
            attr.sched_policy = SCHED_RR;
            attr.sched_priority = 95;
        }
        break;

        // 잘못된 입력에 대한 예외 처리
        default:
        {
            printf("Invalid input\n");

            return -1;
        }
        break;
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
            // 생성된 자식 프로세스에서는 스케줄러 정책이 Nice 값을 변경하는 CFS를 사용하는 경우
            // 먼저 생성되는 자식 프로세스들 부터 7개씩 각각 Nice 값을 19, 0, -20으로 설정
            if((policy == POLICY_CFS_NICE) && (i < 7))
            {
                attr.sched_nice = 19;
            }
            else if((policy == POLICY_CFS_NICE) && (i >= 7 && i < 14))
            {
                attr.sched_nice = 0;
            }
            else if((policy == POLICY_CFS_NICE) && (i >= 14))
            {
                attr.sched_nice = -20;
            }

            // 어떠한 변경도 하지 않는 Default CFS를 제외한 다른 정책들을 선택한 경우
            // 해당 정책을 자식 프로세스들에게 적용하기 위해 sys_sched_setattr 시스템콜을 호출
            if(policy != POLICY_CFS_DEFAULT)
            {
                // flag에 별도의 값을 설정하지 않았기 때문에 자식 프로세스가 부모 프로세스의 스케줄링 속성을 상속 받는다
                schedResult = syscall(SYS_sched_setattr, pid, &attr, 0);
                if(schedResult == -1)
                {
                    printf("sched_attr error\n");

                    return -1;
                }
            }

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

        // 자식 프로세스들이 자신의 실행시간을 누적한 shared memory의 값을 가져옴
        totalTime = *shared_memory;

        // shared memory 해제
        munmap(shared_memory, SHARED_MEMORY_SIZE);
        close(shared_memory_id);
        shm_unlink("/my_shared_memory");

        // 최종 실행 결과를 출력
        Print_Result(policy, totalTime, timeQuantum);
    }

    // 생성된 자식 프로세스인 경우
    else if(pid == 0)
    {
        // 실행시간을 측정하기 위해 start time을 측정
        clock_gettime(CLOCK_REALTIME, &startTimeSpec);

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

        // 실행시간을 측정하기 위해 end time을 측정
        clock_gettime(CLOCK_REALTIME, &endTimeSpec);

        // 측정한 start time과 end time을 로컬 시간으로 변경
        localtime_r((time_t *)&startTimeSpec.tv_sec, &startTM);
        localtime_r((time_t *)&endTimeSpec.tv_sec, &endTM);


        // 경과 시간 계산
        if(endTimeSpec.tv_nsec < startTimeSpec.tv_nsec)
        {
            elapsedTimeSpec.tv_sec = endTimeSpec.tv_sec - startTimeSpec.tv_sec - 1;
            elapsedTimeSpec.tv_nsec = 1000000000 + endTimeSpec.tv_nsec - startTimeSpec.tv_nsec;
        }
        else
        {
            elapsedTimeSpec.tv_sec = endTimeSpec.tv_sec - startTimeSpec.tv_sec;
            elapsedTimeSpec.tv_nsec = endTimeSpec.tv_nsec - startTimeSpec.tv_nsec;
        }

        elapsedTime = (double)elapsedTimeSpec.tv_sec + (double)elapsedTimeSpec.tv_nsec / 1000000000.0;

        // 실행시간을 출력하기 위해 문자열로 변환
        strftime(startTimeBuffer, sizeof(startTimeBuffer), "%H:%M:%S", &startTM);
        strftime(endTimeBuffer, sizeof(endTimeBuffer), "%H:%M:%S", &endTM);

        if(policy == POLICY_CFS_DEFAULT || policy == POLICY_CFS_NICE)
        {
            // 스케줄러 정책이 CFS이면 Nice 값도 같이 출력
            printf("PID: %d | NICE: %d | Start time: %s.%06ld | End time: %s.%06ld | Elapsed time: %f\n", 
            getpid(), attr.sched_nice, startTimeBuffer, startTimeSpec.tv_nsec / 1000, endTimeBuffer, endTimeSpec.tv_nsec / 1000, elapsedTime);
        }
        else
        {
            printf("PID: %d | Start time: %s.%06ld | End time: %s.%06ld | Elapsed time: %f\n", 
            getpid(), startTimeBuffer, startTimeSpec.tv_nsec / 1000, endTimeBuffer, endTimeSpec.tv_nsec / 1000, elapsedTime);
        }

        // 자식 프로세스는 자신의 실행시간을 shared memory에 누적 저장함
        *shared_memory += elapsedTime;
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


void Print_Result(int policy, double totalTime, int timeQuantum)
{
    // 스케줄러 정책에 따라 policy 종류와 평균 실행시간을 출력
    // 라운드 로빈을 사용한 경우에는 time quantum도 같이 출력
    switch(policy)
    {
        case POLICY_CFS_DEFAULT:
        {
            printf("Scheduling Policy: CFS_DEFAULT | Average elapsed time: %f\n", (double)(totalTime / MAX_CHILD_PROCESS_CNT));
        }
        break;

        case POLICY_CFS_NICE:
        {
            printf("Scheduling Policy: CFS_NICE | Average elapsed time: %f\n", (double)(totalTime / MAX_CHILD_PROCESS_CNT));
        }
        break;

        case POLICY_RT_FIFO:
        {
            printf("Scheduling Policy: RT_FIFO | Average elapsed time: %f\n", (double)(totalTime / MAX_CHILD_PROCESS_CNT));
        }
        break;

        case POLICY_RT_RR:
        {
            printf("Scheduling Policy: RT_RR | Time Quantum: %d ms | Average elapsed time: %f\n", timeQuantum, (double)(totalTime / MAX_CHILD_PROCESS_CNT));
        }
        break;
    }


    return;
}