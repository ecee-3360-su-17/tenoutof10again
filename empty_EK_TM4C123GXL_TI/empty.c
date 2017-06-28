/* Copyright (c) 2015, Texas Instruments Incorporated
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * *  Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * *  Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * *  Neither the name of Texas Instruments Incorporated nor the names of
 *    its contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/*
 *  ======== empty.c ========
 */
/* XDCtools Header files */
#include <xdc/std.h>
#include <xdc/runtime/System.h>

/* BIOS Header files */
#include <ti/sysbios/BIOS.h>
#include <ti/sysbios/knl/Task.h>

/* TI-RTOS Header files */
#include <ti/drivers/GPIO.h>
// #include <ti/drivers/I2C.h>
// #include <ti/drivers/SDSPI.h>
// #include <ti/drivers/SPI.h>
 #include <ti/drivers/UART.h>
// #include <ti/drivers/Watchdog.h>
// #include <ti/drivers/WiFi.h>

/* BIOS Header files */
#include <ti/sysbios/knl/Mailbox.h>

#define NUMMSGS         3      // Number of messages
#define BUFFSIZE        6
/* Board Header file */
#include "Board.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define TASKSTACKSIZE     768
#define TASKSTACKSIZE1     768

Task_Struct task0Struct;
Char task0Stack[TASKSTACKSIZE];
Task_Struct task1Struct;
Char task1Stack[TASKSTACKSIZE];

/*
 *  ======== Mailbox MsgObj ========
 *  Task for this function is created statically. See the project's .cfg file.
 */

typedef struct MsgObj {
    int        val;            // Message value
} MsgObj, *Msg;

/*Void senderFxn(UArg arg0, UArg arg1)
{
    Mailbox_Handle mbxHandle = (Mailbox_Handle)arg0;
    int count = 0;
    while (1) {
        MsgObj msg;
        msg.val = count;
        Mailbox_post(mbxHandle, &msg, BIOS_WAIT_FOREVER);
        count++;
    }
}
Void reciverFxn(UArg arg0, UArg arg1)
{
    Mailbox_Handle mbxHandle = (Mailbox_Handle)arg0;
    while (1) {
      MsgObj msg;
      Mailbox_pend(mbxHandle, &msg, BIOS_WAIT_FOREVER);
      Task_sleep((UInt)arg0);
      GPIO_toggle(Board_LED1);
      if(msg.val % 10 == 0)
          GPIO_toggle(Board_LED2);
    }
}
*/
/*
 *  ======== echoFxn ========
 *  Task for this function is created statically. See the project's .cfg file.
 */
Void echoFxn(UArg arg0, UArg arg1)
{
    Mailbox_Handle mbxHandle = (Mailbox_Handle)arg0;

    char input;
    UART_Handle uart;
    UART_Params uartParams;
    const char echoPrompt[] = "\fEchoing characters:\r\n";

    /* Create a UART with data processing off. */
    UART_Params_init(&uartParams);
    uartParams.writeDataMode = UART_DATA_BINARY;
    uartParams.readDataMode = UART_DATA_BINARY;
    uartParams.readReturnMode = UART_RETURN_FULL;
    uartParams.readEcho = UART_ECHO_OFF;
    uartParams.baudRate = 9600;
    uart = UART_open(Board_UART0, &uartParams);

    if (uart == NULL) {
        System_abort("Error opening the UART");
    }

   UART_write(uart, echoPrompt, sizeof(echoPrompt));
   int buff_count=0;
   int num = 0;
   char *buff = (char*)malloc(sizeof(char)*BUFFSIZE);



   /* Loop forever echoing */
    while (1) {
        UART_read(uart, &input, 1);

     if (input != '\0'){

        if (buff_count>=BUFFSIZE){

            free(buff);
            char *buff = (char*)malloc(sizeof(char)*BUFFSIZE);
            buff_count = 0;
         }
        if (input == '\r'){
            if (strcmp(buff ,"off") == 0)
                   num = 0;
            if (strcmp(buff ,"on") == 0)
                    num = 1;
            if (strcmp(buff ,"blue") == 0)
                    num = 2;
            if (strcmp(buff ,"green") == 0)
                    num = 3;
            if (strcmp(buff,"red") == 0)
                    num = 4;

            free(buff);
            char *buff = (char*)malloc(sizeof(char)*BUFFSIZE);
            buff_count = 0;
            }
        else{
          buff[buff_count] = input;
          buff_count++;
        }
     }


            MsgObj msg;
            msg.val = num;
            Mailbox_post(mbxHandle, &msg, BIOS_WAIT_FOREVER);
            UART_write(uart, &input, 1);



    }
}


/*
 *  ======== heartBeatFxn ========
 *  Toggle the Board_LED0. The Task_sleep is determined by arg0 which
 *  is configured for the heartBeat Task instance.
 */
Void heartBeatFxn(UArg arg0, UArg arg1)
{
    Mailbox_Handle mbxHandle = (Mailbox_Handle)arg0;
      while(1){
          MsgObj msg;
          Mailbox_pend(mbxHandle, &msg, BIOS_WAIT_FOREVER);
                 switch(msg.val){
                         case 0://turn off all LEDs

                             GPIO_write(Board_LED0|Board_LED1|Board_LED2, 0);
                             break;
                         case 1://turn on all LEDs

                             GPIO_write(Board_LED0|Board_LED1|Board_LED2, 1);
                             break;
                         case 2://turn on blue LED
                             GPIO_toggle(Board_LED0);

                             break;
                         case 3://turn on green LED
                             GPIO_toggle(Board_LED1);;

                             break;
                         case 4://turn on red LED
                             GPIO_toggle(Board_LED2);
                             break;
                         default://turn off all LEDs
                             GPIO_toggle(Board_LED2);
                             //GPIO_write(Board_LED0|Board_LED1|Board_LED2, 0);
                             break;
                 }
          }
}

/*
 *  ======== main ========
 */
int main(void)
{
    Task_Params taskParams;

    /* Call board init functions */
    Board_initGeneral();
    Board_initGPIO();
    // Board_initI2C();
    // Board_initSDSPI();
    // Board_initSPI();
    Board_initUART();
    // Board_initUSB(Board_USBDEVICE);
    // Board_initWatchdog();
    // Board_initWiFi();

    Mailbox_Struct mbxStruct;

    //Construct a Mailbox Instance
    Mailbox_Params mbxParams;
    Mailbox_Params_init(&mbxParams);
    Mailbox_construct(&mbxStruct,sizeof(MsgObj), 2, &mbxParams, NULL);

    Mailbox_Handle mbxHandle;
    mbxHandle = Mailbox_handle(&mbxStruct);

    Task_Params taskParams1;
    Task_Params_init(&taskParams1);
    taskParams1.stackSize = TASKSTACKSIZE1;
    taskParams1.arg0 = (UArg)mbxHandle;
    taskParams1.stack = &task1Stack;
    taskParams1.instance->name = "echo";
    Task_construct(&task1Struct, (Task_FuncPtr)echoFxn, &taskParams1, NULL);

    /* Construct heartBeat Task  thread */
    Task_Params_init(&taskParams);
    taskParams.arg0 = (UArg)mbxHandle;
    taskParams.stackSize = TASKSTACKSIZE;
    taskParams.stack = &task0Stack;
    Task_construct(&task0Struct, (Task_FuncPtr)heartBeatFxn, &taskParams, NULL);



    /* Turn on user LED */
    //GPIO_write(Board_LED0, Board_LED_ON);


   System_printf("Starting the UART Echo example\nSystem provider is set to "
                     "SysMin. Halt the target to view any SysMin contents in "
                     "ROV.\n");

    /* SysMin will only print to the console when you call flush or exit */
    System_flush();

    /* Start BIOS */
    BIOS_start();

    return (0);
}
