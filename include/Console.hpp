#pragma once

#include <string>

#include "DynamicArray.hpp"

namespace Color {

inline const char* const kReset = "\033[0m";
inline const char* const kBold = "\033[1m";
inline const char* const kDim = "\033[2m";
inline const char* const kRed = "\033[31m";
inline const char* const kGreen = "\033[32m";
inline const char* const kYellow = "\033[33m";
inline const char* const kMagenta = "\033[35m";
inline const char* const kCyan = "\033[36m";

}

std::string Paint(const std::string& text, const char* color);
std::string PadRight(const std::string& text, size_t width);

void ClearScreen();

void DrawWindow(const std::string& title);
void DrawBox(const std::string& title, const DynamicArray<std::string>& lines);

void Say(const std::string& text);
void SayHeader(const std::string& text);
void SayGood(const std::string& text);
void SayError(const std::string& text);
void SayNote(const std::string& text);

void Pause();
void RunCommand(const std::string& title, const std::string& command);
