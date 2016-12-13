// QEDChannelDelay.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <stdio.h>
#include <MALLOC.H>
#include <MEMORY.H>

int getSteps(__int64 *steps)
{
	return 0;
}

int getStepsByBase(__int64 *steps, int base)
{
	return 0;
}

QKDChannelDelaytemp(int scanLen)
{
	__int64 *pStepsA, *pStepsB;
	__int64 diff;
	
	int n, nTemp;

	unsigned char *delay;

	int cntA, cntB;
	int cntBTemp;
	
	pStepsA = (__int64 *)malloc(10000*sizeof(__int64));
	pStepsB = (__int64 *)malloc(10000*sizeof(__int64));
	delay = (unsigned char *)malloc(scanLen);

	while(1)
	{
		diff = pStepsA[cntA] - pStepsB[cntB];
		if(diff < -scanLen) //B比A大时 A++  B计数
		{
			cntA++;
			cntB = cntBTemp;
			cntBTemp = 0;
		}else if (diff < scanLen)//范围内，统计
		{
			if (cntBTemp)//记入第一个范围内B
			{
				cntBTemp = cntB;
			}
			delay[diff+scanLen]++;
			cntB++;
		}else
		{
			cntB++;
		}
////////////////////////////////////////////////////////////
		if (cntA > n)
		{
			if (n < nTemp)
			{
				break;
			}
			n = getSteps(pStepsA);
			cntA = 0;
		}

		if (cntB > n)
		{
			if (n < nTemp)
			{
				break;
			}
			n = getStepsByBase(pStepsB, cntBTemp);
			cntB = 0;
			cntBTemp = 0;
		}
	}
}


struct SSteps
{
	__int64 *steps;
	int cnt;
};

void QKDChannelDelay(int scanLen, SSteps stepsA, SSteps stepsB, unsigned char* delay)
{
	__int64 diff;
	__int64 *pStepsA, *pStepsB;
	int cntA = 0, cntB = 0;	
	int cntBTemp = 0;
	bool firstOneInRang = true;
	
	pStepsA = stepsA.steps;
	pStepsB = stepsB.steps;

	while(1)
	{
		diff = pStepsA[cntA] - pStepsB[cntB];
		if(diff < -scanLen) //B比A大时 A++  B计数
		{
			cntA++;
			if (!firstOneInRang)
			{
				cntB = cntBTemp;
				firstOneInRang = true;
			}
		}else if (diff < scanLen)//范围内，统计
		{
			if (firstOneInRang)//记入第一个范围内B
			{
				cntBTemp = cntB;
				firstOneInRang = false;
			}
			delay[diff+scanLen]++;
			cntB++;
		}else
		{
			cntB++;
		}

		if (cntA > stepsA.cnt)
		{
			break;
		}

		if (cntB > stepsB.cnt)
		{
			break;
		}
	}
}

int timeToStep(__int64* pTimeA, int cnt, int rang, __int64* pSteps, int offset)
{
	int i;
	for (i = 0; i < cnt; i++)
	{
		pSteps[i] = (pTimeA[i] + offset)/rang;
	}
	return 0;
}

int main(int argc, char* argv[])
{
	FILE *fpA, *fpB;
	FILE* fp;

	char buf[9];

	__int64 *pTimeA[4];
	__int64 *pTimeB[4];

	SSteps stepsA,stepsB;

	unsigned char* delay;
//	unsigned char delay[200];

	int scanLen = 100000;
	delay = (unsigned char*)malloc(2*scanLen);


	for(int i = 0; i < 4; i++)
	{
		pTimeA[i] = (__int64*)malloc(1024*1024*8);
		pTimeB[i] = (__int64*)malloc(1024*1024*8);
	}
	int cntA[4] ={0};
	int cntB[4] ={0};
	char path[100];
	for (i=0; i<4; i++)
	{
		sprintf(path,"cali_recv%d.dat",i);
		fpA = fopen(path,"rb");
		while(fread(buf, 1, 8, fpA))
		{
			pTimeA[i][cntA[i]]=*(__int64*)buf;
			cntA[i]++;
		}
		fclose(fpA);
	}


	for (i=0; i<4; i++)
	{
		sprintf(path,"cali_send%d.dat",i);
		fpB = fopen(path,"rb");
		while(fread(buf, 1, 8, fpB))
		{
			pTimeB[i][cntB[i]]=*(__int64*)buf;
			cntB[i]++;
		}
		fclose(fpB);
	}
	stepsA.steps = (__int64*)malloc(1024*1024*8);
	stepsB.steps = (__int64*)malloc(1024*1024*8);
	fp = fopen("delay.csv","w");
	for (int l =-20; l<20;l++)
	{
		for (i = 0; i < 4; i++)
		{
			timeToStep(pTimeA[i], cntA[i], 1000, stepsA.steps, 100*l);
			stepsA.cnt = cntA[i];
			for(int j = 0; j < 4; j++)
			{	
				memset(delay,0,scanLen*2);
				timeToStep(pTimeB[j], cntB[j], 1000, stepsB.steps, 0);
				stepsB.cnt = cntB[j];
				QKDChannelDelay(scanLen, stepsA,stepsB,delay);
				int a=0,b;
				for(int k = 0; k < 2*scanLen; k++)
				{
					if (delay[k]>a)
					{
						a=delay[k];
						b=k;
					}
					//fprintf(fp, "%d,%d",a,k);
				}
				fprintf(fp, "%d,%d,",a,b);
				//fprintf(fp,"\n");
			}
		}
		fprintf(fp,"\n");
	}
	fclose(fp);


	return 0;
}




// 
// 	while(fread(buf, 1, 9, fpA))
// 	{
// 		switch(buf[1])
// 		{
// 			case 0x01:
// 				pTimeA[0][cntA[0]] = *(__int64*)(buf+1);
// 				cntA[0]++;
// 				break;
// 			case 0x02:
// 				pTimeA[1][cntA[1]] = *(__int64*)(buf+1);
// 				cntA[1]++;
// 				break;
// 			case 0x03:
// 				pTimeA[2][cntA[2]] = *(__int64*)(buf+1);
// 				cntA[2]++;
// 				break;
// 			case 0x04:
// 				pTimeA[3][cntA[3]] = *(__int64*)(buf+1);
// 				cntA[3]++;
// 				break;
// 		}
// 	}
// 
// 	fclose(fpA);
// 	fpB = fopen("B.dat","rb");
// 
// 	while(fread(buf, 1, 9, fpB))
// 	{
// 		switch(buf[1])
// 		{
// 			case 0x01:
// 				pTimeB[0][cntB[0]] = *(__int64*)(buf+1);
// 				cntB[0]++;
// 				break;
// 			case 0x02:
// 				pTimeB[1][cntB[1]] = *(__int64*)(buf+1);
// 				cntB[1]++;
// 				break;
// 			case 0x03:
// 				pTimeB[2][cntB[2]] = *(__int64*)(buf+1);
// 				cntB[2]++;
// 				break;
// 			case 0x04:
// 				pTimeB[3][cntB[3]] = *(__int64*)(buf+1);
// 				cntB[3]++;
// 				break;
// 		}
// 	}