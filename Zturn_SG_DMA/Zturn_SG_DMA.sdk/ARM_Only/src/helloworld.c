/******************************************************************************
*
* Copyright (C) 2009 - 2014 Xilinx, Inc.  All rights reserved.
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in
* all copies or substantial portions of the Software.
*
* Use of the Software is limited solely to applications:
* (a) running on a Xilinx device, or
* (b) that interact with a Xilinx device through a bus or interconnect.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
* XILINX  BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
* WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
* OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
*
* Except as contained in this notice, the name of the Xilinx shall not be used
* in advertising or otherwise to promote the sale, use or other dealings in
* this Software without prior written authorization from Xilinx.
*
******************************************************************************/

/*
 * helloworld.c: simple test application
 *
 * This application configures UART 16550 to baud rate 9600.
 * PS7 UART (Zynq) is not initialized by this application, since
 * bootrom/bsp configures it to baud rate 115200
 *
 * ------------------------------------------------
 * | UART TYPE   BAUD RATE                        |
 * ------------------------------------------------
 *   uartns550   9600
 *   uartlite    Configurable only in HW design
 *   ps7_uart    115200 (configured by bootrom/bsp)
 */

#include <stdio.h>
#include "platform.h"
#include "xil_printf.h"

//#include <stdio.h>
//#include <stdlib.h>
//#include <math.h>
//#include "pbPlots.h"
//#include "supportLib.h"
#include "data_10k.h"

float X[14];
float zt[14]={0,0,0,0,0,0,0,0,0,0,0,0,0,0};
float xt_old[14]={0,0,0,0,0,0,0,0,0,0,0,0,0,0};
float temp[14]={0,0,0,0,0,0,0,0,0,0,0,0,0,0};
float yztT[14]={0,0,0,0,0,0,0,0,0,0,0,0,0,0};
float wxxT[14]={0,0,0,0,0,0,0,0,0,0,0,0,0,0};

//initialization
float W[14] ={0.12573022, 0.13210486, 0.64042265, 0.10490012, 0.53566937, 0.36159505, 1.30400005, 0.94708096, 0.70373524, 1.26542147, 0.62327446, 0.04132598, 2.32503077, 0.21879166};
// float W[14];
float M = 1;

float lr=100000;
float eta=2e-4;
float iteration = 1e5;

float ybar;
float wzt;
float Reg;
float eta_;
float wx ;
float y_;
float y_yT;
float yTy_ ;

int DLU(){


	int i,j;
	int inst=0;
	int rows=10;
	int sr=0;
	//if(inst==0){

		for(i=sr;i<rows;i++){
			//ap_uint<32> t= ap_uint<32> (inStream.read().data);
			for(j=0;j<14;j++){
				X[j]=Xt[i][j];
			}
		//stage1
			Reg = lr / (lr+i);
			eta_ = Reg * eta;
		//stage2
			int c;
			//zt= Xt + xt_old
			for(c=0; c<14; c++){
			//#pragma HLS UNROLL
				//zt[c] = X[c];
				zt[c] =X[c]+xt_old[c];
				xt_old[c]=X[c];
			}
		//stage3
			//wzt = W[14:1] @ zt[1:14] {MAC}
			for(c=0;c<14;c++){
			//#pragma HLS UNROLL
				temp[c]=W[c] * zt[c];
			}
			wzt= temp[0]+temp[1]+temp[2]+temp[3]+
			temp[4]+temp[5]+temp[6]+temp[7]+temp[8]+temp[9]
			+temp[10]+temp[11]+temp[12]+temp[13];
			//ybar = M @ wzt
			ybar = wzt * M;
			//yztT = ybar @ ztT[1:14]{zt transpose)
			for(c=0;c<14;c++){
			//#pragma HLS UNROLL
				yztT[c]= ybar * zt[c];
			}
			//wx = W[1:14] @ xt [14:1]  {MAC, in this section X=xt_old}
			for(c=0;c<14;c++){
			//#pragma HLS UNROLL
				temp[c]=W[c] * xt_old[c];
			}
			wx= temp[0]+temp[1]+temp[2]+temp[3]+
			temp[4]+temp[5]+temp[6]+temp[7]+temp[8]+temp[9]
			+temp[10]+temp[11]+temp[12]+temp[13];
			//wxxT = wx @ xT
			for(c=0;c<14;c++){
			//#pragma HLS UNROLL
				wxxT[c]= wx * X[c];
			}
			//W = W + eta_ * (yztT - wxxT)

			for(c=0;c<14;c++){
			//#pragma HLS UNROLL
				W[c]= W[c] + eta_ * (yztT[c] - wxxT[c]);
			}
		//stage4
			// M = M/1-eta_
			M = M / (1-eta_);
			// y_ = M @ ybar
			y_ = M * ybar;
			//y_yT = y_ @ yT {yT is ybar transpose}
			y_yT = y_ * y_;
			//yTy_ = ybar @ y_
			yTy_ = ybar * y_;
			// M = M - ( eta_ / 1 + eta_ yTy_ ) * y_yT
			M = M - ((eta_ / (1 + (eta_ * yTy_))) * y_yT);
		}
		AxiTimerHelper();
		startTimer();
		printf("HW test finished\n");
		stopTimer();
		double timeHW= getElapsedTimerInSeconds();
		printf("HW test finished in %f useconds\n",timeHW*1000000);

	//}

	return 0;
}



int main()
{
    init_platform();

    //print("Hello World\n\r");
    DLU();
    cleanup_platform();
    return 0;
}
