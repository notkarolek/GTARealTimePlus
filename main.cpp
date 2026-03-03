#include <mod/amlmod.h>
#include <mod/logger.h>
#include <mod/config.h>
#include <time.h>

MYMOD(net.rusjj.notkarolek.realtimeplus, GTA Real Time Plus, 1.4, RusJJ/notkarolek)

void* hGame;
int lastDay;
Config cfgLocal("RealTimePlus"); 
Config* cfg = &cfgLocal;

bool bUseSystemTime = true;
int iStaticHour = 0;
int iStaticMinute = 00;

char *CurrentDay, *ms_nGameClockMonths, *ms_nGameClockDays, *ms_nGameClockHours, *ms_nGameClockMinutes, *ms_nGameClockSeconds;
unsigned int *ms_nMillisecondsPerGameMinute;
int *DaysPassed;

void (*IncrementStat)(uint16_t, float);

inline struct tm* Now()
{
    time_t tmp = time(NULL);
    return localtime(&tmp);
}

DECL_HOOKv(ClockUpdate_SA)
{
    if(bUseSystemTime)
    {
        auto now = Now();
        
        if(now->tm_yday != lastDay) IncrementStat(0x86, 1.0f);
        lastDay = now->tm_yday;

        *CurrentDay = now->tm_wday + 1;
        *ms_nGameClockMonths = now->tm_mon + 1;
        *ms_nGameClockDays = now->tm_mday;
        *ms_nGameClockHours = now->tm_hour;
        *ms_nGameClockMinutes = now->tm_min;
        *ms_nGameClockSeconds = now->tm_sec;
    }
    else
    {
        auto now = Now();
        if(now->tm_yday != lastDay) IncrementStat(0x86, 1.0f);
        lastDay = now->tm_yday;

        *CurrentDay = now->tm_wday + 1;
        *ms_nGameClockMonths = now->tm_mon + 1;
        *ms_nGameClockDays = now->tm_mday;
        *ms_nGameClockHours = iStaticHour;
        *ms_nGameClockMinutes = iStaticMinute;
        *ms_nGameClockSeconds = 0;
    }
}

DECL_HOOKv(ClockUpdate_VC)
{
    if(bUseSystemTime)
    {
        auto now = Now();

        if(now->tm_yday != lastDay) ++(*DaysPassed);
        lastDay = now->tm_yday;

        *ms_nGameClockHours = now->tm_hour;
        *ms_nGameClockMinutes = now->tm_min;
        *ms_nGameClockSeconds = now->tm_sec;
    }
    else
    {
        auto now = Now();
        if(now->tm_yday != lastDay) ++(*DaysPassed);
        lastDay = now->tm_yday;

        *ms_nGameClockHours = iStaticHour;
        *ms_nGameClockMinutes = iStaticMinute;
        *ms_nGameClockSeconds = 0;
    }
}
extern "C" void OnModLoad()
{
    logger->SetTag(modinfo->Name());
    
    bUseSystemTime = cfg->GetInt("systemtime", 1) != 0;
    
    const char* staticTimeStr = cfg->GetString("statictime", "0.00");
    int hour = 0, minute = 00;
    
    char* endptr;
    hour = strtol(staticTimeStr, &endptr, 10);
    if(*endptr == '.' && *(endptr+1) != '\0')
    {
        minute = strtol(endptr+1, NULL, 10);
    }
    
    if(hour < 0) hour = 0;
    if(hour > 23) hour = 23;
    if(minute < 0) minute = 0;
    if(minute > 59) minute = 59;
    
    iStaticHour = hour;
    iStaticMinute = minute;
    
    logger->Info("Config loaded: systemtime=%d, statictime=%d.%02d", 
                 bUseSystemTime ? 1 : 0, iStaticHour, iStaticMinute);
    
    hGame = aml->GetLibHandle("libGTASA.so");
    if(hGame != NULL)
    {
        SET_TO(CurrentDay, aml->GetSym(hGame, "_ZN6CClock10CurrentDayE"));
        SET_TO(ms_nGameClockMonths, aml->GetSym(hGame, "_ZN6CClock19ms_nGameClockMonthsE"));
        SET_TO(ms_nGameClockDays, aml->GetSym(hGame, "_ZN6CClock17ms_nGameClockDaysE"));
        SET_TO(IncrementStat, aml->GetSym(hGame, "_ZN6CStats13IncrementStatEtf"));
        HOOK(ClockUpdate_SA, aml->GetSym(hGame, "_ZN6CClock6UpdateEv"));
    }
    else
    {
        hGame = aml->GetLibHandle("libGTAVC.so");
        if(hGame == NULL) hGame = aml->GetLibHandle("libR1.so");
        if(hGame != NULL)
        {
            HOOK(ClockUpdate_VC, aml->GetSym(hGame, "_ZN6CClock6UpdateEv"));
            SET_TO(DaysPassed, aml->GetSym(hGame, "_ZN6CStats10DaysPassedE"));
        }
        else
        {
            logger->Error("The game is not supported!");
            return;
        }
    }
    SET_TO(ms_nGameClockHours,   aml->GetSym(hGame, "_ZN6CClock18ms_nGameClockHoursE"));
    SET_TO(ms_nGameClockMinutes, aml->GetSym(hGame, "_ZN6CClock20ms_nGameClockMinutesE"));
    SET_TO(ms_nGameClockSeconds, aml->GetSym(hGame, "_ZN6CClock20ms_nGameClockSecondsE"));

    lastDay = Now()->tm_yday;
}