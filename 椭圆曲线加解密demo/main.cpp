// 平方剩余.cpp : Defines the entry point for the console application.
//

//**********************************************************************
/*
	有限域F(p)上椭圆曲线Ep(a,b)(y^2 = x*x*x + a*x + b mod p)的加解密demo;
基本流程：
	1，alice 选定椭圆曲线的相关参数 p,a,b，选择生成元G，以及随机数k，计算生成元G的阶数n；公布参数 PK = (p,a,b,G,n,kG)；
	2，bob 获取alice公布的参数PK，生成随机数r，将要传输的消息编码成椭圆上的点得横坐标，计算纵坐标后得到椭圆上的点M
		计算 C1 = M + rkG; C2 = rG;将加密后的C1，C2发送给alice；
	3，alice接收到C1，C2开始解密运算得到原消息 C1 - kC2 = M + rkG - krG = M  ；
易出错点:
	1，对点进行数乘运算中间结果会出现“零点”，而一个点和它的负点相加应为“零点”，而这种运算不符合计算公式，需要特殊判断和处理；
	2，定义（0，0）为“零点”，（0,0）不会在Ep(a,b)上，只要 b mod p != 0;
补充信息：
	椭圆上的加法和倍数运算计算公式 R = P + Q, P = (x1,y1),Q = (x2,y2),R = (x3,y3);
		x3 = s^2 - x1 - x2 mod p
		y3 = s*(x1 - x3) - y1 mod p
	这里
		s = (y2 - y1)/(x2 - x1) mod p;如果 P != Q;(横坐标相等时并不适用，横坐标相等时置为“零点”)
		s = (3*(x1)^2 + a)/(2*y1) mod p;如果 P = Q;

*/
//**********************************************************************
#include "stdafx.h"
#include <stdlib.h>
#include <time.h>
#include <string.h>

void* ecc(int a,int b,int p,int &count);
void pointOrder(int &n, int x0, int y0,int a, int p);
void pointAddMtiple(int &x,int &y,int x0,int y0,int m,int a, int p);
void pointAdd(int &x,int &y,int x1,int y1,int x2,int y2,int a, int p);
void modeInverse(int &val,int x, int p);
bool judge(int a, int b, int p);
int main(int argc, char* argv[])
{
	int x = 0, y = 0,x0 = 0, y0 = 0,n = 0;
	int a = 1,b = 1,p = 23;
	int k,r,t;
	int xM,yM,xK,yK,xC1,yC1,xC2,yC2,xTemp,yTemp,xN,yN;
	int *element = NULL,eleCount = 0;
	printf("Hello World!\n");
	if(!judge(a,b,p))
	{
		printf("Ep(a,b) invalid!\n");
		return 0;
	}
	element = (int *)ecc(a,b,p,eleCount);
	if(!element)
	{
		printf("error,memory error\n");
		return 0;
	}
	//getchar();
	x = x0 = element[0];
	y = y0 = element[1];
	for(int i = 0; i < 30; i++)
	{
		printf("%d(%d %d) = ",i+2,x0,y0);
		pointAdd(x,y,x,y,x0,y0,a,p);
		if((x == x0) && ((y + y0) % p == 0))
		{
			n = i+3;
			printf("生成了生成元的负元素，生成元的阶数为 %d\n",n);
			printf("无法计算无穷远点-零点\n",n);
			break;
		}
	}
	srand(time(NULL));
	for(int j = 0; j < eleCount; j++)//每个生成元都测试一遍，消息随机选择椭圆上的点进行加解密；
	{
		x0 = element[2*j];
		y0 = element[2*j+1];
		pointOrder(n,x0,y0,a,p);
		printf("finished one step: the order of G = (%d,%d) is %d\n",x0,y0,n);
		k = rand() % n;
		r = rand() % n;
		//k = 13,r = 23;
		//k = 17,r = 0;
		//alice give K=kG;
		pointAddMtiple(xK,yK,x0,y0,k,a,p);
		printf("finished one step: K = kG = (%d,%d)\n",xK,yK);
		//bob encrypt
		t = rand() % eleCount;
		xM = element[2*t];
		yM = element[2*t+1];
		printf("choose k = %d, r = %d, t = %d, G = (%d,%d), M = (%d,%d)\n",k,r,t,x0,y0,xM,yM);
		pointAddMtiple(xTemp,yTemp,xK,yK,r,a,p);
		printf("finished one step: rK = rkG = (%d,%d)\n",xTemp,yTemp);
		pointAdd(xC1,yC1,xM,yM,xTemp,yTemp,a,p);
		printf("finished one step: C1 = M + rK = (%d,%d)\n",xC1,yC1);
		pointAddMtiple(xC2,yC2,x0,y0,r,a,p);
		printf("finished one step: C2 = rG = (%d,%d)\n",xC2,yC2);
		//alice decrypt
		pointAddMtiple(xTemp,yTemp,xC2,yC2,k,a,p);
		yTemp = (p - yTemp) % p;
		printf("finished one step: -kC2 = -krG = (%d,%d)\n",xTemp,xTemp);
		pointAdd(xN,yN,xC1,yC1,xTemp,yTemp,a,p);
		printf("finished one step: N = C1 - kC2 = M + rkG -krG = M = (%d,%d)\n",xN,yN);
		printf("compare now :(%d,%d) = (%d %d)\n",xM,yM,xN,yN);
		if((xM == xN) && (yM == yN))
		{
			printf("encrypt decrypt successfully (k,r,t) = (%d,%d,%d)\n",k,r,t);
		}else
		{
			printf("encrypt decrypt failed (k,r,t) = (%d,%d,%d)\n",k,r,t);
		}
	}
	
	//getchar();
	return 0;
}
///*************************************************************************//
/*
(0,0)不在椭圆Ep(a,b)上当 b mod p != 0，这时引入为椭圆的“零点”（或者无穷远点）
*/
///*************************************************************************//

//输出有限域F(p)上的椭圆Ep(a,b)上的所有点；Ep(a,b)：y^2 = x^3 + a*x + b mod p
void* ecc(int a,int b,int p,int &count)
{
	printf("Ep(a,b)：y^2 = x^3 + %d*x + %d mod %d\n",a,b,p);
	int ry = 0, rx = 0, x = 0, y = 0;
	int *outElement = NULL,*newMem = NULL,memLen = 1024;
	outElement = (int*) malloc(memLen);
	count = 0;
	if(!outElement)
	{
		printf("error,memory error\n");
		return NULL;
	}
	for(int i = 0; i < p; i++)
	{
		for(int j = 0; j < p; j++)
		{
			x = i, y = j;
			ry = (y*y) % p;
			rx = (x*x*x + a*x + b) % p;
			if(rx == ry)
			{
				printf("%4d: (x y) = (%8d,%8d) \n",count+1,x,y);
				if(count + 2 > memLen)
				{
					//内存倍增
					memLen *= 2;
					newMem = (int*)malloc(memLen);
					if(!newMem)
					{
						free(outElement);
						return NULL;
					}else
					{
						memmove(newMem,outElement,memLen/2);
						free(outElement);
						outElement = newMem;
					}

				}
				outElement[2*count] = x;
				outElement[2*count+1] = y;
				count++;

			}

		}
	}
	printf("\n");
	return outElement;
}

//有限域F(p)上的椭圆Ep(a,b)上计算生成元的阶数
void pointOrder(int &n, int x0, int y0,int a, int p)
{
	int x = x0, y = y0;
	for(int i = 2; ; i++)
	{
		pointAdd(x,y,x,y,x0,y0,a,p);
		if((x == x0) && ((y + y0) % p == 0))
		{
			n = i + 1;
			break;
		}

	}
	printf("the order of (%d %d):%d\n",x0,y0,n);
	return;
}





//有限域F(p)上的椭圆Ep(a,b)上做点加 (x y) = m(x0 y0),m > 0
void pointAddMtiple(int &x,int &y,int x0,int y0,int m,int a, int p)
{
	if(0 > m)
	{
		printf("error,invalid param\n");
		return;
	}else if(0 == m)
	{
		x = 0;
		y = 0;
		printf("waring,0G = o\n");
		return;
	}
	x = x0;
	y = y0;
	for(int i = 1; i < m; i++)
	{
		pointAdd(x,y,x,y,x0,y0,a,p);
	}
	printf("%d(%d,%d) = (%d %d)\n",m,x0,y0,x,y);
	return;
}

//有限域F(p)上的椭圆Ep(a,b)上做点加 (x y) = (x1 y1) + (x2 y2)
void pointAdd(int &x,int &y,int x1,int y1,int x2,int y2,int a, int p)
{
	int s = 0,temp = 0;
	if((x1 == 0)&&(y1 == 0))
	{
		x = x2;
		y = y2;
		return;
	}
	if((x2 == 0)&&(y2 == 0))
	{
		x = x1;
		y = y1;
		return;
	}
	if((x1 == x2)&&( (y1 + y2) % p == 0))
	{
		x = 0;
		y = 0;
		return;
	}

	if(x1 == x2)
	{
		modeInverse(temp,2*y1,p);
		s = (3*x1*x1 + a)*temp % p;
	}
	else
	{
		modeInverse(temp,x2-x1,p);
		s = (y2 - y1)*temp % p;
	}
	x = (s*s -x1 -x2) % p;
	x = (x < 0)?(x+p):x;
	y = (s*(x1 - x) - y1) % p;
	y = (y < 0)?(y+p):y;
	printf("(%d,%d) = (%d,%d) + (%d,%d) \n",x,y,x1,y1,x2,y2);
}

// 计算模反元素 val = x^(-1) mod p, val @ [0,p);
void modeInverse(int &val,int x, int p)
{
	int r1 = x, r2 = p,temp,q, u1 = 0, u2 = 0;
	r1 = x % p;
	r1 = (r1 < 0)?(r1+p):r1;
	u1 = 1;
	if(1 == r1)
	{
		val = u1 % p;
		val = (val < 0)? (val + p):val;
		return;
	}
	r2 = p % r1;
	q = p / r1;
	u2 = -q;
	if(1 == r2)
	{
		val = u2 %p;
		val = (val < 0)? (val + p):val;
		return;
	}
	while((1 != r2) && (0 != r2))
	{
		temp = r1 % r2;
		q = r1 / r2;
		r1 = r2;
		r2 = temp;
		temp = u1 - q*u2;
		u1 = u2;
		u2 = temp;
	}
	if(1 == r2)
	{
		val = u2 % p;
		val = (val < 0)? (val + p):val;
	}else
	{
		printf("error, invalid param (x,p) = (%d,%d)\n",x,p);
	}
	return;
}


bool judge(int a, int b, int p)
{
	return ((4*a*a*a + 27*b*b) % p);
}