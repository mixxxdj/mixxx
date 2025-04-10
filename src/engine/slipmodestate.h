#pragma once

enum class SlipModeState {
    Disabled = 0,
    Armed = 1, // Slip mode was enabled while in a loop, the slip position will start running
               // when this loop get disabled
    Running = 2,
};
