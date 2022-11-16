#include <iostream>
#include <thread>
#include <vector>
#include <chrono>

#include <stdio.h>
#include <Windows.h>

int screenWidth = 120;			// Console Screen Size X (columns)
int screenHeight = 30;			// Console Screen Size Y (rows)
std::wstring tetromino[7];
int fieldWidth = 12;
int fieldHeight = 18;
unsigned char* field = nullptr;

int Rotate(int x, int y, int r) 
{
    switch (r % 4)
    {
    case 0: return y * 4 + x;        //0 or 360 Deg
    case 1: return 12 + y - (x * 4); //90 Deg
    case 2: return 15 - (y * 4) - x; //180 Deg
    case 3: return 3 - y + (x * 4);  //270 Deg 
    }
}

bool PieceFits(int ID, int rotation, int posX, int posY)
{
    for (int x = 0; x < 4; x++)
    {
        for(int y = 0; y < 4; y++)
        {
            int pieceIndex = Rotate(x, y, rotation);
            int fieldIndex = (posY + y) * fieldWidth + (posX + x);

            if(posX + x >= 0 && posX + x <= fieldWidth)
            {
                if(posY + y >= 0 && posY + y <= fieldHeight)
                {
                    if (tetromino[ID][pieceIndex] == L'X' && field[fieldIndex] != 0)
                    {
                        return false;
                    }
                }
            }
        }
    }

    return true;
}

int main()
{
    //Create Assets
    tetromino[0].append(L"..X.");
    tetromino[0].append(L"..X.");
    tetromino[0].append(L"..X.");
    tetromino[0].append(L"..X.");

    tetromino[1].append(L"..X.");
    tetromino[1].append(L".XX.");
    tetromino[1].append(L".X..");
    tetromino[1].append(L"....");

    tetromino[2].append(L".X..");
    tetromino[2].append(L".XX.");
    tetromino[2].append(L"..X.");
    tetromino[2].append(L"....");

    tetromino[3].append(L"....");
    tetromino[3].append(L".XX.");
    tetromino[3].append(L".XX.");
    tetromino[3].append(L"....");

    tetromino[4].append(L"..X.");
    tetromino[4].append(L".XX.");
    tetromino[4].append(L"..X.");
    tetromino[4].append(L"....");

    tetromino[5].append(L"....");
    tetromino[5].append(L".XX.");
    tetromino[5].append(L"..X.");
    tetromino[5].append(L"..X.");

    tetromino[6].append(L"....");
    tetromino[6].append(L".XX.");
    tetromino[6].append(L".X..");
    tetromino[6].append(L".X..");

    field = new unsigned char[fieldWidth * fieldHeight];
    for (int x = 0; x < fieldWidth; x++)
    {
        for (int y = 0; y < fieldHeight; y++) 
        {
            field[y * fieldWidth + x] = (x == 0 || x == fieldWidth - 1 || y == fieldHeight - 1) ? 9 : 0;
        }
    }

    // Create Screen Buffer
    wchar_t* screen = new wchar_t[screenWidth * screenHeight];
    for (int i = 0; i < screenWidth * screenHeight; i++) screen[i] = L' ';
    HANDLE console = CreateConsoleScreenBuffer(GENERIC_READ | GENERIC_WRITE, 0, NULL, CONSOLE_TEXTMODE_BUFFER, NULL);
    SetConsoleActiveScreenBuffer(console);
    DWORD dwBytesWritten = 0;
     
    bool gameOver = false;

    int currentPiece = 0;
    int currentRotation = 0;
    int currentX = fieldWidth / 2;
    int currentY = 0;

    bool keys[5];
    bool rotateKeyDown = false;

    int speed = 20;
    int speedCounter = 0;
    bool forceDown = false;
    int pieceCount = 0;
    int score = 0;

    std::vector<int> lines;

    while (!gameOver)
    {
        //Timing
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
        speedCounter++;
        forceDown = (speedCounter == speed);
        //Input
        for (int i = 0; i < 5; i++)
        {
            keys[i] = (0x8000 & GetAsyncKeyState((unsigned char)("\x27\x25\x28ZX"[i]))) != 0;
        }
        //Left
        if (keys[0])
        {
            if (PieceFits(currentPiece, currentRotation, currentX + 1, currentY))
            {
                currentX++;
            }
        }
        //Right
        if(keys[1])
        {
            if(PieceFits(currentPiece, currentRotation, currentX - 1, currentY))
            {
                currentX--;
            }
        }
        //Down
        if (keys[2])
        {
            if (PieceFits(currentPiece, currentRotation, currentX, currentY + 1))
            {
                currentY++;
            }
        }
        //ZX
        if(keys[3] || keys[4])
        {
            if (keys[3] && PieceFits(currentPiece, currentRotation + 3, currentX, currentY) && !rotateKeyDown)
            {
                currentRotation = currentRotation + 3;
            }
            else if (keys[4] && PieceFits(currentPiece, currentRotation + 1, currentX, currentY) && !rotateKeyDown)
            {
                currentRotation = currentRotation + 1;
            }
            rotateKeyDown = true;
        }
        else 
        {
            rotateKeyDown = false;
        }

        if(forceDown)
        {
            if(PieceFits(currentPiece, currentRotation, currentX, currentY + 1))
            {
                currentY++;
            }
            else
            {
                //Place piece
                for (int x = 0; x < 4; x++)
                {
                    for (int y = 0; y < 4; y++)
                    {
                        if (tetromino[currentPiece][Rotate(x, y, currentRotation)] == L'X')
                        {
                            field[(currentY + y) * fieldWidth + (currentX + x)] = currentPiece + 1;
                        }
                    }
                }

                pieceCount++;
                if (pieceCount % 10 == 0)
                {
                    if (speed >= 10)
                    {
                        speed--;
                    }
                }

                //Line logic
                for(int y = 0; y < 4; y++)
                {
                    if(currentY + y < fieldHeight - 1)
                    {
                        bool line = true;
                        for(int x = 1; x < fieldWidth - 1; x++)
                        {
                            line &= (field[(currentY + y) * fieldWidth + x]) != 0;
                        }

                        if (line)
                        {
                            for (int x = 1; x < fieldWidth - 1; x++)
                            {
                                field[(currentY + y) * fieldWidth + x] = 8;
                            }
                            lines.push_back(currentY + y);
                        }
                    }
                }

                score += 25;
                if (!lines.empty())
                {
                    score += (1 << lines.size() * 100);
                }

                //new piece
                currentX = fieldWidth / 2;
                currentY = 0;
                currentRotation = 0;
                currentPiece = rand() % 7;

                //Lose check
                gameOver = !PieceFits(currentPiece, currentRotation, currentX, currentY);
            }

            speedCounter = 0;
        }

        //Rendering

        //Draw Field
        for (int x = 0; x < fieldWidth; x++)
        {
            for (int y = 0; y < fieldHeight; y++)
            {
                screen[(y + 2) * screenWidth + (x + 2)] = L" 0123456=#"[field[y * fieldWidth + x]];
            }
        }

        //Draw Current Piece
        for (int x = 0; x < 4; x++) 
        {
            for (int y = 0; y < 4; y++) 
            {
                if (tetromino[currentPiece][Rotate(x, y, currentRotation)] == L'X')
                {
                    screen[(currentY + y + 2) * screenWidth + (currentX + x + 2)] = currentPiece + 48;
                }
            }
        }

        //Draw Score
        swprintf_s(&screen[2 * screenWidth + 16], 16, L"SCORE: %8d", score);

        if (!lines.empty())
        {
            WriteConsoleOutputCharacter(console, screen, screenWidth * screenHeight, { 0,0 }, &dwBytesWritten);
            std::this_thread::sleep_for(std::chrono::milliseconds(400));

            for (auto& v : lines)
                for (int x = 1; x < fieldWidth - 1; x++)
                {
                    for (int y = v; y > 0; y--)
                        field[y * fieldWidth + x] = field[(y - 1) * fieldWidth + x];
                    field[x] = 0;
                }

            lines.clear();
        }

        //Display Frame
        WriteConsoleOutputCharacter(console, screen, screenWidth * screenHeight, { 0,0 }, &dwBytesWritten);
    }
    CloseHandle(console);
    std::cout << "Game Over! You Scored:" << score << std::endl;
    system("pause");

    return 0;
}
