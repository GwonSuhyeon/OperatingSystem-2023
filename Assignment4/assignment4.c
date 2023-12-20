#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <math.h>


// 가상주소 길이
#define VIRTUAL_ADDR_LEN_18_BIT 18
#define VIRTUAL_ADDR_LEN_19_BIT 19
#define VIRTUAL_ADDR_LEN_20_BIT 20

// 페이지 크기
#define PAGE_SIZE_1_KB 1
#define PAGE_SIZE_2_KB 2
#define PAGE_SIZE_4_KB 4

// 물리메모리 크기
#define PYSICAL_MEMORY_SIZE_32_KB 32
#define PYSICAL_MEMORY_SIZE_64_KB 64

// 페이지 교체 알고리즘 종류
#define PAGE_REPLACEMENT_OPTIMAL 1
#define PAGE_REPLACEMENT_FIFO 2
#define PAGE_REPLACEMENT_LRU 3
#define PAGE_REPLACEMENT_SECOND_CHANCE 4

// 가상주소 입력 방식
#define AUTO_INPUT 1
#define NON_AUTO_INPUT 2

// 초기 할당되지 않은 프레임 설정 값
#define EMPTY_FRAME_VALUE -1

#define INVALID_VALUE -1

#define PAGE_FAULT 1
#define PAGE_HIT 2

// 최대 가상주소 입력 개수
#define MAX_ADDR_CNT 5000


// 프레임 관리 구조체
typedef struct Frame
{
    int pageNumber; // 페이지 넘버

    int OptimalPeriod; // optimal 프레임 사용 주기
    int LRUPeriod; // lru 프레임 사용 주기
    int SecondBit; // second chance 참조 비트

    int sequence; // 프레임 순서

} Frame;


int Generate_Virtual_Addr(char *fileName, int virtualAddrLen); // 가상주소 랜덤 생성

int Read_Input_File(char *filePath); // 가상주소 파일 읽기

int Get_Page_Offset(int pageSize); // 페이지 오프셋 계산
int Get_Page_Cnt(int virtualAddrLen, int pageOffset); // 전체 페이지 개수 계산
int Get_Frame_Cnt(int pageSize, int pysicalMemorySize); // 전체 프레임 개수 계산

int Run_Simulation(Frame *frames, int replacementType, int inputType, int virtualAddrLen, int pageSize, int pageOffset, int frameCnt, int pageCnt, char *fileName); // 시뮬레이터 실행
int Execute_Replacement(Frame *frames, int replacementType, int pageNumber, int pageNumberIdx, int replacedFrame, int frameCnt, int pageCnt); // 페이지 교체 알고리즘 실행

// 해당하는 페이지 교체 알고리즘 연산 수행
int Replacement_FIFO(Frame *frames, int pageNumber, int replacedFrame, int frameCnt);
int Replacement_OPTIMAL(Frame *frames, int pageNumber, int pageNumberIdx, int replacedFrame, int frameCnt, int pageCnt);
int Replacement_LRU(Frame *frames, int pageNumber, int replacedFrame, int frameCnt);
int Replacement_Second_Chance(Frame *frames, int pageNumber, int replacedFrame, int frameCnt);

// page fault 검사
int Check_Page_Fault(Frame *frames, int pageNumber, int *frameNumber, int frameCnt);

// 프레임 입력 순서 갱신
int Update_Frame_Sequence(Frame *frames, int idx, int frameCnt);

// 결과 출력
void Print_Output();

// 결과 저장
int Save_Output(int replacementType);


int VirtualAddr[MAX_ADDR_CNT]; // 가상주소 저장

int PageNumberList[MAX_ADDR_CNT]; // page number 저장

// 페이지 교체 알고리즘 수행 후의 frame number, 물리주소, fault/hit 여부 저장
int OutputFrameNumber[MAX_ADDR_CNT];
int OutputPysicalAddr[MAX_ADDR_CNT];
int OutputFaultHit[MAX_ADDR_CNT];

int currentFrame = -1; // 현재까지 할당된 프레임 개수

// Second-Chance 원형큐 인덱스
int FrontIdx = 0;
int BackIdx = 0;


int main(void)
{
    int promptInput = 0;

    int virtualAddrLen = 0;
    int pageSize = 0;
    int pysicalMemorySize = 0;
    int replacementType = 0;
    int inputType = 0;

    int pageOffset = 0;
    int pageCnt = 0;

    int frameCnt = 0;
    Frame *frames = NULL;

    char fileName[1024] = "\0";


    // 가상주소 길이 입력
    promptInput = 0;
    printf("A. Simulation에 사용할 가상주소 길이를 선택하시오 (1. 18bits  2. 19bits  3. 20bits): ");
    scanf("%d", &promptInput);
    if(promptInput == 1)
    {
        virtualAddrLen = VIRTUAL_ADDR_LEN_18_BIT;
    }
    else if(promptInput == 2)
    {
        virtualAddrLen = VIRTUAL_ADDR_LEN_19_BIT;
    }
    else if(promptInput == 3)
    {
        virtualAddrLen = VIRTUAL_ADDR_LEN_20_BIT;
    }
    else
    {
        printf("Invalid Input\n");

        return -1;
    }

    // 페이지 크기 입력
    promptInput = 0;
    printf("\nB. Simulation에 사용할 페이지(프레임)의 크기를 선택하시오 (1. 1KB  2. 2KB  3. 4KB): ");
    scanf("%d", &promptInput);
    if(promptInput == 1)
    {
        pageSize = PAGE_SIZE_1_KB;
    }
    else if(promptInput == 2)
    {
        pageSize = PAGE_SIZE_2_KB;
    }
    else if(promptInput == 3)
    {
        pageSize = PAGE_SIZE_4_KB;
    }
    else
    {
        printf("Invalid Input\n");

        return -1;
    }

    // 물리메모리 크기 입력
    promptInput = 0;
    printf("\nC. Simulation에 사용할 물리메모리의 크기를 선택하시오 (1. 32KB  2. 64KB): ");
    scanf("%d", &promptInput);
    if(promptInput == 1)
    {
        pysicalMemorySize = PYSICAL_MEMORY_SIZE_32_KB;
    }
    else if(promptInput == 2)
    {
        pysicalMemorySize = PYSICAL_MEMORY_SIZE_64_KB;
    }
    else
    {
        printf("Invalid Input\n");

        return -1;
    }

    // 페이지 교체 알고리즘 방식 입력
    promptInput = 0;
    printf("\nD. Simulation에 적용할 Page Replacement 알고리즘을 선택하시오\n(1. Optimal  2. FIFO  3. LRU  4. Second-Chance): ");
    scanf("%d", &promptInput);
    if(promptInput == 1)
    {
        replacementType = PAGE_REPLACEMENT_OPTIMAL;
    }
    else if(promptInput == 2)
    {
        replacementType = PAGE_REPLACEMENT_FIFO;
    }
    else if(promptInput == 3)
    {
        replacementType = PAGE_REPLACEMENT_LRU;
    }
    else if(promptInput == 4)
    {
        replacementType = PAGE_REPLACEMENT_SECOND_CHANCE;
    }
    else
    {
        printf("Invalid Input\n");

        return -1;
    }

    // 가상주소 입력 방식 입력
    promptInput = 0;
    memset(fileName, 0, 1024);
    printf("\nE. 가상주소 스트링 입력방식을 선택하시오\n(1. input.in 자동 생성  2. 기존 파일 사용): ");
    scanf("%d", &promptInput);
    if(promptInput == 1)
    {
        inputType = AUTO_INPUT;

        sprintf(fileName, "%s", "input.in");
    }
    else if(promptInput == 2)
    {
        inputType = NON_AUTO_INPUT;

        printf("\nF. 입력 파일 이름을 입력하시오: ");
        scanf("%s", fileName);
    }
    else
    {
        printf("Invalid Input\n");

        return -1;
    }

    // 페이지 오프셋 계산
    pageOffset = Get_Page_Offset(pageSize);
    if(pageOffset == INVALID_VALUE)
    {
        printf("Get page offset failed\n");

        return -1;
    }

    // 페이지 개수 계산
    pageCnt = Get_Page_Cnt(virtualAddrLen, pageOffset);
    if(pageCnt == INVALID_VALUE)
    {
        printf("Get page cnt failed\n");

        return -1;
    }

    // 프레임 개수 계산
    frameCnt = Get_Frame_Cnt(pageSize, pysicalMemorySize);
    if(frameCnt == INVALID_VALUE)
    {
        printf("Get frame cnt failed\n");

        return -1;
    }

    // 전체 프레임 개수만큼의 프레임 배열 생성
    frames = (Frame *)malloc(frameCnt * sizeof(Frame));
    if(frames == NULL)
    {
        printf("Frames memory allocate failed\n");

        return -1;
    }

    // 프레임 배열 초기화
    for(int i = 0; i < frameCnt; i++)
    {
        (frames + i)->pageNumber = EMPTY_FRAME_VALUE;
        (frames + i)->OptimalPeriod = 0;
        (frames + i)->LRUPeriod = 0;
        (frames + i)->SecondBit = 0;
    }


    // 시뮬레이터 실행
    Run_Simulation(frames, replacementType, inputType, virtualAddrLen, pageSize, pageOffset, frameCnt, pageCnt, fileName);


    // 프레임 배열 해제
    if(frames != NULL)
    {
        free(frames);

        frames = NULL;
    }


    return 0;
}


int Get_Page_Offset(int pageSize)
{
    if(pageSize == PAGE_SIZE_1_KB)
    {
        return 10;
    }
    else if(pageSize == PAGE_SIZE_2_KB)
    {
        return 11;
    }
    else if(pageSize == PAGE_SIZE_4_KB)
    {
        return 12;
    }


    return INVALID_VALUE;
}


int Get_Page_Cnt(int virtualAddrLen, int pageOffset)
{
    int pageNumberBit = 0;

    int pageCnt = INVALID_VALUE;

    
    if(virtualAddrLen != VIRTUAL_ADDR_LEN_18_BIT && virtualAddrLen != VIRTUAL_ADDR_LEN_19_BIT && virtualAddrLen != VIRTUAL_ADDR_LEN_20_BIT)
    {
        return INVALID_VALUE;
    }

    if(pageOffset != 10 && pageOffset != 11 & pageOffset != 12)
    {
        return INVALID_VALUE;
    }

    pageNumberBit = virtualAddrLen - pageOffset;

    for(int i = 0; i < pageNumberBit; i++)
    {
        if(i == 0)
        {
            pageCnt = 1;
        }
        else
        {
            pageCnt += (int)pow(2, i);
        }
    }

    
    return pageCnt;
}


int Get_Frame_Cnt(int pageSize, int pysicalMemorySize)
{
    int frameCnt = INVALID_VALUE;

    
    if(pageSize != PAGE_SIZE_1_KB && pageSize != PAGE_SIZE_2_KB && pageSize != PAGE_SIZE_4_KB)
    {
        return INVALID_VALUE;
    }
    
    if(pysicalMemorySize != PYSICAL_MEMORY_SIZE_32_KB && pysicalMemorySize != PYSICAL_MEMORY_SIZE_64_KB)
    {
        return INVALID_VALUE;
    }

    frameCnt = pysicalMemorySize / pageSize;


    return frameCnt;
}


int Run_Simulation(Frame *frames, int replacementType, int inputType, int virtualAddrLen, int pageSize, int pageOffset, int frameCnt, int pageCnt, char *fileName)
{
    int *addressBit = NULL; // 10진수 가상주소의 비트 저장 배열

    int num = 0;

    int pageNumber = 0;
    int pageBitIdx = 0; // page number bit 영역의 인덱스

    int isPageFault = 0;

    int frameNumber = 0;
    int replacedFrame = -1;

    int offset = 0;

    int pysicalAddr = 0;
    int pysicalBitIdx = 0;


    // 가상주소 임의 생성 시 랜덤한 가상주소 생성
    if(inputType == AUTO_INPUT)
    {
        Generate_Virtual_Addr(fileName, virtualAddrLen);
    }

    // 가상주소 파일 읽기
    Read_Input_File(fileName);

    // 10진수인 가상주소의 비트 저장 배열 생성
    addressBit = (int *)malloc(virtualAddrLen * sizeof(int));
    if(addressBit == NULL)
    {
        return -1;
    }

    // 입력된 모든 가상주소에 대해서 반복
    for(int i = 0; i < MAX_ADDR_CNT; i++)
    {
        pageNumber = 0;
        pageBitIdx = 0;

        // 가상주소 비트 배열 초기화
        for(int k = 0; k < virtualAddrLen; k++)
        {
            *(addressBit + k) = 0;
        }

        // 10진수 가상주소의 비트 계산
        num = VirtualAddr[i];
        for(int k = 0; k < virtualAddrLen; k++)
        {
            *(addressBit + k) = num % 2;

            num = num / 2;
        }

        // page number 계산
        pageBitIdx = 0;
        for(int k = pageOffset; k < virtualAddrLen; k++)
        {
            if(*(addressBit + k) == 1)
            {
                if(pageBitIdx == 0)
                {
                    pageNumber = 1;
                }
                else
                {
                    pageNumber += (int)pow(2, pageBitIdx);
                }
            }

            pageBitIdx++;
        }

        
        PageNumberList[i] = pageNumber;
    }

    
    // 입력된 모든 가상주소에 대해서 반복
    for(int i = 0; i < MAX_ADDR_CNT; i++)
    {
        num = 0;
        pageNumber = 0;
        pageBitIdx = 0;
        isPageFault = 0;
        pysicalAddr = 0;
        pysicalBitIdx = 0;

        // 가상주소 비트 배열 초기화
        for(int k = 0; k < virtualAddrLen; k++)
        {
            *(addressBit + k) = 0;
        }


        // 10진수 가상주소의 비트 계산
        num = VirtualAddr[i];
        for(int k = 0; k < virtualAddrLen; k++)
        {
            *(addressBit + k) = num % 2;

            num = num / 2;
        }


        // page fault/hit 검사
        isPageFault = Check_Page_Fault(frames, PageNumberList[i], &frameNumber, frameCnt);
        if(isPageFault == INVALID_VALUE)
        {
            return -1;
        }

        if(isPageFault == PAGE_FAULT)
        {
            // page fault 발생한 경우에 페이지 교체 알고리즘 연산 수행

            replacedFrame = Execute_Replacement(frames, replacementType, PageNumberList[i], i, replacedFrame, frameCnt, pageCnt);

            frameNumber = replacedFrame;
        }
        else if(isPageFault == PAGE_HIT)
        {
            // page hit 발생한 경우에 적용중인 페이지 교체 알고리즘에 따라서
            // 프레임 주기, 참조비트 등을 갱신

            if(replacementType == PAGE_REPLACEMENT_OPTIMAL)
            {
                // Optimal 적용중인 경우

                int period = 0;


                // 현재 hit된 프레임 외의 모든 프레임 주기 갱신
                for(int k = 0; k <= currentFrame; k++)
                {
                    if((k != frameNumber) && ((frames + k)->OptimalPeriod > 0))
                    {
                        (frames + k)->OptimalPeriod -= 1;
                    }
                }

                // 교체된 프레임의 다음 사용주기 갱신
                for(int k = (i + 1); k < MAX_ADDR_CNT; k++)
                {
                    period++;

                    if(*(PageNumberList + k) == PageNumberList[i])
                    {
                        (frames + frameNumber)->OptimalPeriod = period;

                        break;
                    }

                    if((k + 1) == MAX_ADDR_CNT)
                    {
                        (frames + frameNumber)->OptimalPeriod = period + 1;
                    }
                }
            }

            if(replacementType == PAGE_REPLACEMENT_LRU)
            {
                // LRU 적용중인 경우

                // 현재 hit된 프레임과 그 외의 프레임들의 주기 갱신
                for(int i = 0; i <= currentFrame; i++)
                {
                    if(i == frameNumber)
                    {
                        (frames + i)->LRUPeriod = 0;
                    }
                    else
                    {
                        (frames + i)->LRUPeriod += 1;
                    }
                }
            }

            if(replacementType == PAGE_REPLACEMENT_SECOND_CHANCE)
            {
                // Second-Chance 적용중인 경우

                // 프레임 참조비트 갱신
                 (frames + frameNumber)->SecondBit = 1;
            }
        }

        // 물리주소 계산을 위한 오프셋 크기 계산
        offset = 0;
        pysicalBitIdx = 0;
        for(int k = 0; k < pageOffset; k++)
        {
            if(*(addressBit + k) == 1)
            {
                if(pysicalBitIdx == 0)
                {
                    offset = 1;
                }
                else
                {
                    offset += (int)pow(2, pysicalBitIdx);
                }
            }

            pysicalBitIdx++;
        }
        

        OutputFrameNumber[i] = frameNumber;
        OutputPysicalAddr[i] = (frameNumber * (int)pow(2, pageOffset)) + offset; // 물리주소 계산
        OutputFaultHit[i] = isPageFault;
    }

    // 메모리 해제
    if(addressBit != NULL)
    {
        free(addressBit);

        addressBit = NULL;
    }


    // 시뮬레이션 수행 결과 저장
    Save_Output(replacementType);


    return 0;
}


int Generate_Virtual_Addr(char *fileName, int virtualAddrLen)
{
    FILE *fp = NULL;

    int maxAddr = 0;
    int randomAddr = 0;

    int writeResult = 0;

    char buffer[10] = "\0";


    // 랜덤 가상주소 값 저장할 파일 생성
    fp = fopen(fileName, "w");
    if(fp == NULL)
    {
        printf("Random input file create failed\n");

        return INVALID_VALUE;
    }


    srand(time(NULL));

    
    // 생성할 가상주소 최대값 설정
    if(virtualAddrLen == VIRTUAL_ADDR_LEN_18_BIT)
    {
        maxAddr = 262143;
    }
    else if(virtualAddrLen == VIRTUAL_ADDR_LEN_19_BIT)
    {
        maxAddr = 524287;
    }
    else if(virtualAddrLen == VIRTUAL_ADDR_LEN_20_BIT)
    {
        maxAddr = 1048575;
    }
    else
    {
        return INVALID_VALUE;
    }


    // 생성되는 가상주소 값 저장
    for(int i = 0; i < MAX_ADDR_CNT; i++)
    {
        memset(buffer, 0, 10);

        randomAddr = rand() % (maxAddr + 1);

        if(i < (MAX_ADDR_CNT - 1))
        {
            sprintf(buffer, "%d\n", randomAddr);
        }
        else
        {
            sprintf(buffer, "%d", randomAddr);
        }

        writeResult = fputs(buffer, fp);
        if(writeResult == EOF)
        {
            printf("Random input file write failed\n");

            if(fp != NULL)
            {
                fclose(fp);
            }

            fp = NULL;

            return INVALID_VALUE;
        }
    }

    // 파일 닫기
    if(fp != NULL)
    {
        fclose(fp);

        fp = NULL;
    }


    return 0;
}


int Read_Input_File(char *fileName)
{
    FILE *fp = NULL;

    char buffer[10] = "\0";

    int idx = 0;


    // 가상주소 파일 오픈
    fp = fopen(fileName, "r");

    if (fp == NULL)
    {
        printf("Virtual Address file open failed\n");

        return INVALID_VALUE;
    }

    // 파일에서 가상주소 값 읽기
    while(fgets(buffer, sizeof(buffer), fp) != NULL)
    {
        VirtualAddr[idx] = atoi(buffer);

        idx++;
    }

    // 파일 닫기
    if(fp != NULL)
    {
        fclose(fp);

        fp = NULL;
    }


    return 0;
}


int Check_Page_Fault(Frame *frames, int pageNumber, int *frameNumber, int frameCnt)
{
    int isPageFault = PAGE_FAULT;


    if(frames == NULL)
    {
        return INVALID_VALUE;
    }

    // page fault/hit 여부 검사
    for(int i = 0; i < frameCnt; i++)
    {
        if((frames + i) == NULL)
        {
            isPageFault = INVALID_VALUE;

            break;
        }

        if((frames + i)->pageNumber == pageNumber)
        {
            isPageFault = PAGE_HIT;

            *frameNumber = i;

            break;
        }
    }


    return isPageFault;
}


int Execute_Replacement(Frame *frames, int replacementType, int pageNumber, int pageNumberIdx, int replacedFrame, int frameCnt, int pageCnt)
{
    int replaceResult = 0;


    // page fault가 발생한 page number에 대하여 교체할 프레임 선택하기 위한 알고리즘 수행
    switch(replacementType)
    {
        case PAGE_REPLACEMENT_OPTIMAL:
        {
            replaceResult = Replacement_OPTIMAL(frames, pageNumber, pageNumberIdx, replacedFrame, frameCnt, pageCnt);
        }
        break;

        case PAGE_REPLACEMENT_FIFO:
        {
            replaceResult = Replacement_FIFO(frames, pageNumber, replacedFrame, frameCnt);
        }
        break;

        case PAGE_REPLACEMENT_LRU:
        {
            replaceResult = Replacement_LRU(frames, pageNumber, replacedFrame, frameCnt);
        }
        break;

        case PAGE_REPLACEMENT_SECOND_CHANCE:
        {
            replaceResult = Replacement_Second_Chance(frames, pageNumber, replacedFrame, frameCnt);
        }
        break;
    }

    if(replaceResult == INVALID_VALUE)
    {
        return -1;
    }


    return replaceResult;
}


int Replacement_FIFO(Frame *frames, int pageNumber, int replacedFrame, int frameCnt)
{
    int idx = replacedFrame;


    // 들어온 순서에 따라서 교체할 프레임 선택
    if((idx + 1) == frameCnt)
    {
        idx = 0;
    }
    else
    {
        idx += 1;
    }

    (frames + idx)->pageNumber = pageNumber;


    return idx;
}


int Replacement_OPTIMAL(Frame *frames, int pageNumber, int pageNumberIdx, int replacedFrame, int frameCnt, int pageCnt)
{
    int period = 0;
    int maxPeriod = 0;
    int minSequence = 0;
    int frameIdx = 0;


    // 프레임 개수가 최대 프레임 개수보다 적을 경우
    if(currentFrame < (frameCnt - 1))
    {
        currentFrame += 1;

        // 프레임 할당
        (frames + currentFrame)->pageNumber = pageNumber;

        // 프레임 순서 갱신
        (frames + currentFrame)->sequence = (currentFrame + 1);

        // 교체된 프레임의 다음 사용주기 갱신
        for(int i = (pageNumberIdx + 1); i < MAX_ADDR_CNT; i++)
        {
            period++;

            if(*(PageNumberList + i) == pageNumber)
            {
                (frames + currentFrame)->OptimalPeriod = period;

                break;
            }

            if((i + 1) == MAX_ADDR_CNT)
            {
                (frames + currentFrame)->OptimalPeriod = period + 1;
            }
        }

        // 교체된 프레임 외의 프레임들의 다음 사용주기 갱신
        for(int i = 0; i < currentFrame; i++)
        {
            if((frames + i)->OptimalPeriod > 0)
            {
                (frames + i)->OptimalPeriod -= 1;
            }
        }


        return currentFrame;
    }


    // 프레임이 이미 최대 프레임 개수만큼 존재하는 경우
    for(int i = 0; i < frameCnt; i++)
    {
        if(i == 0)
        {
            maxPeriod = (frames + i)->OptimalPeriod;
            minSequence = (frames + i)->sequence;

            frameIdx = i;
        }
        else
        {
            // 프레임들의 다음 사용주기를 비교하면서 교체할 프레임 탐색
            if((frames + i)->OptimalPeriod > maxPeriod)
            {
                maxPeriod = (frames + i)->OptimalPeriod;
                minSequence = (frames + i)->sequence;

                frameIdx = i;
            }
            else if((frames + i)->OptimalPeriod == maxPeriod)
            {
                // 다음 사용주기가 동일한 프레임이 2개 이상 발견된 경우
                // 들어온 프레임 순서를 비교하여 교체할 프레임 탐색
                if((frames + i)->sequence < minSequence)
                {
                    maxPeriod = (frames + i)->OptimalPeriod;
                    minSequence = (frames + i)->sequence;

                    frameIdx = i;
                }
            }
        }
    }

    // 프레임 교체
    (frames + frameIdx)->pageNumber = pageNumber;

    // 프레임 순서 갱신
    Update_Frame_Sequence(frames, frameIdx, frameCnt);

    // 교체된 프레임의 다음 사용주기 갱신
    period = 0;
    for(int i = (pageNumberIdx + 1); i < MAX_ADDR_CNT; i++)
    {
        period++;

        if(*(PageNumberList + i) == pageNumber)
        {
            (frames + frameIdx)->OptimalPeriod = period;

            break;
        }

        if((i + 1) == MAX_ADDR_CNT)
        {
            (frames + frameIdx)->OptimalPeriod = period + 1;
        }
    }

    // 교체된 프레임 외의 프레임들의 다음 사용주기 갱신
    for(int i = 0; i < frameCnt; i++)
    {
        if((i != frameIdx) && ((frames + i)->OptimalPeriod) > 0)
        {
            (frames + i)->OptimalPeriod -= 1;
        }
    }


    return frameIdx;
}


int Replacement_LRU(Frame *frames, int pageNumber, int replacedFrame, int frameCnt)
{
    int maxPeriod = 0;
    int minSequence = 0;
    int frameIdx = 0;


    // 프레임 개수가 최대 프레임 개수보다 적을 경우
    if(currentFrame < (frameCnt - 1))
    {
        currentFrame += 1;

        // 프레임 할당
        (frames + currentFrame)->pageNumber = pageNumber;

        // 프레임 순서 갱신
        (frames + currentFrame)->sequence = (currentFrame + 1);

        // 사용주기 초기 값 설정
        (frames + currentFrame)->LRUPeriod = 0;

        // 교체된 프레임 외의 프레임들의 사용주기 갱신
        for(int i = 0; i < currentFrame; i++)
        {
            (frames + i)->LRUPeriod += 1;
        }


        return currentFrame;
    }

    // 프레임이 이미 최대 프레임 개수만큼 존재하는 경우
    for(int i = 0; i < frameCnt; i++)
    {
        if(i == 0)
        {
            maxPeriod = (frames + i)->LRUPeriod;
            minSequence = (frames + i)->sequence;

            frameIdx = i;
        }
        else
        {
            // 프레임들의 사용주기를 비교하면서 교체할 프레임 탐색
            if((frames + i)->LRUPeriod > maxPeriod)
            {
                maxPeriod = (frames + i)->LRUPeriod;
                minSequence = (frames + i)->sequence;

                frameIdx = i;
            }
            else if((frames + i)->LRUPeriod == maxPeriod)
            {
                // 다음 사용주기가 동일한 프레임이 2개 이상 발견된 경우
                // 들어온 프레임 순서를 비교하여 교체할 프레임 탐색
                if((frames + i)->sequence < minSequence)
                {
                    maxPeriod = (frames + i)->LRUPeriod;
                    minSequence = (frames + i)->sequence;

                    frameIdx = i;
                }
            }
        }
    }

    // 프레임 교체
    (frames + frameIdx)->pageNumber = pageNumber;

    // 프레임 순서 갱신
    Update_Frame_Sequence(frames, frameIdx, frameCnt);

    // 프레임들의 사용주기 갱신
    for(int i = 0; i < frameCnt; i++)
    {
        if(i == frameIdx)
        {
            (frames + i)->LRUPeriod = 0;
        }
        else
        {
            (frames + i)->LRUPeriod += 1;
        }
    }
    

    return frameIdx;
}


int Replacement_Second_Chance(Frame *frames, int pageNumber, int replacedFrame, int frameCnt)
{
    int frameIdx = 0;


    // 프레임 개수가 최대 프레임 개수보다 적을 경우
    if(BackIdx < frameCnt)
    {
        // 페이지 할당
        (frames + BackIdx)->pageNumber = pageNumber;

        // 프레임 참조비트 초기 값 설정
        (frames + BackIdx)->SecondBit = 0;

        // 원형큐 유지하기 위해 인덱스 조정
        BackIdx += 1;


        return (BackIdx - 1);
    }

    // 프레임이 이미 최대 프레임 개수만큼 존재하는 경우
    while(1)
    {
        frameIdx = FrontIdx;

        // 프레임들의 참조비트를 확인하여 교체할 프레임 탐색 및 참조비트 갱신
        if((frames + frameIdx)->SecondBit == 0)
        {
            // 프레임 교체하는 경우

            // 프레임 교체
            (frames + frameIdx)->pageNumber = pageNumber;

            // 프레임 참조비트 초기 값 설정
            (frames + frameIdx)->SecondBit = 0;

            // 원형큐 유지하기 위해 인덱스 조정
            if(FrontIdx < (frameCnt - 1))
            {
                FrontIdx++;

                if(BackIdx == (frameCnt - 1))
                {
                    BackIdx = 0;
                }
                else
                {
                    BackIdx++;
                }
            }
            else
            {
                FrontIdx = 0;
            }

            break;
        }
        else if((frames + frameIdx)->SecondBit == 1)
        {
            // 프레임 참조비트 갱신하는 경우

            // 프에임 참조비트 갱신
            (frames + frameIdx)->SecondBit = 0;

            // 원형큐 유지하기 위해 인덱스 조정
            if(FrontIdx < (frameCnt - 1))
            {
                FrontIdx++;

                if(BackIdx == (frameCnt - 1))
                {
                    BackIdx = 0;
                }
                else
                {
                    BackIdx++;
                }
            }
            else
            {
                FrontIdx = 0;
            }
        }
    }


    return frameIdx;
}


int Update_Frame_Sequence(Frame *frames, int idx, int frameCnt)
{
    // 프레임 순서 갱신
    for(int i = 0; i < frameCnt; i++)
    {
        if(i == idx)
        {
            (frames + i)->sequence = frameCnt;
        }
        else
        {
            (frames + i)->sequence -= 1;
        }
    }


    return 0;
}


void Print_Output()
{
    int faultCnt = 0;

    char pageFaultHit = 0;
    
    char buffer[10] = "\0";


    printf("No.\t\tV.A.\t\tPage No.\t\tFrame No.\t\tP.A.\t\tPage Fault\n");

    for(int i = 0; i < MAX_ADDR_CNT; i++)
    {
        if(OutputFaultHit[i] == PAGE_FAULT)
        {
            faultCnt++;

            pageFaultHit = 'F';
        }
        else if(OutputFaultHit[i] == PAGE_HIT)
        {
            pageFaultHit = 'H';
        }

        printf("%d\t\t%d\t\t%d\t\t\t%d\t\t\t%d\t\t%c\n", (i + 1), VirtualAddr[i], PageNumberList[i], OutputFrameNumber[i], OutputPysicalAddr[i], pageFaultHit);
    }

    if(faultCnt >= 1000)
    {
        sprintf(buffer, "%d\n", faultCnt);

        printf("Total Number of Page Faults: %c,%s\n", buffer[0], &buffer[1]);
    }
    else
    {
        printf("Total Number of Page Faults: %d\n", faultCnt);
    }


    return;
}


int Save_Output(int replacementType)
{
    FILE *fp = NULL;

    int faultCnt = 0;

    int writeResult = 0;

    char pageFaultHit = 0;

    char fileName[20] = "output.";
    char buffer[256] = "\0";
    char faultStr[10] = "\0";


    if(replacementType == PAGE_REPLACEMENT_OPTIMAL)
    {
        strcat(fileName, "opt");
    }
    else if(replacementType == PAGE_REPLACEMENT_FIFO)
    {
        strcat(fileName, "fifo");
    }
    else if(replacementType == PAGE_REPLACEMENT_LRU)
    {
        strcat(fileName, "lru");
    }
    else if(replacementType == PAGE_REPLACEMENT_SECOND_CHANCE)
    {
        strcat(fileName, "sc");
    }

    fp = fopen(fileName, "w");
    if(fp == NULL)
    {
        printf("Output file open failed\n");

        return -1;
    }

    memset(buffer, 0, 256);
    sprintf(buffer, "%-15s%-15s%-15s%-15s%-15s%-15s\n", "No.", "V.A.", "Page No.", "Frame No.", "P.A.", "Page Fault");
    
    writeResult = fputs(buffer, fp);
    if(writeResult == EOF)
    {
        printf("Output file write failed\n");

        if(fp != NULL)
        {
            fclose(fp);
        }

        fp = NULL;

        return -1;
    }
    
    for(int i = 0; i < MAX_ADDR_CNT; i++)
    {
        if(OutputFaultHit[i] == PAGE_FAULT)
        {
            faultCnt++;

            pageFaultHit = 'F';
        }
        else if(OutputFaultHit[i] == PAGE_HIT)
        {
            pageFaultHit = 'H';
        }

        memset(buffer, 0, 256);
        sprintf(buffer, "%-15d%-15d%-15d%-15d%-15d%c\n", (i + 1), VirtualAddr[i], PageNumberList[i], OutputFrameNumber[i], OutputPysicalAddr[i], pageFaultHit);

        writeResult = fputs(buffer, fp);
        if(writeResult == EOF)
        {
            printf("Output file write failed\n");

            if(fp != NULL)
            {
                fclose(fp);
            }

            fp = NULL;

            return -1;
        }
    }

    memset(buffer, 0, 256);
    if(faultCnt >= 1000)
    {
        memset(faultStr, 0, 10);

        sprintf(faultStr, "%d", faultCnt);
        sprintf(buffer, "%s%c,%s", "Total Number of Page Faults: ", faultStr[0], &faultStr[1]);
    }
    else
    {
        sprintf(buffer, "%s%d", "Total Number of Page Faults: ", faultCnt);
    }

    writeResult = fputs(buffer, fp);
    if(writeResult == EOF)
    {
        printf("Output file write failed\n");

        if(fp != NULL)
        {
            fclose(fp);
        }

        fp = NULL;

        return -1;
    }

    if(fp != NULL)
    {
        fclose(fp);

        fp = NULL;
    }


    return 0;
}