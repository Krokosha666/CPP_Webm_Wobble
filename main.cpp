// temptestingg.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

// Original : https://github.com/OIRNOIR/WebM-Maker-Thing-Idk  , i recommend using theirs.


#include <iostream>
#include <windows.h>
#include <math.h>
#include <string>
#include <filesystem>
#include <cstdio>
#include <memory>
#include <stdexcept>
#include <array>
#include <fstream>
#include <thread>

using namespace std;

// ANYONE CAN MODIFY THIS HERE  \VVVVVV/

bool has8MBmax = true;


void calcFramSize(int index, int maxFrames, int maxFrameSizeX, int maxFrameSizeY, int *curFrameSizeX, int *curFrameSizeY) 
{// it is in for loop, for each frame this gets called to get whatever number desired here, it could be even random

    float progress = float(index) / maxFrames;


    int calcSizeX = clamp(int(floor(maxFrameSizeX * progress)), 5, maxFrameSizeX);

    int calcSizeY = clamp(int(floor(maxFrameSizeY * pow(progress, 2))), 5, maxFrameSizeY);


    *curFrameSizeX = calcSizeX;// ffmpeg can not make video that is 0 by 0 resolution, so don't do that
    *curFrameSizeY = calcSizeY;
}





// !!! EXTREME SPAGHETTI CODE AHEAD !!! \\\/// !!! EXTREME SPAGHETTI CODE AHEAD !!! \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\;


auto getTMPFolder() {//stolen from somewhere, get TEMP folder in appdata

    static char buf[MAX_PATH + 1] = { 0 };
    if (buf[0]) {
        return buf;
    }
    GetTempPathA(sizeof buf, buf);

    return buf;
}

template<typename T>
int arrSize(T const& val) { return sizeof(val) / sizeof(val[0]); }

int randMax(int max) {//random
    return (rand() % max) + 1;
}

int randClamp(int min, int max) {//random in between
    return ((rand() % (max - min + 1))) + min;
}

string randomSTR(int length) {//random string from lowercase letters and numbers

    char letters[] = { 'a','e','i','o','u','y','b','c','d','f','g','h','j','k','l','m','n','p','q','r','s','t','v','w','x','z' };
    int arrlen = arrSize(letters);

    string str = "";

    for (int i = 0; i < length; i++) {

        str = str + letters[randMax(arrlen)];

    }

    return str;

}


string createAppTempFolder() {//to have less crap in main function
    char* tmpPath = getTMPFolder();

    string additional = "webmWobble";

    //it will retry eithere for same name already existing or internal folder creation failure
    int maxRetry = 70;
    int curRetry = 0;
    string newPath;
    do {
        newPath = "";
        string randName = additional + randomSTR(2 + curRetry / 30);

        newPath = string(tmpPath) + randName + "\\";

        curRetry++;
        if (curRetry >= maxRetry) { return "f"; }
    } while (!CreateDirectoryA(newPath.c_str(), NULL));

    if (newPath.substr(newPath.find_last_of("\\"))=="\\") { }
    else if (filesystem::exists(newPath + "\\")) {}
    else { return "f"; }

    //string tmpP4APP;
    return newPath;
}

wstring s2ws(const string& s, bool isUtf8 = false)//stolen from somewhere, string to LPCWSTR converter
{
    int len;
    int slength = (int)s.length() + 1;
    len = MultiByteToWideChar(isUtf8 ? CP_UTF8 : CP_ACP, 0, s.c_str(), slength, 0, 0);
    wstring buf;
    buf.resize(len);
    MultiByteToWideChar(isUtf8 ? CP_UTF8 : CP_ACP, 0, s.c_str(), slength,
        const_cast<wchar_t*>(buf.c_str()), len);
    return buf;
}

string GetCurEXEFilePath() {//get path to current EXE

    char thatPath[MAX_PATH];
    GetModuleFileNameA(NULL, thatPath, MAX_PATH);

    return string(thatPath);
}

string GetCurEXEDirectory()//would be useful in other projects
{
    string EXEpath = GetCurEXEFilePath();

    return EXEpath.substr(0, EXEpath.find_last_of("\\/") + 1);
}


string sysExecWreturn(const char* cmd) {//stolen from somewhere, it's system() but returns output as string instead of that int
    array<char, 128> buffer;
    string result;
    unique_ptr<FILE, decltype(&_pclose)> pipe(_popen(cmd, "r"), _pclose);
    if (!pipe) {
        throw std::runtime_error("popen() failed!");
    }
    while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
        result += buffer.data();
    }
    return result;
}
int countFilesInDir(string dir) {//would be useful in other projects, counts files in folder
    int count = 0;
    for (auto& p : std::filesystem::directory_iterator(dir)) { ++count; }
    return count;
}


#define APP_NAME "testapp"

int main(int argc, char* argv[])// MAIN FUNCTION ------------------------------------------------------------------------------------------------
{
    //"ffmpeg - y - i \"test.mp4\" - vn - c:a libvorbis \"straud.webm\""
      //  "ffmpeg - y - i \"testnoaud.mp4\" - vn - c:a libvorbis \"straud.webm\""

        //std::cout << system("ffmpeg -y -i \"test.mp4\" -vn -c:a libvorbis \"straud.webm\"") << std::endl;

        //std::cout << system("ffmpeg -y -i \"testnoaud.mp4\" -vn -c:a libvorbis \"straud.webm\"") << std::endl;


    //return 0;

    srand((unsigned int)time(NULL));//initalizing randomizer

    //check input

    string vidPath = "";

    if (argc > 1) {
        ifstream test(argv[1]);
        if (test)
        {
            vidPath = argv[1];
        }
    }


    if (vidPath != "") {


        string outputvidPath = vidPath.substr(0, vidPath.find_last_of(".")) + "_new.webm";


        //creating folder in temp to store the images
        string AppTempFold = createAppTempFolder();

        //checking if it was created
        if (AppTempFold == "f") { MessageBoxA(NULL, "temp folder creation failure", APP_NAME, MB_ICONERROR); return -1; }
        else {
            //if creation was succesful, means anything wil be possible, so just create subfolders ----------------------------------------------
            std::cout << "Temp folder was created!" << std::endl;

            string oriFramesDir = AppTempFold + "oriFrames\\";

            string ModiFramesDir = AppTempFold + "modFrames\\";

            CreateDirectoryA(oriFramesDir.c_str(), NULL);

            CreateDirectoryA(ModiFramesDir.c_str(), NULL);

            // HWND amog = GetConsoleWindow();

             //get current directory --------------------------------------------------------------------------

            string EXEdir = GetCurEXEDirectory();

            //Find path for FFmpeg and FFprobe --------------------------------------------------------------------------
            string fMPGpath = EXEdir + "ffmpeg.exe";
            string fPROpath = EXEdir + "ffprobe.exe";
            if (!std::filesystem::exists(fMPGpath) || !std::filesystem::exists(fPROpath)) {
                fMPGpath = "ffmpeg";
                fMPGpath = "ffprobe";
                if ((system("ffmpeg -h") == 1) || (system("ffprobe -h") == 1)) {
                    MessageBoxA(NULL, "Search for FFmpeg with FFprobe returned negative.\nIt has to be either in PATH or is same folder as this .EXE.", APP_NAME, MB_ICONERROR);
                }
            }


            // getting video sizes and fps --------------------------------------------------------------------------

            string thereturnf = sysExecWreturn((fPROpath + " -v error -select_streams v -show_entries stream=r_frame_rate,width,height " + vidPath + "\"").c_str());

            thereturnf = thereturnf.substr(thereturnf.find("]") + 2);

            thereturnf = thereturnf.substr(0, thereturnf.find("[/STREAM]"));

            int vidwid = stoi(thereturnf.substr(thereturnf.find("dth=") + 4, thereturnf.find("height=") - thereturnf.find("dth=") - 5));

            int vidhei = stoi(thereturnf.substr(thereturnf.find("ght=") + 4, thereturnf.find("r_") - thereturnf.find("ght=") - 5));

            string vidfpsMath = thereturnf.substr(thereturnf.find("te=") + 3);

            int vidfps = stoi(vidfpsMath.substr(0, vidfpsMath.find("/"))) / stoi(vidfpsMath.substr(vidfpsMath.find("/") + 1));

            std::cout << "video properties: " << vidwid << "x" << vidhei << " " << vidfps << "fps" << std::endl;

            //extracting audio and checking if even there is audio --------------------------------------------------------------------------

            string audioPlace = AppTempFold + "audio.webm";

            bool hasAudio = system((fMPGpath + " -y -i \"" + vidPath + "\" -vn -c:a libvorbis -loglevel panic \"" + audioPlace + "\"").c_str()) == 0;

            std::cout << "audio " << (hasAudio ? "present" : "missing") << "." << std::endl;

            // extract frames --------------------------------------------------------------------------

            std::cout << "Started extracting frames..." << std::endl;

            system((fMPGpath + " -y -i \"" + vidPath + "\" -loglevel error \"" + oriFramesDir + "%d.png" + "\"").c_str());
            int AllframeCount = countFilesInDir(oriFramesDir);

            std::cout << "Extracted " << AllframeCount << " frames." << std::endl;


            // !!!!! MODIFY frames, also write in txt for ffmpeg concat --------------------------------------------------------------------------

            string txtfile = AppTempFold + "FFMPGconcat.txt";
            ofstream myfile(txtfile, fstream::binary);
            if (myfile.is_open())
            {

                int desiredX = vidwid;
                int desiredY = vidhei;

                bool limitBR = false;//for free discord limit it to 8mb, doen't work as effectively
                int BRkb;
                uintmax_t inputFileSize = filesystem::file_size(vidPath) / 1000;
                if (inputFileSize > 7000) {
                    desiredX = double(desiredX) * 0.7;
                    desiredY = double(desiredY) * 0.7;
                    limitBR = true;
                    int vidseconds = AllframeCount / vidfps;
                    BRkb = 6000 / vidseconds;
                    std::cout << "Size too large for discord 8MB limit, will attempt compression..." << std::endl;
                }


                int lastX = desiredX;
                int lastY = desiredY;

                string laststr = "Modified 0/" + to_string(AllframeCount) + ".";

                string laterwrttfile = "";

                std::cout << "Modified 0/" << AllframeCount << ".";
                for (int i = 1; i <= AllframeCount; i++) {

                    if (i > 1) {
                        // here is the frame rezise part finally bruh
                        calcFramSize(i, AllframeCount, desiredX, desiredY, &lastX, &lastY);

                    }//and ask ffmpeg to resize it
                    string pathtocurmodfile = ModiFramesDir + to_string(i) + ".webm";


                    // not test
                    system((fMPGpath + " -y -i \"" + oriFramesDir + to_string(i) + ".png" + "\" -c:v vp8 -b:v " + (limitBR ? (to_string(BRkb) + "K") : "1M") + " -crf 28 -vf scale=" + to_string(lastX) + "x" + to_string(lastY) + " -aspect " + to_string(lastX) + ":" + to_string(lastY) + " -r " + (to_string(stoi(vidfpsMath.substr(0, vidfpsMath.find("/")))) + "/" + to_string(stoi(vidfpsMath.substr(vidfpsMath.find("/") + 1)))) + " -f webm  -loglevel error \"" + pathtocurmodfile + "\"").c_str());

                    // output progress
                    laterwrttfile = laterwrttfile + "file '" + pathtocurmodfile + "'\n";

                    cout << string(laststr.length(), '\b');
                    laststr = "Modified " + to_string(i) + "/" + to_string(AllframeCount) + ".";
                    cout << laststr;

                }
                std::cout << std::endl;
                myfile << laterwrttfile;
                myfile.flush();

                //ffmpeg -y -f concat -safe 0 -i "${workLocations.tempConcatList}"${audioFlag ? ` -i "${workLocations.tempAudio}" ` : ' '}-c copy "${workLocations.outputFile}"

                

                bool concatFine = 0 == system((fMPGpath + " -y -f concat -safe 0 -i \"" + txtfile + "\"" + (hasAudio ? " -i \"" + audioPlace + "\"" : "") + " -c copy -loglevel error \"" + outputvidPath + "\"").c_str());;

                if (!concatFine) {
                    MessageBoxA(NULL, "failure detected when combining everything into .webm", APP_NAME, MB_ICONERROR);
                }
                else {

                    if (filesystem::file_size(outputvidPath) > 80000000) {
                        std::cout << "Sorry, size is bigger than 8MB, try to lower resolution idk." << std::endl;
                    }

                    std::cout << ".webm has been created." << std::endl;
                }

                myfile.close();
            }
            else {
                MessageBoxA(NULL, "failed to create text file", APP_NAME, MB_ICONERROR);
            }

            // TEMPORARY asking to stop app, (to see folder in TEMP) --------------------------------------------------------------------------

            //int amogus = MessageBoxA(NULL, "stop app?", APP_NAME, MB_ICONQUESTION);

            //deleting that folder in TEMP --------------------------------------------------------------------------


            //try {
            filesystem::remove_all(AppTempFold);
            //} catch () {  }

            std::wstring wrapper = s2ws(AppTempFold, false);
            LPCWSTR result = wrapper.c_str();

            //MessageBox(NULL, result, L"quick check if in temp", MB_ICONQUESTION);


            //std::cout << "Last error code: " << GetLastError() << std::endl;

            std::cout << "Shutting down." << std::endl;

            return 0;
        }

    } else { MessageBoxA(NULL, "input file required", APP_NAME, MB_ICONERROR); }

}


// Run program: Ctrl + F5 or Debug > Start Without Debugging menu
// Debug program: F5 or Debug > Start Debugging menu

// Tips for Getting Started: 
//   1. Use the Solution Explorer window to add/manage files
//   2. Use the Team Explorer window to connect to source control
//   3. Use the Output window to see build output and other messages
//   4. Use the Error List window to view errors
//   5. Go to Project > Add New Item to create new code files, or Project > Add Existing Item to add existing code files to the project
//   6. In the future, to open this project again, go to File > Open > Project and select the .sln file




/*
string randomName(int length) {// stolen from stackoverflow, would be useful

    char consonents[] = { 'b','c','d','f','g','h','j','k','l','m','n','p','q','r','s','t','v','w','x','z' };
    char vowels[] = { 'a','e','i','o','u','y' };

    string name = "";

    int random = rand() % 2;
    int count = 0;

    for (int i = 0; i < length; i++) {

        if (random < 2 && count < 2) {
            name = name + consonents[rand() % 19];
            count++;
        }
        else {
            name = name + vowels[rand() % 5];
            count = 0;
        }

        random = rand() % 2;

    }

    //std::cout << size << std::endl;

    return name;

}
*/