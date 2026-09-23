#include "tx_arm.h"
#include "buttons.h"

namespace TxArm {

bool isArmed() {
    return Buttons::isHeld(Buttons::BACK);
}

} // namespace TxArm
