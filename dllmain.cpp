// dllmain.cpp : Defines the entry point for the DLL application.
#include "pch.h"
#include "offsets.h"
#include <Windows.h>
#include <iostream>
#include <sstream> //for converting name into cstring
#include <unordered_map>     //for caching localPlayer Variables to fix crashing and improve perofrmance
#include <thread>
#include <chrono> //included to try and slow down ridiciouslous draw speeds.

struct Vec2 { float x, y; };
struct Vec4 { float x,y,z,w; };

Vec3 entPlayerBody;
Vec3 entPlayerBody2;
Vec3 locPlayerBody;
Vec3 nuEntPlayerBody;

uintptr_t getBase() {
    return (uintptr_t)GetModuleHandle(L"sauerbraten.exe");
}
int getNumPlayers(){
    return *(int*)(getBase() + 0x346C9C);
}
int getTeamNum() {
    return *(int*)(getBase() + 0x34D864);
}

//esp variables
DWORDLONG viewMatrix = getBase() + 0x32D040;
HWND hwndSauerbratenClient;
HBRUSH Brush;
HDC hdcSauerbratenClient;
HFONT Font;
float Matrix[16];
COLORREF TextCOLOR;
COLORREF TextCOLORRED;

//esp functions
void DrawFilledRect(int x, int y, int w, int h)
{
    RECT rect = { x,y,x + w,y + h };
    FillRect(hdcSauerbratenClient, &rect, Brush);
}
void DrawBorderBox(int x, int y, int w, int h, int thickness)
{
    DrawFilledRect(x, y, w, thickness);
    DrawFilledRect(x, y, thickness, h);
    DrawFilledRect((x + w), y, thickness, h);
    DrawFilledRect(x, y + h, w + thickness, thickness);
}
void DrawLine(int targetX, int targetY)
{
    MoveToEx(hdcSauerbratenClient, 464, 267, NULL);
    LineTo(hdcSauerbratenClient, targetX, targetY);
}
void DrawString(int x, int y, COLORREF color, const char* text)
{
    SetTextAlign(hdcSauerbratenClient, TA_CENTER | TA_NOUPDATECP);
    SetBkColor(hdcSauerbratenClient, RGB(0, 0, 0));
    SetBkMode(hdcSauerbratenClient, TRANSPARENT);
    SetTextColor(hdcSauerbratenClient, color);
    SelectObject(hdcSauerbratenClient, Font);
    TextOutA(hdcSauerbratenClient, x, y, text, strlen(text));
    DeleteObject(Font);
}
bool WorldToScreen(Vec3 pos, Vec2& screen, float matrix[16], int windowWidth, int windowHeight) // 3D to 2D
{
    //Matrix-vector Product, multiplying world(eye) coordinates by projection matrix = clipCoords
    Vec4 clipCoords;
    clipCoords.x = pos.x * matrix[0] + pos.y * matrix[4] + pos.z * matrix[8] + matrix[12];
    clipCoords.y = pos.x * matrix[1] + pos.y * matrix[5] + pos.z * matrix[9] + matrix[13];
    clipCoords.z = pos.x * matrix[2] + pos.y * matrix[6] + pos.z * matrix[10] + matrix[14];
    clipCoords.w = pos.x * matrix[3] + pos.y * matrix[7] + pos.z * matrix[11] + matrix[15];

    if (clipCoords.w < 0.1f)
        return false;

    //perspective division, dividing by clip.W = Normalized Device Coordinates
    Vec3 NDC;
    NDC.x = clipCoords.x / clipCoords.w;
    NDC.y = clipCoords.y / clipCoords.w;
    NDC.z = clipCoords.z / clipCoords.w;

    //Transform to window coordinates
    screen.x = (windowWidth / 2 * NDC.x) + (NDC.x + windowWidth / 2);
    screen.y = -(windowHeight / 2 * NDC.y) + (NDC.y + windowHeight / 2);
    return true;
}

DWORD WINAPI HackThread(HMODULE hModule)
{
    AllocConsole();
    FILE* f;
    freopen_s(&f, "CONOUT$", "w", stdout);

    uintptr_t modBaze = (uintptr_t)GetModuleHandle(L"sauerbraten.exe");
    std::cout << "pompompurin\n";
    std::cout << std::hex << std::uppercase << "viewMatrixAddy: " << viewMatrix << std::endl;

    //global variables
    bool bHealth = false, bAmmo = false, bRecoil = false, entHealth = false;

    while (true)
    {
        uintptr_t* localPlayer = *(uintptr_t**)(*(uintptr_t**)(getBase() + 0x3472E0) + 0x0);
        uintptr_t* serverBase = *(uintptr_t**)(modBaze + 0x2A2560);
        uintptr_t  viewAngleYaw, viewAnglePitch, viewAngleRoll; //cached values.. not a redefinition
        uintptr_t* entBodyX = 0; uintptr_t* entBodyY = 0; uintptr_t* entBodyZ = 0; //cached values.. not a redefinition
            float* localPlayerBodyX = 0; float* localPlayerBodyY = 0; float* localPlayerBodyZ = 0; //cached values.. not a redefinition
        uintptr_t* nUentBodyX = 0; uintptr_t* nUentBodyY = 0; uintptr_t* nUentBodyZ = 0; //cached values.. not a redefinition
        uintptr_t* entBody2X = 0; uintptr_t* entBody2Y = 0; uintptr_t* entBody2Z = 0; //cached values.. not a redefinition
        uintptr_t* nUent2BodyX = 0; uintptr_t* nUent2BodyY = 0; uintptr_t* nUent2BodyZ = 0; //cached values.. not a redefinition

        //caching viewAngles
        std::unordered_map <uintptr_t*, Vec3> localViewAngles;
        auto it = localViewAngles.find(localPlayer);
        if (it != localViewAngles.end()) {
            Vec3 viewAngles = it->second;
        }
        else {
            viewAngleYaw = ((uintptr_t)serverBase + 0x3C);
            viewAnglePitch = ((uintptr_t)serverBase + 0x40);
            viewAngleRoll = ((uintptr_t)serverBase + 0x44);
        }
        //caching viewAngles end..

        if (GetAsyncKeyState(VK_NUMPAD8) & 1)
        {
            bHealth = !bHealth;
        }
        if (GetAsyncKeyState(VK_END) & 1)
        {
            break;
        }

        //esp stuff
        TextCOLOR = RGB(255, 255, 255);
        hwndSauerbratenClient = FindWindow(0, L"Cube 2: Sauerbraten");
        //esp stuff end

        ent* aimLocalPlayer = (ent*)*(uintptr_t**)(*(uintptr_t**)(getBase() + 0x3472E0) + 0x0);
        uintptr_t entityList = *(uintptr_t*)(getBase() + 0x3472E0);

        if (entityList)
        {
            aimLocalPlayer->health = 86.2;
            aimLocalPlayer->armor = 8662;
            //esp stuff
            memcpy(&Matrix, (LPCVOID*)(viewMatrix), sizeof(Matrix));
            hdcSauerbratenClient = GetDC(hwndSauerbratenClient);
            Vec2 vScreen;
            Vec2 vHead;
            //esp stuff end

            ent* nEntity = aimLocalPlayer;
            float dist = 1000.f;

            for (int i = 1; i < getNumPlayers(); i++) {
                ent* entity = *(ent**)(entityList + 0x8 * i);
                uintptr_t uEntity = (uintptr_t)entity;

                uintptr_t* localPlayer = *(uintptr_t**)(*(uintptr_t**)(getBase() + 0x3472E0) + 0x0);

                //caching entitiesPos
                std::unordered_map <uintptr_t, Vec3> entityPos;
                auto epos = entityPos.find(uEntity);
                if (epos != entityPos.end())
                {
                    Vec3 ePos = epos->second;
                } else {
                    entBodyX = (uintptr_t*)(uEntity + 0x394);
                    entBodyY = (uintptr_t*)(uEntity + 0x398);
                    entBodyZ = (uintptr_t*)(uEntity + 0x39C);
                }
                float epBX = *(float*)entBodyX; float epBY = *(float*)entBodyY; float epBZ = *(float*)entBodyZ;
                Vec3 entityPlayerBody = (Vec3(epBX, epBY, epBZ));
                //caching entitesPosEnd

                //caching localPlayerPos
                std::unordered_map <uintptr_t*, Vec3> localPlayerCachePos;
                auto lpos = localPlayerCachePos.find(localPlayer);
                if (lpos != localPlayerCachePos.end())
                {
                    Vec3 lPos = lpos->second;
                } else {
                    localPlayerBodyX = (float*)((uintptr_t)localPlayer + 0x394);
                    localPlayerBodyY = (float*)((uintptr_t)localPlayer + 0x398);
                    localPlayerBodyZ = (float*)((uintptr_t)localPlayer + 0x39C);
                }
                float lpBX = *localPlayerBodyX; float lpBY = *localPlayerBodyY; float lpBZ = *localPlayerBodyZ;
                Vec3 localPlayerBody = (Vec3(lpBX, lpBY, lpBZ));
                //cashing localPlayerPosEnd

                int indexNum = 128; //using this for bot's name in string.

                std::ostringstream oss;
                oss << "bot [" << indexNum + (i-1) << "]";
                std::string nameStr = oss.str();
                const char* name = nameStr.c_str(); //trick to get it into a const char* string.

                float currDist = (localPlayerBody - entityPlayerBody).hypo3();

                //esp stuff
                if (WorldToScreen(entityPlayerBody, vScreen, Matrix, 928, 534));
                {
                        //window height and width
                        #define VIRTUAL_SCREEN_WIDTH 928.f
                        #define VIRTUAL_SCREEN_HEIGHT 534.f

                        //variable specific to each game
                        #define PLAYER_HEIGHT 2.5f
                        #define PLAYER_WIDTH 1.5f

                        //multiply players width by this to get the height
                        #define PLAYER_ASPECT_RATIO PLAYER_HEIGHT / PLAYER_WIDTH


                        //In ESP:
                        //4000 is a magic number that works for this games unit of measure
                        //It has to be modified for every new game you use this framework on
                        float scale = (1500 / currDist) * (928 / VIRTUAL_SCREEN_WIDTH);
                        float x = vScreen.x - scale;
                        float y = vScreen.y - scale * 2 * PLAYER_ASPECT_RATIO;
                        float width = scale * 2;
                        float height = scale * PLAYER_ASPECT_RATIO * 2;

                        if (entity->health > 50 && entity->health <= 200)
                        {
                            Brush = CreateSolidBrush(RGB(0, 255, 0));
                            DrawBorderBox(x, y, width, height, 1.f);
                            DeleteObject(Brush);

                            char healthChar[255];
                            sprintf_s(healthChar, sizeof(healthChar), "%i", (int)entity->health);
                            DrawString(vScreen.x, vScreen.y + 12, RGB(0, 255, 0), healthChar);
                            DrawString(vScreen.x, vScreen.y, RGB(255, 255, 255), name);
                        }
                        else if (entity->health > 25 && entity->health < 50)
                        {
                            Brush = CreateSolidBrush(RGB(255, 165, 0));
                            DrawBorderBox(x, y, width, height, 1.f);
                            DeleteObject(Brush);

                            char healthChar[255];
                            sprintf_s(healthChar, sizeof(healthChar), "%i", (int)entity->health);
                            DrawString(vScreen.x, vScreen.y + 12, RGB(255, 165, 0), healthChar);
                            DrawString(vScreen.x, vScreen.y, RGB(255, 255, 255), name);
                        }
                        else if (entity->health > 0 && entity->health <= 25)
                        {
                            Brush = CreateSolidBrush(RGB(255, 0, 0));
                            DrawBorderBox(x, y, width, height, 1.f);
                            DeleteObject(Brush);

                            char healthChar[255];
                            sprintf_s(healthChar, sizeof(healthChar), "%i", (int)entity->health);
                            DrawString(vScreen.x, vScreen.y + 12, RGB(255, 0, 0), healthChar);
                            DrawString(vScreen.x, vScreen.y, RGB(255, 255, 255), name);
                        }
                        else
                        {
                            Brush = CreateSolidBrush(RGB(255, 255, 255));
                            DrawBorderBox(x, y, width, height, 1.f);
                            DeleteObject(Brush);

                            char healthChar[255];
                            sprintf_s(healthChar, sizeof(healthChar), "%i", (int)entity->health);
                            DrawString(vScreen.x, vScreen.y + 12, RGB(255, 255, 255), healthChar);
                            DrawString(vScreen.x, vScreen.y, RGB(255, 255, 255), "dead");
                        }
                        

                        if (GetAsyncKeyState(VK_F2) & 1)
                        {
                            DrawLine(vScreen.x, vScreen.y);
                        }
                    }
                

                locPlayerBody = localPlayerBody;
                entPlayerBody = entityPlayerBody;
                
                if (entity == NULL)
                {
                    std::cout << "NULL\n";
                    break;
                }
                
                if ((entityPlayerBody - localPlayerBody).hypo3() < dist && entity->health > 0)
                {
                    nEntity = entity;
                    uintptr_t nUEntity = (uintptr_t)nEntity;

                    //caching new Entities Pos
                    std::unordered_map <uintptr_t, Vec3> nuEntCachePos;
                    auto nupos = nuEntCachePos.find(nUEntity);
                    if (nupos != nuEntCachePos.end())
                    {
                        Vec3 nuPos = nupos->second;
                    } else {
                        nUentBodyX = (uintptr_t*)(nUEntity + 0x394);
                        nUentBodyY = (uintptr_t*)(nUEntity + 0x398);
                        nUentBodyZ = (uintptr_t*)(nUEntity + 0x39C);
                    }
                    float nuepBX = *(float*)nUentBodyX; float nuepBY = *(float*)nUentBodyY; float nuepBZ = *(float*)nUentBodyZ;
                    Vec3 nuEntityPlayerBody = (Vec3(nuepBX, nuepBY, nuepBZ));
                    nuEntPlayerBody = nuEntityPlayerBody;
                    //caching new Entities pos end

                    dist = (localPlayerBody - nuEntityPlayerBody).hypo3();
                }

                ent* entity2 = entity + (0x2 * (i-1)); //offset into the second entity of the entity list.
                uintptr_t uEntity2 = (uintptr_t)entity2;

                //caching ent2 pos
                std::unordered_map <uintptr_t, Vec3> ent2CachePos;
                auto e2pos = ent2CachePos.find(uEntity2);
                if (e2pos != ent2CachePos.end())
                {
                    Vec3 e2Pos = e2pos->second;
                }
                else
                {
                    entBody2X = (uintptr_t*)(uEntity2 + 0x394);
                    entBody2Y = (uintptr_t*)(uEntity2 + 0x398);
                    entBody2Z = (uintptr_t*)(uEntity2 + 0x39C);
                }
                float ep2BX = *(float*)entBody2X; float ep2BY = *(float*)entBody2Y; float ep2BZ = *(float*)entBody2Z;
                Vec3 epBody2 = (Vec3(ep2BX, ep2BY, ep2BZ));
                //caching end2 pos end.
                entPlayerBody2 = epBody2;

                if (entity2 == NULL)
                {
                    std::cout << "NULL";
                    break;
                }
                if ((epBody2 - localPlayerBody).hypo3() < dist && entity2->health > 0)
                {
                    nEntity = entity;
                    uintptr_t nUEntity = (uintptr_t)nEntity;
                   
                    std::unordered_map <uintptr_t, Vec3> nuEnt2CachePos;
                    auto nu2pos = nuEnt2CachePos.find(nUEntity);
                    if (nu2pos != nuEnt2CachePos.end())
                    {
                        Vec3 nu2Pos = nu2pos->second;
                    } else {
                        nUent2BodyX = (uintptr_t*)(nUEntity + 0x394);
                        nUent2BodyY = (uintptr_t*)(nUEntity + 0x398);
                        nUent2BodyZ = (uintptr_t*)(nUEntity + 0x39C);
                    }
                    float nuepBX = *(float*)nUent2BodyX; float nuepBY = *(float*)nUent2BodyY; float nuepBZ = *(float*)nUent2BodyZ;
                    Vec3 nuEntityPlayerBody = (Vec3(nuepBX, nuepBY, nuepBZ));

                    nuEntPlayerBody = nuEntityPlayerBody;

                    dist = (localPlayerBody - nuEntityPlayerBody).hypo3();
                }
            }
            Vec3 deltaChange = (nuEntPlayerBody - locPlayerBody);

            float yaw = atan2f(deltaChange.y, deltaChange.x) * 180 / 3.141592653;
            float hyp = sqrt(deltaChange.x * deltaChange.x + deltaChange.y * deltaChange.y);
            float pitch = atan2f(deltaChange.z, hyp) * 180 / 3.141592653;

            pitch + 90;



            if (GetAsyncKeyState(VK_RBUTTON) & 0x8001)
            {
                *(float*)viewAnglePitch = pitch;
                *(float*)viewAngleYaw = yaw + 270;
            }

        }
        Sleep(2);
    }
    FreeConsole();
    fclose(f);
    FreeLibraryAndExitThread(hModule, 0);
    
    return 0;
}


BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
                     )
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
        CloseHandle(CreateThread(nullptr, 0, (LPTHREAD_START_ROUTINE)HackThread, hModule, 0, nullptr));
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}

