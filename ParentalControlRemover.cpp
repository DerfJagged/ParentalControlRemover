//--------------------------------------------------------------------------------------
// ParentalControlRemover.cpp
//
// A tool to display the parental control code.
//--------------------------------------------------------------------------------------
#include <xtl.h>
#include <AtgApp.h>
#include <AtgFont.h>
#include <AtgInput.h>
#include <AtgUtil.h>
#include "ParentalControlRemover.h"

// The following was documented in:
// https://github.com/oukiar/freestyledash/blob/master/Freestyle/Tools/Generic/ExConfig.h
#define NT_SUCCESS(Status) (((NTSTATUS)(Status)) >= 0)
#define XCONFIG_USER_CATEGORY       0x0003
#define XCONFIG_USER_PC_PASSWORD    0x0017    
#define XCONFIG_USER_PC_FLAGS       0x000F

bool show_text = false;
float text_start_time = 0.0f; // in seconds
float text_duration   = 5.0f; // 5 seconds
wchar_t status_message[255];
wchar_t parentalControlCodeText[64];
DWORD parentalControlCode = 666;
bool initialCodeDisplayed = false;
char* passcodeButtons[4];

class ParentalControlRemover : public ATG::Application {
    ATG::Font m_Font16; // 16-point font class
    ATG::Font m_Font12; // 12-point font class
	ATG::GAMEPAD* m_pGamepad;
    HRESULT DrawTextContent();

private:
    virtual HRESULT Initialize();
    virtual HRESULT Update();
    virtual HRESULT Render();
};

VOID __cdecl main() {
    ParentalControlRemover atgApp;
    ATG::GetVideoSettings( &atgApp.m_d3dpp.BackBufferWidth, &atgApp.m_d3dpp.BackBufferHeight );
    atgApp.m_d3dpp.PresentationInterval = D3DPRESENT_INTERVAL_ONE;
    atgApp.Run();
}

HRESULT ParentalControlRemover::Initialize() {
    if( FAILED( m_Font16.Create( "game:\\Media\\Fonts\\Arial_16.xpr" ) ) )
        return ATGAPPERR_MEDIANOTFOUND;
    if( FAILED( m_Font12.Create( "game:\\Media\\Fonts\\Arial_12.xpr" ) ) )
        return ATGAPPERR_MEDIANOTFOUND;

    m_Font12.SetWindow( ATG::GetTitleSafeArea() );
    m_Font16.SetWindow( ATG::GetTitleSafeArea() );
	
    return S_OK;
}

HRESULT ParentalControlRemover::Update() {
    // Get the current gamepad status
    m_pGamepad = ATG::Input::GetMergedInput();

	if (initialCodeDisplayed == false) {
		parentalControlCode = GetParentalControlCode();
		if (parentalControlCode == 0) {
			ShowTextForSeconds(L"No passcode set", 3);
		}
		initialCodeDisplayed = true;
	}

	if( m_pGamepad->wPressedButtons & XINPUT_GAMEPAD_A ) {
		// Press (A) to clear parental controls
		ClearParentalControls();
		parentalControlCode = GetParentalControlCode();
	}
	else if( m_pGamepad->wPressedButtons & XINPUT_GAMEPAD_B ) {
		// Press (B) to Reboot
		HalReturnToFirmware(6);
	}

	return S_OK;
}

DWORD GetParentalControlCode() {
	DWORD passcode = 0;
	DWORD size = sizeof(passcode);

	NTSTATUS status = ExGetXConfigSetting(
		XCONFIG_USER_CATEGORY,
		XCONFIG_USER_PC_PASSWORD,
		&passcode,
		size,
		&size
	);

	if (NT_SUCCESS(status)) {
		// Passcode now contains the 4-byte parental control code
	}
	else {
		ShowTextForSeconds(L"Failed to retreive passcode", 3);
	}

	if (passcode == 0) {
		swprintf(parentalControlCodeText, L"Passcode: Not set");
	}
	else {
		// Convert passcode to button names
		passcodeButtons[0] = GetButtonName((passcode & 0xFF << 24) >> 24);
		passcodeButtons[1] = GetButtonName((passcode & 0xFF << 16) >> 16);
		passcodeButtons[2] = GetButtonName((passcode & 0xFF <<  8) >>  8);
		passcodeButtons[3] = GetButtonName((passcode & 0xFF <<  0) >>  0);
		swprintf(parentalControlCodeText, L"Passcode: %S - %S - %S - %S", passcodeButtons[0], passcodeButtons[1], passcodeButtons[2], passcodeButtons[3]);
		
		// Print passcode as bytes
		//swprintf(parentalControlCodeText, L"Passcode: %08X", passcode);
	}
	return passcode;
}

void ClearParentalControls() {
	if (parentalControlCode != 0) {
		// Turn off Parental Controls
		char flags = 0x0; // 0 = turn off parental controls
		DWORD flagsSize = sizeof(flags);
		NTSTATUS status1 = ExSetXConfigSetting(
			XCONFIG_USER_CATEGORY,
			XCONFIG_USER_PC_FLAGS,
			&flags,
			flagsSize
		);

		// Wipe passcode
		// If not wiped, it will ask for the old passcode if you turn Parental Controls back on
		DWORD passcode = 0x00000000; // 0 = no passcode
		DWORD passcodeSize = sizeof(passcode);
		NTSTATUS status2 = ExSetXConfigSetting(
			XCONFIG_USER_CATEGORY,
			XCONFIG_USER_PC_PASSWORD,
			&passcode,
			passcodeSize
		);

		if (NT_SUCCESS(status1) && NT_SUCCESS(status2)) {
			ShowTextForSeconds(L"Turned off parental controls", 3);
		}
		else {
			ShowTextForSeconds(L"Failed to turn off parental controls", 3);
		}
	}
	else {
		ShowTextForSeconds(L"No passcode set", 3);
	}
}

char* GetButtonName(char button) {
	switch (button) {
		case 0x01:
			return "X";
		case 0x02:
			return "Y";
		case 0x03:
			return "DPAD LEFT";
		case 0x04:
			return "DPAD RIGHT";
		case 0x05:
			return "DPAD UP";
		case 0x06:
			return "DPAD DOWN";
		case 0x09:
			return "LT";
		case 0x0A:
			return "RT";
		case 0x0B:
			return "LB";
		case 0x0C:
			return "RB";
		default:
			return "UNK";
	}
}

HRESULT ParentalControlRemover::DrawTextContent() {
    D3DRECT rc = m_Font16.m_rcWindow;
	
	//// BIG TEXT
    m_Font16.Begin();
    m_Font16.DrawText( ( rc.x2 - rc.x1 ) / 2.0f, 50, 0xffffffff, L"Parental Control Remover", ATGFONT_CENTER_X );
	m_Font16.DrawText( ( rc.x2 - rc.x1 ) / 2.0f, 125, 0xffffffff, parentalControlCodeText, ATGFONT_CENTER_X );

	// Temporary message printed to screen
	if (show_text) {
		// Compute time elapsed since start of showing text
		LARGE_INTEGER freq, counter;
		QueryPerformanceFrequency(&freq);
		QueryPerformanceCounter(&counter);
		float now = (float)counter.QuadPart / (float)freq.QuadPart;
		float elapsed = now - text_start_time;

		// Hide after X seconds
		if (elapsed < text_duration) {
			m_Font16.DrawText( (( rc.x2 - rc.x1 ) / 2.0f), 350, 0xffffffff, status_message, ATGFONT_CENTER_X );
		}
		else {
			show_text = false;
		}
	}
	
    m_Font16.End();

	//// SMALL TEXT
    m_Font12.Begin();
	m_Font12.DrawText( ( rc.x2 - rc.x1 ) / 2.0f - 160, 200, 0xffffffff, L"Press (A) to clear Parental Controls\nPress (B) to reboot", ATGFONT_LEFT );
	m_Font12.DrawText( rc.x1 - 200.0f, rc.y2 - 100.0f, 0xffffffff, L"Made by Derf\nConsoleMods.org", ATGFONT_LEFT );
    m_Font12.End();

    return S_OK;
}

HRESULT ParentalControlRemover::Render() {
    ATG::RenderBackground( 0x00000000, 0x00000000 ); //Black
    DrawTextContent();
	m_pd3dDevice->Present( NULL, NULL, NULL, NULL );
    return S_OK;
}

void ShowTextForSeconds(const wchar_t* message, float seconds) {
    show_text = true;
	wcsncpy(status_message, message, 255);
	LARGE_INTEGER freq, counter;
    QueryPerformanceFrequency(&freq);
    QueryPerformanceCounter(&counter);
    text_start_time = (float)counter.QuadPart / (float)freq.QuadPart;
	text_duration = seconds;
}