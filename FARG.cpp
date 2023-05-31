#include <iostream>
#include <windows.h>
#include <gdiplus.h>
#pragma comment(lib, "gdiplus.lib")


//GetEncoderClsid Fn

int GetEncoderClsid(const WCHAR* format, CLSID* pClsid) {
    UINT num = 0;           // number of image encoders
    UINT size = 0;          // size of the image encoder array in bytes

    // Get the size of the image encoder array
    Gdiplus::GetImageEncodersSize(&num, &size);
    if (size == 0)
        return -1;          // Failed to retrieve the encoder array size

    // Allocate memory for the image encoder array
    Gdiplus::ImageCodecInfo* pImageCodecInfo = (Gdiplus::ImageCodecInfo*)(malloc(size));
    if (pImageCodecInfo == nullptr)
        return -1;          // Failed to allocate memory for the encoder array

    // Get the image encoders
    Gdiplus::GetImageEncoders(num, size, pImageCodecInfo);

    // Find the encoder for the specified format
    for (UINT i = 0; i < num; ++i) {
        if (wcscmp(pImageCodecInfo[i].MimeType, format) == 0) {
            *pClsid = pImageCodecInfo[i].Clsid;
            free(pImageCodecInfo);
            return i;       // Return the index of the encoder
        }
    }

    free(pImageCodecInfo);
    return -1;              // Specified encoder not found
}


void TakeScreenshot(const std::wstring& processName, const std::wstring& outputFileName) {
    // Get the handle of the target process
    HWND hwnd = FindWindow(nullptr, processName.c_str());
    if (hwnd == nullptr) {
        std::cout << "Failed to find the specified process." << std::endl;
        return;
    }

    // Get the dimensions of the window
    RECT rect;
    if (!GetClientRect(hwnd, &rect)) {
        std::cout << "Failed to get the dimensions of the window." << std::endl;
        return;
    }

    // Create a bitmap to hold the screenshot
    HDC hdcScreen = GetDC(nullptr);
    HDC hdcWindow = GetDC(hwnd);
    HBITMAP hBitmap = CreateCompatibleBitmap(hdcScreen, rect.right - rect.left, rect.bottom - rect.top);
    HGDIOBJ hOldBitmap = SelectObject(hdcWindow, hBitmap);

    // Copy the contents of the window to the bitmap
    if (!BitBlt(hdcWindow, 0, 0, rect.right - rect.left, rect.bottom - rect.top, hdcScreen, 0, 0, SRCCOPY)) {
        std::cout << "Failed to copy the window contents." << std::endl;
        return;
    }

    // Save the bitmap as an image file
    Gdiplus::Bitmap bitmap(hBitmap, nullptr);
    CLSID clsid;
    if (GetEncoderClsid(L"image/png", &clsid) != -1) {
        bitmap.Save(outputFileName.c_str(), &clsid, nullptr);
        std::wcout << L"Screenshot saved as: " << outputFileName << std::endl; // Use std::wcout for wide character strings
    }

    // Clean up resources
    SelectObject(hdcWindow, hOldBitmap);
    DeleteObject(hBitmap);
    ReleaseDC(hwnd, hdcWindow);
    ReleaseDC(nullptr, hdcScreen);
}

int main() {
    std::wstring processName = L"Spotify"; // Replace with the target process name
    std::wstring outputFileName = L"FARG.png"; // Replace with the desired output file name

    // Initialize GDI+
    Gdiplus::GdiplusStartupInput gdiplusStartupInput;
    ULONG_PTR gdiplusToken;
    Gdiplus::GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, nullptr);

    // Take the screenshot
    TakeScreenshot(processName, outputFileName);

    // Shutdown GDI+
    Gdiplus::GdiplusShutdown(gdiplusToken);

    return 0;
}
