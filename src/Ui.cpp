#include "Ui.hpp"

#include <iostream>
#include <string>

#include "Benchmark.hpp"
#include "Console.hpp"
#include "Demo.hpp"
#include "DynamicArray.hpp"
#include "Tests.hpp"

namespace {

std::string Section(const std::string& name) {
    return "  " + Paint(name, Color::kGreen);
}

std::string Item(const std::string& key, const std::string& name,
                 const std::string& description) {
    return "   " + Paint(key, Color::kYellow) + "  " +
           Paint(PadRight(name, 16), Color::kBold) +
           Paint(description, Color::kDim);
}

DynamicArray<std::string> MenuLines() {
    DynamicArray<std::string> lines;
    lines.Append("");
    lines.Append(Section("ДЕМОНСТРАЦИЯ"));
    lines.Append(Item("1", "UnqPtr", "перемещение, массивы, удаление без virtual"));
    lines.Append(Item("2", "ShrdPtr", "счетчик ссылок, несколько владельцев"));
    lines.Append(Item("3", "MemorySpan", "MsPtr и проверка границ"));
    lines.Append(Item("4", "DynamicArray", "контейнер на UnqPtr<T[]>"));
    lines.Append(Item("5", "Все демо", "пункты 1-4 подряд"));
    lines.Append("");
    lines.Append(Section("ПРОВЕРКИ"));
    lines.Append(Item("6", "Тесты", "все блоки, 52 теста"));
    lines.Append(Item("7", "Нагрузка", "быстрый тест, от 10 до 100 000 объектов"));
    lines.Append(Item("8", "Нагрузка", "полный тест, от 10 до 10 000 000 объектов"));
    lines.Append(Item("9", "Графики", "построить по results/results.csv"));
    lines.Append("");
    lines.Append(Item("0", "Выход", ""));
    lines.Append("");
    return lines;
}

void PrintMenu(const std::string& hint) {
    DrawBox("Умные указатели · ДЗ №1", MenuLines());
    if (!hint.empty()) {
        std::cout << "  " << Paint(hint, Color::kRed) << "\n";
    }
    std::cout << "\n  " << Paint("▸ Ваш выбор: ", Color::kCyan);
    std::cout.flush();
}

void ShowTests() {
    DrawWindow("Тесты: UnqPtr, ShrdPtr, DynamicArray, MemorySpan, MsPtr");
    RunAllTests();
    std::cout << "\n";
    SayGood("Готово");
    Pause();
}

void ShowBenchmark(bool quick) {
    DrawWindow(quick ? "Нагрузочный тест (быстрый)"
                     : "Нагрузочный тест (полный, около 20 секунд)");
    bool saved = RunBenchmark(quick);
    std::cout << "\n";
    if (saved) {
        SayGood("Готово");
    } else {
        SayError("Результаты не сохранены");
    }
    Pause();
}

bool HandleChoice(const std::string& choice) {
    if (choice == "1") {
        DemoUnqPtr();
    } else if (choice == "2") {
        DemoShrdPtr();
    } else if (choice == "3") {
        DemoMemorySpan();
    } else if (choice == "4") {
        DemoDynamicArray();
    } else if (choice == "5") {
        DemoAll();
    } else if (choice == "6") {
        ShowTests();
    } else if (choice == "7") {
        ShowBenchmark(true);
    } else if (choice == "8") {
        ShowBenchmark(false);
    } else if (choice == "9") {
        RunCommand("Графики", "python3 scripts/plot.py");
    } else {
        return false;
    }
    return true;
}

} // namespace

void RunUi() {
    std::string choice;
    std::string hint;
    while (true) {
        PrintMenu(hint);
        if (!std::getline(std::cin, choice) || choice == "0") {
            break;
        }
        bool known = HandleChoice(choice);
        hint = known ? "" : "Нет пункта «" + choice + "», введите число от 0 до 9";
    }
    ClearScreen();
}
