#include <iostream>
#include <cstdlib>
#include <fstream>
#include <bitset>
#include <sstream>
#ifdef _WIN32
    #include <conio.h>
    #include <windows.h>
    void enable() {
        HANDLE hStdin = GetStdHandle(STD_INPUT_HANDLE);
        DWORD mode;
        GetConsoleMode(hStdin, &mode);
        SetConsoleMode(hStdin, mode & ~ENABLE_ECHO_INPUT);
    }
    void disable() {
        HANDLE hStdin = GetStdHandle(STD_INPUT_HANDLE);
        DWORD mode;
        GetConsoleMode(hStdin, &mode);
        SetConsoleMode(hStdin, mode | ENABLE_ECHO_INPUT);
    }

#else
    #include <termios.h>
    #include <unistd.h>
    #include <sys/ioctl.h>
    int _getch() {
        int ch;
        ch = getchar();
        return ch;
    }

    int _kbhit() {
        static const int STDIN = 0;
        static bool initialized = false;
    
        if (!initialized) {
            termios term;
            tcgetattr(STDIN, &term);
            term.c_lflag &= ~ICANON;
            tcsetattr(STDIN, TCSANOW, &term);
            setbuf(stdin, NULL);
            initialized = true;
        }
    
        int bytesWaiting;
        ioctl(STDIN, FIONREAD, &bytesWaiting);
        return bytesWaiting > 0;
    }
    void enable() {
        static termios oldt, newt;
        tcgetattr(STDIN_FILENO, &oldt);
        newt = oldt;
        newt.c_lflag &= ~(ICANON | ECHO);
        tcsetattr(STDIN_FILENO, TCSANOW, &newt);

    }
    void disable() {
        static termios oldt;
        tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    }
#endif
void parse(std::string &insiders, std::string &word, const int key, bool &edit, int &index) {
    
    std::cout << '\n' << word << std::endl;
    int bit_i = 0;
    std::cout << "Enter bits: ";
    std::cout << (char)key;
    if ((key == 48 || key == 49) && insiders[index] != ' ') {
        insiders[index] = (char)key;
        word[bit_i] = (char)key;
        index++;
        bit_i++;
    }
    else {
        std::cout << "\n Editing finished \n";
        std::cout << "press 's' to see a bits, press 'e' to edit this bits" << std::endl;
        edit = false;
        word = "";
        bit_i = 0;
    }
}
void write_to_file(std::string &insiders, const char *filename) {
    std::ofstream output(filename, std::ios::binary);
    std::istringstream iss(insiders);
    std::string byte_str;
    while (iss >> byte_str) {
        std::bitset<8> bits(byte_str);
        unsigned char byte = static_cast<unsigned char>(bits.to_ulong());
        output.write(reinterpret_cast<char*>(&byte), sizeof(byte));
    }
    output.close();
}
int main(int argc, char *argv[]) {
    if (argc != 2) {
        std::cout << "Only 2 arguments, example: " << argv[0] << " <file.exe>" << std::endl;
        return 0;
    }
    std::ifstream file(argv[1], std::ios::binary);
    std::string insiders = "";
    std::string word = "";
    unsigned char byte;
    while (file.read(reinterpret_cast<char*>(&byte), 1)) {
        std::bitset <8> bits(byte);
        insiders += bits.to_string();
        insiders += " ";
    }
    file.close();
    int index = 0;
    int index_tmp = 0;
    bool edit = false;
    bool is_first = true;
    int key = 0;
    int iteration = 0;
    int bit_i = 0;
    std::cout << "press 's' to see a bits, press 'e' to edit this bits, press 'a' to see all values at once, press 'w' to write edited values in the file\n";
    enable();
    while (true) {
        iteration++;
        if (_kbhit()) {
            key = _getch();
            if (edit) {
                if (is_first) {
                    is_first = false;
                }
                std::cout << "Entered bits: ";
                std::cout << (char)key;
                if ((key == 48 || key == 49) && insiders[index] != ' ') {
                    insiders[index] = (char)key;
                    word[bit_i] = (char)key;
                    index++;
                    bit_i++;
                    std::cout << '\n' << word << " " << static_cast<char>(std::stoi(word, nullptr, 2)) << '\n';
                }
                else {
                    std::cout << "\n\nEditing finished\n";
                    std::cout << "press 's' to see a bits, press 'e' to edit this bits, press 'a' to see all values at once, press 'w' to write edited values in the file" << std::endl;
                    edit = false;
                    word = "";
                    bit_i = 0;
                }
            }
            else if (key == 115) {
                word = "";
                while (insiders[index] != ' ') {
                    std::cout << insiders[index];
                    word += insiders[index];
                    index++;
                }
                std::cout << ' ' << std::endl;
                index++;
            }
            else if (key == 101) {
                edit = true;
                std::cout << " \nEntered editing mode\n";
                std::cout << "(Press 1 or 0 to edit bits)\n\n";
                index_tmp = index;
                while (insiders[index_tmp] != ' ') {
                    word += insiders[index_tmp];
                    index_tmp++;
                }
                std::cout << "Current value: " << word << '\n';
                if (is_first || iteration == 2) {
                    index = 0;
                    continue;
                }
                else {
                    index -= 2;
                    while (insiders[index] != ' ') {
                        index--;
                    }
                    index++;
                    continue;
                }
            }
            else if (key == 97) {
                for (char x : insiders) {
                    if (isspace(x)) {
                        std::cout << " ";
                    }
                    else {
                        std::cout << x;
                    }
                }
                std::cout << std::endl;
                break;
            }
            else if (key == 119) {
                write_to_file(insiders, argv[1]);
                std::cout << "File overwrited successfully" << std::endl;
                break;
            }
        }
    }
    disable();
    return 0;
}
