#include "Demo.hpp"

#include <iostream>
#include <stdexcept>
#include <string>

#include "Console.hpp"
#include "DynamicArray.hpp"
#include "MemorySpan.hpp"
#include "ShrdPtr.hpp"
#include "UnqPtr.hpp"

namespace {

void Destroyed(const std::string& name) {
    std::cout << "      " << Paint("↳ вызван ~" + name + "()", Color::kMagenta)
              << "\n";
}

struct Engine {
    int power = 150;
    ~Engine() { Destroyed("Engine"); }
};

struct Radio {
    int volume = 7;
    ~Radio() { Destroyed("Radio"); }
};

struct Car : Engine, Radio {
    ~Car() { Destroyed("Car"); }
};

}

void DemoUnqPtr() {
    DrawWindow("Демо: UnqPtr — единоличное владение, только перемещение");
    SayNote("Car наследуется от Engine и Radio, виртуальных деструкторов нет");

    SayHeader("Создание и перемещение");
    {
        UnqPtr<Car> car(new Car);
        Say("создан UnqPtr<Car>, мощность двигателя = " +
            std::to_string(car->power));
        SayNote("UnqPtr<Car> copy = car;  не скомпилируется: копирование запрещено");

        UnqPtr<Radio> radio(std::move(car));
        Say("UnqPtr<Car> перемещен в UnqPtr<Radio> (второй базовый класс)");
        Say(std::string("старый указатель пуст: ") + (car ? "нет" : "да"));
        Say("громкость радио = " + std::to_string(radio->volume));

        SayHeader("Удаление через указатель на Radio");
    }
    SayGood("объект Car удален целиком: вызваны все три деструктора");

    SayHeader("Массив UnqPtr<int[]>");
    UnqPtr<int[]> numbers(new int[5]);
    std::string text = "значения через operator[]:";
    for (int i = 0; i < 5; ++i) {
        numbers[i] = i * i;
        text += " " + std::to_string(numbers[i]);
    }
    Say(text);
    SayNote("массив освобождается через delete[]");
    Pause();
}

void DemoShrdPtr() {
    DrawWindow("Демо: ShrdPtr — совместное владение, счетчик ссылок");
    SayNote("ShrdPtr хранит только UnqPtr<T>* (ведущий указатель) и size_t* счетчик;");
    SayNote("к объекту обращается и удаляет его только UnqPtr");

    SayHeader("Копирование и приведение к базовым классам");
    ShrdPtr<Car> first{UnqPtr<Car>(new Car)};
    Say("ShrdPtr<Car> создан из UnqPtr, счетчик = " +
        std::to_string(first.UseCount()));
    {
        ShrdPtr<Car> copy = first;
        Say("скопировали ShrdPtr<Car>, счетчик = " +
            std::to_string(first.UseCount()));
        ShrdPtr<Radio> asRadio = first;
        ShrdPtr<Engine> asEngine = first;
        Say("привели к ShrdPtr<Radio> и ShrdPtr<Engine>, счетчик = " +
            std::to_string(first.UseCount()));
        Say("громкость радио = " + std::to_string(asRadio->volume) +
            ", мощность двигателя = " + std::to_string(asEngine->power));
    }
    SayGood("три владельца вышли из области видимости, счетчик = " +
            std::to_string(first.UseCount()) + ", объект еще жив");

    SayHeader("Освобождаем последнего владельца");
    first = ShrdPtr<Car>();
    SayGood("счетчик = " + std::to_string(first.UseCount()) + ", объект удален");
    Pause();
}

void DemoMemorySpan() {
    DrawWindow("Демо: MemorySpan и MsPtr — безопасная арифметика указателей");
    MemorySpan<int> span(5);
    for (size_t i = 0; i < span.GetLength(); ++i) {
        span.Set(i, static_cast<int>(i + 1) * 10);
    }
    Say("MemorySpan<int> длины " + std::to_string(span.GetLength()));

    SayHeader("Обход и арифметика");
    std::string text = "обход через MsPtr:";
    MsPtr<int> end = span.Locate(0) + 5;
    for (MsPtr<int> it = span.Locate(0); it != end; ++it) {
        text += " " + std::to_string(*it);
    }
    Say(text);
    MsPtr<int> last = span.Locate(4);
    Say("последний элемент = " + std::to_string(*last) +
        ", расстояние от начала = " + std::to_string(last - span.Locate(0)));

    SayHeader("Get возвращает копию");
    UnqPtr<int> copy = span.Get(2);
    *copy = 999;
    SayGood("копию изменили на 999, а в массиве по-прежнему " +
            std::to_string(*span.Locate(2)));

    SayHeader("Попытки выйти за границы");
    try {
        *(last + 1);
    } catch (const std::out_of_range& error) {
        SayError(std::string("*(last + 1)  → ") + error.what());
    }
    try {
        span.Locate(0) - 1;
    } catch (const std::out_of_range& error) {
        SayError(std::string("начало - 1   → ") + error.what());
    }
    try {
        span.Locate(10);
    } catch (const std::out_of_range& error) {
        SayError(std::string("Locate(10)   → ") + error.what());
    }
    Pause();
}

void DemoDynamicArray() {
    DrawWindow("Демо: DynamicArray на основе UnqPtr<T[]>");
    SayHeader("Заполнение");
    DynamicArray<ShrdPtr<Car>> cars;
    for (int i = 0; i < 6; ++i) {
        cars.Append(ShrdPtr<Car>(UnqPtr<Car>(new Car)));
    }
    Say("добавили 6 машин, длина = " + std::to_string(cars.GetLength()) +
        " (емкость выросла с 4 до 8)");
    ShrdPtr<Car> shared = cars[2];
    Say("взяли еще одного владельца cars[2], счетчик = " +
        std::to_string(shared.UseCount()));
    try {
        cars.Get(6);
    } catch (const std::out_of_range& error) {
        SayError(std::string("cars.Get(6) → ") + error.what());
    }

    SayHeader("Очищаем массив: 5 машин удаляются, общая остается");
    cars = DynamicArray<ShrdPtr<Car>>();

    SayHeader("Освобождаем последнего владельца cars[2]");
    shared = ShrdPtr<Car>();
    SayGood("все 6 машин удалены, утечек нет");
    Pause();
}

void DemoAll() {
    DemoUnqPtr();
    DemoShrdPtr();
    DemoMemorySpan();
    DemoDynamicArray();
}
