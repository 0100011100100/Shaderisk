#include <Windows.h>
#include <iostream>
#include <string>

bool InjectDLL(const std::string& processName, const std::string& dllPath) {
    HANDLE hProcess = NULL;
    LPVOID pDllPath = NULL;
    HMODULE hKernel32 = GetModuleHandle("Kernel32.dll");

    // Open the target process
    hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, FindProcessID(processName.c_str()));
    if (!hProcess) {
        std::cerr << "Failed to open process!" << std::endl;
        return false;
    }

    // Allocate memory in the target process
    pDllPath = VirtualAllocEx(hProcess, NULL, dllPath.size() + 1, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
    if (!pDllPath) {
        std::cerr << "Failed to allocate memory!" << std::endl;
        CloseHandle(hProcess);
        return false;
    }

    // Write the DLL path to the allocated memory
    WriteProcessMemory(hProcess, pDllPath, dllPath.c_str(), dllPath.size() + 1, NULL);

    // Inject the DLL
    HANDLE hThread = CreateRemoteThread(hProcess, NULL, 0, (LPTHREAD_START_ROUTINE)GetProcAddress(hKernel32, "LoadLibraryA"), pDllPath, 0, NULL);
    if (!hThread) {
        std::cerr << "Failed to create remote thread!" << std::endl;
        VirtualFreeEx(hProcess, pDllPath, 0, MEM_RELEASE);
        CloseHandle(hProcess);
        return false;
    }

    // Wait for the DLL to load
    WaitForSingleObject(hThread, INFINITE);

    // Clean up
    VirtualFreeEx(hProcess, pDllPath, 0, MEM_RELEASE);
    CloseHandle(hThread);
    CloseHandle(hProcess);

    return true;
}

int main() {
    std::string processName = "RobloxPlayerBeta.exe";
    std::string dllPath = "C:\\Path\\To\\Your\\Shader.dll";

    if (InjectDLL(processName, dllPath)) {
        std::cout << "DLL successfully injected!" << std::endl;
    } else {
        std::cerr << "DLL injection failed!" << std::endl;
    }

    return 0;
}
