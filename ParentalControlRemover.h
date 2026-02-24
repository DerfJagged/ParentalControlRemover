#pragma once
#ifndef _ParentalControlRemover_H
#define _ParentalControlRemover_H

typedef long NTSTATUS;
extern "C" void __stdcall HalReturnToFirmware(int type);
extern "C" NTSTATUS __stdcall ExGetXConfigSetting(
    unsigned short category,
    unsigned short setting,
    void* buffer,
    unsigned long bufferSize,
    unsigned long* requiredSize
);

extern "C" NTSTATUS __stdcall ExSetXConfigSetting(
    unsigned short category,
    unsigned short setting,
    const void* buffer,
    unsigned long bufferSize
);

DWORD GetParentalControlCode();
DWORD GetHiddenSettingsCode();
void ClearParentalControls();
void ShowTextForSeconds(const wchar_t *message, float seconds);
char* GetParentalControlsButtonName(char button);
char* GetHiddenSettingsButtonName(char button);

#endif
