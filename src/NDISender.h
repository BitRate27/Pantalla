#pragma once
#include "global.h"
#include <string>
#include <vector>
#include "DXGICapture.h"

void CaptureLoop(std::wstring deviceName, std::string ndiName);

class NDISender {
public:
	NDISender(std::wstring deviceName, std::string ndiName);
	~NDISender();
	std::wstring getDeviceName() const { return m_deviceName; }
	std::string getNdiName() const { return m_ndiName; }

private:
	void CaptureLoop(std::wstring deviceName, std::string ndiName);
	std::wstring m_deviceName;
	std::string m_ndiName;
	std::thread m_captureThread;
	std::atomic<bool> m_captureRunning;
	DXGICapture m_capture;
};
