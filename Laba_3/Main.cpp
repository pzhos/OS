#include <Windows.h>
#include <iostream>

using namespace std;


int myArraySize = 0;
int* myArray = nullptr;
CRITICAL_SECTION myCriticalSection;
HANDLE* myThreadHandles;
HANDLE* myThreadStartEvents;
HANDLE* myThreadStopEvents;
HANDLE* myThreadExitEvents;
HANDLE myMutex;

// Функция, выполняемая в каждом потоке
DWORD WINAPI markerFunction(LPVOID threadIndex)
{
    // Ожидание сигнала для старта потока
    WaitForSingleObject(myThreadStartEvents[(int)threadIndex], INFINITE);

    int markedNumbersCounter = 0;
    srand((int)threadIndex);

    while (true) {
        // Вход в критическую секцию для безопасной работы с общими данными
        EnterCriticalSection(&myCriticalSection);

        // Генерация случайного числа и попытка пометить элемент массива
        int randomNumber = rand() % myArraySize;
        if (myArray[randomNumber] == 0) {
            Sleep(5);
            myArray[randomNumber] = (int)threadIndex + 1;
            markedNumbersCounter++;
            Sleep(5);
            LeaveCriticalSection(&myCriticalSection);
        }
        else {
            // Вывод информации о неудачной попытке и остановка потока
            cout << " Количество потоков: " << (int)threadIndex + 1 << "\n";
            cout << " Количество помеченных элементов: " << markedNumbersCounter << "\n";
            cout << "\ Индекс элемента, который не может быть помечен: " << randomNumber << "\n";

            LeaveCriticalSection(&myCriticalSection);

            // Сигнал остановки и ожидание подтверждения завершения потока
            SetEvent(myThreadStopEvents[(int)threadIndex]);
            ResetEvent(myThreadStartEvents[(int)threadIndex]);

            HANDLE threadStartExitPair[]{ 
                myThreadStartEvents[(int)threadIndex],
                myThreadExitEvents[(int)threadIndex]
            };

            if (WaitForMultipleObjects(2, threadStartExitPair, FALSE, INFINITE) == WAIT_OBJECT_0 + 1) {
                // Удаление пометок текущего потока из массива и завершение потока
                EnterCriticalSection(&myCriticalSection);
                for (size_t i = 0; i < myArraySize; i++) {
                    if (myArray[i] == (int)threadIndex + 1) {
                        myArray[i] = 0;
                    }
                }
                LeaveCriticalSection(&myCriticalSection);

                ExitThread(NULL);
            }
            else {
                ResetEvent(myThreadStopEvents[(int)threadIndex]);
                continue;
            }
        }
    }
}

int main()
{
    // Инициализация локали и ввод размера массива и количества потоков
    setlocale(LC_ALL, "Russian");
    int numberOfThreads = 0;
    cout << "Введите размер массива элементов: ";
    cin >> myArraySize;
    myArray = new int[myArraySize] {};
    cout << "Введите количество потоков : ";
    cin >> numberOfThreads;

    // Инициализация критической секции и других структур данных
    InitializeCriticalSection(&myCriticalSection);

    myThreadHandles = new HANDLE[numberOfThreads];
    myThreadStartEvents = new HANDLE[numberOfThreads];
    myThreadStopEvents = new HANDLE[numberOfThreads];
    myThreadExitEvents = new HANDLE[numberOfThreads];
    myMutex = CreateMutex(NULL, FALSE, NULL);

    // Создание и запуск потоков
    for (int i = 0; i < numberOfThreads; i++) {
        myThreadHandles[i] = CreateThread(NULL, 1, markerFunction, (LPVOID)i, NULL, NULL);
        myThreadStartEvents[i] = CreateEvent(NULL, TRUE, FALSE, NULL);
        myThreadStopEvents[i] = CreateEvent(NULL, TRUE, FALSE, NULL);
        myThreadExitEvents[i] = CreateEvent(NULL, TRUE, FALSE, NULL);
    }

    // Запуск потоков
    for (int i = 0; i < numberOfThreads; i++) {
        SetEvent(myThreadStartEvents[i]);
    }

    // Обработка завершения работы потоков
    int numberOfCompletedThreads = 0;
    bool* isThreadExited = new bool[numberOfThreads] {};
    while (numberOfCompletedThreads < numberOfThreads) {
        // Ожидание завершения всех потоков
        WaitForMultipleObjects(numberOfThreads, myThreadStopEvents, TRUE, INFINITE);

        // Вывод итогового массива после завершения всех потоков
        myMutex = OpenMutex(NULL, FALSE, NULL);
        cout << "Итоговый массив: ";
        for (int i = 0; i < myArraySize; i++) {
            cout << myArray[i] << " ";
        }
        cout << "\n";
        ReleaseMutex(myMutex);

        // Запрос пользователя остановить конкретный поток
        int stopMarkerId;
        cout << "Введите номер потока, который вы хотите остановить: ";
        cin >> stopMarkerId;
        stopMarkerId--;

        if (!isThreadExited[stopMarkerId]) {
            // Обработка завершения выбранного потока
            numberOfCompletedThreads++;
            isThreadExited[stopMarkerId] = true;

            SetEvent(myThreadExitEvents[stopMarkerId]);
            WaitForSingleObject(myThreadHandles[stopMarkerId], INFINITE);
            CloseHandle(myThreadHandles[stopMarkerId]);
            CloseHandle(myThreadExitEvents[stopMarkerId]);
            CloseHandle(myThreadStartEvents[stopMarkerId]);
        }

        // Вывод текущего состояния массива после остановки потока
        myMutex = OpenMutex(NULL, FALSE, NULL);
        cout << "Массив: ";
        for (int i = 0; i < myArraySize; i++) {
            cout << myArray[i] << " ";
        }
        cout << "\n";
        ReleaseMutex(myMutex);

        // Подготовка к запуску оставшихся потоков
        for (int i = 0; i < numberOfThreads; i++) {
            if (!isThreadExited[i]) {
                ResetEvent(myThreadStopEvents[i]);
                SetEvent(myThreadStartEvents[i]);
            }
        }
    }

    // Очистка рес
}