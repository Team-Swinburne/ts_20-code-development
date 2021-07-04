// #include <mbed.h>
// #include <CAN.h>
// #include "unit_test.h"
// // #include "relays.h"

// // TEST PRECHARGE RELAY SEQUENCING
// void test_relays(Relay &precharge_relay, AIR &AIR_neg_relay, AIR &AIR_pos_relay, int delay){
// 	while(1){
// 		precharge_relay.set_relay(!precharge_relay.get_relay());
// 		AIR_neg_relay.set_relay(!precharge_relay.get_relay());
// 		AIR_pos_relay.set_relay(!precharge_relay.get_relay());

// 		wait(delay);
// 	}
// }

// // void test_precharge_sequence(float highest_voltage){
// // 	__disable_irq();
// // 	setup();
// // 	__enable_irq();

// // 	heartbeat_state = 1;

// // 	float R = 4000;
// // 	float C = 0.000270;
// // 	float t = 0;

// // 	battery_voltage = highest_voltage;
// // 	mc_voltage = 0;

// // 	bool latch_test_voltage = false;

// // 	while(1){
// // 		if (precharge_button_state)
// // 			latch_test_voltage = true;

// // 		if (latch_test_voltage == true){
// // 			t = heartbeat_counter-precharge_start_time;

// // 			mc_voltage = highest_voltage*(1-exp(-(t/(R*C))));
// // 		}

// // 		wait(0.01);
// // 		pc.printf("TIME :: %d RC :: %f THEORETICAL :: %f INT_SIMULATED MC VOLTAGE :: %d \r\n", t, R*C, mc_voltage);

// // 		// Manually check analogue values.
// // 		pdoc_temperature = NTC_voltageToTemperature(adc_to_voltage(pdoc_adc.readADC_SingleEnded(0), 32768, 6.144), 3380);
// // 		// pc.printf("PDOC_TEMPERAUTRE: %d \r\n", pdoc_temperature);
// // 		pdoc_ref_temperature = NTC_voltageToTemperature(adc_to_voltage(pdoc_adc.readADC_SingleEnded(1), 32768, 6.144), 3380);
// // 		// pc.printf("PDOC_REF_TEMPERAUTRE: %d \r\n", pdoc_ref_temperature);
		
// // 		stated();
// // 		errord();
// // 		// warnd();
// // 		wait(0.001);
// // 	}
// // }

// // void print_current_status(float delay){

// // 	// setup();
// // 	wait(1);

// // 	while(1){
// // 		pc.printf("ts_20 Precharge Controller (STM32F103C8T6 128k) \
// // 		\r\nCOMPILED: %s: %s\r\n",__DATE__, __TIME__);

// // 		pc.printf("\r\n \r\n -- DEVICE STATUS --\r\n");
// // 		pc.printf("HEARTBEAT_STATE: %d \r\n", heartbeat_state);
// // 		pc.printf("HEARTBEAT_COUNTER: %d \r\n", heartbeat_counter);

// // 		// pc.printf("\r\n \r\n -- ERROR SEMIPHORES -- \r\n");
// // 		// pc.printf("ERROR_CODE: %u \r\n", error_code);
// // 		// pc.printf("WARNING_CODE: %u \r\n", warning_code);

// // 		pc.printf("\r\n \r\n -- PERIPHERY STATUSES -- \r\n");
// // 		pc.printf("PRECHARGE_BUTTON_STATE: %d\r\n", precharge_button_state);
// // 		pc.printf("CHARGE_MODE_ACTIVATED: %d \r\n", charge_mode_activated);
// // 		pc.printf("DISCHARGE_STATE: %d \r\n", discharge_state);
// // 		pc.printf("ORION_CONNECTED: %d \r\n", orion_connected);
// // 		pc.printf("ORION_SAFETY_OK %d \r\n", orion_safety_ok);

// // 		pc.printf("\r\n \r\n -- COUNTERS --  \r\n");
// // 		pc.printf("PRECHARGE_START_TIME: %d \r\n", precharge_start_time);
// // 		pc.printf("ORION_LAST_CONNECTION: %d \r\n", orion_last_connection);
		
// // 		pc.printf("\r\n \r\n -- ORION ANALOGUE -- \r\n");
// // 		pc.printf("ORION_LOW_VOLTAGE: %d \r\n", orion_low_voltage);
// // 		pc.printf("ORION_HIGH_VOLTAGE: %d \r\n", orion_high_voltage);
// // 		pc.printf("ORION_HIGH_TEMPERATURE: %d \r\n", orion_high_temperature);

// // 		pc.printf("\r\n \r\n -- INTERNAL ANALOGUE -- \r\n");
// // 		pc.printf("PDOC_TEMPERAUTRE: %d \r\n", pdoc_temperature);
// // 		pc.printf("PDOC_REF_TEMPERAUTRE: %d \r\n", pdoc_ref_temperature);
// // 		pc.printf("MC_VOLTAGE: %d \r\n", mc_voltage);
// // 		pc.printf("BATTERY_VOLTAGE: %d \r\n", battery_voltage);

// // 		pc.printf("\r\n \r\n \r\n \r\n");

// // 		stated();
// // 		// errord();
// // 		updateanalogd();
// // 		// warnd();
// // 		wait(delay);
// // 	}
// // }

// // void print_raw_adc(float delay){
// // 	// setup();
// // 	// wait(1);

// // 	while(delay){
// // 		updateanalogd();
		
// // 		pc.printf("\r\n \r\n-- STATUS -- \r\n");

// // 		pc.printf("\r\n-- PDOC_ADC -- \r\n");
// // 		pc.printf("PDOC_RAW 0: %d \r\n", pdoc_adc.readADC_SingleEnded(0));
// // 		pc.printf("PDOC_RAW 1: %d \r\n", pdoc_adc.readADC_SingleEnded(1));
// // 		pc.printf("PDOC_RAW 2: %d \r\n", pdoc_adc.readADC_SingleEnded(0));
// // 		pc.printf("PDOC_RAW 3: %d \r\n", pdoc_adc.readADC_SingleEnded(1));

// // 		pc.printf("\r\n-- MC_VOLTAGE_ADC -- \r\n");
// // 		pc.printf("MC_VOLTAGE_RAW 0: %d \r\n", mc_hv_sense_adc.readADC_SingleEnded(0));
// // 		pc.printf("MC_VOLTAGE_RAW 1: %d \r\n", mc_hv_sense_adc.readADC_SingleEnded(1));
// // 		pc.printf("MC_VOLTAGE_RAW 2: %d \r\n", mc_hv_sense_adc.readADC_SingleEnded(2));
// // 		pc.printf("MC_VOLTAGE_RAW 3: %d \r\n", mc_hv_sense_adc.readADC_SingleEnded(3));
		
// // 		pc.printf("\r\n-- BATTERY_VOLTAGE_ADC -- \r\n");
// // 		pc.printf("BATTERY_VOLTAGE_RAW 0: %d \r\n", batt_hv_sense_adc.readADC_SingleEnded(0));
// // 		pc.printf("BATTERY_VOLTAGE_RAW 1: %d \r\n", batt_hv_sense_adc.readADC_SingleEnded(1));
// // 		pc.printf("BATTERY_VOLTAGE_RAW 2: %d \r\n", batt_hv_sense_adc.readADC_SingleEnded(2));
// // 		pc.printf("BATTERY_VOLTAGE_RAW 3: %d \r\n", batt_hv_sense_adc.readADC_SingleEnded(3));		
		
// // 		pc.printf("\r\n \r\n \r\n \r\n");

// // 		wait(delay);
// // 	}
// // }