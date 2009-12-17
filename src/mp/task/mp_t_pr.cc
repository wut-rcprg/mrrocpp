// ------------------------------------------------------------------------
//             mp_m_pr.cc - powielanie rysunku - wersja wielorobotowa
//
//                      MASTER PROCESS (MP) - main()
//
// ------------------------------------------------------------------------

#include <stdio.h>
#include <unistd.h>

#include "lib/typedefs.h"
#include "lib/impconst.h"
#include "lib/com_buf.h"
#include "lib/srlib.h"
#include "mp/mp.h"
#include "mp/generator/mp_g_force.h"
#include "ecp_mp/sensor/ecp_mp_s_schunk.h"
#include "mp/task/mp_t_pr.h"
#include "mp/generator/mp_g_common.h"

#include <boost/foreach.hpp>

namespace mrrocpp {
namespace mp {
namespace task {

task* return_created_mp_task (lib::configurator &_config)
{
	return new pr(_config);
}

void pr::mp_short_move_up(void)
{
/*
	lib::trajectory_description tdes;

	tdes.arm_type = lib::XYZ_EULER_ZYZ;
	tdes.interpolation_node_no = 1;
	tdes.internode_step_no = 200;
	tdes.value_in_step_no = tdes.internode_step_no - 2;
	// Wspolrzedne kartezjanskie XYZ i katy Eulera ZYZ
	tdes.coordinate_delta[0] = 0.0; // przyrost wspolrzednej X
	tdes.coordinate_delta[1] = 0.0;// przyrost wspolrzednej Y
	tdes.coordinate_delta[2] = 0.005;   // przyrost wspolrzednej Z
	tdes.coordinate_delta[3] = 0.0;   // przyrost wspolrzednej FI
	tdes.coordinate_delta[4] = 0.0;   // przyrost wspolrzednej TETA
	tdes.coordinate_delta[5] = 0.0;   // przyrost wspolrzednej PSI
	tdes.coordinate_delta[6] = 0.0;   // przyrost wspolrzednej PSI

	// Generator trajektorii prostoliniowej
	generator::tight_coop tcg(*this, tdes, tdes);
	tcg.robot_m = robot_m;
	tcg.Move();
	*/
}


pr::pr(lib::configurator &_config) : task(_config)
{
	// Powolanie czujnikow
	sensor_m[lib::SENSOR_FORCE_ON_TRACK] =
		new ecp_mp::sensor::schunk (lib::SENSOR_FORCE_ON_TRACK, "[vsp_force_irp6ot]", *this);

	sensor_m[lib::SENSOR_FORCE_POSTUMENT] =
		new ecp_mp::sensor::schunk (lib::SENSOR_FORCE_POSTUMENT, "[vsp_force_irp6p]", *this);

	// Konfiguracja wszystkich czujnikow
	BOOST_FOREACH(ecp_mp::sensor_item_t & sensor_item, sensor_m) {
		sensor_item.second->to_vsp.parameters=1; // biasowanie czujnika
		sensor_item.second->configure_sensor();
	}
}

void pr::main_task_algorithm(void)
{
	generator::nose_run_force mp_nrf_gen(*this, 8);
	mp_nrf_gen.robot_m = robot_m;
	mp_nrf_gen.sensor_m = sensor_m;
	generator::drawing_teach_in_force mp_dtif_gen(*this, 8);
	mp_dtif_gen.robot_m = robot_m;
	mp_dtif_gen.sensor_m = sensor_m;
	// printf("przed wait for start \n");
	// Oczekiwanie na zlecenie START od UI



	BOOST_FOREACH(ecp_mp::sensor_item_t & sensor_item, sensor_m) {
		sensor_item.second->to_vsp.parameters=1; // biasowanie czujnika
		sensor_item.second->configure_sensor();
	}

	sr_ecp_msg->message("NOWA SERIA");
	sr_ecp_msg->message("Wodzenie za nos do pozycji rozpoczecia nauki");
	sr_ecp_msg->message("Nastepny etap - nacisnij PULSE MP trigger");

	mp_nrf_gen.Move();

	if (choose_option ("1 - Load drawing, 2 - Learn drawing", 2) == lib::OPTION_ONE)
	{
		sr_ecp_msg->message("Wczytywanie trajektorii");
		mp_dtif_gen.load_file ();
	} else {

		sr_ecp_msg->message("Wodzenie za nos");
		sr_ecp_msg->message("Nastepny etap - nacisnij PULSE MP trigger");
		mp_nrf_gen.Move();

		sr_ecp_msg->message("Uczenie trajektorii");
		sr_ecp_msg->message("Nastepny etap - nacisnij PULSE MP trigger");

		mp_dtif_gen.flush_pose_list(); // Usuniecie listy pozycji, o ile istnieje
		mp_dtif_gen.teach_or_move=YG_TEACH;
		mp_dtif_gen.Move();

		sr_ecp_msg->message("Krotki ruch w gore");
		mp_short_move_up();

		sr_ecp_msg->message("Wodzenie za nos");
		sr_ecp_msg->message("Nastepny etap - nacisnij PULSE MP trigger");
		mp_nrf_gen.Move();
	}

	while (operator_reaction ("Reproduce drawing?"))
	{
		sr_ecp_msg->message("Wodzenie za nos do poczatku odtwarzania");
		sr_ecp_msg->message("Nastepny etap - nacisnij PULSE MP trigger");
		mp_nrf_gen.Move();

		sr_ecp_msg->message("Odtwarzanie nauczonej trajektorii");

		mp_dtif_gen.teach_or_move=YG_MOVE;
		mp_dtif_gen.Move();

		sr_ecp_msg->message("Krotki ruch w gore");
		mp_short_move_up();

		sr_ecp_msg->message("Wodzenie za nos");
		sr_ecp_msg->message("Nastepny etap - nacisnij PULSE MP trigger");
		mp_nrf_gen.Move();

	}


	if ( operator_reaction ("Save drawing ") ) {
		sr_ecp_msg->message("Zapisywanie trajektorii");
		mp_dtif_gen.save_file (lib::PF_VELOCITY);
	}

};

} // namespace task
} // namespace mp
} // namespace mrrocpp
