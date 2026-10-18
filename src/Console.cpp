#include "Console.hpp"

#include <cstdlib>
#include <iostream>

namespace {

const size_t kInnerWidth = 70;

size_t DisplayWidth(const std::string& text) {
    size_t width = 0;
    bool insideEscape = false;
    for (unsigned char c : text) {
        if (insideEscape) {
            insideEscape = c != 'm';
        } else if (c == '\033') {
            insideEscape = true;
        } else if ((c & 0xC0) != 0x80) {
            ++width;
        }
    }
    return width;
}

std::string Repeat(const std::string& piece, size_t count) {
    std::string result;
    for (size_t i = 0; i < count; ++i) {
        result += piece;
    }
    return result;
}

void BorderLine(const char* left, const char* right) {
    std::cout << Paint(left + Repeat("─", kInnerWidth) + right, Color::kCyan)
              << "\n";
}

void ContentLine(const std::string& text) {
    std::cout << Paint("│", Color::kCyan) << PadRight(text, kInnerWidth)
              << Paint("│", Color::kCyan) << "\n";
}

} // namespace

std::string Paint(const std::string& text, const char* color) {
    return color + text + Color::kReset;
}

std::string PadRight(const std::string& text, size_t width) {
    size_t current = DisplayWidth(text);
    if (current >= width) {
        return text;
    }
    return text + std::string(width - current, ' ');
}

void ClearScreen() { std::cout << "\033[H\033[2J\033[3J"; }

void DrawWindow(const std::string& title) {
    ClearScreen();
    BorderLine("╭", "╮");
    ContentLine("  " + Paint("◆ " + title, Color::kBold));
    BorderLine("╰", "╯");
    std::cout << "\n";
}

void DrawBox(const std::string& title, const DynamicArray<std::string>& lines) {
    ClearScreen();
    BorderLine("╭", "╮");
    ContentLine("  " + Paint("◆ " + title, Color::kBold));
    BorderLine("├", "┤");
    for (size_t i = 0; i < lines.GetLength(); ++i) {
        ContentLine(lines[i]);
    }
    BorderLine("╰", "╯");
}

void Say(const std::string& text) { std::cout << "  " << text << "\n"; }

void SayHeader(const std::string& text) {
    std::cout << "\n  " << Paint("▸ " + text, Color::kYellow) << "\n";
}

void SayGood(const std::string& text) {
    std::cout << "  " << Paint("✔ " + text, Color::kGreen) << "\n";
}

void SayError(const std::string& text) {
    std::cout << "  " << Paint("✘ " + text, Color::kRed) << "\n";
}

void SayNote(const std::string& text) {
    std::cout << "  " << Paint(text, Color::kDim) << "\n";
}

void Pause() {
    std::cout << "\n  " << Paint("Нажмите Enter, чтобы продолжить...", Color::kDim);
    std::string ignored;
    std::getline(std::cin, ignored);
}

void RunCommand(const std::string& title, const std::string& command) {
    DrawWindow(title);
    SayNote("$ " + command);
    std::cout << "\n";
    std::cout.flush();
    int code = std::system(command.c_str());
    std::cout << "\n";
    if (code == 0) {
        SayGood("Готово");
    } else {
        SayError("Команда завершилась с ошибкой. Выполните make и запускайте "
                 "программу из корня проекта.");
    }
    Pause();
}
