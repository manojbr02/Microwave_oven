/*
 * File:   main.c
 * Author: manoj
 *
 * Created on 9 December, 2021, 9:35 PM
 */


#include <xc.h>
#include "clcd.h"
#include "main.h"
#include "matrix_keypad.h"
#include "timers.h"

#pragma config WDTE = OFF //watching timer enable bit

char sec=0,min=0,temp=0;
unsigned char operation_mode , reset_flag, conv_mode_op;

static void init_config(void) {
    //write your initalization here
    init_clcd();  //Intialization of clcd
    init_matrix_keypad(); //intailization of matrix keypad
    init_timer2(); // initailization of timer 2
    
    PEIE = 1;
    GIE = 1;
    
    FAN_DDR = 0; // making fan as a output
    FAN =OFF; //fan is off
    
    BUZZER_DDR =0; //output
    BUZZER = OFF;
    
    DOOR_DDR= 1; //input
}

void main(void) {
    unsigned char key;
    
    init_config();
    power_on_screen();
    clear_clcd();
    
    
    operation_mode= COOKING_MODE_DISPLAY;

    while (1) {
        //write your code here
        key= read_matrix_keypad(STATE);
        
        if((operation_mode == MICRO_MODE) || (operation_mode ==GRILL_MODE) || (operation_mode ==CONVECTION_MODE))
        {
            ;
        }
        
        else if(key==1 && operation_mode != TIME_DISPLAY) //if key1 is pressed
        {
            operation_mode= MICRO_MODE;
            reset_flag= MODE_RESET;
            clear_clcd();
            clcd_print("  Power = 900W  ", LINE2(0));
            __delay_ms(1500);
            clear_clcd();
        }
        
        else if(key ==2 && operation_mode != TIME_DISPLAY)
        {
            operation_mode =GRILL_MODE;
            reset_flag= MODE_RESET;
            clear_clcd();
        }
        
        else if(key ==3)
        {
            operation_mode =CONVECTION_MODE;
            conv_mode_op= SET_TEMP;
            reset_flag=RESET_TEMP;
            clear_clcd();
        }
        
        else if(key==4)
        {
            if(operation_mode== COOKING_MODE_DISPLAY)
            {
               min=0;
               sec=30;
               FAN=ON;
               TMR2ON=ON;
               clear_clcd();
               operation_mode= TIME_DISPLAY;
            }
            else if(operation_mode== TIME_DISPLAY)
            {
                //increment by 30 sec
                sec= sec + 30;
                if(sec>= 60)
                {
                    min++;
                    sec= sec-60;
                }
            }
            else if(operation_mode== PAUSE)
            {
                TMR2ON =ON;
                FAN =ON;
                operation_mode= RESUME;
            }
        }
        
        else if(key ==5) //pause the current task
        {
            operation_mode= PAUSE;
            
        }
        
        else if(key ==6) //terminate /stop the current task
        {
            operation_mode= STOP;
            clear_clcd();
            
        }
        
        
        switch(operation_mode)
        {
            case  COOKING_MODE_DISPLAY:
                cooking_mode_display();
                break;
                
            case MICRO_MODE:
                set_time(key);
                break;
                
            case GRILL_MODE:
                set_time(key);
                break;
                
            case CONVECTION_MODE:
                if(conv_mode_op== SET_TEMP)
                {
                    set_temp(key);
                    if(conv_mode_op== SET_TIME)
                    {
                        continue;
                    }
                }
                else if(conv_mode_op== SET_TIME)
                {
                    set_time(key);
                }
                break;
                
            case TIME_DISPLAY:
                time_display_screen();
                break;
                
            case STOP:
                TMR2ON =OFF;
                FAN =OFF;
                operation_mode= COOKING_MODE_DISPLAY;
                break;
                
            case PAUSE:
                TMR2ON =OFF;
                FAN =OFF;
                break;
                
            case RESUME:
                time_display_screen();
                break;
                
        }
        
        reset_flag=RESET_NOTHING;

    }

}

void power_on_screen(void)
{
    for(int i=0;i<16;i++)
    {
        clcd_putch(BLOCK, LINE1(i));
    }
    clcd_print("  Powering ON  ", LINE2(0));
    clcd_print(" Microwave Oven ", LINE3(0));
    for(int i=0;i<16;i++)
    {
        clcd_putch(BLOCK, LINE4(i));
    }
    
    __delay_ms(3000);
}

void clear_clcd(void)
{
    clcd_write(CLEAR_DISP_SCREEN, INST_MODE);
    __delay_us(100);
}

void cooking_mode_display(void)
{
    clcd_print("1.Micro", LINE1(0));
    clcd_print("2.Grill", LINE2(0));
    clcd_print("3.Convection", LINE3(0));
    clcd_print("4.Start", LINE4(0));
}

void set_time(unsigned char key)
{
    static int wait, blink, key_count, blink_pos;
    
    if(reset_flag==MODE_RESET)
    {
        wait=15;
        blink=0;
        key_count=0;
        blink_pos= 0;
        sec=0;
        min=0;
        key= ALL_RELEASED;
        clcd_print("SET TIME (MM:SS)", LINE1(0));
        clcd_print("TIME -", LINE2(0));
        clcd_print(":",LINE2(9));
        clcd_print("*:CLEAR  #:ENTER", LINE4(0));
    }
    
    if(key!='*' && key!='#' && key!=ALL_RELEASED)
    {
        key_count++;
        if(key_count <= 2)
        {
           sec = sec*10 + key; 
           blink_pos= 0;
        }
        else if(key_count > 2 && key_count <= 4)
        {
            min = min*10 +key;
            blink_pos= 1;
        }
        
    }
    if(key == '*')
    {
        if(key_count <= 2)
        {
            sec =0;
            key_count=0;
        }
        else if(key_count > 2 && key_count <= 4)
        {
            min= 0;
            key_count=2;
        }
    }
    if(key == '#')
    {
        clear_clcd();
        operation_mode= TIME_DISPLAY;
        FAN = ON; // fan is on
        TMR2ON = ON;
    }
    
    if(wait++ ==15)
    {
        wait=0;
        blink= !blink;
        //display sec  in numbers
        //sec is 0 1 2 .....59 assume sec is 12
        clcd_putch(sec/10 + '0', LINE2(10)); //'1'
        clcd_putch(sec%10 + '0', LINE2(11)); //'2'
        
        //to display min
        clcd_putch(min/10 + '0', LINE2(7)); //'1'
        clcd_putch(min%10 + '0', LINE2(8)); //'2'
    }
    
    if(blink)
    {
        switch(blink_pos)
        {
            case 0:
                clcd_print("  ", LINE2(10));
                break;
            case 1:
                clcd_print("  ", LINE2(7));
                break;
        }
        
    }    
    
}

void door_status_check(void)
{
    //door open event
    if(DOOR== OPEN)
    {
        //pause/stop the oven
        TMR2ON= OFF;
        FAN= OFF;
        BUZZER= ON;
        clear_clcd();
        clcd_print("DOOR STATUS:OPEN", LINE2(0));
        clcd_print("Please CLOSE ", LINE3(0));
        //wait till door is closed
        while(DOOR != OPEN) //when door does not closed
        {
            ;
        }
        TMR2ON= ON;
        FAN= ON;
        BUZZER= OFF;
        clear_clcd();
    }
}

void time_display_screen(void)
{
    door_status_check();
    
    clcd_print(" TIME =", LINE1(0));
    
    clcd_putch(sec/10 + '0', LINE1(12)); //'1'
    clcd_putch(sec%10 + '0', LINE1(13)); //'2'
        
    clcd_putch(min/10 + '0', LINE1(9)); //'1'
    clcd_putch(min%10 + '0', LINE1(10)); //'2'
    
    clcd_print(":",LINE1(11));
    
    clcd_print(" 4.Start/Resume",LINE2(0));
    clcd_print(" 5.Pause",LINE3(0));
    clcd_print(" 6.Stop",LINE4(0));
    
    if(sec == 0 && min == 0)
    {
        clear_clcd();
        clcd_print("   TIME UP !!", LINE2(0));
        TMR2ON = OFF;
        FAN = OFF;
        BUZZER = ON;
        __delay_ms(3000);
        BUZZER = OFF;
        clear_clcd();
        
        operation_mode= COOKING_MODE_DISPLAY;
    }
    
}

void set_temp(unsigned char key)
{
    static int wait ,blink ,key_count;
    static char temp_arr[4];
    if(reset_flag== RESET_TEMP)
    {
        key= ALL_RELEASED;
        temp=0;
        wait=15;
        blink=0;
        key_count=0;
        //display temp screen
        clcd_print(" SET TEMP. ( C)", LINE1(0));
        clcd_putch(DEGREE, LINE1(12));
        clcd_print("  TEMP = ", LINE2(0));
        clcd_print("*:CLEAR #:ENTER", LINE4(0));
    }
        
        //to read temp
      if(key!='*' && key!='#' && key!=ALL_RELEASED)
      {
        key_count++;
        if(key_count <= 3)
        {
           temp = temp*10 + key; 
        }
        
      }
        
        else if(key =='*')
        {
            temp= 0;
            key_count= 0;
        }
        
        else if(key =='#')
        {
            clear_clcd();
            clcd_print("  PRE-HEATING   ", LINE1(0));
            clcd_print(" TIME Remn.=", LINE3(0));
            
            // set pre heating time
            sec= 10;
            TMR2ON= ON;
            while(sec !=0)
            {
                clcd_putch(sec/100 + '0', LINE3(12)); //'1'
                clcd_putch((sec/10)%10 + '0', LINE3(13)); //'2'
                clcd_putch(sec%10 +'0', LINE3(14));
            }
            TMR2ON= OFF;
            clear_clcd();
            conv_mode_op = SET_TIME;
            reset_flag= MODE_RESET;
            
        }

        temp_arr[0]=(temp/100) + '0';
        temp_arr[1]=(temp/10) %10 + '0';
        temp_arr[2]= (temp%10) + '0';
        temp_arr[3]= '\0';
        
        if(wait++ ==15)
        {
           wait=0;
           blink= !blink;
           
           clcd_print(temp_arr, LINE2(9));
        }

       if(blink)
       {
           clcd_print("   ", LINE2(8));
       }
            
        
}