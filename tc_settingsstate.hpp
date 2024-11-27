#include "tc_state.hpp"

namespace touch_chess
{

class Settings: public State
{
public:
  Settings() = default;
  ~Settings() override = default;

  void enter() override;

  State_t step(unsigned) override;
};

} // namespace touch_chess
