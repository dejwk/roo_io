#pragma once
#include <utility>
#include <vector>
#define HIGH 1
#define LOW 0
#define OUTPUT 1
extern std::vector<std::pair<int, int>> pin_writes;
extern int configured_pin;
inline void digitalWrite(int pin, int value) {
  pin_writes.emplace_back(pin, value);
}
inline void pinMode(int pin, int) { configured_pin = pin; }
