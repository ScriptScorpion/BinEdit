#include <iostream>
#include <cstdlib>
#include <cctype>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <list>
#include <algorithm>
#include <bitset>
#include <sstream>
#include "key_map.h"

#ifdef _WIN32
    #include <conio.h>
    #include <windows.h>
    void enable(const bool state) {
        if (state) {
            static HANDLE hStdin = GetStdHandle(STD_INPUT_HANDLE);
            static DWORD mode;
            GetConsoleMode(hStdin, &mode);
            SetConsoleMode(hStdin, mode & ~ENABLE_ECHO_INPUT);
        }
        else {
            SetConsoleMode(hStdin, mode | ENABLE_ECHO_INPUT);
        }
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
    void enable(const bool state) {
        static termios oldt, newt;
        if (state) {
            tcgetattr(STDIN_FILENO, &oldt);
            newt = oldt;
            newt.c_lflag &= ~(ICANON | ECHO);
            tcsetattr(STDIN_FILENO, TCSANOW, &newt);
        }
        else {
            tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
        }

    }
#endif

void out_of_the_bounds(const std::list<std::string>::iterator val, const std::list <std::string> all_vals) {
    if (val == all_vals.end()) {
        enable(false);
        std::exit(0);
    }
}
void write_to_file(const std::list <std::string> all_vals, const char *filename) {
    std::ofstream output(filename, std::ios::binary);
    for (const std::string &s : all_vals) {
        unsigned char byte = static_cast<unsigned char>(std::stoi(s, nullptr, 2));
        output.write(reinterpret_cast<char*>(&byte), sizeof(byte));
    }
    output.close();
}
std::string remove_special_chars(const std::string name) {
    std::string name_copy;
    name_copy.reserve(name.length());
    for (const char c : name) {
        if (std::isalnum(c)) {
            name_copy += c;
        }
    }
    return name_copy;
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        std::cerr << "Only 2 arguments are possible " << "(type <" << argv[0] << " -h> for more help)" << std::endl;
        return 1;
    }
    if (strcmp(argv[1], "-h") == 0) {
        std::cout << "Usage: " << argv[0] << " <FILE>\n\n";
        std::cout << "Keybindings: \n\n";
        std::cout << "    a - list all and exit\n";
        std::cout << "    b - go backward\n";
        std::cout << "    e - edit current line\n";
        std::cout << "    q - exit from the program\n";
        std::cout << "    s - go forward\n";
        std::cout << "    w - write to the file\n";
        std::cout << std::endl;
        return 0;
    }

    std::ifstream file(argv[1], std::ios::binary);
    if (!file.good()) {
        std::cerr << "Error: file don't exists" << std::endl;
        return 1;
    }

    std::filesystem::path file_ext1(argv[0]);
    std::filesystem::path file_ext2(argv[1]);
    if (remove_special_chars(file_ext1.replace_extension("").string()) == remove_special_chars(file_ext2.replace_extension("").string())) {
        std::cerr << "Error: incorrect input file" << std::endl;
        return 1;
    }

    std::string word {};
    std::list <std::string> all_values;
    unsigned char byte;
    while (file.read(reinterpret_cast<char*>(&byte), 1)) {
        std::bitset <8> bits(byte);
        all_values.push_back(bits.to_string());
    }
    file.close();
    std::list<std::string>::iterator index {all_values.begin()}; 
    bool edit {false};
    bool running {true};
    int key {0};
    unsigned int pos {1};
    uint8_t inline_index {0};
    enable(true);
    std::cout << pos << '/' << all_values.size() << '\n';
    while (running) {
        out_of_the_bounds(index, all_values);
        if (!edit) {
            std::cout << "Current value: " << *index << " " << static_cast<char>(std::stoi(*index, nullptr, 2)) << '\n';
        }

        key = _getch();

        if (edit) {
            std::cout << "Entered bits: ";
            std::cout << (char)key;
            if ((key == Keys::zero || key == Keys::one) && inline_index < word.size()) {
                word[inline_index] = static_cast<char>(key);
                std::cout << '\n' << word << " " << static_cast<char>(std::stoi(word, nullptr, 2)) << '\n';
                ++inline_index;
            }
            else {
                if (word != (*index))  {
                    std::replace(all_values.begin(), all_values.end(), *index, word);
                }
                std::cout << "\n\nEditing finished\n";
                edit = false;
                inline_index = 0;
                word.clear();
            }
            continue;
        }
        else {
            switch(key) {
                case Keys::s: {
                    if (pos < all_values.size()) {
                        ++pos;
                        ++index;
                        std::cout << pos << '/' << all_values.size() << '\n';
                    }
                    break;
                }

                case Keys::b: {
                    if (pos > 1) {
                        --pos;
                        --index;
                        std::cout << pos << '/' << all_values.size() << '\n';
                    }
                    break;
                }

                case Keys::e: { 
                    edit = true;
                    word = (*index);
                    std::cout << " \nEntered editing mode\n";
                    std::cout << "(Press 1 or 0 to edit bits)\n\n";
                    break;
                }

                case Keys::w: {
                    write_to_file(all_values, argv[1]);
                    std::cout << "File overwrited successfully" << std::endl;
                    running = false;
                    break;
                }

                case Keys::q: {
                    running = false;
                    break;
                }
                case Keys::a: { 
                    for (const std::string &s : all_values) {
                        std::cout << s << " ";
                    }
                    std::cout << std::endl;
                    running = false;
                    break;
                }
                
                default: {
                    std::cout << "\nInvalid command\n";
                    break;
                }
            }
        }
    }
    enable(false);
    return 0;
}