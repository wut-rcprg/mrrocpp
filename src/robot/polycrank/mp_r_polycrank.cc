#include "robot/polycrank/mp_r_polycrank.h"
#include "robot/polycrank/polycrank_const.h"

namespace mrrocpp {
namespace mp {
namespace robot {

polycrank::polycrank(task::task &mp_object_l) :
			motor_driven(lib::polycrank::ROBOT_POLYCRANK, lib::polycrank::ECP_SECTION, mp_object_l, lib::polycrank::POLYCRANK_NUM_OF_SERVOS)
{
}

} // namespace robot
} // namespace mp
} // namespace mrrocpp

