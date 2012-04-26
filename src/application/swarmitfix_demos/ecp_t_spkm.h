#if !defined(_ECP_T_SPKM_SWARM_DEMO_SINGLE_AGENT_H)
#define _ECP_T_SPKM_SWARM_DEMO_SINGLE_AGENT_H

#include <boost/shared_ptr.hpp>

#include "robot/spkm/ecp_r_spkm.h"
#include "base/ecp/ecp_task.h"
#include "ecp_g_spkm.h"

namespace mrrocpp {
namespace ecp {
namespace spkm {
namespace task {

class swarmitfix : public common::task::_task <ecp::spkm::robot>
{
public:
	//! Constructor
	swarmitfix(lib::configurator &_config);

	// methods for ECP template to redefine in concrete classes
	void mp_2_ecp_next_state_string_handler(void);
};

} // namespace task
} // namespace irp6p
} // namespace ecp
} // namespace mrrocpp

#endif