/*
 * Name: kowsik
 * Date: 14/02/25
 * Desc: Program for Car Black Box using PIC16F877A
 */

#include "main.h"

#pragma config WDTE = OFF

extern unsigned char return_time;
unsigned char sw_press = NO_PRESS;

void init_config(void)
{
    // to initialize uart with 9600 baud rate
    init_uart(9600);
    // to initialize i2c to access RTC and external EEPROM
    init_i2c(100000);
    // to initialize RTC
    init_ds1307();
    // to initialize ADC
    init_adc();
    // to initialize digital keypad
    init_digital_keypad();
    // to initialize clcd
    init_clcd();
    // to initialize timer 2
    init_timer2();
    // to enable gllobal interrupt 
    GIE = 1;
    // to enable external peripheral interupt
    PEIE = 1;
}

void main(void)
{
    init_config();
    
    unsigned char key, pre_key = ALL_RELEASED;
    unsigned char delay = 0;
    unsigned char control_flag = DASHBOARD_SCREEN;
    unsigned char reset_flag;
    char event[3] = "ON";
    unsigned char speed = 0;
    char *gear[] = {"GN", "GR", "G1", "G2", "G3", "G4"};
    unsigned char gr = 0;
    unsigned char menu_pos = 0, view = 0, clear = 0, chang_pass = 0, time = 0;
    
    log_event(event, speed);
    ext_eeprom_24C02_str_write(0x00, "1010");
    
    while(1)
    {
        speed = read_adc() / 10.3;
        
        if(control_flag == DASHBOARD_SCREEN || control_flag == LOGIN_SCREEN || control_flag == CHANG_PASS_SCREEN)
        {
            key = read_digital_keypad(STATE);
            for(unsigned int i = 300; i--;);
        }
        else
        {
            key = read_digital_keypad(LEVEL);
            for(unsigned int i = 20000; i--;);
        }
        
        if(key == SW1)
        {
            strcpy(event, "CO");
            log_event(event, speed);
        }
        else if(key == SW2 && gr < 6)
        {
            strcpy(event, gear[gr]);
            gr++;
            log_event(event, speed);
        }
        else if(key == SW3 && gr > 0)
        {
            --gr;
            strcpy(event, gear[gr]);
            log_event(event, speed);
        }
        else if(control_flag == DASHBOARD_SCREEN && (key == SW4 || key == SW5))
        {
            control_flag = LOGIN_SCREEN;
            clear_screen();
            clcd_print("Enter Password", LINE1(1));
            clcd_write(LINE2(4), INST_MODE);
            __delay_us(100);
            clcd_write(DISP_ON_AND_CURSOR_ON, INST_MODE);
            __delay_us(100);
            reset_flag = RESET_PASSWORD;
            TMR2ON = 1;
        }
        else if(control_flag == MAIN_MENU_SCREEN && sw_press == LONG_PRESS && key == SW4)
        {
            switch(menu_pos)
            {
                case 0:
                    control_flag = VIEW_LOG_SCREEN;
                    reset_flag = RESET_VIEW;
                    clear_screen();
                    break;
                case 1:
                    control_flag = CLEAR_LOG_SCREEN;
                    reset_flag = RESET_CLEAR;
                    clear_screen();
                    break;
                case 2:
                    control_flag = DOWNLOAD_LOG_SCREEN;
                    clear_screen();
                    break;
                case 3:
                    control_flag = SET_TIME_SCREEN;
                    reset_flag = RESET_TIME;
                    clear_screen();
                    break;
                case 4:
                    control_flag = CHANG_PASS_SCREEN;
                    reset_flag = RESET_PASSWORD;
                    clear_screen();
                    break;
            }
            TMR2ON = 0;
            delay = 0;
            key = ALL_RELEASED;
            pre_key = ALL_RELEASED;
            sw_press = NO_PRESS;
        }
        else if(control_flag == MAIN_MENU_SCREEN && sw_press == LONG_PRESS && key == SW5)
        {
            clear_screen();
            control_flag = DASHBOARD_SCREEN;
            delay = 0;
            key = ALL_RELEASED;
            pre_key = ALL_RELEASED;
            sw_press = NO_PRESS;
        }
        
        switch(control_flag)
        {
            case DASHBOARD_SCREEN:
                display_dashboard(event, speed);
                break;
                
            case LOGIN_SCREEN:
                switch(login(key, reset_flag))
                {
                    case RETURN_BACK_DASH:
                        control_flag = DASHBOARD_SCREEN;
                        clear_screen();
                        clcd_write(DISP_ON_AND_CURSOR_OFF, INST_MODE);
                        __delay_us(100);
                        TMR2ON = 0;
                        break;
                    case LOGIN_SUCCESS:
                        control_flag = MAIN_MENU_SCREEN;
                        clear_screen();
                        reset_flag = RESET_MENU;
                        continue;
                }
                break;
                
            case MAIN_MENU_SCREEN:
                if(key == SW4)
                {
                    pre_key = key;
                    if(++delay >= 10)
                    {
                        sw_press = LONG_PRESS;
                    }
                }
                else if(key == SW5)
                {
                    pre_key = key;
                    if(++delay >= 10)
                    {
                        sw_press = LONG_PRESS;
                    }
                }
                else if(delay > 0 && delay < 10)
                {
                    sw_press = SHORT_PRESS;
                    key = pre_key;
                    pre_key = ALL_RELEASED;
                    delay = 0;
                }
                else
                {
                    delay = 0;
                }
                
                if(sw_press == SHORT_PRESS)
                {
                    menu_pos = menu_screen(key, reset_flag);
                    sw_press = NO_PRESS;
                }
                else if(sw_press == NO_PRESS)
                {
                    key = ALL_RELEASED;
                    menu_pos = menu_screen(key, reset_flag);
                }
                
                if(menu_pos == RETURN_BACK_DASH)
                {
                    menu_pos = 0;
                    control_flag = DASHBOARD_SCREEN;
                    clear_screen();
                    TMR2ON = 0;
                }
                break;
                
            case VIEW_LOG_SCREEN:
                if(key == SW5)
                {
                    pre_key = key;
                    if(++delay >= 10)
                    {
                        sw_press = LONG_PRESS;
                    }
                }
                else if(key == SW4)
                {
                    pre_key = key;
                    if(++delay >= 10)
                    {
                        sw_press = LONG_PRESS;
                    }
                }
                else if(delay > 0 && delay < 10)
                {
                    sw_press = SHORT_PRESS;
                    key = pre_key;
                    pre_key = ALL_RELEASED;
                    delay = 0;
                }
                else
                {
                    delay = 0;
                }
                
                if(sw_press == SHORT_PRESS)
                {
                    view = view_log_screen(key, reset_flag);
                    sw_press = NO_PRESS;
                }
                else if(sw_press == LONG_PRESS)
                {
                    key = pre_key;
                    pre_key = ALL_RELEASED;
                    view = view_log_screen(key, reset_flag);
                    sw_press = NO_PRESS;
                }
                else if(sw_press == NO_PRESS)
                {
                    key = ALL_RELEASED;
                    view = view_log_screen(key, reset_flag);
                }
                
                switch(view)
                {
                    case RETURN_BACK_MENU:
                        view = 0;
                        control_flag = MAIN_MENU_SCREEN;
                        delay = 0;
                        clear_screen();
                        return_time = 5;
                        TMR2ON = 1;
                        reset_flag = RESET_MENU;
                        continue;
                    case RETURN_BACK_DASH:
                        view = 0;
                        delay = 0;
                        control_flag = DASHBOARD_SCREEN;
                        clear_screen();
                        break;
                }
                break;
                
            case CLEAR_LOG_SCREEN:
                if(key == SW5)
                {
                    pre_key = key;
                    if(++delay >= 10)
                    {
                        sw_press = LONG_PRESS;
                    }
                }
                else if(key == SW4)
                {
                    pre_key = key;
                    if(++delay >= 10)
                    {
                        sw_press = LONG_PRESS;
                    }
                }
                else if(delay > 0 && delay < 10)
                {
                    sw_press = NO_PRESS;
                    key = pre_key;
                    pre_key = ALL_RELEASED;
                    delay = 0;
                }
                else
                {
                    delay = 0;
                }
                
                if(sw_press == LONG_PRESS)
                {
                    key = pre_key;
                    pre_key = ALL_RELEASED;
                    clear = clear_log_screen(key, reset_flag);
                    sw_press = NO_PRESS;
                }
                else if(sw_press == NO_PRESS)
                {
                    key = ALL_RELEASED;
                    clear = clear_log_screen(key, reset_flag);
                }
                
                switch(clear)
                {
                    case RETURN_BACK_MENU:
                        clear = 0;
                        control_flag = MAIN_MENU_SCREEN;
                        delay = 0;
                        clear_screen();
                        return_time = 5;
                        TMR2ON = 1;
                        reset_flag = RESET_MENU;
                        continue;
                    case RETURN_BACK_DASH:
                        clear = 0;
                        delay = 0;
                        control_flag = DASHBOARD_SCREEN;
                        clear_screen();
                        break;
                }
                break;
                
            case DOWNLOAD_LOG_SCREEN:
                download_log();
                control_flag = DASHBOARD_SCREEN;
                clear_screen();
                break;
                
            case SET_TIME_SCREEN:
                if(key == SW5)
                {
                    pre_key = key;
                    if(++delay >= 10)
                    {
                        sw_press = LONG_PRESS;
                    }
                }
                else if(key == SW4)
                {
                    pre_key = key;
                    if(++delay >= 10)
                    {
                        sw_press = LONG_PRESS;
                    }
                }
                else if(delay > 0 && delay < 10)
                {
                    sw_press = SHORT_PRESS;
                    key = pre_key;
                    pre_key = ALL_RELEASED;
                    delay = 0;
                }
                else
                {
                    delay = 0;
                }
                
                if(sw_press == SHORT_PRESS)
                {
                    time = set_time_screen(key, reset_flag);
                    sw_press = NO_PRESS;
                }
                else if(sw_press == LONG_PRESS)
                {
                    key = pre_key;
                    pre_key = ALL_RELEASED;
                    time = set_time_screen(key, reset_flag);
                    sw_press = NO_PRESS;
                }
                else if(sw_press == NO_PRESS)
                {
                    key = ALL_RELEASED;
                    time = set_time_screen(key, reset_flag);
                }
                
                switch(time)
                {
                    case RETURN_BACK_DASH:
                        time = 0;
                        control_flag = DASHBOARD_SCREEN;
                        clear_screen();
                        break;
                    case RETURN_BACK_MENU:
                        time = 0;
                        control_flag = MAIN_MENU_SCREEN;
                        delay = 0;
                        clear_screen();
                        return_time = 5;
                        TMR2ON = 1;
                        reset_flag = RESET_MENU;
                        continue;
                }
                break;
                
            case CHANG_PASS_SCREEN:
                chang_pass = change_pass_screen(key, reset_flag);
                
                if(chang_pass == RETURN_BACK_MENU)
                {
                    chang_pass = 0;
                    control_flag = MAIN_MENU_SCREEN;
                    clear_screen();
                    return_time = 5;
                    TMR2ON = 1;
                    reset_flag = RESET_MENU;
                    continue;
                }
                else if(chang_pass == RETURN_BACK_DASH)
                {
                    chang_pass = 0;
                    control_flag = DASHBOARD_SCREEN;
                    clear_screen();
                    TMR2ON = 0;
                }
                break;
        }
        
        reset_flag = RESET_NOTHING;
    }
    return ;
}
