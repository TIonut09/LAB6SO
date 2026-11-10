#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

typedef struct {
    int counter;
} SharedData;

void process_logic(SharedData *shared, HANDLE sem, const char *process_name) {
    srand(GetTickCount() ^ GetCurrentProcessId());
    while (1) {
        WaitForSingleObject(sem, INFINITE);
        if (shared->counter >= 1000) {
            ReleaseSemaphore(sem, 1, NULL);
            break;
        }
        int coin = (rand() % 2) + 1;
        while (coin == 2 && shared->counter < 1000) {
            shared->counter++;
            printf("[%s] a incrementat contorul la: %d\n", process_name, shared->counter);
            coin = (rand() % 2) + 1;
        }
        ReleaseSemaphore(sem, 1, NULL);
        Sleep(1); // pauza foarte scurta
    }
}

int main(int argc, char *argv[]) {
    HANDLE hMapFile = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, sizeof(SharedData), "LabSharedMem");
    if (hMapFile == NULL) return 1;

    SharedData *shared = (SharedData*) MapViewOfFile(hMapFile, FILE_MAP_ALL_ACCESS, 0, 0, sizeof(SharedData));
    if (shared == NULL) return 1;

    HANDLE hSemaphore = CreateSemaphore(NULL, 1, 1, "LabSemaphore");
    if (hSemaphore == NULL) return 1;

    if (argc == 1) {
        shared->counter = 0;
        
        STARTUPINFO si;
        PROCESS_INFORMATION pi;
        ZeroMemory(&si, sizeof(si));
        si.cb = sizeof(si);
        ZeroMemory(&pi, sizeof(pi));

        char cmd[MAX_PATH];
        // Folosim sprintf_s in loc de sprintf pentru ca Visual Studio cere functii sigure
        sprintf_s(cmd, sizeof(cmd), "\"%s\" child", argv[0]);

        if (!CreateProcess(NULL, cmd, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi)) return 1;

        process_logic(shared, hSemaphore, "Proces PARINTE");

        WaitForSingleObject(pi.hProcess, INFINITE);
        printf("\nContor final: %d\n", shared->counter);

        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);
    } else {
        process_logic(shared, hSemaphore, "Proces COPIL");
    }

    UnmapViewOfFile(shared);
    CloseHandle(hMapFile);
    CloseHandle(hSemaphore);

    return 0;
}
