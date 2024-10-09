#pragma once
#include <cstdint>

namespace MetaModule::ButtonExpander
{

static constexpr unsigned NumTotalButtons = sizeof(uint32_t) * 8; //1 bit per button, and state is stored in uint32_t
static constexpr unsigned NumButtonsPerExpander = 8;			  //as defined by hardware
static constexpr unsigned MaxExpanders = NumTotalButtons / NumButtonsPerExpander;

} // namespace MetaModule::ButtonExpander
