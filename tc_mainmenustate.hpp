/*
Copyright (c) 2024 Andrey V. Skvortsov

This work is licensed under the terms of the MIT license.  
For a copy, see LICENSE file.
*/

#pragma once

#include "tc_state.hpp"

namespace touch_chess
{

class MainMenu: public State
{
public:
  MainMenu() = default;

  ~MainMenu() = default;

  // Interface of the State
  void enter() override;

  State_t step(unsigned current_time) override;

private:
  void __drawButtons();
  void __loadGame();
  int8_t __step = 0;
  bool __noSpiffsConf = true;
};

} // namespace touch_chess
