// ------------------------------------------------------------------------
//                                   hi_rydz.cc
//
// Funkcje do obslugi sprzetu (serwomechanizmow cyfrowych) dla robota conveyor
//
// Ostatnia modyfikacja: styczen 2006
// cala komunikacja ze sprzetem przerzucona do oblsugi przerwania
// ze wzgledu na drugi proces korzystajacy z tego samego przerwania - tasmociag
// ------------------------------------------------------------------------

#include <math.h>

#include "lib/typedefs.h"
#include "lib/impconst.h"
#include "lib/com_buf.h"
#include "edp/common/hi_rydz.h"

namespace mrrocpp {
namespace edp {
namespace common {

// Konstruktor
hardware_interface::hardware_interface( manip_and_conv_effector &_master )
        : master(_master)
{}

// Destruktor
hardware_interface::~hardware_interface( void )
{}

// Sprawdzenie czy wystapil blad sprzetowy
bool hardware_interface::is_hardware_error (void)
{
    return false;
}

void hardware_interface::insert_set_value ( int drive_number, double set_value)
{ // Wprowadzenie wartosci zadanej PWM
    robot_control[drive_number].adr_offset_plus_0 = (lib::WORD) fabs(set_value);
    if (set_value < 0 )
        robot_control[drive_number].adr_offset_plus_0 |= 0x300;
    else
        robot_control[drive_number].adr_offset_plus_0 |= 0x200;
}

// dla wybranej osi
int hardware_interface::get_current ( int drive_number )
{       // Pobranie pradu
    return meassured_current[drive_number];
}

// Pobranie przyrostu polozenia wybranej osi
double hardware_interface::get_increment ( int drive_number )
{
    return current_position_inc[drive_number];
}

// Pobranie polozenia wybranej osi
long int hardware_interface::get_position ( int drive_number )
{
    return current_absolute_position[drive_number];
}

// Obsluga sprzetu
uint64_t hardware_interface::read_write_hardware ( void )
{
    return 0;
}

// Zerowanie licznikow polozenia
void hardware_interface::reset_counters ( void )
{}

void hardware_interface::start_synchro ( int drive_number )
{}

void hardware_interface::finish_synchro ( int drive_number )
{}

// Sprawdzenie czy pojawilo sie zero  (synchronizacji rezolwera)
bool hardware_interface::is_impulse_zero ( int drive_number )
{
    if ( robot_status[drive_number].adr_offset_plus_0 & 0x0100 )
        return true;
    else
        return false;
}

// Zerowanie licznikow polozenia
void hardware_interface::reset_position (int i)
{
    current_absolute_position[i] =  0L;
    previous_absolute_position[i] = 0L;
    current_position_inc[i] = 0.0;
}

} // namespace common
} // namespace edp
} // namespace mrrocpp

