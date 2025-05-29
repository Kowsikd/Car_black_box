#include "main.h"

char time[7];
unsigned char clock_reg[3];
char log[11], view_log[11], down_log[11];      //hhmmssevsp\0
char log_pos, log_count = 0;
char log_flag = 1, sec;
unsigned char return_time;
char *menu[] = {"View Log", "Clear Log", "Download Log", "Set Time", "Change Passwrd"};
char menu_pos;
extern unsigned char sw_press;

static void get_time()
{
    clock_reg[0] = read_ds1307(HOUR_ADDR); // HH -> BCD
    clock_reg[1] = read_ds1307(MIN_ADDR); // MM -> BCD
    clock_reg[2] = read_ds1307(SEC_ADDR); // SS -> BCD
    
    /* To store the time in HHMMSS format */
      // "HHMMSS"
    // HH -> 
    time[0] = ((clock_reg[0] >> 4) & 0x03) + '0';
    time[1] = (clock_reg[0] & 0x0F) + '0';
    
    // MM 
    time[2] = ((clock_reg[1] >> 4) & 0x07) + '0';
    time[3] = (clock_reg[1] & 0x0F) + '0';
    
    // SS
    time[4] = ((clock_reg[2] >> 4) & 0x07) + '0';
    time[5] = (clock_reg[2] & 0x0F) + '0';
    time[6] = '\0';
}

static void display_time()
{
    get_time();
    
    // HH:MM:SS 
    clcd_putch(time[0], LINE2(2));
    clcd_putch(time[1], LINE2(3));
    clcd_putch(':', LINE2(4));
    clcd_putch(time[2], LINE2(5));
    clcd_putch(time[3], LINE2(6));
    clcd_putch(':', LINE2(7));
    clcd_putch(time[4], LINE2(8));
    clcd_putch(time[5], LINE2(9));
}

void display_dashboard(char event[], unsigned char speed)
{
    clcd_print("TIME     E  SP", LINE1(2));
    
    //display time
    display_time();
    
    //display event
    clcd_print(event, LINE2(11));
    
    //display speed
    clcd_putch((speed / 10) + '0', LINE2(14));
    clcd_putch((speed % 10) + '0', LINE2(15));
}

void store_event(void)
{
    char addr = 0x05;
    if(log_pos == 10)
    {
        log_pos = 0;
    }
    addr = 0x05 + (log_pos * 10);
    ext_eeprom_24C02_str_write(addr, log);
    log_pos++;
    
    if(log_count < 9)
    {
        log_count++;
    }
}

void log_event(unsigned char event[], unsigned char speed)
{
    log_flag = 1;
    
    get_time();
    
    strncpy(log, time, 6);
    strncpy(&log[6], event, 2);
    
    log[8] = (speed / 10) + '0';
    log[9] = (speed % 10) + '0';
    log[10] = '\0';
    
    store_event();
}

unsigned char login(unsigned char key, unsigned char reset_flag)
{
    static char user_pass[5];
    static unsigned char i, attempt_left;
    
    if(reset_flag == RESET_PASSWORD)
    {
        i = 0;
        attempt_left = 3;
        user_pass[0] = '\0';
        user_pass[1] = '\0';
        user_pass[2] = '\0';
        user_pass[3] = '\0';
        key = ALL_RELEASED;
        return_time = 5;
    }
    
    if(return_time == 0)
    {
        return RETURN_BACK_DASH;
    }
    
    if(key == SW4 && i < 4)
    {
        clcd_putch('*', LINE2(4+i));
        user_pass[i] = '1';
        i++;
        return_time = 5;
    }
    else if(key == SW5 && i < 4)
    {
        clcd_putch('*', LINE2(4+i));
        user_pass[i] = '0';
        i++;
        return_time = 5;
    }
    
    if(i == 4)
    {
        char save_pass[4];
        
        for(int j = 0; j < 4; j++)
        {
            save_pass[j] = ext_eeprom_24C02_read(j);
        }
        
        if(strncmp(user_pass, save_pass, 4) == 0)
        {
            clear_screen();
            clcd_write(DISP_ON_AND_CURSOR_OFF, INST_MODE);
            __delay_us(100);
            clcd_print("Login Success", LINE1(1));
            __delay_ms(1000);
            return LOGIN_SUCCESS;
        }
        else
        {
            attempt_left--;
            if(attempt_left == 0)
            {
                clear_screen();
                clcd_write(DISP_ON_AND_CURSOR_OFF, INST_MODE);
                __delay_us(100);
                clcd_print("You are blocked", LINE1(0));
                clcd_print("Wait for ", LINE2(0));
                clcd_print("sec", LINE2(12));
                
                sec = 60;
                
                while(sec)
                {
                    clcd_putch((sec / 10) + '0', LINE2(9));
                    clcd_putch((sec % 10) + '0', LINE2(10));
                }
                
                attempt_left = 3;
            }
            else
            {
                clear_screen();
                clcd_write(DISP_ON_AND_CURSOR_OFF, INST_MODE);
                __delay_us(100);
                clcd_print("Wrong Password", LINE1(1));
                clcd_print("Attempts Left", LINE2(0));
                clcd_putch(attempt_left + '0', LINE2(14));
                __delay_ms(2000);
            }
            
            clear_screen();
            clcd_print("Enter Password", LINE1(1));
            clcd_write(LINE2(4), INST_MODE);
            __delay_us(100);
            clcd_write(DISP_ON_AND_CURSOR_ON, INST_MODE);
            __delay_us(100);
            i = 0;
            return_time = 5;
        }
    }
}

unsigned char menu_screen(unsigned char key, unsigned char reset_flag)
{
    if(reset_flag == RESET_MENU)
    {
        return_time = 5;
        menu_pos = 0;
        key = ALL_RELEASED;
    }
    
    if(return_time == 0)
    {
        return RETURN_BACK_DASH;
    }
    
    if(key == SW4 && menu_pos > 0)
    {
        menu_pos--;
        clear_screen();
        return_time = 5;
    }
    else if(key == SW5 && menu_pos < 4)
    {
        menu_pos++;
        clear_screen();
        return_time = 5;
    }
    
    if(menu_pos == 4)
    {
        clcd_putch('*', LINE2(1));
        clcd_print(menu[menu_pos - 1], LINE1(2));
        clcd_print(menu[menu_pos], LINE2(2));
    }
    else
    {
        clcd_putch('*', LINE1(1));
        clcd_print(menu[menu_pos], LINE1(2));
        clcd_print(menu[menu_pos + 1], LINE2(2));
    }
    
    return menu_pos;
}

unsigned char view_log_screen(unsigned char key, unsigned char reset_flag)
{
    static char i;
    
    if(reset_flag == RESET_VIEW)
    {
        i = 0;
        key = ALL_RELEASED;
    }
    
    if(log_flag == 1)
    {
        clcd_print("# TIME     E  SP", LINE1(0));

        if(key == SW4 && sw_press == SHORT_PRESS)
        {
            if(i - 1 < 0)
            {
                i = log_count - 1;
            }
            else
            {
                i--;
            }
        }
        else if(key == SW5 && sw_press == SHORT_PRESS)
        {
            if(i + 1 == log_count)
            {
                i = 0;
            }
            else
            {
                i++;
            }
        }
        
        char addr = 0x05 + (i * 10);
        
        for(char j = 0; j < 10; j++)
        {
            view_log[j] = ext_eeprom_24C02_read(addr + j);
        }
        
        //view_log[10] = '\0';
        //clcd_print(view_log, LINE2(2));
        
        clcd_putch(i + '0', LINE2(0));
        clcd_putch(view_log[0], LINE2(2));
        clcd_putch(view_log[1], LINE2(3));
        clcd_putch(':', LINE2(4));
        clcd_putch(view_log[2], LINE2(5));
        clcd_putch(view_log[3], LINE2(6));
        clcd_putch(':', LINE2(7));
        clcd_putch(view_log[4], LINE2(8));
        clcd_putch(view_log[5], LINE2(9));
        
        clcd_putch(view_log[6], LINE2(11));
        clcd_putch(view_log[7], LINE2(12));
        
        clcd_putch(view_log[8], LINE2(14));
        clcd_putch(view_log[9], LINE2(15));
    }
    else
    {
        clcd_print("No Log Found", LINE1(2));
    }
    
    if(key == SW4 && sw_press == LONG_PRESS)
    {
        return RETURN_BACK_MENU;
    }
    else if(key == SW5 && sw_press == LONG_PRESS)
    {
        return RETURN_BACK_DASH;
    }
}

unsigned char clear_log_screen(unsigned char key, unsigned char reset_flag)
{
    if(reset_flag == RESET_CLEAR)
    {
        if(log_flag == 1)
        {
            log_flag = 0;
            log_count = 0;
            log_pos = 0;
            clcd_print("Log Cleared", LINE1(2));
            clcd_print("Successfully", LINE2(2));
        }
        else
        {
            clcd_print("No Log Found", LINE1(2));
        }
        
        key = ALL_RELEASED;
        sw_press = NO_PRESS;
    }
    
    if(key == SW4 && sw_press == LONG_PRESS)
    {
        return RETURN_BACK_MENU;
    }
    else if(key == SW5 && sw_press == LONG_PRESS)
    {
        return RETURN_BACK_DASH;
    }
}

void download_log(void)
{
    if(log_flag == 1)
    {
        puts("Log is in the format of Time, Event, Speed  ");
        for(char i = 0; i < log_count; i++)
        {
            char addr = 0x05 + (i * 10);
            for(char j = 0; j < 10; j++)
            {
                down_log[j] = ext_eeprom_24C02_read(addr + j);
            }
            
            puts("Log.No: ");
            putchar((i + 1) + '0');
            puts(" -> ");
            putchar(down_log[0]);
            putchar(down_log[1]);
            putchar(':');
            putchar(down_log[2]);
            putchar(down_log[3]);
            putchar(':');
            putchar(down_log[4]);
            putchar(down_log[5]);
            puts("   ");
            putchar(down_log[6]);
            putchar(down_log[7]);
            puts("     ");
            putchar(down_log[8]);
            putchar(down_log[9]);
            puts("  ");
        }
        
        clcd_print("Logs Downloaded", LINE1(1));
        clcd_print("successfully", LINE2(2));
        __delay_ms(2000);
    }
    else if(log_flag == 0)
    {
        puts("No Log Found  ");
    }
}

void save_time(unsigned char hour, unsigned char min, unsigned char sec)
{
    hour = ((hour / 10) << 4) | (hour % 10);
    min = ((min / 10) << 4) | (min % 10);
    sec = ((sec / 10) << 4) | (sec % 10);
    
    write_ds1307(HOUR_ADDR, hour);
    write_ds1307(MIN_ADDR, min);
    write_ds1307(SEC_ADDR, sec);
}

unsigned char set_time_screen(unsigned char key, unsigned char reset_flag)
{
    static unsigned char hour, min, sec;
    static unsigned char field_change, flag, delay;
    
    if(reset_flag == RESET_TIME)
    {
        get_time();
        
        hour = ((clock_reg[0] >> 4) & 0x03);
        hour = (hour * 10) + (clock_reg[0] & 0x0F);
        
        min = ((clock_reg[1] >> 4) & 0x07);
        min = (min * 10) + ((clock_reg[1] & 0x0F));
        
        sec = ((clock_reg[2] >> 4) & 0x07);
        sec = (sec * 10) + ((clock_reg[2] & 0x0F));
        
        field_change = 2;
        flag = 0;
        delay = 5;
        
        key = ALL_RELEASED;
        
        clcd_print("Time <HH:MM:SS>", LINE1(0));
    }
    
    if(key == SW4 && sw_press == SHORT_PRESS)
    {
        if(field_change == 0)
        {
            hour++;
            if(hour == 24)
            {
                hour = 0;
            }
        }
        else if(field_change == 1)
        {
            min++;
            if(min == 60)
            {
                min = 0;
            }
        }
        else if(field_change == 2)
        {
            sec++;
            if(sec == 60)
            {
                sec = 0;
            }
        }
        
        flag = 0;
    }
    else if(key == SW4 && sw_press == LONG_PRESS)
    {
        return RETURN_BACK_MENU;
    }
    else if(key == SW5 && sw_press == SHORT_PRESS)
    {
        if(field_change > 0)
        {
            field_change--;
        }
        else if(field_change == 0)
        {
            field_change = 2;
        }
    }
    else if(key == SW5 && sw_press == LONG_PRESS)
    {
        save_time(hour, min, sec);
        clear_screen();
        clcd_print("Time Changed", LINE1(2));
        clcd_print("Successfully", LINE2(2));
        __delay_ms(2000);
        return RETURN_BACK_DASH;
    }
    
    
    if(field_change == 0)
    {
        if(--delay == 0)
        {
            delay = 5;
            if(flag == 0)
            {
                clcd_putch((hour / 10) + '0', LINE2(0));
                clcd_putch((hour % 10) + '0', LINE2(1));
                flag = 1;
            }
            else
            {
                clcd_print("  ", LINE2(0));
                flag = 0;
            }
        }

        clcd_putch(':', LINE2(2));
        clcd_putch((min / 10) + '0', LINE2(3));
        clcd_putch((min % 10) + '0', LINE2(4));
        clcd_putch(':', LINE2(5));
        clcd_putch((sec / 10) + '0', LINE2(6));
        clcd_putch((sec % 10) + '0', LINE2(7));
    }
    else if(field_change == 1)
    {
        clcd_putch((hour / 10) + '0', LINE2(0));
        clcd_putch((hour % 10) + '0', LINE2(1));
        clcd_putch(':', LINE2(2));

        if(--delay == 0)
        {
            delay = 5;
            if(flag == 0)
            {
                clcd_putch((min / 10) + '0', LINE2(3));
                clcd_putch((min % 10) + '0', LINE2(4));
                flag = 1;
            }
            else
            {
                clcd_print("  ", LINE2(3));
                flag = 0;
            }
        }

        clcd_putch(':', LINE2(5));
        clcd_putch((sec / 10) + '0', LINE2(6));
        clcd_putch((sec % 10) + '0', LINE2(7));
    }
    else if(field_change == 2)
    {
        clcd_putch((hour / 10) + '0', LINE2(0));
        clcd_putch((hour % 10) + '0', LINE2(1));
        clcd_putch(':', LINE2(2));
        clcd_putch((min / 10) + '0', LINE2(3));
        clcd_putch((min % 10) + '0', LINE2(4));
        clcd_putch(':', LINE2(5));

        if(--delay == 0)
        {
            delay = 5;
            if(flag == 0)
            {
                clcd_putch((sec / 10) + '0', LINE2(6));
                clcd_putch((sec % 10) + '0', LINE2(7));
                flag = 1;
            }
            else
            {
                clcd_print("  ", LINE2(6));
                flag = 0;
            }
        }
    }   
}

unsigned char change_pass_screen(unsigned char key, unsigned char reset_flag)
{
    static char enter_pass[5];
    static char re_enter_pass[5];
    static unsigned char pass_flag, i;
    
    if(reset_flag == RESET_PASSWORD)
    {
        pass_flag = 1;
        i = 0;
        key = ALL_RELEASED;
        return_time = 5;
        TMR2ON = 1;
    }
    
    if(return_time == 0)
    {
        clcd_write(DISP_ON_AND_CURSOR_OFF, INST_MODE);
        __delay_us(100);
        return RETURN_BACK_DASH;
    }
    
    if(pass_flag == 1)
    {
        clcd_print("Enter New Passwd", LINE1(0));
        
        clcd_write(LINE2(4), INST_MODE);
        __delay_us(100);
        clcd_write(DISP_ON_AND_CURSOR_ON, INST_MODE);
        __delay_us(100);
        
        pass_flag = 0;
    }
    else if(pass_flag == 2)
    {
        clcd_print("Re Enter New Pass", LINE1(0));
        
        clcd_write(LINE2(4), INST_MODE);
        __delay_us(100);
        clcd_write(DISP_ON_AND_CURSOR_ON, INST_MODE);
        __delay_us(100);
        
        pass_flag = 3;
    }
    
    if(key == SW4 && i < 4)
    {
        clcd_putch('*', LINE2(4+i));
        if(pass_flag == 0)
        {
            enter_pass[i] = '1';
        }
        else if(pass_flag == 3)
        {
            re_enter_pass[i] = '1';
        }
        i++;
        
        return_time = 5;
    }
    else if(key == SW5 && i < 4)
    {
        clcd_putch('*', LINE2(4+i));
        if(pass_flag == 0)
        {
            enter_pass[i] = '0';
        }
        else if(pass_flag == 3)
        {
            re_enter_pass[i] = '0';
        }
        i++;
        
        return_time = 5;
    }
    
    if(pass_flag == 0 && i == 4)
    {
        pass_flag = 2;
        i = 0;
        clear_screen();
    }
    
    if(pass_flag == 3 && i == 4)
    {
        enter_pass[4] = '\0';
        re_enter_pass[4] = '\0';
        
        TMR2ON = 0;
        clcd_write(DISP_ON_AND_CURSOR_OFF, INST_MODE);
        __delay_us(100);
        clear_screen();
            
        if(strncmp(enter_pass, re_enter_pass, 4) == 0)
        {
            ext_eeprom_24C02_str_write(0x00, enter_pass);
            
            clcd_print("Password Changed", LINE1(0));
            clcd_print("Successfully", LINE2(2));
            __delay_ms(2000);
        }
        else
        {
            clcd_print("Password Mismatc", LINE1(0));
            __delay_ms(2000);
        }
        
        i = 0;
        return RETURN_BACK_MENU;
    }
}

void clear_screen(void)
{
    clcd_write(CLEAR_DISP_SCREEN, INST_MODE);
    __delay_us(500);
}
