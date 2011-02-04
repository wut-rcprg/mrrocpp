/*!
 * @file
 * @brief File contains mp base robot definition
 * @author twiniars <twiniars@ia.pw.edu.pl>, Warsaw University of Technology
 *
 * @ingroup mp
 */

#include <cstring>
#include <unistd.h>
#include <cerrno>
#include <sys/types.h>
#include <sys/wait.h>

#include "base/lib/datastr.h"

#include "base/mp/MP_main_error.h"
#include "base/mp/mp_task.h"
#include "base/mp/mp_robot.h"


#include "base/lib/messip/messip_dataport.h"


#include <boost/foreach.hpp>

namespace mrrocpp {
namespace mp {
namespace robot {

// -------------------------------------------------------------------
robot::robot(lib::robot_name_t l_robot_name, const std::string & _section_name, task::task &mp_object_l, int _number_of_servos) :
	ecp_mp::robot(l_robot_name), number_of_servos(_number_of_servos), mp_object(mp_object_l),
			continuous_coordination(false), communicate_with_ecp(true), sr_ecp_msg(*(mp_object_l.sr_ecp_msg)),
			ecp_scoid(0), ecp_opened(false), ecp_pulse_code(0), new_pulse(false), new_pulse_checked(false)
{
	mp_command.pulse_to_ecp_sent = false;

	std::string node_name(mp_object.config.value <std::string> ("node_name", _section_name));

	std::string
			ecp_attach_point(mp_object.config.return_attach_point_name(lib::configurator::CONFIG_SERVER, "ecp_attach_point", _section_name));

	ECP_pid = mp_object.config.process_spawn(_section_name);

	if (ECP_pid < 0) {
		uint64_t e = errno; // kod bledu
		perror("Failed to spawn ECP");
		sr_ecp_msg.message(lib::SYSTEM_ERROR, e, "mp: Failed to spawn ECP");
		throw common::MP_main_error(lib::SYSTEM_ERROR, 0);
	}

	// handle ECP's name_open() call
	ecp_scoid = mp_object.wait_for_name_open();
	ecp_opened = true;

	// nawiazanie komunikacji z ECP
	unsigned int tmp = 0;
	// kilka sekund  (~1) na otworzenie urzadzenia

		while ((ECP_fd = messip::port_connect(ecp_attach_point)) == NULL)

		if ((tmp++) < lib::CONNECT_RETRY)
			usleep(lib::CONNECT_DELAY);
		else {
			uint64_t e = errno; // kod bledu
			fprintf(stderr, "Connect to ECP failed at channel '%s'\n", ecp_attach_point.c_str());
			perror("Connect to ECP failed");
			sr_ecp_msg.message(lib::SYSTEM_ERROR, e, "Connect to ECP failed");
			throw common::MP_main_error(lib::SYSTEM_ERROR, 0);
		}
}
// -------------------------------------------------------------------


robot::~robot()
{
	fprintf(stderr, "robot::~robot()\n");

	if (ECP_fd) {
		messip::port_disconnect(ECP_fd);
	}

	if (kill(ECP_pid, SIGTERM) == -1) {
		perror("kill()");
		fprintf(stderr, "kill failed for robot %s pid %d\n", lib::toString(robot_name).c_str(), ECP_pid);
	} else {
		if (waitpid(ECP_pid, NULL, 0) == -1) {
			perror("waitpid()");
		}
	}

}

// Wysyla puls do Mp przed oczekiwaniem na spotkanie
void robot::send_pulse_to_ecp(int pulse_code, int pulse_value)
{
	if ((!(mp_command.pulse_to_ecp_sent)) && (!new_pulse) && (!continuous_coordination) && (!(mp_command.command
			== lib::NEXT_STATE))) {


		if (messip::port_send_pulse(ECP_fd, pulse_code, pulse_value) < 0)

		{
			perror("MsgSendPulse()");
		}
		mp_command.pulse_to_ecp_sent = true;
	}
}

// ------------------------------------------------------------------------
void robot::start_ecp(void)
{

	mp_command.command = lib::START_TASK;


		if(messip::port_send(ECP_fd, 0, 0, mp_command, ecp_reply_package) < 0) {

		uint64_t e = errno;
		perror("Send to ECP failed");
		sr_ecp_msg.message(lib::SYSTEM_ERROR, e, "mp: Send to ECP failed");
		throw common::MP_main_error(lib::SYSTEM_ERROR, 0);
	}
	mp_command.pulse_to_ecp_sent = false;
	// by Y - ECP_ACKNOWLEDGE zamienione na lib::TASK_TERMINATED
	// w celu uproszczenia programowania zadan wielorobotowych
	if (ecp_reply_package.reply != lib::TASK_TERMINATED) {
		// Odebrano od ECP informacje o bledzie
		printf("Error w start_ecp w ECP\n");
		throw common::MP_main_error(lib::NON_FATAL_ERROR, ECP_ERRORS);
	}
}
// ------------------------------------------------------------------------


// -------------------------------------------------------------------
void robot::execute_motion(void)
{ // zlecenie wykonania ruchu


		if(messip::port_send(ECP_fd, 0, 0, mp_command, ecp_reply_package) < 0) {

		// Blad komunikacji miedzyprocesowej - wyjatek
		uint64_t e = errno;
		perror("Send to ECP failed");
		sr_ecp_msg.message(lib::SYSTEM_ERROR, e, "mp: Send() to ECP failed");
		throw MP_error(lib::SYSTEM_ERROR, (uint64_t) 0);
	}
	mp_command.pulse_to_ecp_sent = false;

	if (ecp_reply_package.reply == lib::ERROR_IN_ECP) {
		// Odebrano od ECP informacje o bledzie
		throw MP_error(lib::NON_FATAL_ERROR, ECP_ERRORS);
	}
	// W.S. ...
	// Ewentualna aktualizacja skladowych robota na podstawie ecp_reply
}
// ---------------------------------------------------------------


// -------------------------------------------------------------------
void robot::terminate_ecp(void)
{ // zlecenie STOP zakonczenia ruchu
	mp_command.command = lib::STOP;

	//	std::cerr << "mp terminate_ecp 1" << std::endl;


		if(messip::port_send(ECP_fd, 0, 0, mp_command, ecp_reply_package) < 0) {

		// Blad komunikacji miedzyprocesowej - wyjatek
		uint64_t e = errno;
		perror("Send to ECP failed ?");
		sr_ecp_msg.message(lib::SYSTEM_ERROR, e, "mp: Send() to ECP failed");
		throw MP_error(lib::SYSTEM_ERROR, (uint64_t) 0);
	}
	//	std::cerr << "mp terminate_ecp 2" << std::endl;
	mp_command.pulse_to_ecp_sent = false;
	if (ecp_reply_package.reply == lib::ERROR_IN_ECP) {
		// Odebrano od ECP informacje o bledzie
		throw MP_error(lib::NON_FATAL_ERROR, ECP_ERRORS);
	}
}
// ---------------------------------------------------------------


MP_error::MP_error(lib::error_class_t err0, uint64_t err1) :
	error_class(err0), error_no(err1)
{
}

} // namespace robot
} // namespace mp
} // namespace mrrocpp

