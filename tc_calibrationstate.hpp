/*
Copyright (c) 2024 Andrey V. Skvortsov

This work is licensed under the terms of the MIT license.  
For a copy, see LICENSE file.
*/

#pragma once

#include "tc_state.hpp"


namespace touch_chess
{

class CalibrateState: public touch_chess::State
{
public:
  CalibrateState() = default;

  ~CalibrateState() = default;

  // Interface of the State
  void enter() override;

  touch_chess::State_t step(unsigned current_time) override;

private:

  uint8_t m_step;

  bool m_waitTouch;

  uint16_t m_coords[4][2];
};

} // namespace touch_chess
