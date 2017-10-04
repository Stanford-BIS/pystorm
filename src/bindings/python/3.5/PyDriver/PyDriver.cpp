#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/stl_bind.h>
#include <pybind11/functional.h>

// This undef is due to a MacOS stupidity with  termios.h`
// https://developer.apple.com/legacy/library/documentation/Darwin/Reference/ManPages/man3/tcsetattr.3.html
#undef B0
#include <Driver.h>
#include <model/BDModelDriver.h>

void bind_unknown_unknown(std::function< pybind11::module &(std::string const &namespace_) > &M)
{
	// pystorm::bddriver::bdpars::BDHornEP file: line:50
	pybind11::enum_<pystorm::bddriver::bdpars::BDHornEP>(M("pystorm::bddriver::bdpars"), "BDHornEP", "///////////////////////////////////////////")
		.value("ADC", pystorm::bddriver::bdpars::BDHornEP::ADC)
		.value("DAC_DIFF_G", pystorm::bddriver::bdpars::BDHornEP::DAC_DIFF_G)
		.value("DAC_SYN_INH", pystorm::bddriver::bdpars::BDHornEP::DAC_SYN_INH)
		.value("DAC_SYN_PU", pystorm::bddriver::bdpars::BDHornEP::DAC_SYN_PU)
		.value("DAC_UNUSED", pystorm::bddriver::bdpars::BDHornEP::DAC_UNUSED)
		.value("DAC_DIFF_R", pystorm::bddriver::bdpars::BDHornEP::DAC_DIFF_R)
		.value("DAC_SOMA_OFFSET", pystorm::bddriver::bdpars::BDHornEP::DAC_SOMA_OFFSET)
		.value("DAC_SYN_LK", pystorm::bddriver::bdpars::BDHornEP::DAC_SYN_LK)
		.value("DAC_SYN_DC", pystorm::bddriver::bdpars::BDHornEP::DAC_SYN_DC)
		.value("DAC_SYN_PD", pystorm::bddriver::bdpars::BDHornEP::DAC_SYN_PD)
		.value("DAC_ADC_BIAS_2", pystorm::bddriver::bdpars::BDHornEP::DAC_ADC_BIAS_2)
		.value("DAC_ADC_BIAS_1", pystorm::bddriver::bdpars::BDHornEP::DAC_ADC_BIAS_1)
		.value("DAC_SOMA_REF", pystorm::bddriver::bdpars::BDHornEP::DAC_SOMA_REF)
		.value("DAC_SYN_EXC", pystorm::bddriver::bdpars::BDHornEP::DAC_SYN_EXC)
		.value("DELAY_DCTFIFO", pystorm::bddriver::bdpars::BDHornEP::DELAY_DCTFIFO)
		.value("DELAY_PGFIFO", pystorm::bddriver::bdpars::BDHornEP::DELAY_PGFIFO)
		.value("DELAY_TAT0", pystorm::bddriver::bdpars::BDHornEP::DELAY_TAT0)
		.value("DELAY_TAT1", pystorm::bddriver::bdpars::BDHornEP::DELAY_TAT1)
		.value("DELAY_PAT", pystorm::bddriver::bdpars::BDHornEP::DELAY_PAT)
		.value("DELAY_MM", pystorm::bddriver::bdpars::BDHornEP::DELAY_MM)
		.value("DELAY_AM", pystorm::bddriver::bdpars::BDHornEP::DELAY_AM)
		.value("INIT_FIFO_DCT", pystorm::bddriver::bdpars::BDHornEP::INIT_FIFO_DCT)
		.value("INIT_FIFO_HT", pystorm::bddriver::bdpars::BDHornEP::INIT_FIFO_HT)
		.value("NEURON_CONFIG", pystorm::bddriver::bdpars::BDHornEP::NEURON_CONFIG)
		.value("NEURON_DUMP_TOGGLE", pystorm::bddriver::bdpars::BDHornEP::NEURON_DUMP_TOGGLE)
		.value("NEURON_INJECT", pystorm::bddriver::bdpars::BDHornEP::NEURON_INJECT)
		.value("PROG_AMMM", pystorm::bddriver::bdpars::BDHornEP::PROG_AMMM)
		.value("PROG_PAT", pystorm::bddriver::bdpars::BDHornEP::PROG_PAT)
		.value("PROG_TAT0", pystorm::bddriver::bdpars::BDHornEP::PROG_TAT0)
		.value("PROG_TAT1", pystorm::bddriver::bdpars::BDHornEP::PROG_TAT1)
		.value("RI", pystorm::bddriver::bdpars::BDHornEP::RI)
		.value("TOGGLE_POST_FIFO0", pystorm::bddriver::bdpars::BDHornEP::TOGGLE_POST_FIFO0)
		.value("TOGGLE_POST_FIFO1", pystorm::bddriver::bdpars::BDHornEP::TOGGLE_POST_FIFO1)
		.value("TOGGLE_PRE_FIFO", pystorm::bddriver::bdpars::BDHornEP::TOGGLE_PRE_FIFO)
		.value("COUNT", pystorm::bddriver::bdpars::BDHornEP::COUNT);

;

	// pystorm::bddriver::bdpars::FPGARegEP file: line:93
	pybind11::enum_<pystorm::bddriver::bdpars::FPGARegEP>(M("pystorm::bddriver::bdpars"), "FPGARegEP", "///////////////////////////////////////////")
		.value("SF_FILTS_USED", pystorm::bddriver::bdpars::FPGARegEP::SF_FILTS_USED)
		.value("SF_INCREMENT_CONSTANT0", pystorm::bddriver::bdpars::FPGARegEP::SF_INCREMENT_CONSTANT0)
		.value("SF_INCREMENT_CONSTANT1", pystorm::bddriver::bdpars::FPGARegEP::SF_INCREMENT_CONSTANT1)
		.value("SF_DECAY_CONSTANT0", pystorm::bddriver::bdpars::FPGARegEP::SF_DECAY_CONSTANT0)
		.value("SF_DECAY_CONSTANT1", pystorm::bddriver::bdpars::FPGARegEP::SF_DECAY_CONSTANT1)
		.value("SG_GENS_USED", pystorm::bddriver::bdpars::FPGARegEP::SG_GENS_USED)
		.value("SG_GENS_EN0", pystorm::bddriver::bdpars::FPGARegEP::SG_GENS_EN0)
		.value("SG_GENS_EN1", pystorm::bddriver::bdpars::FPGARegEP::SG_GENS_EN1)
		.value("SG_GENS_EN2", pystorm::bddriver::bdpars::FPGARegEP::SG_GENS_EN2)
		.value("SG_GENS_EN3", pystorm::bddriver::bdpars::FPGARegEP::SG_GENS_EN3)
		.value("SG_GENS_EN4", pystorm::bddriver::bdpars::FPGARegEP::SG_GENS_EN4)
		.value("SG_GENS_EN5", pystorm::bddriver::bdpars::FPGARegEP::SG_GENS_EN5)
		.value("SG_GENS_EN6", pystorm::bddriver::bdpars::FPGARegEP::SG_GENS_EN6)
		.value("SG_GENS_EN7", pystorm::bddriver::bdpars::FPGARegEP::SG_GENS_EN7)
		.value("SG_GENS_EN8", pystorm::bddriver::bdpars::FPGARegEP::SG_GENS_EN8)
		.value("SG_GENS_EN9", pystorm::bddriver::bdpars::FPGARegEP::SG_GENS_EN9)
		.value("SG_GENS_EN10", pystorm::bddriver::bdpars::FPGARegEP::SG_GENS_EN10)
		.value("SG_GENS_EN11", pystorm::bddriver::bdpars::FPGARegEP::SG_GENS_EN11)
		.value("SG_GENS_EN12", pystorm::bddriver::bdpars::FPGARegEP::SG_GENS_EN12)
		.value("SG_GENS_EN13", pystorm::bddriver::bdpars::FPGARegEP::SG_GENS_EN13)
		.value("SG_GENS_EN14", pystorm::bddriver::bdpars::FPGARegEP::SG_GENS_EN14)
		.value("SG_GENS_EN15", pystorm::bddriver::bdpars::FPGARegEP::SG_GENS_EN15)
		.value("TM_UNIT_LEN", pystorm::bddriver::bdpars::FPGARegEP::TM_UNIT_LEN)
		.value("TM_PC_TIME_ELAPSED0", pystorm::bddriver::bdpars::FPGARegEP::TM_PC_TIME_ELAPSED0)
		.value("TM_PC_TIME_ELAPSED1", pystorm::bddriver::bdpars::FPGARegEP::TM_PC_TIME_ELAPSED1)
		.value("TM_PC_TIME_ELAPSED2", pystorm::bddriver::bdpars::FPGARegEP::TM_PC_TIME_ELAPSED2)
		.value("TM_PC_SEND_HB_UP_EVERY0", pystorm::bddriver::bdpars::FPGARegEP::TM_PC_SEND_HB_UP_EVERY0)
		.value("TM_PC_SEND_HB_UP_EVERY1", pystorm::bddriver::bdpars::FPGARegEP::TM_PC_SEND_HB_UP_EVERY1)
		.value("TM_PC_SEND_HB_UP_EVERY2", pystorm::bddriver::bdpars::FPGARegEP::TM_PC_SEND_HB_UP_EVERY2)
		.value("TM_PC_RESET_TIME", pystorm::bddriver::bdpars::FPGARegEP::TM_PC_RESET_TIME)
		.value("TS_REPORT_TAGS", pystorm::bddriver::bdpars::FPGARegEP::TS_REPORT_TAGS)
		.value("BD_RESET", pystorm::bddriver::bdpars::FPGARegEP::BD_RESET)
		.value("NOP", pystorm::bddriver::bdpars::FPGARegEP::NOP)
		.value("COUNT", pystorm::bddriver::bdpars::FPGARegEP::COUNT);

;

	// pystorm::bddriver::bdpars::FPGAChannelEP file: line:134
	pybind11::enum_<pystorm::bddriver::bdpars::FPGAChannelEP>(M("pystorm::bddriver::bdpars"), "FPGAChannelEP", "///////////////////////////////////////////")
		.value("SG_PROGRAM_MEM", pystorm::bddriver::bdpars::FPGAChannelEP::SG_PROGRAM_MEM)
		.value("COUNT", pystorm::bddriver::bdpars::FPGAChannelEP::COUNT);

;

	// pystorm::bddriver::bdpars::BDFunnelEP file: line:154
	pybind11::enum_<pystorm::bddriver::bdpars::BDFunnelEP>(M("pystorm::bddriver::bdpars"), "BDFunnelEP", "///////////////////////////////////////////")
		.value("DUMP_AM", pystorm::bddriver::bdpars::BDFunnelEP::DUMP_AM)
		.value("DUMP_MM", pystorm::bddriver::bdpars::BDFunnelEP::DUMP_MM)
		.value("DUMP_PAT", pystorm::bddriver::bdpars::BDFunnelEP::DUMP_PAT)
		.value("DUMP_POST_FIFO0", pystorm::bddriver::bdpars::BDFunnelEP::DUMP_POST_FIFO0)
		.value("DUMP_POST_FIFO1", pystorm::bddriver::bdpars::BDFunnelEP::DUMP_POST_FIFO1)
		.value("DUMP_PRE_FIFO", pystorm::bddriver::bdpars::BDFunnelEP::DUMP_PRE_FIFO)
		.value("DUMP_TAT0", pystorm::bddriver::bdpars::BDFunnelEP::DUMP_TAT0)
		.value("DUMP_TAT1", pystorm::bddriver::bdpars::BDFunnelEP::DUMP_TAT1)
		.value("NRNI", pystorm::bddriver::bdpars::BDFunnelEP::NRNI)
		.value("OVFLW0", pystorm::bddriver::bdpars::BDFunnelEP::OVFLW0)
		.value("OVFLW1", pystorm::bddriver::bdpars::BDFunnelEP::OVFLW1)
		.value("RO_ACC", pystorm::bddriver::bdpars::BDFunnelEP::RO_ACC)
		.value("RO_TAT", pystorm::bddriver::bdpars::BDFunnelEP::RO_TAT)
		.value("COUNT", pystorm::bddriver::bdpars::BDFunnelEP::COUNT);

;

	// pystorm::bddriver::bdpars::FPGAOutputEP file: line:175
	pybind11::enum_<pystorm::bddriver::bdpars::FPGAOutputEP>(M("pystorm::bddriver::bdpars"), "FPGAOutputEP", "///////////////////////////////////////////")
		.value("UPSTREAM_HB", pystorm::bddriver::bdpars::FPGAOutputEP::UPSTREAM_HB)
		.value("SF_OUTPUT", pystorm::bddriver::bdpars::FPGAOutputEP::SF_OUTPUT)
		.value("NOP", pystorm::bddriver::bdpars::FPGAOutputEP::NOP)
		.value("COUNT", pystorm::bddriver::bdpars::FPGAOutputEP::COUNT);

;

	// pystorm::bddriver::bdpars::BDMemId file: line:192
	pybind11::enum_<pystorm::bddriver::bdpars::BDMemId>(M("pystorm::bddriver::bdpars"), "BDMemId", "///////////////////////////////////////////")
		.value("AM", pystorm::bddriver::bdpars::BDMemId::AM)
		.value("MM", pystorm::bddriver::bdpars::BDMemId::MM)
		.value("TAT0", pystorm::bddriver::bdpars::BDMemId::TAT0)
		.value("TAT1", pystorm::bddriver::bdpars::BDMemId::TAT1)
		.value("PAT", pystorm::bddriver::bdpars::BDMemId::PAT)
		.value("FIFO_DCT", pystorm::bddriver::bdpars::BDMemId::FIFO_DCT)
		.value("FIFO_PG", pystorm::bddriver::bdpars::BDMemId::FIFO_PG)
		.value("COUNT", pystorm::bddriver::bdpars::BDMemId::COUNT);

;

	// pystorm::bddriver::bdpars::ConfigSomaID file: line:213
	pybind11::enum_<pystorm::bddriver::bdpars::ConfigSomaID>(M("pystorm::bddriver::bdpars"), "ConfigSomaID", "///////////////////////////////////////////\n Neuron configuration options")
		.value("GAIN_0", pystorm::bddriver::bdpars::ConfigSomaID::GAIN_0)
		.value("GAIN_1", pystorm::bddriver::bdpars::ConfigSomaID::GAIN_1)
		.value("OFFSET_0", pystorm::bddriver::bdpars::ConfigSomaID::OFFSET_0)
		.value("OFFSET_1", pystorm::bddriver::bdpars::ConfigSomaID::OFFSET_1)
		.value("ENABLE", pystorm::bddriver::bdpars::ConfigSomaID::ENABLE)
		.value("SUBTRACT_OFFSET", pystorm::bddriver::bdpars::ConfigSomaID::SUBTRACT_OFFSET);

;

	// pystorm::bddriver::bdpars::ConfigSynapseID file: line:222
	pybind11::enum_<pystorm::bddriver::bdpars::ConfigSynapseID>(M("pystorm::bddriver::bdpars"), "ConfigSynapseID", "")
		.value("SYN_DISABLE", pystorm::bddriver::bdpars::ConfigSynapseID::SYN_DISABLE)
		.value("ADC_DISABLE", pystorm::bddriver::bdpars::ConfigSynapseID::ADC_DISABLE);

;

	// pystorm::bddriver::bdpars::SomaStatusId file: line:227
	pybind11::enum_<pystorm::bddriver::bdpars::SomaStatusId>(M("pystorm::bddriver::bdpars"), "SomaStatusId", "")
		.value("DISABLED", pystorm::bddriver::bdpars::SomaStatusId::DISABLED)
		.value("ENABLED", pystorm::bddriver::bdpars::SomaStatusId::ENABLED);

;

	// pystorm::bddriver::bdpars::SomaGainId file: line:229
	pybind11::enum_<pystorm::bddriver::bdpars::SomaGainId>(M("pystorm::bddriver::bdpars"), "SomaGainId", "")
		.value("ONE_FOURTH", pystorm::bddriver::bdpars::SomaGainId::ONE_FOURTH)
		.value("ONE_THIRD", pystorm::bddriver::bdpars::SomaGainId::ONE_THIRD)
		.value("ONE_HALF", pystorm::bddriver::bdpars::SomaGainId::ONE_HALF)
		.value("ONE", pystorm::bddriver::bdpars::SomaGainId::ONE);

;

	// pystorm::bddriver::bdpars::SomaOffsetSignId file: line:231
	pybind11::enum_<pystorm::bddriver::bdpars::SomaOffsetSignId>(M("pystorm::bddriver::bdpars"), "SomaOffsetSignId", "")
		.value("POSITIVE", pystorm::bddriver::bdpars::SomaOffsetSignId::POSITIVE)
		.value("NEGATIVE", pystorm::bddriver::bdpars::SomaOffsetSignId::NEGATIVE);

;

	// pystorm::bddriver::bdpars::SomaOffsetMultiplierId file: line:233
	pybind11::enum_<pystorm::bddriver::bdpars::SomaOffsetMultiplierId>(M("pystorm::bddriver::bdpars"), "SomaOffsetMultiplierId", "")
		.value("ZERO", pystorm::bddriver::bdpars::SomaOffsetMultiplierId::ZERO)
		.value("ONE", pystorm::bddriver::bdpars::SomaOffsetMultiplierId::ONE)
		.value("TWO", pystorm::bddriver::bdpars::SomaOffsetMultiplierId::TWO)
		.value("THREE", pystorm::bddriver::bdpars::SomaOffsetMultiplierId::THREE);

;

	// pystorm::bddriver::bdpars::SynapseStatusId file: line:235
	pybind11::enum_<pystorm::bddriver::bdpars::SynapseStatusId>(M("pystorm::bddriver::bdpars"), "SynapseStatusId", "")
		.value("ENABLED", pystorm::bddriver::bdpars::SynapseStatusId::ENABLED)
		.value("DISABLED", pystorm::bddriver::bdpars::SynapseStatusId::DISABLED);

;

	// pystorm::bddriver::bdpars::DiffusorCutStatusId file: line:237
	pybind11::enum_<pystorm::bddriver::bdpars::DiffusorCutStatusId>(M("pystorm::bddriver::bdpars"), "DiffusorCutStatusId", "")
		.value("CLOSE", pystorm::bddriver::bdpars::DiffusorCutStatusId::CLOSE)
		.value("OPEN", pystorm::bddriver::bdpars::DiffusorCutStatusId::OPEN);

;

	// pystorm::bddriver::bdpars::DiffusorCutLocationId file: line:239
	pybind11::enum_<pystorm::bddriver::bdpars::DiffusorCutLocationId>(M("pystorm::bddriver::bdpars"), "DiffusorCutLocationId", "")
		.value("NORTH_LEFT", pystorm::bddriver::bdpars::DiffusorCutLocationId::NORTH_LEFT)
		.value("NORTH_RIGHT", pystorm::bddriver::bdpars::DiffusorCutLocationId::NORTH_RIGHT)
		.value("WEST_TOP", pystorm::bddriver::bdpars::DiffusorCutLocationId::WEST_TOP)
		.value("WEST_BOTTOM", pystorm::bddriver::bdpars::DiffusorCutLocationId::WEST_BOTTOM);

;

	{ // pystorm::bddriver::bdpars::BDPars file: line:247
		pybind11::class_<pystorm::bddriver::bdpars::BDPars> cl(M("pystorm::bddriver::bdpars"), "BDPars", "BDPars holds all the nitty-gritty information about the BD hardware's parameters.\n\n BDPars contains several array data members containing structs, keyed by enums.\n The enums refer to particular hardware elements or concepts, such as the name of a memory,\n register, or a particular type of programming word.\n BDPars is fully public, but Driver only has a const reference.");
		cl.def(pybind11::init<>());

		cl.def(pybind11::init<const class pystorm::bddriver::bdpars::BDPars &>(), pybind11::arg(""));
        cl.def_readonly_static("config_soma_mem_", &pystorm::bddriver::bdpars::BDPars::config_soma_mem_);
        cl.def_readonly_static("config_synapse_mem_", &pystorm::bddriver::bdpars::BDPars::config_synapse_mem_);
        cl.def_readonly_static("config_diff_cut_mem_", &pystorm::bddriver::bdpars::BDPars::config_diff_cut_mem_);
		cl.def_readonly("NumCores", &pystorm::bddriver::bdpars::BDPars::NumCores);
		cl.def_readonly("DnEPFPGARegOffset", &pystorm::bddriver::bdpars::BDPars::DnEPFPGARegOffset);
		cl.def_readonly("DnEPFPGANumReg", &pystorm::bddriver::bdpars::BDPars::DnEPFPGANumReg);
		cl.def_readonly("DnEPFPGAChannelOffset", &pystorm::bddriver::bdpars::BDPars::DnEPFPGAChannelOffset);
		cl.def_readonly("DnEPFPGANumChan", &pystorm::bddriver::bdpars::BDPars::DnEPFPGANumChan);
		cl.def_readonly("DnEPFPGABitsPerReg", &pystorm::bddriver::bdpars::BDPars::DnEPFPGABitsPerReg);
		cl.def_readonly("DnEPFPGABitsPerChannel", &pystorm::bddriver::bdpars::BDPars::DnEPFPGABitsPerChannel);
		cl.def_readonly("DnWordsPerFrame", &pystorm::bddriver::bdpars::BDPars::DnWordsPerFrame);
		cl.def_readonly("DnTimeUnitsPerHB", &pystorm::bddriver::bdpars::BDPars::DnTimeUnitsPerHB);
		cl.def_readwrite("Dn_EP_size_", &pystorm::bddriver::bdpars::BDPars::Dn_EP_size_);
		cl.def_readwrite("Up_EP_size_", &pystorm::bddriver::bdpars::BDPars::Up_EP_size_);
		cl.def_readwrite("mem_info_", &pystorm::bddriver::bdpars::BDPars::mem_info_);
		cl.def("DnEPCodeFor", (unsigned char (pystorm::bddriver::bdpars::BDPars::*)(pystorm::bddriver::bdpars::BDHornEP) const) &pystorm::bddriver::bdpars::BDPars::DnEPCodeFor, "C++: pystorm::bddriver::bdpars::BDPars::DnEPCodeFor(pystorm::bddriver::bdpars::BDHornEP) const --> unsigned char", pybind11::arg("ep"));
		cl.def("DnEPCodeFor", (unsigned char (pystorm::bddriver::bdpars::BDPars::*)(pystorm::bddriver::bdpars::FPGARegEP) const) &pystorm::bddriver::bdpars::BDPars::DnEPCodeFor, "C++: pystorm::bddriver::bdpars::BDPars::DnEPCodeFor(pystorm::bddriver::bdpars::FPGARegEP) const --> unsigned char", pybind11::arg("ep"));
		cl.def("DnEPCodeFor", (unsigned char (pystorm::bddriver::bdpars::BDPars::*)(pystorm::bddriver::bdpars::FPGAChannelEP) const) &pystorm::bddriver::bdpars::BDPars::DnEPCodeFor, "C++: pystorm::bddriver::bdpars::BDPars::DnEPCodeFor(pystorm::bddriver::bdpars::FPGAChannelEP) const --> unsigned char", pybind11::arg("ep"));
		cl.def("UpEPCodeFor", (unsigned char (pystorm::bddriver::bdpars::BDPars::*)(pystorm::bddriver::bdpars::BDFunnelEP) const) &pystorm::bddriver::bdpars::BDPars::UpEPCodeFor, "C++: pystorm::bddriver::bdpars::BDPars::UpEPCodeFor(pystorm::bddriver::bdpars::BDFunnelEP) const --> unsigned char", pybind11::arg("ep"));
		cl.def("UpEPCodeFor", (unsigned char (pystorm::bddriver::bdpars::BDPars::*)(pystorm::bddriver::bdpars::FPGAOutputEP) const) &pystorm::bddriver::bdpars::BDPars::UpEPCodeFor, "C++: pystorm::bddriver::bdpars::BDPars::UpEPCodeFor(pystorm::bddriver::bdpars::FPGAOutputEP) const --> unsigned char", pybind11::arg("ep"));
		cl.def("DnEPCodeIsBDHornEP", (bool (pystorm::bddriver::bdpars::BDPars::*)(unsigned char) const) &pystorm::bddriver::bdpars::BDPars::DnEPCodeIsBDHornEP, "C++: pystorm::bddriver::bdpars::BDPars::DnEPCodeIsBDHornEP(unsigned char) const --> bool", pybind11::arg("ep"));
		cl.def("DnEPCodeIsFPGARegEP", (bool (pystorm::bddriver::bdpars::BDPars::*)(unsigned char) const) &pystorm::bddriver::bdpars::BDPars::DnEPCodeIsFPGARegEP, "C++: pystorm::bddriver::bdpars::BDPars::DnEPCodeIsFPGARegEP(unsigned char) const --> bool", pybind11::arg("ep"));
		cl.def("DnEPCodeIsFPGAChannelEP", (bool (pystorm::bddriver::bdpars::BDPars::*)(unsigned char) const) &pystorm::bddriver::bdpars::BDPars::DnEPCodeIsFPGAChannelEP, "C++: pystorm::bddriver::bdpars::BDPars::DnEPCodeIsFPGAChannelEP(unsigned char) const --> bool", pybind11::arg("ep"));
		cl.def("UpEPCodeIsBDFunnelEP", (bool (pystorm::bddriver::bdpars::BDPars::*)(unsigned char) const) &pystorm::bddriver::bdpars::BDPars::UpEPCodeIsBDFunnelEP, "C++: pystorm::bddriver::bdpars::BDPars::UpEPCodeIsBDFunnelEP(unsigned char) const --> bool", pybind11::arg("ep"));
		cl.def("UpEPCodeIsFPGAOutputEP", (bool (pystorm::bddriver::bdpars::BDPars::*)(unsigned char) const) &pystorm::bddriver::bdpars::BDPars::UpEPCodeIsFPGAOutputEP, "C++: pystorm::bddriver::bdpars::BDPars::UpEPCodeIsFPGAOutputEP(unsigned char) const --> bool", pybind11::arg("ep"));
		cl.def("GetUpEPs", (class std::vector<unsigned char, class std::allocator<unsigned char> > (pystorm::bddriver::bdpars::BDPars::*)() const) &pystorm::bddriver::bdpars::BDPars::GetUpEPs, "C++: pystorm::bddriver::bdpars::BDPars::GetUpEPs() const --> class std::vector<unsigned char, class std::allocator<unsigned char> >");
		cl.def("BDHornEPIsInputStream", (bool (pystorm::bddriver::bdpars::BDPars::*)(pystorm::bddriver::bdpars::BDHornEP) const) &pystorm::bddriver::bdpars::BDPars::BDHornEPIsInputStream, "C++: pystorm::bddriver::bdpars::BDPars::BDHornEPIsInputStream(pystorm::bddriver::bdpars::BDHornEP) const --> bool", pybind11::arg("ep"));
		cl.def("BDHornEPIsMem", (bool (pystorm::bddriver::bdpars::BDPars::*)(pystorm::bddriver::bdpars::BDHornEP) const) &pystorm::bddriver::bdpars::BDPars::BDHornEPIsMem, "C++: pystorm::bddriver::bdpars::BDPars::BDHornEPIsMem(pystorm::bddriver::bdpars::BDHornEP) const --> bool", pybind11::arg("ep"));
		cl.def("BDHornEPIsReg", (bool (pystorm::bddriver::bdpars::BDPars::*)(pystorm::bddriver::bdpars::BDHornEP) const) &pystorm::bddriver::bdpars::BDPars::BDHornEPIsReg, "C++: pystorm::bddriver::bdpars::BDPars::BDHornEPIsReg(pystorm::bddriver::bdpars::BDHornEP) const --> bool", pybind11::arg("ep"));
		cl.def("GetBDRegs", (class std::vector<pystorm::bddriver::bdpars::BDHornEP, class std::allocator<pystorm::bddriver::bdpars::BDHornEP> > (pystorm::bddriver::bdpars::BDPars::*)() const) &pystorm::bddriver::bdpars::BDPars::GetBDRegs, "C++: pystorm::bddriver::bdpars::BDPars::GetBDRegs() const --> class std::vector<pystorm::bddriver::bdpars::BDHornEP, class std::allocator<pystorm::bddriver::bdpars::BDHornEP> >");
	}
}

void bind_unknown_unknown_1(std::function< pybind11::module &(std::string const &namespace_) > &M)
{
	// pystorm::bddriver::driverpars::DriverParId file: line:11
	pybind11::enum_<pystorm::bddriver::driverpars::DriverParId>(M("pystorm::bddriver::driverpars"), "DriverParId", "")
		.value("ENC_BUF_IN_CAPACITY", pystorm::bddriver::driverpars::DriverParId::ENC_BUF_IN_CAPACITY)
		.value("DEC_BUF_IN_CAPACITY", pystorm::bddriver::driverpars::DriverParId::DEC_BUF_IN_CAPACITY)
		.value("ENC_BUF_OUT_CAPACITY", pystorm::bddriver::driverpars::DriverParId::ENC_BUF_OUT_CAPACITY)
		.value("DEC_BUF_OUT_CAPACITY", pystorm::bddriver::driverpars::DriverParId::DEC_BUF_OUT_CAPACITY)
		.value("ENC_CHUNK_SIZE", pystorm::bddriver::driverpars::DriverParId::ENC_CHUNK_SIZE)
		.value("ENC_TIMEOUT_US", pystorm::bddriver::driverpars::DriverParId::ENC_TIMEOUT_US)
		.value("DEC_CHUNK_SIZE", pystorm::bddriver::driverpars::DriverParId::DEC_CHUNK_SIZE)
		.value("DEC_TIMEOUT_US", pystorm::bddriver::driverpars::DriverParId::DEC_TIMEOUT_US)
		.value("BD_STATE_TRAFFIC_DRAIN_US", pystorm::bddriver::driverpars::DriverParId::BD_STATE_TRAFFIC_DRAIN_US)
		.value("COMM_TYPE", pystorm::bddriver::driverpars::DriverParId::COMM_TYPE)
		.value("BDMODELCOMM_TRY_FOR_US", pystorm::bddriver::driverpars::DriverParId::BDMODELCOMM_TRY_FOR_US)
		.value("BDMODELCOMM_MAX_TO_READ", pystorm::bddriver::driverpars::DriverParId::BDMODELCOMM_MAX_TO_READ)
		.value("BDMODELCOMM_SLEEP_FOR_US", pystorm::bddriver::driverpars::DriverParId::BDMODELCOMM_SLEEP_FOR_US)
		.value("DUMPPAT_TIMEOUT_US", pystorm::bddriver::driverpars::DriverParId::DUMPPAT_TIMEOUT_US)
		.value("DUMPTAT_TIMEOUT_US", pystorm::bddriver::driverpars::DriverParId::DUMPTAT_TIMEOUT_US)
		.value("DUMPMM_TIMEOUT_US", pystorm::bddriver::driverpars::DriverParId::DUMPMM_TIMEOUT_US)
		.value("RECVSPIKES_TIMEOUT_US", pystorm::bddriver::driverpars::DriverParId::RECVSPIKES_TIMEOUT_US)
		.value("RECVTAGS_TIMEOUT_US", pystorm::bddriver::driverpars::DriverParId::RECVTAGS_TIMEOUT_US)
		.value("LastDriverParId", pystorm::bddriver::driverpars::DriverParId::LastDriverParId)
		.export_values();

;

	// pystorm::bddriver::driverpars::DriverStringParId file: line:36
	pybind11::enum_<pystorm::bddriver::driverpars::DriverStringParId>(M("pystorm::bddriver::driverpars"), "DriverStringParId", "")
		.value("SOFT_COMM_IN_FNAME", pystorm::bddriver::driverpars::DriverStringParId::SOFT_COMM_IN_FNAME)
		.value("SOFT_COMM_OUT_FNAME", pystorm::bddriver::driverpars::DriverStringParId::SOFT_COMM_OUT_FNAME)
		.value("LastDriverStringParId", pystorm::bddriver::driverpars::DriverStringParId::LastDriverStringParId)
		.export_values();

;

	{ // pystorm::bddriver::driverpars::DriverPars file: line:52
		pybind11::class_<pystorm::bddriver::driverpars::DriverPars> cl(M("pystorm::bddriver::driverpars"), "DriverPars", "Stores parameters that modify driver object parameters/functions");
		cl.def(pybind11::init<>());

		cl.def("Get", (unsigned int (pystorm::bddriver::driverpars::DriverPars::*)(pystorm::bddriver::driverpars::DriverParId) const) &pystorm::bddriver::driverpars::DriverPars::Get, "C++: pystorm::bddriver::driverpars::DriverPars::Get(pystorm::bddriver::driverpars::DriverParId) const --> unsigned int", pybind11::arg("par_id"));
		cl.def("Get", (const std::string * (pystorm::bddriver::driverpars::DriverPars::*)(pystorm::bddriver::driverpars::DriverStringParId) const) &pystorm::bddriver::driverpars::DriverPars::Get, "C++: pystorm::bddriver::driverpars::DriverPars::Get(pystorm::bddriver::driverpars::DriverStringParId) const --> const std::string *", pybind11::return_value_policy::automatic, pybind11::arg("par_id"));
	}
}

void bind_unknown_unknown_2(std::function< pybind11::module &(std::string const &namespace_) > &M)
{
	{ // pystorm::bddriver::BDState file: line:24
		pybind11::class_<pystorm::bddriver::BDState> cl(M("pystorm::bddriver"), "BDState", "Keeps track of currently set register values, toggle states, memory values, etc.\n\n Also encodes timing assumptions: e.g. as soon as the traffic toggles are turned\n off in software, it is not necessarily safe to start programming memories:\n there is some amount of time that we must wait for the traffic to drain completely.\n the length of this delay is kept in DriverPars, and is used by BDState to implement\n an interface that the driver can use to block until it is safe.");
		cl.def(pybind11::init<const class pystorm::bddriver::bdpars::BDPars *, const class pystorm::bddriver::driverpars::DriverPars *>(), pybind11::arg("bd_pars"), pybind11::arg("driver_pars"));

		cl.def(pybind11::init<const class pystorm::bddriver::BDState &>(), pybind11::arg(""));

		cl.def("SetMem", (void (pystorm::bddriver::BDState::*)(pystorm::bddriver::bdpars::BDMemId, unsigned int, const class std::vector<unsigned long, class std::allocator<unsigned long> > &)) &pystorm::bddriver::BDState::SetMem, "C++: pystorm::bddriver::BDState::SetMem(pystorm::bddriver::bdpars::BDMemId, unsigned int, const class std::vector<unsigned long, class std::allocator<unsigned long> > &) --> void", pybind11::arg("mem_id"), pybind11::arg("start_addr"), pybind11::arg("data"));
		cl.def("GetMem", (const class std::vector<unsigned long, class std::allocator<unsigned long> > * (pystorm::bddriver::BDState::*)(pystorm::bddriver::bdpars::BDMemId) const) &pystorm::bddriver::BDState::GetMem, "C++: pystorm::bddriver::BDState::GetMem(pystorm::bddriver::bdpars::BDMemId) const --> const class std::vector<unsigned long, class std::allocator<unsigned long> > *", pybind11::return_value_policy::automatic, pybind11::arg("mem_id"));
		cl.def("SetReg", (void (pystorm::bddriver::BDState::*)(pystorm::bddriver::bdpars::BDHornEP, unsigned long)) &pystorm::bddriver::BDState::SetReg, "C++: pystorm::bddriver::BDState::SetReg(pystorm::bddriver::bdpars::BDHornEP, unsigned long) --> void", pybind11::arg("reg_id"), pybind11::arg("data"));
		cl.def("GetReg", (const struct std::pair<const unsigned long, bool> (pystorm::bddriver::BDState::*)(pystorm::bddriver::bdpars::BDHornEP) const) &pystorm::bddriver::BDState::GetReg, "C++: pystorm::bddriver::BDState::GetReg(pystorm::bddriver::bdpars::BDHornEP) const --> const struct std::pair<const unsigned long, bool>", pybind11::arg("reg_id"));
		cl.def("SetNeuronConfigMem", (void (pystorm::bddriver::BDState::*)(unsigned int, unsigned int, unsigned int, pystorm::bddriver::bdpars::ConfigSomaID, unsigned int)) &pystorm::bddriver::BDState::SetNeuronConfigMem, "C++: pystorm::bddriver::BDState::SetNeuronConfigMem(unsigned int, unsigned int, unsigned int, pystorm::bddriver::bdpars::ConfigSomaID, unsigned int) --> void", pybind11::arg("core_id"), pybind11::arg("tile_id"), pybind11::arg("elem_id"), pybind11::arg("config_type"), pybind11::arg("config_value"));
		cl.def("SetNeuronConfigMem", (void (pystorm::bddriver::BDState::*)(unsigned int, unsigned int, unsigned int, pystorm::bddriver::bdpars::ConfigSynapseID, unsigned int)) &pystorm::bddriver::BDState::SetNeuronConfigMem, "C++: pystorm::bddriver::BDState::SetNeuronConfigMem(unsigned int, unsigned int, unsigned int, pystorm::bddriver::bdpars::ConfigSynapseID, unsigned int) --> void", pybind11::arg("core_id"), pybind11::arg("tile_id"), pybind11::arg("elem_id"), pybind11::arg("config_type"), pybind11::arg("config_value"));
		cl.def("SetNeuronConfigMem", (void (pystorm::bddriver::BDState::*)(unsigned int, unsigned int, unsigned int, pystorm::bddriver::bdpars::DiffusorCutLocationId, unsigned int)) &pystorm::bddriver::BDState::SetNeuronConfigMem, "C++: pystorm::bddriver::BDState::SetNeuronConfigMem(unsigned int, unsigned int, unsigned int, pystorm::bddriver::bdpars::DiffusorCutLocationId, unsigned int) --> void", pybind11::arg("core_id"), pybind11::arg("tile_id"), pybind11::arg("elem_id"), pybind11::arg("config_type"), pybind11::arg("config_value"));
        cl.def("GetSomaConfigMem", &pystorm::bddriver::BDState::GetSomaConfigMem);
        cl.def("GetSynapseConfigMem", &pystorm::bddriver::BDState::GetSynapseConfigMem);
        cl.def("GetDiffusorConfigMem", &pystorm::bddriver::BDState::GetDiffusorConfigMem);
		cl.def("SetToggle", (void (pystorm::bddriver::BDState::*)(pystorm::bddriver::bdpars::BDHornEP, bool, bool)) &pystorm::bddriver::BDState::SetToggle, "C++: pystorm::bddriver::BDState::SetToggle(pystorm::bddriver::bdpars::BDHornEP, bool, bool) --> void", pybind11::arg("reg_id"), pybind11::arg("traffic_en"), pybind11::arg("dump_en"));
		cl.def("GetToggle", (class std::tuple<bool, bool, bool> (pystorm::bddriver::BDState::*)(pystorm::bddriver::bdpars::BDHornEP) const) &pystorm::bddriver::BDState::GetToggle, "C++: pystorm::bddriver::BDState::GetToggle(pystorm::bddriver::bdpars::BDHornEP) const --> class std::tuple<bool, bool, bool>", pybind11::arg("reg_id"));
		cl.def("AreTrafficRegsOff", (bool (pystorm::bddriver::BDState::*)() const) &pystorm::bddriver::BDState::AreTrafficRegsOff, "C++: pystorm::bddriver::BDState::AreTrafficRegsOff() const --> bool");
		cl.def("IsTrafficOff", (bool (pystorm::bddriver::BDState::*)() const) &pystorm::bddriver::BDState::IsTrafficOff, "is traffic_en == false for all traffic_regs_\n\nC++: pystorm::bddriver::BDState::IsTrafficOff() const --> bool");
		cl.def("WaitForTrafficOff", (void (pystorm::bddriver::BDState::*)() const) &pystorm::bddriver::BDState::WaitForTrafficOff, "has AreTrafficRegsOff been true for traffic_drain_us\n\nC++: pystorm::bddriver::BDState::WaitForTrafficOff() const --> void");
	}
	{ // pystorm::bddriver::Driver file:Driver.h line:96
		pybind11::class_<pystorm::bddriver::Driver> cl(M("pystorm::bddriver"), "Driver", "Driver provides low-level, but not dead-stupid, control over the BD hardware.\n Driver tries to provide a complete but not needlessly tedious interface to BD.\n It also tries to prevent the user to do anything that would crash the chip.\n\n Driver looks like this:\n\n                              (user/HAL)\n\n  ---[fns]--[fns]--[fns]----------------------[fns]-----------------------[fns]----  API\n       |      |      |            |             A                           A\n       V      V      V            |             |                           |\n  [private fns, e.g. PackWords]   |        [XXXX private fns, e.g. UnpackWords XXXX]\n          |        |              |             A                           A\n          V        V           [BDState]        |                           |\n   [MutexBuffer:enc_buf_in_]      |      [M.B.:dec_buf_out_[0]]    [M.B.:dec_buf_out_[0]] ...\n              |                   |                   A                   A\n              |                   |                   |                   |\n   ----------------------------[BDPars]------------------------------------------- funnel/horn payloads,\n              |                   |                   |                   |           organized by leaf\n              V                   |                   |                   |\n      [Encoder:encoder_]          |        [XXXXXXXXXXXX Decoder:decoder_ XXXXXXXXXX]\n              |                   |                        A\n              V                   |                        |\n   [MutexBuffer:enc_buf_out_]     |           [MutexBuffer:dec_buf_in_]\n              |                   |                      A\n              |                   |                      |\n  --------------------------------------------------------------------------------- raw data\n              |                                          |\n              V                                          |\n         [XXXXXXXXXXXXXXXXXXXX Comm:comm_ XXXXXXXXXXXXXXXXXXXX]\n                               |      A\n                               V      |\n  --------------------------------------------------------------------------------- USB\n\n                              (Braindrop)\n\n At the heart of driver are a few primary components:\n\n - Encoder\n     Inputs: raw payloads (already serialized, if necessary) and BD horn ids to send them to\n     Outputs: inputs suitable to send to BD, packed into char stream\n     Spawns its own thread.\n\n - Decoder\n     Inputs: char stream of outputs from BD\n     Outputs: one stream per horn leaf of raw payloads from that leaf\n     Spawns its own thread.\n\n - Comm\n     Communicates with BD using libUSB, taking inputs from/giving outputs to\n     the Encoder/Decoder. Spawns its own thread.\n\n - MutexBuffers\n     Provide thread-safe communication and buffering for the inputs and outputs of Encoder\n     and decoder. Note that there are many decoder output buffers, one per funnel leaf.\n\n - BDPars\n     Holds all the nitty-gritty hardware information. The rest of the driver doesn't know\n     anything about word field orders or sizes, for example.\n\n - BDState\n     Software model of the hardware state. Keep track of all the memory words that have\n     been programmed, registers that have been set, etc.\n     Also keeps track of timing assumptions, e.g. whether the traffic has drained after\n     turning off all of the toggles that stop it.");

		cl.def(pybind11::init<>());

		cl.def_readwrite("SetSomaConfigMemory", &pystorm::bddriver::Driver::SetSomaConfigMemory);
		cl.def_readwrite("EnableSoma", &pystorm::bddriver::Driver::EnableSoma);
		cl.def_readwrite("DisableSoma", &pystorm::bddriver::Driver::DisableSoma);
		cl.def_readwrite("SetSynapseConfigMemory", &pystorm::bddriver::Driver::SetSynapseConfigMemory);
		cl.def_readwrite("EnableSynapse", &pystorm::bddriver::Driver::EnableSynapse);
		cl.def_readwrite("DisableSynapse", &pystorm::bddriver::Driver::DisableSynapse);
		cl.def_readwrite("EnableSynapseADC", &pystorm::bddriver::Driver::EnableSynapseADC);
		cl.def_readwrite("DisableSynapseADC", &pystorm::bddriver::Driver::DisableSynapseADC);
		cl.def_readwrite("SetDiffusorConfigMemory", &pystorm::bddriver::Driver::SetDiffusorConfigMemory);
		cl.def_readwrite("OpenDiffusorCut", &pystorm::bddriver::Driver::OpenDiffusorCut);
		cl.def_readwrite("CloseDiffusorCut", &pystorm::bddriver::Driver::CloseDiffusorCut);
		cl.def_readwrite("OpenDiffusorAllCuts", &pystorm::bddriver::Driver::OpenDiffusorAllCuts);
		cl.def_readwrite("CloseDiffusorAllCuts", &pystorm::bddriver::Driver::CloseDiffusorAllCuts);
		cl.def("GetBDPars", (const class pystorm::bddriver::bdpars::BDPars * (pystorm::bddriver::Driver::*)()) &pystorm::bddriver::Driver::GetBDPars, "C++: pystorm::bddriver::Driver::GetBDPars() --> const class pystorm::bddriver::bdpars::BDPars *", pybind11::return_value_policy::reference_internal);
		cl.def("GetDriverPars", (const class pystorm::bddriver::driverpars::DriverPars * (pystorm::bddriver::Driver::*)()) &pystorm::bddriver::Driver::GetDriverPars, "C++: pystorm::bddriver::Driver::GetDriverPars() --> const class pystorm::bddriver::driverpars::DriverPars *", pybind11::return_value_policy::automatic);
		cl.def("GetState", (const class pystorm::bddriver::BDState * (pystorm::bddriver::Driver::*)(unsigned int)) &pystorm::bddriver::Driver::GetState, "C++: pystorm::bddriver::Driver::GetState(unsigned int) --> const class pystorm::bddriver::BDState *", pybind11::return_value_policy::automatic, pybind11::arg("core_id"));
		cl.def("testcall", (void (pystorm::bddriver::Driver::*)(const std::string &)) &pystorm::bddriver::Driver::testcall, "C++: pystorm::bddriver::Driver::testcall(const class std::__cxx11::basic_string<char> &) --> void", pybind11::arg("msg"));
		cl.def("Start", (void (pystorm::bddriver::Driver::*)()) &pystorm::bddriver::Driver::Start, "starts child workers, e.g. encoder and decoder\n\nC++: pystorm::bddriver::Driver::Start() --> void");
		cl.def("Stop", (void (pystorm::bddriver::Driver::*)()) &pystorm::bddriver::Driver::Stop, "stops the child workers\n\nC++: pystorm::bddriver::Driver::Stop() --> void");
		cl.def("InitBD", (void (pystorm::bddriver::Driver::*)()) &pystorm::bddriver::Driver::InitBD, "Initializes hardware state\n Calls Flush immediately\n\nC++: pystorm::bddriver::Driver::InitBD() --> void");
		cl.def("InitFIFO", (void (pystorm::bddriver::Driver::*)(unsigned int)) &pystorm::bddriver::Driver::InitFIFO, "Clears BD FIFOs\n Calls Flush immediately\n\nC++: pystorm::bddriver::Driver::InitFIFO(unsigned int) --> void", pybind11::arg("core_id"));
		cl.def("Flush", (void (pystorm::bddriver::Driver::*)()) &pystorm::bddriver::Driver::Flush, "Flush queued up downstream traffic\n Commits queued-up messages (sends enough nops to flush the USB)\n By default, many configuration calls will call Flush()\n Notably, the Neuron config calls do not call Flush()\n\nC++: pystorm::bddriver::Driver::Flush() --> void");
		cl.def("SetTagTrafficState", [](pystorm::bddriver::Driver &o, unsigned int  const &a0, bool  const &a1) -> void { return o.SetTagTrafficState(a0, a1); }, "", pybind11::arg("core_id"), pybind11::arg("en"));
		cl.def("SetTagTrafficState", (void (pystorm::bddriver::Driver::*)(unsigned int, bool, bool)) &pystorm::bddriver::Driver::SetTagTrafficState, "Control tag traffic\n\nC++: pystorm::bddriver::Driver::SetTagTrafficState(unsigned int, bool, bool) --> void", pybind11::arg("core_id"), pybind11::arg("en"), pybind11::arg("flush"));
		cl.def("SetSpikeTrafficState", [](pystorm::bddriver::Driver &o, unsigned int  const &a0, bool  const &a1) -> void { return o.SetSpikeTrafficState(a0, a1); }, "", pybind11::arg("core_id"), pybind11::arg("en"));
		cl.def("SetSpikeTrafficState", (void (pystorm::bddriver::Driver::*)(unsigned int, bool, bool)) &pystorm::bddriver::Driver::SetSpikeTrafficState, "Control spike traffic from neuron array to datapath\n\nC++: pystorm::bddriver::Driver::SetSpikeTrafficState(unsigned int, bool, bool) --> void", pybind11::arg("core_id"), pybind11::arg("en"), pybind11::arg("flush"));
		cl.def("SetSpikeDumpState", [](pystorm::bddriver::Driver &o, unsigned int  const &a0, bool  const &a1) -> void { return o.SetSpikeDumpState(a0, a1); }, "", pybind11::arg("core_id"), pybind11::arg("en"));
		cl.def("SetSpikeDumpState", (void (pystorm::bddriver::Driver::*)(unsigned int, bool, bool)) &pystorm::bddriver::Driver::SetSpikeDumpState, "Control spike traffic from neuron array to driver\n\nC++: pystorm::bddriver::Driver::SetSpikeDumpState(unsigned int, bool, bool) --> void", pybind11::arg("core_id"), pybind11::arg("en"), pybind11::arg("flush"));
		cl.def("SetDACValue", [](pystorm::bddriver::Driver &o, unsigned int  const &a0, pystorm::bddriver::bdpars::BDHornEP  const &a1, unsigned int  const &a2) -> void { return o.SetDACValue(a0, a1, a2); }, "", pybind11::arg("core_id"), pybind11::arg("signal_id"), pybind11::arg("value"));
		cl.def("SetDACValue", (void (pystorm::bddriver::Driver::*)(unsigned int, pystorm::bddriver::bdpars::BDHornEP, unsigned int, bool)) &pystorm::bddriver::Driver::SetDACValue, "Program DAC value\n\nC++: pystorm::bddriver::Driver::SetDACValue(unsigned int, pystorm::bddriver::bdpars::BDHornEP, unsigned int, bool) --> void", pybind11::arg("core_id"), pybind11::arg("signal_id"), pybind11::arg("value"), pybind11::arg("flush"));
		cl.def("SetDACValue", (void (pystorm::bddriver::Driver::*)(unsigned int, pystorm::bddriver::bdpars::BDHornEP, float, bool)) &pystorm::bddriver::Driver::SetDACValue, "", pybind11::arg("core_id"), pybind11::arg("signal_id"), pybind11::arg("value"), pybind11::arg("flush"));
		cl.def("SetDACValue", [](pystorm::bddriver::Driver &o, unsigned int  const &a0, pystorm::bddriver::bdpars::BDHornEP  const &a1, float const &a2) -> void { return o.SetDACValue(a0, a1, a2); }, "", pybind11::arg("core_id"), pybind11::arg("signal_id"), pybind11::arg("value"));
		cl.def("SetDACtoADCConnectionState", [](pystorm::bddriver::Driver &o, unsigned int  const &a0, pystorm::bddriver::bdpars::BDHornEP  const &a1, bool  const &a2) -> void { return o.SetDACtoADCConnectionState(a0, a1, a2); }, "", pybind11::arg("core_id"), pybind11::arg("dac_signal_id"), pybind11::arg("en"));
		cl.def("SetDACtoADCConnectionState", (void (pystorm::bddriver::Driver::*)(unsigned int, pystorm::bddriver::bdpars::BDHornEP, bool, bool)) &pystorm::bddriver::Driver::SetDACtoADCConnectionState, "Make DAC-to-ADC connection for calibration for a particular DAC\n\nC++: pystorm::bddriver::Driver::SetDACtoADCConnectionState(unsigned int, pystorm::bddriver::bdpars::BDHornEP, bool, bool) --> void", pybind11::arg("core_id"), pybind11::arg("dac_signal_id"), pybind11::arg("en"), pybind11::arg("flush"));
        cl.def("GetDACScaling", &pystorm::bddriver::Driver::GetDACScaling, "", pybind11::arg("dac_signal_id"));
        cl.def("GetDACUnitCurrent", &pystorm::bddriver::Driver::GetDACUnitCurrent, "", pybind11::arg("dac_signal_id"));
        cl.def("GetDACDefaultCount", &pystorm::bddriver::Driver::GetDACDefaultCount, "", pybind11::arg("dac_signal_id"));
		cl.def("SetADCScale", (void (pystorm::bddriver::Driver::*)(unsigned int, bool, const std::string &)) &pystorm::bddriver::Driver::SetADCScale, "Set large/small current scale for either ADC\n\nC++: pystorm::bddriver::Driver::SetADCScale(unsigned int, bool, const class std::__cxx11::basic_string<char> &) --> void", pybind11::arg("core_id"), pybind11::arg("adc_id"), pybind11::arg("small_or_large"));
		cl.def("SetADCTrafficState", (void (pystorm::bddriver::Driver::*)(unsigned int, bool)) &pystorm::bddriver::Driver::SetADCTrafficState, "Turn ADC output on\n\nC++: pystorm::bddriver::Driver::SetADCTrafficState(unsigned int, bool) --> void", pybind11::arg("core_id"), pybind11::arg("en"));
		cl.def("SetSomaEnableStatus", (void (pystorm::bddriver::Driver::*)(unsigned int, unsigned int, pystorm::bddriver::bdpars::SomaStatusId)) &pystorm::bddriver::Driver::SetSomaEnableStatus, "Enable/Disable Soma\n Map between memory and status\n     _KILL       Status\n       0         DISABLED\n       1         ENABLED\n\nC++: pystorm::bddriver::Driver::SetSomaEnableStatus(unsigned int, unsigned int, pystorm::bddriver::bdpars::SomaStatusId) --> void", pybind11::arg("core_id"), pybind11::arg("soma_id"), pybind11::arg("status"));
		cl.def("SetSomaGain", (void (pystorm::bddriver::Driver::*)(unsigned int, unsigned int, pystorm::bddriver::bdpars::SomaGainId)) &pystorm::bddriver::Driver::SetSomaGain, "Set Soma gain (post rectifier)\n Map between memory and gain values:\n     G<1>        G<0>        Gain\n      0           0          ONE_FOURTH (1/4)\n      0           1          ONE_THIRD (1/3)\n      1           0          ONE_HALF (1/2)\n      1           1          ONE (1)\n\nC++: pystorm::bddriver::Driver::SetSomaGain(unsigned int, unsigned int, pystorm::bddriver::bdpars::SomaGainId) --> void", pybind11::arg("core_id"), pybind11::arg("soma_id"), pybind11::arg("gain"));
		cl.def("SetSomaOffsetSign", (void (pystorm::bddriver::Driver::*)(unsigned int, unsigned int, pystorm::bddriver::bdpars::SomaOffsetSignId)) &pystorm::bddriver::Driver::SetSomaOffsetSign, "Set offset sign (pre rectifier)\n Map between memory and sign\n     _ENPOSBIAS  Sign\n       0         POSITIVE\n       1         NEGATIVE\n\nC++: pystorm::bddriver::Driver::SetSomaOffsetSign(unsigned int, unsigned int, pystorm::bddriver::bdpars::SomaOffsetSignId) --> void", pybind11::arg("core_id"), pybind11::arg("soma_id"), pybind11::arg("offset_sign"));
		cl.def("SetSomaOffsetMultiplier", (void (pystorm::bddriver::Driver::*)(unsigned int, unsigned int, pystorm::bddriver::bdpars::SomaOffsetMultiplierId)) &pystorm::bddriver::Driver::SetSomaOffsetMultiplier, "Set Soma offset gain (pre rectifier)\n Map between memory and gain values:\n     B<1>        B<0>        Gain\n      0           0          ZERO (0)\n      0           1          ONE (1)\n      1           0          TWO (2)\n      1           1          THREE (3)\n\nC++: pystorm::bddriver::Driver::SetSomaOffsetMultiplier(unsigned int, unsigned int, pystorm::bddriver::bdpars::SomaOffsetMultiplierId) --> void", pybind11::arg("core_id"), pybind11::arg("soma_id"), pybind11::arg("soma_offset_multiplier"));
		cl.def("SetSynapseEnableStatus", (void (pystorm::bddriver::Driver::*)(unsigned int, unsigned int, pystorm::bddriver::bdpars::SynapseStatusId)) &pystorm::bddriver::Driver::SetSynapseEnableStatus, "Enable/Disable Synapse\n Map between memory and status\n     KILL        Status\n       0         ENABLED\n       1         DISABLED\n\nC++: pystorm::bddriver::Driver::SetSynapseEnableStatus(unsigned int, unsigned int, pystorm::bddriver::bdpars::SynapseStatusId) --> void", pybind11::arg("core_id"), pybind11::arg("synapse_id"), pybind11::arg("synapse_status"));
		cl.def("SetSynapseADCStatus", (void (pystorm::bddriver::Driver::*)(unsigned int, unsigned int, pystorm::bddriver::bdpars::SynapseStatusId)) &pystorm::bddriver::Driver::SetSynapseADCStatus, "Enable/Disable Synapse ADC\n Map between memory and status\n     _ADC        Status\n       0         ENABLED\n       1         DISABLED\n\nC++: pystorm::bddriver::Driver::SetSynapseADCStatus(unsigned int, unsigned int, pystorm::bddriver::bdpars::SynapseStatusId) --> void", pybind11::arg("core_id"), pybind11::arg("synapse_id"), pybind11::arg("synapse_status"));
		cl.def("SetDiffusorCutStatus", (void (pystorm::bddriver::Driver::*)(unsigned int, unsigned int, pystorm::bddriver::bdpars::DiffusorCutLocationId, pystorm::bddriver::bdpars::DiffusorCutStatusId)) &pystorm::bddriver::Driver::SetDiffusorCutStatus, "C++: pystorm::bddriver::Driver::SetDiffusorCutStatus(unsigned int, unsigned int, pystorm::bddriver::bdpars::DiffusorCutLocationId, pystorm::bddriver::bdpars::DiffusorCutStatusId) --> void", pybind11::arg("core_id"), pybind11::arg("tile_id"), pybind11::arg("cut_id"), pybind11::arg("status"));
		cl.def("SetDiffusorAllCutsStatus", (void (pystorm::bddriver::Driver::*)(unsigned int, unsigned int, pystorm::bddriver::bdpars::DiffusorCutStatusId)) &pystorm::bddriver::Driver::SetDiffusorAllCutsStatus, "Set all the diffusor cuts' status for a tile\n\nC++: pystorm::bddriver::Driver::SetDiffusorAllCutsStatus(unsigned int, unsigned int, pystorm::bddriver::bdpars::DiffusorCutStatusId) --> void", pybind11::arg("core_id"), pybind11::arg("tile_id"), pybind11::arg("status"));
		cl.def("SetMemoryDelay", [](pystorm::bddriver::Driver &o, unsigned int  const &a0, pystorm::bddriver::bdpars::BDMemId  const &a1, unsigned int  const &a2, unsigned int  const &a3) -> void { return o.SetMemoryDelay(a0, a1, a2, a3); }, "", pybind11::arg("core_id"), pybind11::arg("mem_id"), pybind11::arg("read_value"), pybind11::arg("write_value"));
		cl.def("SetMemoryDelay", (void (pystorm::bddriver::Driver::*)(unsigned int, pystorm::bddriver::bdpars::BDMemId, unsigned int, unsigned int, bool)) &pystorm::bddriver::Driver::SetMemoryDelay, "Set memory delay line value\n\nC++: pystorm::bddriver::Driver::SetMemoryDelay(unsigned int, pystorm::bddriver::bdpars::BDMemId, unsigned int, unsigned int, bool) --> void", pybind11::arg("core_id"), pybind11::arg("mem_id"), pybind11::arg("read_value"), pybind11::arg("write_value"), pybind11::arg("flush"));
		cl.def("SetMem", (void (pystorm::bddriver::Driver::*)(unsigned int, pystorm::bddriver::bdpars::BDMemId, const class std::vector<unsigned long, class std::allocator<unsigned long> > &, unsigned int)) &pystorm::bddriver::Driver::SetMem, "Program a memory.\n BDWords must be constructed as the correct word type for the mem_id\n\nC++: pystorm::bddriver::Driver::SetMem(unsigned int, pystorm::bddriver::bdpars::BDMemId, const class std::vector<unsigned long, class std::allocator<unsigned long> > &, unsigned int) --> void", pybind11::arg("core_id"), pybind11::arg("mem_id"), pybind11::arg("data"), pybind11::arg("start_addr"));
		cl.def("DumpMem", (class std::vector<unsigned long, class std::allocator<unsigned long> > (pystorm::bddriver::Driver::*)(unsigned int, pystorm::bddriver::bdpars::BDMemId)) &pystorm::bddriver::Driver::DumpMem, "Dump the contents of one of the memories.\n BDWords must subsequently be unpacked as the correct word type for the mem_id\n\nC++: pystorm::bddriver::Driver::DumpMem(unsigned int, pystorm::bddriver::bdpars::BDMemId) --> class std::vector<unsigned long, class std::allocator<unsigned long> >", pybind11::arg("core_id"), pybind11::arg("mem_id"));
		cl.def("SetPreFIFODumpState", (void (pystorm::bddriver::Driver::*)(unsigned int, bool)) &pystorm::bddriver::Driver::SetPreFIFODumpState, "Dump copy of traffic pre-FIFO\n\nC++: pystorm::bddriver::Driver::SetPreFIFODumpState(unsigned int, bool) --> void", pybind11::arg("core_id"), pybind11::arg("dump_en"));
		cl.def("SetPostFIFODumpState", (void (pystorm::bddriver::Driver::*)(unsigned int, bool)) &pystorm::bddriver::Driver::SetPostFIFODumpState, "Dump copy of traffic post-FIFO, tag msbs = 0\n\nC++: pystorm::bddriver::Driver::SetPostFIFODumpState(unsigned int, bool) --> void", pybind11::arg("core_id"), pybind11::arg("dump_en"));
		cl.def("GetPreFIFODump", (class std::vector<unsigned long, class std::allocator<unsigned long> > (pystorm::bddriver::Driver::*)(unsigned int)) &pystorm::bddriver::Driver::GetPreFIFODump, "Get pre-FIFO tags recorded during dump\n\nC++: pystorm::bddriver::Driver::GetPreFIFODump(unsigned int) --> class std::vector<unsigned long, class std::allocator<unsigned long> >", pybind11::arg("core_id"));
		cl.def("GetPostFIFODump", (struct std::pair<class std::vector<unsigned long, class std::allocator<unsigned long> >, class std::vector<unsigned long, class std::allocator<unsigned long> > > (pystorm::bddriver::Driver::*)(unsigned int)) &pystorm::bddriver::Driver::GetPostFIFODump, "Get post-FIFO tags recorded during dump\n\nC++: pystorm::bddriver::Driver::GetPostFIFODump(unsigned int) --> struct std::pair<class std::vector<unsigned long, class std::allocator<unsigned long> >, class std::vector<unsigned long, class std::allocator<unsigned long> > >", pybind11::arg("core_id"));
		cl.def("GetFIFOOverflowCounts", (struct std::pair<unsigned int, unsigned int> (pystorm::bddriver::Driver::*)(unsigned int)) &pystorm::bddriver::Driver::GetFIFOOverflowCounts, "Get warning count\n\nC++: pystorm::bddriver::Driver::GetFIFOOverflowCounts(unsigned int) --> struct std::pair<unsigned int, unsigned int>", pybind11::arg("core_id"));
		cl.def("SendSpikes", [](pystorm::bddriver::Driver &o, unsigned int  const &a0, const class std::vector<pystorm::bddriver::BDWord, class std::allocator<pystorm::bddriver::BDWord> > & a1, const class std::vector<unsigned int, class std::allocator<unsigned int> >  &a2) -> void { return o.SendSpikes(a0, a1, a2); }, "", pybind11::arg("core_id"), pybind11::arg("spikes"), pybind11::arg("times"));
		cl.def("SendSpikes", (void (pystorm::bddriver::Driver::*)(unsigned int, const class std::vector<unsigned long, class std::allocator<unsigned long> > &, const class std::vector<unsigned int, class std::allocator<unsigned int> >, bool)) &pystorm::bddriver::Driver::SendSpikes, "Send a stream of spikes to neurons\n\nC++: pystorm::bddriver::Driver::SendSpikes(unsigned int, const class std::vector<unsigned long, class std::allocator<unsigned long> > &, const class std::vector<unsigned int, class std::allocator<unsigned int> >, bool) --> void", pybind11::arg("core_id"), pybind11::arg("spikes"), pybind11::arg("times"), pybind11::arg("flush"));
		cl.def("RecvSpikes", (struct std::pair<class std::vector<unsigned long, class std::allocator<unsigned long> >, class std::vector<unsigned int, class std::allocator<unsigned int> > > (pystorm::bddriver::Driver::*)(unsigned int)) &pystorm::bddriver::Driver::RecvSpikes, "Receive a stream of spikes\n\nC++: pystorm::bddriver::Driver::RecvSpikes(unsigned int) --> struct std::pair<class std::vector<unsigned long, class std::allocator<unsigned long> >, class std::vector<unsigned int, class std::allocator<unsigned int> > >", pybind11::arg("core_id"));
		cl.def("SendTags", [](pystorm::bddriver::Driver &o, unsigned int  const &a0, const class std::vector<pystorm::bddriver::BDWord, class std::allocator<pystorm::bddriver::BDWord> > & a1, const class std::vector<unsigned int, class std::allocator<unsigned int> >  &a2) -> void { return o.SendTags(a0, a1, a2); }, "", pybind11::arg("core_id"), pybind11::arg("tags"), pybind11::arg("times"));
		cl.def("SendTags", (void (pystorm::bddriver::Driver::*)(unsigned int, const class std::vector<unsigned long, class std::allocator<unsigned long> > &, const class std::vector<unsigned int, class std::allocator<unsigned int> >, bool)) &pystorm::bddriver::Driver::SendTags, "Send a stream of tags\n\nC++: pystorm::bddriver::Driver::SendTags(unsigned int, const class std::vector<unsigned long, class std::allocator<unsigned long> > &, const class std::vector<unsigned int, class std::allocator<unsigned int> >, bool) --> void", pybind11::arg("core_id"), pybind11::arg("tags"), pybind11::arg("times"), pybind11::arg("flush"));
		cl.def("RecvTags", (struct std::pair<class std::vector<unsigned long, class std::allocator<unsigned long> >, class std::vector<unsigned int, class std::allocator<unsigned int> > > (pystorm::bddriver::Driver::*)(unsigned int)) &pystorm::bddriver::Driver::RecvTags, "Receive a stream of tags\n receive from both tag output leaves, the Acc and TAT\n\nC++: pystorm::bddriver::Driver::RecvTags(unsigned int) --> struct std::pair<class std::vector<unsigned long, class std::allocator<unsigned long> >, class std::vector<unsigned int, class std::allocator<unsigned int> > >", pybind11::arg("core_id"));
		cl.def("GetRegState", (const struct std::pair<const unsigned long, bool> (pystorm::bddriver::Driver::*)(unsigned int, pystorm::bddriver::bdpars::BDHornEP) const) &pystorm::bddriver::Driver::GetRegState, "Get register contents by name.\n\nC++: pystorm::bddriver::Driver::GetRegState(unsigned int, pystorm::bddriver::bdpars::BDHornEP) const --> const struct std::pair<const unsigned long, bool>", pybind11::arg("core_id"), pybind11::arg("reg_id"));
		cl.def("GetMemState", (const class std::vector<unsigned long, class std::allocator<unsigned long> > * (pystorm::bddriver::Driver::*)(pystorm::bddriver::bdpars::BDMemId, unsigned int) const) &pystorm::bddriver::Driver::GetMemState, "Get software state of memory contents: this DOES NOT dump the memory.\n\nC++: pystorm::bddriver::Driver::GetMemState(pystorm::bddriver::bdpars::BDMemId, unsigned int) const --> const class std::vector<unsigned long, class std::allocator<unsigned long> > *", pybind11::return_value_policy::automatic, pybind11::arg("mem_id"), pybind11::arg("core_id"));
	}
}

void bind_unknown_unknown_3(std::function< pybind11::module &(std::string const &namespace_) > &M)
{
	{ // pystorm::bddriver::bdmodel::BDModel file: line:21
		pybind11::class_<pystorm::bddriver::bdmodel::BDModel> cl(M("pystorm::bddriver::bdmodel"), "BDModel", "BDModel pretends to be the BD hardware.\n Public ifc is threadsafe.");
		cl.def(pybind11::init<const class pystorm::bddriver::bdpars::BDPars *, const class pystorm::bddriver::driverpars::DriverPars *>(), pybind11::arg("bd_pars"), pybind11::arg("driver_pars"));

		cl.def("ParseInput", (void (pystorm::bddriver::bdmodel::BDModel::*)(const class std::vector<unsigned char, class std::allocator<unsigned char> > &)) &pystorm::bddriver::bdmodel::BDModel::ParseInput, "parse input stream to update internal BDState object and other state\n\nC++: pystorm::bddriver::bdmodel::BDModel::ParseInput(const class std::vector<unsigned char, class std::allocator<unsigned char> > &) --> void", pybind11::arg("input_stream"));
		cl.def("GenerateOutputs", (class std::vector<unsigned char, class std::allocator<unsigned char> > (pystorm::bddriver::bdmodel::BDModel::*)()) &pystorm::bddriver::bdmodel::BDModel::GenerateOutputs, "given internal state, generate requested output stream\n\nC++: pystorm::bddriver::bdmodel::BDModel::GenerateOutputs() --> class std::vector<unsigned char, class std::allocator<unsigned char> >");
		cl.def("PushOutput", (void (pystorm::bddriver::bdmodel::BDModel::*)(unsigned char, const class std::vector<unsigned long, class std::allocator<unsigned long> > &)) &pystorm::bddriver::bdmodel::BDModel::PushOutput, "C++: pystorm::bddriver::bdmodel::BDModel::PushOutput(unsigned char, const class std::vector<unsigned long, class std::allocator<unsigned long> > &) --> void", pybind11::arg("ep"), pybind11::arg("to_append"));
		cl.def("LockState", (const class pystorm::bddriver::BDState * (pystorm::bddriver::bdmodel::BDModel::*)()) &pystorm::bddriver::bdmodel::BDModel::LockState, "lock the model, get a const ptr to the state, examine it as you like...\n\nC++: pystorm::bddriver::bdmodel::BDModel::LockState() --> const class pystorm::bddriver::BDState *", pybind11::return_value_policy::automatic);
		cl.def("UnlockState", (void (pystorm::bddriver::bdmodel::BDModel::*)()) &pystorm::bddriver::bdmodel::BDModel::UnlockState, "then unlock the model when you're done\n\nC++: pystorm::bddriver::bdmodel::BDModel::UnlockState() --> void");
		cl.def("PopSpikes", (class std::vector<unsigned long, class std::allocator<unsigned long> > (pystorm::bddriver::bdmodel::BDModel::*)()) &pystorm::bddriver::bdmodel::BDModel::PopSpikes, "C++: pystorm::bddriver::bdmodel::BDModel::PopSpikes() --> class std::vector<unsigned long, class std::allocator<unsigned long> >");
		cl.def("PopTags", (class std::vector<unsigned long, class std::allocator<unsigned long> > (pystorm::bddriver::bdmodel::BDModel::*)()) &pystorm::bddriver::bdmodel::BDModel::PopTags, "C++: pystorm::bddriver::bdmodel::BDModel::PopTags() --> class std::vector<unsigned long, class std::allocator<unsigned long> >");
	}
}

void bind_MemInfo(std::function< pybind11::module &(std::string const &namespace_) > &M)
{
	{ // pystorm::bddriver::bdpars::MemInfo file: line:203
		pybind11::class_<pystorm::bddriver::bdpars::MemInfo, std::shared_ptr<pystorm::bddriver::bdpars::MemInfo>> cl(M("pystorm::bddriver::bdpars"), "MemInfo", "");
		cl.def(pybind11::init<>());
		cl.def(pybind11::init<const struct pystorm::bddriver::bdpars::MemInfo &>(), pybind11::arg(""));

		cl.def_readwrite("size", &pystorm::bddriver::bdpars::MemInfo::size);
		cl.def_readwrite("prog_leaf", &pystorm::bddriver::bdpars::MemInfo::prog_leaf);
		cl.def_readwrite("dump_leaf", &pystorm::bddriver::bdpars::MemInfo::dump_leaf);
		cl.def_readwrite("delay_reg", &pystorm::bddriver::bdpars::MemInfo::delay_reg);
	}
}

void bind_model_BDModelDriver(std::function< pybind11::module &(std::string const &namespace_) > &M)
{
	{ // pystorm::bddriver::BDModelDriver file:model/BDModelDriver.h line:15
		pybind11::class_<pystorm::bddriver::BDModelDriver, pystorm::bddriver::Driver> cl(M("pystorm::bddriver"), "BDModelDriver", "Specialization of Driver that uses BDModelComm.\n I could have made Driver depend on BDModel, and have an optional constructor\n argument. I opted to subclass instead to contain the dependency to this \n particular (testing-only) use case.");
		cl.def(pybind11::init<>());

		cl.def("GetBDModel", (class pystorm::bddriver::bdmodel::BDModel * (pystorm::bddriver::BDModelDriver::*)()) &pystorm::bddriver::BDModelDriver::GetBDModel, "C++: pystorm::bddriver::BDModelDriver::GetBDModel() --> class pystorm::bddriver::bdmodel::BDModel *", pybind11::return_value_policy::reference_internal);
	}
}

// From BDWord.h
void bind_BDWord(std::function< pybind11::module &(std::string const &namespace_) > &M)
{
	// pystorm::bddriver::ToggleWord file: line:69
	pybind11::enum_<pystorm::bddriver::ToggleWord>(M("pystorm::bddriver"), "ToggleWord", "")
		.value("TRAFFIC_ENABLE", pystorm::bddriver::ToggleWord::TRAFFIC_ENABLE)
		.value("DUMP_ENABLE", pystorm::bddriver::ToggleWord::DUMP_ENABLE)
		.value("FIELDCOUNT", pystorm::bddriver::ToggleWord::FIELDCOUNT);

;

	// pystorm::bddriver::FieldWidth(pystorm::bddriver::ToggleWord) file: line:56
	M("pystorm::bddriver").def("FieldWidth", (unsigned int (*)(pystorm::bddriver::ToggleWord)) &pystorm::bddriver::FieldWidth, "C++: pystorm::bddriver::FieldWidth(pystorm::bddriver::ToggleWord) --> unsigned int", pybind11::arg("field_name"));

	// pystorm::bddriver::FieldHCVal(pystorm::bddriver::ToggleWord) file: line:61
	M("pystorm::bddriver").def("FieldHCVal", (unsigned int (*)(pystorm::bddriver::ToggleWord)) &pystorm::bddriver::FieldHCVal, "C++: pystorm::bddriver::FieldHCVal(pystorm::bddriver::ToggleWord) --> unsigned int", pybind11::arg("field_name"));

	// pystorm::bddriver::DACWord file: line:73
	pybind11::enum_<pystorm::bddriver::DACWord>(M("pystorm::bddriver"), "DACWord", "")
		.value("DAC_TO_ADC_CONN", pystorm::bddriver::DACWord::DAC_TO_ADC_CONN)
		.value("DAC_VALUE", pystorm::bddriver::DACWord::DAC_VALUE)
		.value("FIELDCOUNT", pystorm::bddriver::DACWord::FIELDCOUNT);

;

	// pystorm::bddriver::FieldWidth(pystorm::bddriver::DACWord) file: line:56
	M("pystorm::bddriver").def("FieldWidth", (unsigned int (*)(pystorm::bddriver::DACWord)) &pystorm::bddriver::FieldWidth, "C++: pystorm::bddriver::FieldWidth(pystorm::bddriver::DACWord) --> unsigned int", pybind11::arg("field_name"));

	// pystorm::bddriver::FieldHCVal(pystorm::bddriver::DACWord) file: line:61
	M("pystorm::bddriver").def("FieldHCVal", (unsigned int (*)(pystorm::bddriver::DACWord)) &pystorm::bddriver::FieldHCVal, "C++: pystorm::bddriver::FieldHCVal(pystorm::bddriver::DACWord) --> unsigned int", pybind11::arg("field_name"));

	// pystorm::bddriver::ADCWord file: line:77
	pybind11::enum_<pystorm::bddriver::ADCWord>(M("pystorm::bddriver"), "ADCWord", "")
		.value("ADC_SMALL_LARGE_CURRENT_0", pystorm::bddriver::ADCWord::ADC_SMALL_LARGE_CURRENT_0)
		.value("ADC_SMALL_LARGE_CURRENT_1", pystorm::bddriver::ADCWord::ADC_SMALL_LARGE_CURRENT_1)
		.value("ADC_OUTPUT_ENABLE", pystorm::bddriver::ADCWord::ADC_OUTPUT_ENABLE)
		.value("FIELDCOUNT", pystorm::bddriver::ADCWord::FIELDCOUNT);

;

	// pystorm::bddriver::FieldWidth(pystorm::bddriver::ADCWord) file: line:56
	M("pystorm::bddriver").def("FieldWidth", (unsigned int (*)(pystorm::bddriver::ADCWord)) &pystorm::bddriver::FieldWidth, "C++: pystorm::bddriver::FieldWidth(pystorm::bddriver::ADCWord) --> unsigned int", pybind11::arg("field_name"));

	// pystorm::bddriver::FieldHCVal(pystorm::bddriver::ADCWord) file: line:61
	M("pystorm::bddriver").def("FieldHCVal", (unsigned int (*)(pystorm::bddriver::ADCWord)) &pystorm::bddriver::FieldHCVal, "C++: pystorm::bddriver::FieldHCVal(pystorm::bddriver::ADCWord) --> unsigned int", pybind11::arg("field_name"));

	// pystorm::bddriver::DelayWord file: line:81
	pybind11::enum_<pystorm::bddriver::DelayWord>(M("pystorm::bddriver"), "DelayWord", "")
		.value("READ_DELAY", pystorm::bddriver::DelayWord::READ_DELAY)
		.value("WRITE_DELAY", pystorm::bddriver::DelayWord::WRITE_DELAY)
		.value("FIELDCOUNT", pystorm::bddriver::DelayWord::FIELDCOUNT);

;

	// pystorm::bddriver::FieldWidth(pystorm::bddriver::DelayWord) file: line:56
	M("pystorm::bddriver").def("FieldWidth", (unsigned int (*)(pystorm::bddriver::DelayWord)) &pystorm::bddriver::FieldWidth, "C++: pystorm::bddriver::FieldWidth(pystorm::bddriver::DelayWord) --> unsigned int", pybind11::arg("field_name"));

	// pystorm::bddriver::FieldHCVal(pystorm::bddriver::DelayWord) file: line:61
	M("pystorm::bddriver").def("FieldHCVal", (unsigned int (*)(pystorm::bddriver::DelayWord)) &pystorm::bddriver::FieldHCVal, "C++: pystorm::bddriver::FieldHCVal(pystorm::bddriver::DelayWord) --> unsigned int", pybind11::arg("field_name"));

	// pystorm::bddriver::AMWord file: line:86
	pybind11::enum_<pystorm::bddriver::AMWord>(M("pystorm::bddriver"), "AMWord", "")
		.value("ACCUMULATOR_VALUE", pystorm::bddriver::AMWord::ACCUMULATOR_VALUE)
		.value("THRESHOLD", pystorm::bddriver::AMWord::THRESHOLD)
		.value("STOP", pystorm::bddriver::AMWord::STOP)
		.value("NEXT_ADDRESS", pystorm::bddriver::AMWord::NEXT_ADDRESS)
		.value("FIELDCOUNT", pystorm::bddriver::AMWord::FIELDCOUNT);

;

	// pystorm::bddriver::FieldWidth(pystorm::bddriver::AMWord) file: line:56
	M("pystorm::bddriver").def("FieldWidth", (unsigned int (*)(pystorm::bddriver::AMWord)) &pystorm::bddriver::FieldWidth, "C++: pystorm::bddriver::FieldWidth(pystorm::bddriver::AMWord) --> unsigned int", pybind11::arg("field_name"));

	// pystorm::bddriver::FieldHCVal(pystorm::bddriver::AMWord) file: line:61
	M("pystorm::bddriver").def("FieldHCVal", (unsigned int (*)(pystorm::bddriver::AMWord)) &pystorm::bddriver::FieldHCVal, "C++: pystorm::bddriver::FieldHCVal(pystorm::bddriver::AMWord) --> unsigned int", pybind11::arg("field_name"));

	// pystorm::bddriver::MMWord file: line:90
	pybind11::enum_<pystorm::bddriver::MMWord>(M("pystorm::bddriver"), "MMWord", "")
		.value("WEIGHT", pystorm::bddriver::MMWord::WEIGHT)
		.value("FIELDCOUNT", pystorm::bddriver::MMWord::FIELDCOUNT);

;

	// pystorm::bddriver::FieldWidth(pystorm::bddriver::MMWord) file: line:56
	M("pystorm::bddriver").def("FieldWidth", (unsigned int (*)(pystorm::bddriver::MMWord)) &pystorm::bddriver::FieldWidth, "C++: pystorm::bddriver::FieldWidth(pystorm::bddriver::MMWord) --> unsigned int", pybind11::arg("field_name"));

	// pystorm::bddriver::FieldHCVal(pystorm::bddriver::MMWord) file: line:61
	M("pystorm::bddriver").def("FieldHCVal", (unsigned int (*)(pystorm::bddriver::MMWord)) &pystorm::bddriver::FieldHCVal, "C++: pystorm::bddriver::FieldHCVal(pystorm::bddriver::MMWord) --> unsigned int", pybind11::arg("field_name"));

	// pystorm::bddriver::PATWord file: line:94
	pybind11::enum_<pystorm::bddriver::PATWord>(M("pystorm::bddriver"), "PATWord", "")
		.value("AM_ADDRESS", pystorm::bddriver::PATWord::AM_ADDRESS)
		.value("MM_ADDRESS_LO", pystorm::bddriver::PATWord::MM_ADDRESS_LO)
		.value("MM_ADDRESS_HI", pystorm::bddriver::PATWord::MM_ADDRESS_HI)
		.value("FIELDCOUNT", pystorm::bddriver::PATWord::FIELDCOUNT);

;

	// pystorm::bddriver::FieldWidth(pystorm::bddriver::PATWord) file: line:56
	M("pystorm::bddriver").def("FieldWidth", (unsigned int (*)(pystorm::bddriver::PATWord)) &pystorm::bddriver::FieldWidth, "C++: pystorm::bddriver::FieldWidth(pystorm::bddriver::PATWord) --> unsigned int", pybind11::arg("field_name"));

	// pystorm::bddriver::FieldHCVal(pystorm::bddriver::PATWord) file: line:61
	M("pystorm::bddriver").def("FieldHCVal", (unsigned int (*)(pystorm::bddriver::PATWord)) &pystorm::bddriver::FieldHCVal, "C++: pystorm::bddriver::FieldHCVal(pystorm::bddriver::PATWord) --> unsigned int", pybind11::arg("field_name"));

	// pystorm::bddriver::TATAccWord file: line:98
	pybind11::enum_<pystorm::bddriver::TATAccWord>(M("pystorm::bddriver"), "TATAccWord", "")
		.value("STOP", pystorm::bddriver::TATAccWord::STOP)
		.value("FIXED_0", pystorm::bddriver::TATAccWord::FIXED_0)
		.value("AM_ADDRESS", pystorm::bddriver::TATAccWord::AM_ADDRESS)
		.value("MM_ADDRESS", pystorm::bddriver::TATAccWord::MM_ADDRESS)
		.value("FIELDCOUNT", pystorm::bddriver::TATAccWord::FIELDCOUNT);

;

	// pystorm::bddriver::FieldWidth(pystorm::bddriver::TATAccWord) file: line:39
	M("pystorm::bddriver").def("FieldWidth", (unsigned int (*)(pystorm::bddriver::TATAccWord)) &pystorm::bddriver::FieldWidth, "C++: pystorm::bddriver::FieldWidth(pystorm::bddriver::TATAccWord) --> unsigned int", pybind11::arg("field_name"));

	// pystorm::bddriver::FieldHCVal(pystorm::bddriver::TATAccWord) file: line:44
	M("pystorm::bddriver").def("FieldHCVal", (unsigned int (*)(pystorm::bddriver::TATAccWord)) &pystorm::bddriver::FieldHCVal, "C++: pystorm::bddriver::FieldHCVal(pystorm::bddriver::TATAccWord) --> unsigned int", pybind11::arg("field_name"));

	// pystorm::bddriver::TATSpikeWord file: line:103
	pybind11::enum_<pystorm::bddriver::TATSpikeWord>(M("pystorm::bddriver"), "TATSpikeWord", "")
		.value("STOP", pystorm::bddriver::TATSpikeWord::STOP)
		.value("FIXED_1", pystorm::bddriver::TATSpikeWord::FIXED_1)
		.value("SYNAPSE_ADDRESS_0", pystorm::bddriver::TATSpikeWord::SYNAPSE_ADDRESS_0)
		.value("SYNAPSE_SIGN_0", pystorm::bddriver::TATSpikeWord::SYNAPSE_SIGN_0)
		.value("SYNAPSE_ADDRESS_1", pystorm::bddriver::TATSpikeWord::SYNAPSE_ADDRESS_1)
		.value("SYNAPSE_SIGN_1", pystorm::bddriver::TATSpikeWord::SYNAPSE_SIGN_1)
		.value("UNUSED", pystorm::bddriver::TATSpikeWord::UNUSED)
		.value("FIELDCOUNT", pystorm::bddriver::TATSpikeWord::FIELDCOUNT);

;

	// pystorm::bddriver::FieldWidth(pystorm::bddriver::TATSpikeWord) file: line:39
	M("pystorm::bddriver").def("FieldWidth", (unsigned int (*)(pystorm::bddriver::TATSpikeWord)) &pystorm::bddriver::FieldWidth, "C++: pystorm::bddriver::FieldWidth(pystorm::bddriver::TATSpikeWord) --> unsigned int", pybind11::arg("field_name"));

	// pystorm::bddriver::FieldHCVal(pystorm::bddriver::TATSpikeWord) file: line:44
	M("pystorm::bddriver").def("FieldHCVal", (unsigned int (*)(pystorm::bddriver::TATSpikeWord)) &pystorm::bddriver::FieldHCVal, "C++: pystorm::bddriver::FieldHCVal(pystorm::bddriver::TATSpikeWord) --> unsigned int", pybind11::arg("field_name"));

	// pystorm::bddriver::TATTagWord file: line:108
	pybind11::enum_<pystorm::bddriver::TATTagWord>(M("pystorm::bddriver"), "TATTagWord", "")
		.value("STOP", pystorm::bddriver::TATTagWord::STOP)
		.value("FIXED_2", pystorm::bddriver::TATTagWord::FIXED_2)
		.value("TAG", pystorm::bddriver::TATTagWord::TAG)
		.value("GLOBAL_ROUTE", pystorm::bddriver::TATTagWord::GLOBAL_ROUTE)
		.value("UNUSED", pystorm::bddriver::TATTagWord::UNUSED)
		.value("FIELDCOUNT", pystorm::bddriver::TATTagWord::FIELDCOUNT);

;

	// pystorm::bddriver::FieldWidth(pystorm::bddriver::TATTagWord) file: line:39
	M("pystorm::bddriver").def("FieldWidth", (unsigned int (*)(pystorm::bddriver::TATTagWord)) &pystorm::bddriver::FieldWidth, "C++: pystorm::bddriver::FieldWidth(pystorm::bddriver::TATTagWord) --> unsigned int", pybind11::arg("field_name"));

	// pystorm::bddriver::FieldHCVal(pystorm::bddriver::TATTagWord) file: line:44
	M("pystorm::bddriver").def("FieldHCVal", (unsigned int (*)(pystorm::bddriver::TATTagWord)) &pystorm::bddriver::FieldHCVal, "C++: pystorm::bddriver::FieldHCVal(pystorm::bddriver::TATTagWord) --> unsigned int", pybind11::arg("field_name"));

	// pystorm::bddriver::PATWrite file: line:114
	pybind11::enum_<pystorm::bddriver::PATWrite>(M("pystorm::bddriver"), "PATWrite", "")
		.value("ADDRESS", pystorm::bddriver::PATWrite::ADDRESS)
		.value("FIXED_0", pystorm::bddriver::PATWrite::FIXED_0)
		.value("DATA", pystorm::bddriver::PATWrite::DATA)
		.value("FIELDCOUNT", pystorm::bddriver::PATWrite::FIELDCOUNT);

;

	// pystorm::bddriver::FieldWidth(pystorm::bddriver::PATWrite) file: line:39
	M("pystorm::bddriver").def("FieldWidth", (unsigned int (*)(pystorm::bddriver::PATWrite)) &pystorm::bddriver::FieldWidth, "C++: pystorm::bddriver::FieldWidth(pystorm::bddriver::PATWrite) --> unsigned int", pybind11::arg("field_name"));

	// pystorm::bddriver::FieldHCVal(pystorm::bddriver::PATWrite) file: line:44
	M("pystorm::bddriver").def("FieldHCVal", (unsigned int (*)(pystorm::bddriver::PATWrite)) &pystorm::bddriver::FieldHCVal, "C++: pystorm::bddriver::FieldHCVal(pystorm::bddriver::PATWrite) --> unsigned int", pybind11::arg("field_name"));

	// pystorm::bddriver::PATRead file: line:119
	pybind11::enum_<pystorm::bddriver::PATRead>(M("pystorm::bddriver"), "PATRead", "")
		.value("ADDRESS", pystorm::bddriver::PATRead::ADDRESS)
		.value("FIXED_1", pystorm::bddriver::PATRead::FIXED_1)
		.value("UNUSED", pystorm::bddriver::PATRead::UNUSED)
		.value("FIELDCOUNT", pystorm::bddriver::PATRead::FIELDCOUNT);

;

	// pystorm::bddriver::FieldWidth(pystorm::bddriver::PATRead) file: line:39
	M("pystorm::bddriver").def("FieldWidth", (unsigned int (*)(pystorm::bddriver::PATRead)) &pystorm::bddriver::FieldWidth, "C++: pystorm::bddriver::FieldWidth(pystorm::bddriver::PATRead) --> unsigned int", pybind11::arg("field_name"));

	// pystorm::bddriver::FieldHCVal(pystorm::bddriver::PATRead) file: line:44
	M("pystorm::bddriver").def("FieldHCVal", (unsigned int (*)(pystorm::bddriver::PATRead)) &pystorm::bddriver::FieldHCVal, "C++: pystorm::bddriver::FieldHCVal(pystorm::bddriver::PATRead) --> unsigned int", pybind11::arg("field_name"));

	// pystorm::bddriver::TATSetAddress file: line:124
	pybind11::enum_<pystorm::bddriver::TATSetAddress>(M("pystorm::bddriver"), "TATSetAddress", "")
		.value("FIXED_0", pystorm::bddriver::TATSetAddress::FIXED_0)
		.value("ADDRESS", pystorm::bddriver::TATSetAddress::ADDRESS)
		.value("UNUSED", pystorm::bddriver::TATSetAddress::UNUSED)
		.value("FIELDCOUNT", pystorm::bddriver::TATSetAddress::FIELDCOUNT);

;

	// pystorm::bddriver::FieldWidth(pystorm::bddriver::TATSetAddress) file: line:39
	M("pystorm::bddriver").def("FieldWidth", (unsigned int (*)(pystorm::bddriver::TATSetAddress)) &pystorm::bddriver::FieldWidth, "C++: pystorm::bddriver::FieldWidth(pystorm::bddriver::TATSetAddress) --> unsigned int", pybind11::arg("field_name"));

	// pystorm::bddriver::FieldHCVal(pystorm::bddriver::TATSetAddress) file: line:44
	M("pystorm::bddriver").def("FieldHCVal", (unsigned int (*)(pystorm::bddriver::TATSetAddress)) &pystorm::bddriver::FieldHCVal, "C++: pystorm::bddriver::FieldHCVal(pystorm::bddriver::TATSetAddress) --> unsigned int", pybind11::arg("field_name"));

	// pystorm::bddriver::TATWriteIncrement file: line:129
	pybind11::enum_<pystorm::bddriver::TATWriteIncrement>(M("pystorm::bddriver"), "TATWriteIncrement", "")
		.value("FIXED_1", pystorm::bddriver::TATWriteIncrement::FIXED_1)
		.value("DATA", pystorm::bddriver::TATWriteIncrement::DATA)
		.value("FIELDCOUNT", pystorm::bddriver::TATWriteIncrement::FIELDCOUNT);

;

	// pystorm::bddriver::FieldWidth(pystorm::bddriver::TATWriteIncrement) file: line:39
	M("pystorm::bddriver").def("FieldWidth", (unsigned int (*)(pystorm::bddriver::TATWriteIncrement)) &pystorm::bddriver::FieldWidth, "C++: pystorm::bddriver::FieldWidth(pystorm::bddriver::TATWriteIncrement) --> unsigned int", pybind11::arg("field_name"));

	// pystorm::bddriver::FieldHCVal(pystorm::bddriver::TATWriteIncrement) file: line:44
	M("pystorm::bddriver").def("FieldHCVal", (unsigned int (*)(pystorm::bddriver::TATWriteIncrement)) &pystorm::bddriver::FieldHCVal, "C++: pystorm::bddriver::FieldHCVal(pystorm::bddriver::TATWriteIncrement) --> unsigned int", pybind11::arg("field_name"));

	// pystorm::bddriver::TATReadIncrement file: line:134
	pybind11::enum_<pystorm::bddriver::TATReadIncrement>(M("pystorm::bddriver"), "TATReadIncrement", "")
		.value("FIXED_2", pystorm::bddriver::TATReadIncrement::FIXED_2)
		.value("UNUSED", pystorm::bddriver::TATReadIncrement::UNUSED)
		.value("FIELDCOUNT", pystorm::bddriver::TATReadIncrement::FIELDCOUNT);

;

	// pystorm::bddriver::FieldWidth(pystorm::bddriver::TATReadIncrement) file: line:39
	M("pystorm::bddriver").def("FieldWidth", (unsigned int (*)(pystorm::bddriver::TATReadIncrement)) &pystorm::bddriver::FieldWidth, "C++: pystorm::bddriver::FieldWidth(pystorm::bddriver::TATReadIncrement) --> unsigned int", pybind11::arg("field_name"));

	// pystorm::bddriver::FieldHCVal(pystorm::bddriver::TATReadIncrement) file: line:44
	M("pystorm::bddriver").def("FieldHCVal", (unsigned int (*)(pystorm::bddriver::TATReadIncrement)) &pystorm::bddriver::FieldHCVal, "C++: pystorm::bddriver::FieldHCVal(pystorm::bddriver::TATReadIncrement) --> unsigned int", pybind11::arg("field_name"));

	// pystorm::bddriver::MMSetAddress file: line:139
	pybind11::enum_<pystorm::bddriver::MMSetAddress>(M("pystorm::bddriver"), "MMSetAddress", "")
		.value("FIXED_0", pystorm::bddriver::MMSetAddress::FIXED_0)
		.value("ADDRESS", pystorm::bddriver::MMSetAddress::ADDRESS)
		.value("UNUSED", pystorm::bddriver::MMSetAddress::UNUSED)
		.value("FIELDCOUNT", pystorm::bddriver::MMSetAddress::FIELDCOUNT);

;

	// pystorm::bddriver::FieldWidth(pystorm::bddriver::MMSetAddress) file: line:39
	M("pystorm::bddriver").def("FieldWidth", (unsigned int (*)(pystorm::bddriver::MMSetAddress)) &pystorm::bddriver::FieldWidth, "C++: pystorm::bddriver::FieldWidth(pystorm::bddriver::MMSetAddress) --> unsigned int", pybind11::arg("field_name"));

	// pystorm::bddriver::FieldHCVal(pystorm::bddriver::MMSetAddress) file: line:44
	M("pystorm::bddriver").def("FieldHCVal", (unsigned int (*)(pystorm::bddriver::MMSetAddress)) &pystorm::bddriver::FieldHCVal, "C++: pystorm::bddriver::FieldHCVal(pystorm::bddriver::MMSetAddress) --> unsigned int", pybind11::arg("field_name"));

	// pystorm::bddriver::MMWriteIncrement file: line:144
	pybind11::enum_<pystorm::bddriver::MMWriteIncrement>(M("pystorm::bddriver"), "MMWriteIncrement", "")
		.value("FIXED_1", pystorm::bddriver::MMWriteIncrement::FIXED_1)
		.value("DATA", pystorm::bddriver::MMWriteIncrement::DATA)
		.value("UNUSED", pystorm::bddriver::MMWriteIncrement::UNUSED)
		.value("FIELDCOUNT", pystorm::bddriver::MMWriteIncrement::FIELDCOUNT);

;

	// pystorm::bddriver::FieldWidth(pystorm::bddriver::MMWriteIncrement) file: line:39
	M("pystorm::bddriver").def("FieldWidth", (unsigned int (*)(pystorm::bddriver::MMWriteIncrement)) &pystorm::bddriver::FieldWidth, "C++: pystorm::bddriver::FieldWidth(pystorm::bddriver::MMWriteIncrement) --> unsigned int", pybind11::arg("field_name"));

	// pystorm::bddriver::FieldHCVal(pystorm::bddriver::MMWriteIncrement) file: line:44
	M("pystorm::bddriver").def("FieldHCVal", (unsigned int (*)(pystorm::bddriver::MMWriteIncrement)) &pystorm::bddriver::FieldHCVal, "C++: pystorm::bddriver::FieldHCVal(pystorm::bddriver::MMWriteIncrement) --> unsigned int", pybind11::arg("field_name"));

	// pystorm::bddriver::MMReadIncrement file: line:149
	pybind11::enum_<pystorm::bddriver::MMReadIncrement>(M("pystorm::bddriver"), "MMReadIncrement", "")
		.value("FIXED_2", pystorm::bddriver::MMReadIncrement::FIXED_2)
		.value("UNUSED", pystorm::bddriver::MMReadIncrement::UNUSED)
		.value("FIELDCOUNT", pystorm::bddriver::MMReadIncrement::FIELDCOUNT);

;

	// pystorm::bddriver::FieldWidth(pystorm::bddriver::MMReadIncrement) file: line:39
	M("pystorm::bddriver").def("FieldWidth", (unsigned int (*)(pystorm::bddriver::MMReadIncrement)) &pystorm::bddriver::FieldWidth, "C++: pystorm::bddriver::FieldWidth(pystorm::bddriver::MMReadIncrement) --> unsigned int", pybind11::arg("field_name"));

	// pystorm::bddriver::FieldHCVal(pystorm::bddriver::MMReadIncrement) file: line:44
	M("pystorm::bddriver").def("FieldHCVal", (unsigned int (*)(pystorm::bddriver::MMReadIncrement)) &pystorm::bddriver::FieldHCVal, "C++: pystorm::bddriver::FieldHCVal(pystorm::bddriver::MMReadIncrement) --> unsigned int", pybind11::arg("field_name"));

	// pystorm::bddriver::AMSetAddress file: line:154
	pybind11::enum_<pystorm::bddriver::AMSetAddress>(M("pystorm::bddriver"), "AMSetAddress", "")
		.value("FIXED_0", pystorm::bddriver::AMSetAddress::FIXED_0)
		.value("ADDRESS", pystorm::bddriver::AMSetAddress::ADDRESS)
		.value("UNUSED", pystorm::bddriver::AMSetAddress::UNUSED)
		.value("FIELDCOUNT", pystorm::bddriver::AMSetAddress::FIELDCOUNT);

;

	// pystorm::bddriver::FieldWidth(pystorm::bddriver::AMSetAddress) file: line:39
	M("pystorm::bddriver").def("FieldWidth", (unsigned int (*)(pystorm::bddriver::AMSetAddress)) &pystorm::bddriver::FieldWidth, "C++: pystorm::bddriver::FieldWidth(pystorm::bddriver::AMSetAddress) --> unsigned int", pybind11::arg("field_name"));

	// pystorm::bddriver::FieldHCVal(pystorm::bddriver::AMSetAddress) file: line:44
	M("pystorm::bddriver").def("FieldHCVal", (unsigned int (*)(pystorm::bddriver::AMSetAddress)) &pystorm::bddriver::FieldHCVal, "C++: pystorm::bddriver::FieldHCVal(pystorm::bddriver::AMSetAddress) --> unsigned int", pybind11::arg("field_name"));

	// pystorm::bddriver::AMReadWrite file: line:159
	pybind11::enum_<pystorm::bddriver::AMReadWrite>(M("pystorm::bddriver"), "AMReadWrite", "")
		.value("FIXED_1", pystorm::bddriver::AMReadWrite::FIXED_1)
		.value("DATA", pystorm::bddriver::AMReadWrite::DATA)
		.value("FIELDCOUNT", pystorm::bddriver::AMReadWrite::FIELDCOUNT);

;

	// pystorm::bddriver::FieldWidth(pystorm::bddriver::AMReadWrite) file: line:39
	M("pystorm::bddriver").def("FieldWidth", (unsigned int (*)(pystorm::bddriver::AMReadWrite)) &pystorm::bddriver::FieldWidth, "C++: pystorm::bddriver::FieldWidth(pystorm::bddriver::AMReadWrite) --> unsigned int", pybind11::arg("field_name"));

	// pystorm::bddriver::FieldHCVal(pystorm::bddriver::AMReadWrite) file: line:44
	M("pystorm::bddriver").def("FieldHCVal", (unsigned int (*)(pystorm::bddriver::AMReadWrite)) &pystorm::bddriver::FieldHCVal, "C++: pystorm::bddriver::FieldHCVal(pystorm::bddriver::AMReadWrite) --> unsigned int", pybind11::arg("field_name"));

	// pystorm::bddriver::AMIncrement file: line:164
	pybind11::enum_<pystorm::bddriver::AMIncrement>(M("pystorm::bddriver"), "AMIncrement", "")
		.value("FIXED_2", pystorm::bddriver::AMIncrement::FIXED_2)
		.value("UNUSED", pystorm::bddriver::AMIncrement::UNUSED)
		.value("FIELDCOUNT", pystorm::bddriver::AMIncrement::FIELDCOUNT);

;

	// pystorm::bddriver::FieldWidth(pystorm::bddriver::AMIncrement) file: line:39
	M("pystorm::bddriver").def("FieldWidth", (unsigned int (*)(pystorm::bddriver::AMIncrement)) &pystorm::bddriver::FieldWidth, "C++: pystorm::bddriver::FieldWidth(pystorm::bddriver::AMIncrement) --> unsigned int", pybind11::arg("field_name"));

	// pystorm::bddriver::FieldHCVal(pystorm::bddriver::AMIncrement) file: line:44
	M("pystorm::bddriver").def("FieldHCVal", (unsigned int (*)(pystorm::bddriver::AMIncrement)) &pystorm::bddriver::FieldHCVal, "C++: pystorm::bddriver::FieldHCVal(pystorm::bddriver::AMIncrement) --> unsigned int", pybind11::arg("field_name"));

	// pystorm::bddriver::AMEncapsulation file: line:169
	pybind11::enum_<pystorm::bddriver::AMEncapsulation>(M("pystorm::bddriver"), "AMEncapsulation", "")
		.value("FIXED_0", pystorm::bddriver::AMEncapsulation::FIXED_0)
		.value("PAYLOAD", pystorm::bddriver::AMEncapsulation::PAYLOAD)
		.value("AMMM_STOP", pystorm::bddriver::AMEncapsulation::AMMM_STOP)
		.value("FIELDCOUNT", pystorm::bddriver::AMEncapsulation::FIELDCOUNT);

;

	// pystorm::bddriver::FieldWidth(pystorm::bddriver::AMEncapsulation) file: line:39
	M("pystorm::bddriver").def("FieldWidth", (unsigned int (*)(pystorm::bddriver::AMEncapsulation)) &pystorm::bddriver::FieldWidth, "C++: pystorm::bddriver::FieldWidth(pystorm::bddriver::AMEncapsulation) --> unsigned int", pybind11::arg("field_name"));

	// pystorm::bddriver::FieldHCVal(pystorm::bddriver::AMEncapsulation) file: line:44
	M("pystorm::bddriver").def("FieldHCVal", (unsigned int (*)(pystorm::bddriver::AMEncapsulation)) &pystorm::bddriver::FieldHCVal, "C++: pystorm::bddriver::FieldHCVal(pystorm::bddriver::AMEncapsulation) --> unsigned int", pybind11::arg("field_name"));

	// pystorm::bddriver::MMEncapsulation file: line:174
	pybind11::enum_<pystorm::bddriver::MMEncapsulation>(M("pystorm::bddriver"), "MMEncapsulation", "")
		.value("FIXED_1", pystorm::bddriver::MMEncapsulation::FIXED_1)
		.value("PAYLOAD", pystorm::bddriver::MMEncapsulation::PAYLOAD)
		.value("AMMM_STOP", pystorm::bddriver::MMEncapsulation::AMMM_STOP)
		.value("FIELDCOUNT", pystorm::bddriver::MMEncapsulation::FIELDCOUNT);

;

	// pystorm::bddriver::FieldWidth(pystorm::bddriver::MMEncapsulation) file: line:39
	M("pystorm::bddriver").def("FieldWidth", (unsigned int (*)(pystorm::bddriver::MMEncapsulation)) &pystorm::bddriver::FieldWidth, "C++: pystorm::bddriver::FieldWidth(pystorm::bddriver::MMEncapsulation) --> unsigned int", pybind11::arg("field_name"));

	// pystorm::bddriver::FieldHCVal(pystorm::bddriver::MMEncapsulation) file: line:44
	M("pystorm::bddriver").def("FieldHCVal", (unsigned int (*)(pystorm::bddriver::MMEncapsulation)) &pystorm::bddriver::FieldHCVal, "C++: pystorm::bddriver::FieldHCVal(pystorm::bddriver::MMEncapsulation) --> unsigned int", pybind11::arg("field_name"));

	// pystorm::bddriver::InputTag file: line:180
	pybind11::enum_<pystorm::bddriver::InputTag>(M("pystorm::bddriver"), "InputTag", "")
		.value("COUNT", pystorm::bddriver::InputTag::COUNT)
		.value("TAG", pystorm::bddriver::InputTag::TAG)
		.value("FIELDCOUNT", pystorm::bddriver::InputTag::FIELDCOUNT);

;

	// pystorm::bddriver::FieldWidth(pystorm::bddriver::InputTag) file: line:56
	M("pystorm::bddriver").def("FieldWidth", (unsigned int (*)(pystorm::bddriver::InputTag)) &pystorm::bddriver::FieldWidth, "C++: pystorm::bddriver::FieldWidth(pystorm::bddriver::InputTag) --> unsigned int", pybind11::arg("field_name"));

	// pystorm::bddriver::FieldHCVal(pystorm::bddriver::InputTag) file: line:61
	M("pystorm::bddriver").def("FieldHCVal", (unsigned int (*)(pystorm::bddriver::InputTag)) &pystorm::bddriver::FieldHCVal, "C++: pystorm::bddriver::FieldHCVal(pystorm::bddriver::InputTag) --> unsigned int", pybind11::arg("field_name"));

	// pystorm::bddriver::FIFOInputTag file: line:184
	pybind11::enum_<pystorm::bddriver::FIFOInputTag>(M("pystorm::bddriver"), "FIFOInputTag", "")
		.value("TAG", pystorm::bddriver::FIFOInputTag::TAG)
		.value("FIELDCOUNT", pystorm::bddriver::FIFOInputTag::FIELDCOUNT);

;

	// pystorm::bddriver::FieldWidth(pystorm::bddriver::FIFOInputTag) file: line:56
	M("pystorm::bddriver").def("FieldWidth", (unsigned int (*)(pystorm::bddriver::FIFOInputTag)) &pystorm::bddriver::FieldWidth, "C++: pystorm::bddriver::FieldWidth(pystorm::bddriver::FIFOInputTag) --> unsigned int", pybind11::arg("field_name"));

	// pystorm::bddriver::FieldHCVal(pystorm::bddriver::FIFOInputTag) file: line:61
	M("pystorm::bddriver").def("FieldHCVal", (unsigned int (*)(pystorm::bddriver::FIFOInputTag)) &pystorm::bddriver::FieldHCVal, "C++: pystorm::bddriver::FieldHCVal(pystorm::bddriver::FIFOInputTag) --> unsigned int", pybind11::arg("field_name"));

	// pystorm::bddriver::InputSpike file: line:188
	pybind11::enum_<pystorm::bddriver::InputSpike>(M("pystorm::bddriver"), "InputSpike", "")
		.value("SYNAPSE_SIGN", pystorm::bddriver::InputSpike::SYNAPSE_SIGN)
		.value("SYNAPSE_ADDRESS", pystorm::bddriver::InputSpike::SYNAPSE_ADDRESS)
		.value("FIELDCOUNT", pystorm::bddriver::InputSpike::FIELDCOUNT);

;

	// pystorm::bddriver::FieldWidth(pystorm::bddriver::InputSpike) file: line:56
	M("pystorm::bddriver").def("FieldWidth", (unsigned int (*)(pystorm::bddriver::InputSpike)) &pystorm::bddriver::FieldWidth, "C++: pystorm::bddriver::FieldWidth(pystorm::bddriver::InputSpike) --> unsigned int", pybind11::arg("field_name"));

	// pystorm::bddriver::FieldHCVal(pystorm::bddriver::InputSpike) file: line:61
	M("pystorm::bddriver").def("FieldHCVal", (unsigned int (*)(pystorm::bddriver::InputSpike)) &pystorm::bddriver::FieldHCVal, "C++: pystorm::bddriver::FieldHCVal(pystorm::bddriver::InputSpike) --> unsigned int", pybind11::arg("field_name"));

	// pystorm::bddriver::PreFIFOTag file: line:193
	pybind11::enum_<pystorm::bddriver::PreFIFOTag>(M("pystorm::bddriver"), "PreFIFOTag", "")
		.value("COUNT", pystorm::bddriver::PreFIFOTag::COUNT)
		.value("TAG", pystorm::bddriver::PreFIFOTag::TAG)
		.value("FIELDCOUNT", pystorm::bddriver::PreFIFOTag::FIELDCOUNT);

;

	// pystorm::bddriver::FieldWidth(pystorm::bddriver::PreFIFOTag) file: line:56
	M("pystorm::bddriver").def("FieldWidth", (unsigned int (*)(pystorm::bddriver::PreFIFOTag)) &pystorm::bddriver::FieldWidth, "C++: pystorm::bddriver::FieldWidth(pystorm::bddriver::PreFIFOTag) --> unsigned int", pybind11::arg("field_name"));

	// pystorm::bddriver::FieldHCVal(pystorm::bddriver::PreFIFOTag) file: line:61
	M("pystorm::bddriver").def("FieldHCVal", (unsigned int (*)(pystorm::bddriver::PreFIFOTag)) &pystorm::bddriver::FieldHCVal, "C++: pystorm::bddriver::FieldHCVal(pystorm::bddriver::PreFIFOTag) --> unsigned int", pybind11::arg("field_name"));

	// pystorm::bddriver::PostFIFOTag file: line:197
	pybind11::enum_<pystorm::bddriver::PostFIFOTag>(M("pystorm::bddriver"), "PostFIFOTag", "")
		.value("COUNT", pystorm::bddriver::PostFIFOTag::COUNT)
		.value("TAG", pystorm::bddriver::PostFIFOTag::TAG)
		.value("FIELDCOUNT", pystorm::bddriver::PostFIFOTag::FIELDCOUNT);

;

	// pystorm::bddriver::FieldWidth(pystorm::bddriver::PostFIFOTag) file: line:56
	M("pystorm::bddriver").def("FieldWidth", (unsigned int (*)(pystorm::bddriver::PostFIFOTag)) &pystorm::bddriver::FieldWidth, "C++: pystorm::bddriver::FieldWidth(pystorm::bddriver::PostFIFOTag) --> unsigned int", pybind11::arg("field_name"));

	// pystorm::bddriver::FieldHCVal(pystorm::bddriver::PostFIFOTag) file: line:61
	M("pystorm::bddriver").def("FieldHCVal", (unsigned int (*)(pystorm::bddriver::PostFIFOTag)) &pystorm::bddriver::FieldHCVal, "C++: pystorm::bddriver::FieldHCVal(pystorm::bddriver::PostFIFOTag) --> unsigned int", pybind11::arg("field_name"));

	// pystorm::bddriver::OutputSpike file: line:201
	pybind11::enum_<pystorm::bddriver::OutputSpike>(M("pystorm::bddriver"), "OutputSpike", "")
		.value("NEURON_ADDRESS", pystorm::bddriver::OutputSpike::NEURON_ADDRESS)
		.value("FIELDCOUNT", pystorm::bddriver::OutputSpike::FIELDCOUNT);

;

	// pystorm::bddriver::FieldWidth(pystorm::bddriver::OutputSpike) file: line:56
	M("pystorm::bddriver").def("FieldWidth", (unsigned int (*)(pystorm::bddriver::OutputSpike)) &pystorm::bddriver::FieldWidth, "C++: pystorm::bddriver::FieldWidth(pystorm::bddriver::OutputSpike) --> unsigned int", pybind11::arg("field_name"));

	// pystorm::bddriver::FieldHCVal(pystorm::bddriver::OutputSpike) file: line:61
	M("pystorm::bddriver").def("FieldHCVal", (unsigned int (*)(pystorm::bddriver::OutputSpike)) &pystorm::bddriver::FieldHCVal, "C++: pystorm::bddriver::FieldHCVal(pystorm::bddriver::OutputSpike) --> unsigned int", pybind11::arg("field_name"));

	// pystorm::bddriver::OverflowTag file: line:205
	pybind11::enum_<pystorm::bddriver::OverflowTag>(M("pystorm::bddriver"), "OverflowTag", "")
		.value("FIXED_1", pystorm::bddriver::OverflowTag::FIXED_1)
		.value("FIELDCOUNT", pystorm::bddriver::OverflowTag::FIELDCOUNT);

;

	// pystorm::bddriver::FieldWidth(pystorm::bddriver::OverflowTag) file: line:39
	M("pystorm::bddriver").def("FieldWidth", (unsigned int (*)(pystorm::bddriver::OverflowTag)) &pystorm::bddriver::FieldWidth, "C++: pystorm::bddriver::FieldWidth(pystorm::bddriver::OverflowTag) --> unsigned int", pybind11::arg("field_name"));

	// pystorm::bddriver::FieldHCVal(pystorm::bddriver::OverflowTag) file: line:44
	M("pystorm::bddriver").def("FieldHCVal", (unsigned int (*)(pystorm::bddriver::OverflowTag)) &pystorm::bddriver::FieldHCVal, "C++: pystorm::bddriver::FieldHCVal(pystorm::bddriver::OverflowTag) --> unsigned int", pybind11::arg("field_name"));

	// pystorm::bddriver::AccOutputTag file: line:210
	pybind11::enum_<pystorm::bddriver::AccOutputTag>(M("pystorm::bddriver"), "AccOutputTag", "")
		.value("COUNT", pystorm::bddriver::AccOutputTag::COUNT)
		.value("TAG", pystorm::bddriver::AccOutputTag::TAG)
		.value("GLOBAL_ROUTE", pystorm::bddriver::AccOutputTag::GLOBAL_ROUTE)
		.value("FIELDCOUNT", pystorm::bddriver::AccOutputTag::FIELDCOUNT);

;

	// pystorm::bddriver::FieldWidth(pystorm::bddriver::AccOutputTag) file: line:56
	M("pystorm::bddriver").def("FieldWidth", (unsigned int (*)(pystorm::bddriver::AccOutputTag)) &pystorm::bddriver::FieldWidth, "C++: pystorm::bddriver::FieldWidth(pystorm::bddriver::AccOutputTag) --> unsigned int", pybind11::arg("field_name"));

	// pystorm::bddriver::FieldHCVal(pystorm::bddriver::AccOutputTag) file: line:61
	M("pystorm::bddriver").def("FieldHCVal", (unsigned int (*)(pystorm::bddriver::AccOutputTag)) &pystorm::bddriver::FieldHCVal, "C++: pystorm::bddriver::FieldHCVal(pystorm::bddriver::AccOutputTag) --> unsigned int", pybind11::arg("field_name"));

	// pystorm::bddriver::TATOutputTag file: line:214
	pybind11::enum_<pystorm::bddriver::TATOutputTag>(M("pystorm::bddriver"), "TATOutputTag", "")
		.value("COUNT", pystorm::bddriver::TATOutputTag::COUNT)
		.value("TAG", pystorm::bddriver::TATOutputTag::TAG)
		.value("GLOBAL_ROUTE", pystorm::bddriver::TATOutputTag::GLOBAL_ROUTE)
		.value("FIELDCOUNT", pystorm::bddriver::TATOutputTag::FIELDCOUNT);

;

	// pystorm::bddriver::FieldWidth(pystorm::bddriver::TATOutputTag) file: line:56
	M("pystorm::bddriver").def("FieldWidth", (unsigned int (*)(pystorm::bddriver::TATOutputTag)) &pystorm::bddriver::FieldWidth, "C++: pystorm::bddriver::FieldWidth(pystorm::bddriver::TATOutputTag) --> unsigned int", pybind11::arg("field_name"));

	// pystorm::bddriver::FieldHCVal(pystorm::bddriver::TATOutputTag) file: line:61
	M("pystorm::bddriver").def("FieldHCVal", (unsigned int (*)(pystorm::bddriver::TATOutputTag)) &pystorm::bddriver::FieldHCVal, "C++: pystorm::bddriver::FieldHCVal(pystorm::bddriver::TATOutputTag) --> unsigned int", pybind11::arg("field_name"));

	// pystorm::bddriver::NeuronConfig file: line:226
	pybind11::enum_<pystorm::bddriver::NeuronConfig>(M("pystorm::bddriver"), "NeuronConfig", "")
		.value("ROW_HI", pystorm::bddriver::NeuronConfig::ROW_HI)
		.value("COL_HI", pystorm::bddriver::NeuronConfig::COL_HI)
		.value("ROW_LO", pystorm::bddriver::NeuronConfig::ROW_LO)
		.value("COL_LO", pystorm::bddriver::NeuronConfig::COL_LO)
		.value("BIT_VAL", pystorm::bddriver::NeuronConfig::BIT_VAL)
		.value("BIT_SEL", pystorm::bddriver::NeuronConfig::BIT_SEL)
		.value("FIXED_2", pystorm::bddriver::NeuronConfig::FIXED_2)
		.value("TILE_ADDR", pystorm::bddriver::NeuronConfig::TILE_ADDR)
		.value("FIELDCOUNT", pystorm::bddriver::NeuronConfig::FIELDCOUNT);

;

	// pystorm::bddriver::FieldWidth(pystorm::bddriver::NeuronConfig) file: line:39
	M("pystorm::bddriver").def("FieldWidth", (unsigned int (*)(pystorm::bddriver::NeuronConfig)) &pystorm::bddriver::FieldWidth, "C++: pystorm::bddriver::FieldWidth(pystorm::bddriver::NeuronConfig) --> unsigned int", pybind11::arg("field_name"));

	// pystorm::bddriver::FieldHCVal(pystorm::bddriver::NeuronConfig) file: line:44
	M("pystorm::bddriver").def("FieldHCVal", (unsigned int (*)(pystorm::bddriver::NeuronConfig)) &pystorm::bddriver::FieldHCVal, "C++: pystorm::bddriver::FieldHCVal(pystorm::bddriver::NeuronConfig) --> unsigned int", pybind11::arg("field_name"));

	// pystorm::bddriver::FPGAIO file: line:232
	pybind11::enum_<pystorm::bddriver::FPGAIO>(M("pystorm::bddriver"), "FPGAIO", "")
		.value("PAYLOAD", pystorm::bddriver::FPGAIO::PAYLOAD)
		.value("EP_CODE", pystorm::bddriver::FPGAIO::EP_CODE)
		.value("FIELDCOUNT", pystorm::bddriver::FPGAIO::FIELDCOUNT);

;

	// pystorm::bddriver::FieldWidth(pystorm::bddriver::FPGAIO) file: line:56
	M("pystorm::bddriver").def("FieldWidth", (unsigned int (*)(pystorm::bddriver::FPGAIO)) &pystorm::bddriver::FieldWidth, "C++: pystorm::bddriver::FieldWidth(pystorm::bddriver::FPGAIO) --> unsigned int", pybind11::arg("field_name"));

	// pystorm::bddriver::FieldHCVal(pystorm::bddriver::FPGAIO) file: line:61
	M("pystorm::bddriver").def("FieldHCVal", (unsigned int (*)(pystorm::bddriver::FPGAIO)) &pystorm::bddriver::FieldHCVal, "C++: pystorm::bddriver::FieldHCVal(pystorm::bddriver::FPGAIO) --> unsigned int", pybind11::arg("field_name"));

	// pystorm::bddriver::FPGABYTES file: line:236
	pybind11::enum_<pystorm::bddriver::FPGABYTES>(M("pystorm::bddriver"), "FPGABYTES", "")
		.value("B0", pystorm::bddriver::FPGABYTES::B0)
		.value("B1", pystorm::bddriver::FPGABYTES::B1)
		.value("B2", pystorm::bddriver::FPGABYTES::B2)
		.value("B3", pystorm::bddriver::FPGABYTES::B3)
		.value("FIELDCOUNT", pystorm::bddriver::FPGABYTES::FIELDCOUNT);

;

	// pystorm::bddriver::FieldWidth(pystorm::bddriver::FPGABYTES) file: line:56
	M("pystorm::bddriver").def("FieldWidth", (unsigned int (*)(pystorm::bddriver::FPGABYTES)) &pystorm::bddriver::FieldWidth, "C++: pystorm::bddriver::FieldWidth(pystorm::bddriver::FPGABYTES) --> unsigned int", pybind11::arg("field_name"));

	// pystorm::bddriver::FieldHCVal(pystorm::bddriver::FPGABYTES) file: line:61
	M("pystorm::bddriver").def("FieldHCVal", (unsigned int (*)(pystorm::bddriver::FPGABYTES)) &pystorm::bddriver::FieldHCVal, "C++: pystorm::bddriver::FieldHCVal(pystorm::bddriver::FPGABYTES) --> unsigned int", pybind11::arg("field_name"));

	// pystorm::bddriver::TWOFPGAPAYLOADS file: line:242
	pybind11::enum_<pystorm::bddriver::TWOFPGAPAYLOADS>(M("pystorm::bddriver"), "TWOFPGAPAYLOADS", "")
		.value("LSB", pystorm::bddriver::TWOFPGAPAYLOADS::LSB)
		.value("MSB", pystorm::bddriver::TWOFPGAPAYLOADS::MSB)
		.value("FIELDCOUNT", pystorm::bddriver::TWOFPGAPAYLOADS::FIELDCOUNT);

;

	// pystorm::bddriver::FieldWidth(pystorm::bddriver::TWOFPGAPAYLOADS) file: line:56
	M("pystorm::bddriver").def("FieldWidth", (unsigned int (*)(pystorm::bddriver::TWOFPGAPAYLOADS)) &pystorm::bddriver::FieldWidth, "C++: pystorm::bddriver::FieldWidth(pystorm::bddriver::TWOFPGAPAYLOADS) --> unsigned int", pybind11::arg("field_name"));

	// pystorm::bddriver::FieldHCVal(pystorm::bddriver::TWOFPGAPAYLOADS) file: line:61
	M("pystorm::bddriver").def("FieldHCVal", (unsigned int (*)(pystorm::bddriver::TWOFPGAPAYLOADS)) &pystorm::bddriver::FieldHCVal, "C++: pystorm::bddriver::FieldHCVal(pystorm::bddriver::TWOFPGAPAYLOADS) --> unsigned int", pybind11::arg("field_name"));

	// pystorm::bddriver::THREEFPGAREGS file: line:246
	pybind11::enum_<pystorm::bddriver::THREEFPGAREGS>(M("pystorm::bddriver"), "THREEFPGAREGS", "")
		.value("W0", pystorm::bddriver::THREEFPGAREGS::W0)
		.value("W1", pystorm::bddriver::THREEFPGAREGS::W1)
		.value("W2", pystorm::bddriver::THREEFPGAREGS::W2)
		.value("FIELDCOUNT", pystorm::bddriver::THREEFPGAREGS::FIELDCOUNT);

;

	// pystorm::bddriver::FieldWidth(pystorm::bddriver::THREEFPGAREGS) file: line:56
	M("pystorm::bddriver").def("FieldWidth", (unsigned int (*)(pystorm::bddriver::THREEFPGAREGS)) &pystorm::bddriver::FieldWidth, "C++: pystorm::bddriver::FieldWidth(pystorm::bddriver::THREEFPGAREGS) --> unsigned int", pybind11::arg("field_name"));

	// pystorm::bddriver::FieldHCVal(pystorm::bddriver::THREEFPGAREGS) file: line:61
	M("pystorm::bddriver").def("FieldHCVal", (unsigned int (*)(pystorm::bddriver::THREEFPGAREGS)) &pystorm::bddriver::FieldHCVal, "C++: pystorm::bddriver::FieldHCVal(pystorm::bddriver::THREEFPGAREGS) --> unsigned int", pybind11::arg("field_name"));

    // GetField & PackWord template functions are explicitly expanded here
    M("pystorm::bddriver").def("GetField", pybind11::overload_cast<uint64_t, pystorm::bddriver::ToggleWord>(&pystorm::bddriver::GetField<pystorm::bddriver::ToggleWord>), "Get ToggleWord BDWord field");

    M("pystorm::bddriver").def("PackWord", (uint64_t (*) (std::vector<std::pair<pystorm::bddriver::ToggleWord, uint64_t>>))(&pystorm::bddriver::PackWordVectorized<pystorm::bddriver::ToggleWord>), "Pack ToggleWord BDWord fields");

    M("pystorm::bddriver").def("GetField", pybind11::overload_cast<uint64_t, pystorm::bddriver::DACWord>(&pystorm::bddriver::GetField<pystorm::bddriver::DACWord>), "Get DACWord BDWord field");

    M("pystorm::bddriver").def("PackWord", (uint64_t (*) (std::vector<std::pair<pystorm::bddriver::DACWord, uint64_t>>))(&pystorm::bddriver::PackWordVectorized<pystorm::bddriver::DACWord>), "Pack DACWord BDWord fields");

    M("pystorm::bddriver").def("GetField", pybind11::overload_cast<uint64_t, pystorm::bddriver::ADCWord>(&pystorm::bddriver::GetField<pystorm::bddriver::ADCWord>), "Get ADCWord BDWord field");

    M("pystorm::bddriver").def("PackWord", (uint64_t (*) (std::vector<std::pair<pystorm::bddriver::ADCWord, uint64_t>>))(&pystorm::bddriver::PackWordVectorized<pystorm::bddriver::ADCWord>), "Pack ADCWord BDWord fields");

    M("pystorm::bddriver").def("GetField", pybind11::overload_cast<uint64_t, pystorm::bddriver::DelayWord>(&pystorm::bddriver::GetField<pystorm::bddriver::DelayWord>), "Get DelayWord BDWord field");

    M("pystorm::bddriver").def("PackWord", (uint64_t (*) (std::vector<std::pair<pystorm::bddriver::DelayWord, uint64_t>>))(&pystorm::bddriver::PackWordVectorized<pystorm::bddriver::DelayWord>), "Pack DelayWord BDWord fields");

    M("pystorm::bddriver").def("GetField", pybind11::overload_cast<uint64_t, pystorm::bddriver::AMWord>(&pystorm::bddriver::GetField<pystorm::bddriver::AMWord>), "Get AMWord BDWord field");

    M("pystorm::bddriver").def("PackWord", (uint64_t (*) (std::vector<std::pair<pystorm::bddriver::AMWord, uint64_t>>))(&pystorm::bddriver::PackWordVectorized<pystorm::bddriver::AMWord>), "Pack AMWord BDWord fields");

    M("pystorm::bddriver").def("GetField", pybind11::overload_cast<uint64_t, pystorm::bddriver::MMWord>(&pystorm::bddriver::GetField<pystorm::bddriver::MMWord>), "Get MMWord BDWord field");

    M("pystorm::bddriver").def("PackWord", (uint64_t (*) (std::vector<std::pair<pystorm::bddriver::MMWord, uint64_t>>))(&pystorm::bddriver::PackWordVectorized<pystorm::bddriver::MMWord>), "Pack MMWord BDWord fields");

    M("pystorm::bddriver").def("GetField", pybind11::overload_cast<uint64_t, pystorm::bddriver::PATWord>(&pystorm::bddriver::GetField<pystorm::bddriver::PATWord>), "Get PATWord BDWord field");

    M("pystorm::bddriver").def("PackWord", (uint64_t (*) (std::vector<std::pair<pystorm::bddriver::PATWord, uint64_t>>))(&pystorm::bddriver::PackWordVectorized<pystorm::bddriver::PATWord>), "Pack PATWord BDWord fields");

    M("pystorm::bddriver").def("GetField", pybind11::overload_cast<uint64_t, pystorm::bddriver::TATAccWord>(&pystorm::bddriver::GetField<pystorm::bddriver::TATAccWord>), "Get TATAccWord BDWord field");

    M("pystorm::bddriver").def("PackWord", (uint64_t (*) (std::vector<std::pair<pystorm::bddriver::TATAccWord, uint64_t>>))(&pystorm::bddriver::PackWordVectorized<pystorm::bddriver::TATAccWord>), "Pack TATAccWord BDWord fields");

    M("pystorm::bddriver").def("GetField", pybind11::overload_cast<uint64_t, pystorm::bddriver::TATSpikeWord>(&pystorm::bddriver::GetField<pystorm::bddriver::TATSpikeWord>), "Get TATSpikeWord BDWord field");

    M("pystorm::bddriver").def("PackWord", (uint64_t (*) (std::vector<std::pair<pystorm::bddriver::TATSpikeWord, uint64_t>>))(&pystorm::bddriver::PackWordVectorized<pystorm::bddriver::TATSpikeWord>), "Pack TATSpikeWord BDWord fields");

    M("pystorm::bddriver").def("GetField", pybind11::overload_cast<uint64_t, pystorm::bddriver::TATTagWord>(&pystorm::bddriver::GetField<pystorm::bddriver::TATTagWord>), "Get TATTagWord BDWord field");

    M("pystorm::bddriver").def("PackWord", (uint64_t (*) (std::vector<std::pair<pystorm::bddriver::TATTagWord, uint64_t>>))(&pystorm::bddriver::PackWordVectorized<pystorm::bddriver::TATTagWord>), "Pack TATTagWord BDWord fields");

    M("pystorm::bddriver").def("GetField", pybind11::overload_cast<uint64_t, pystorm::bddriver::PATWrite>(&pystorm::bddriver::GetField<pystorm::bddriver::PATWrite>), "Get PATWrite BDWord field");

    M("pystorm::bddriver").def("PackWord", (uint64_t (*) (std::vector<std::pair<pystorm::bddriver::PATWrite, uint64_t>>))(&pystorm::bddriver::PackWordVectorized<pystorm::bddriver::PATWrite>), "Pack PATWrite BDWord fields");

    M("pystorm::bddriver").def("GetField", pybind11::overload_cast<uint64_t, pystorm::bddriver::PATRead>(&pystorm::bddriver::GetField<pystorm::bddriver::PATRead>), "Get PATRead BDWord field");

    M("pystorm::bddriver").def("PackWord", (uint64_t (*) (std::vector<std::pair<pystorm::bddriver::PATRead, uint64_t>>))(&pystorm::bddriver::PackWordVectorized<pystorm::bddriver::PATRead>), "Pack PATRead BDWord fields");

    M("pystorm::bddriver").def("GetField", pybind11::overload_cast<uint64_t, pystorm::bddriver::TATSetAddress>(&pystorm::bddriver::GetField<pystorm::bddriver::TATSetAddress>), "Get TATSetAddress BDWord field");

    M("pystorm::bddriver").def("PackWord", (uint64_t (*) (std::vector<std::pair<pystorm::bddriver::TATSetAddress, uint64_t>>))(&pystorm::bddriver::PackWordVectorized<pystorm::bddriver::TATSetAddress>), "Pack TATSetAddress BDWord fields");

    M("pystorm::bddriver").def("GetField", pybind11::overload_cast<uint64_t, pystorm::bddriver::TATWriteIncrement>(&pystorm::bddriver::GetField<pystorm::bddriver::TATWriteIncrement>), "Get TATWriteIncrement BDWord field");

    M("pystorm::bddriver").def("PackWord", (uint64_t (*) (std::vector<std::pair<pystorm::bddriver::TATWriteIncrement, uint64_t>>))(&pystorm::bddriver::PackWordVectorized<pystorm::bddriver::TATWriteIncrement>), "Pack TATWriteIncrement BDWord fields");

    M("pystorm::bddriver").def("GetField", pybind11::overload_cast<uint64_t, pystorm::bddriver::TATReadIncrement>(&pystorm::bddriver::GetField<pystorm::bddriver::TATReadIncrement>), "Get TATReadIncrement BDWord field");

    M("pystorm::bddriver").def("PackWord", (uint64_t (*) (std::vector<std::pair<pystorm::bddriver::TATReadIncrement, uint64_t>>))(&pystorm::bddriver::PackWordVectorized<pystorm::bddriver::TATReadIncrement>), "Pack TATReadIncrement BDWord fields");

    M("pystorm::bddriver").def("GetField", pybind11::overload_cast<uint64_t, pystorm::bddriver::MMSetAddress>(&pystorm::bddriver::GetField<pystorm::bddriver::MMSetAddress>), "Get MMSetAddress BDWord field");

    M("pystorm::bddriver").def("PackWord", (uint64_t (*) (std::vector<std::pair<pystorm::bddriver::MMSetAddress, uint64_t>>))(&pystorm::bddriver::PackWordVectorized<pystorm::bddriver::MMSetAddress>), "Pack MMSetAddress BDWord fields");

    M("pystorm::bddriver").def("GetField", pybind11::overload_cast<uint64_t, pystorm::bddriver::MMWriteIncrement>(&pystorm::bddriver::GetField<pystorm::bddriver::MMWriteIncrement>), "Get MMWriteIncrement BDWord field");

    M("pystorm::bddriver").def("PackWord", (uint64_t (*) (std::vector<std::pair<pystorm::bddriver::MMWriteIncrement, uint64_t>>))(&pystorm::bddriver::PackWordVectorized<pystorm::bddriver::MMWriteIncrement>), "Pack MMWriteIncrement BDWord fields");

    M("pystorm::bddriver").def("GetField", pybind11::overload_cast<uint64_t, pystorm::bddriver::MMReadIncrement>(&pystorm::bddriver::GetField<pystorm::bddriver::MMReadIncrement>), "Get MMReadIncrement BDWord field");

    M("pystorm::bddriver").def("PackWord", (uint64_t (*) (std::vector<std::pair<pystorm::bddriver::MMReadIncrement, uint64_t>>))(&pystorm::bddriver::PackWordVectorized<pystorm::bddriver::MMReadIncrement>), "Pack MMReadIncrement BDWord fields");

    M("pystorm::bddriver").def("GetField", pybind11::overload_cast<uint64_t, pystorm::bddriver::AMSetAddress>(&pystorm::bddriver::GetField<pystorm::bddriver::AMSetAddress>), "Get AMSetAddress BDWord field");

    M("pystorm::bddriver").def("PackWord", (uint64_t (*) (std::vector<std::pair<pystorm::bddriver::AMSetAddress, uint64_t>>))(&pystorm::bddriver::PackWordVectorized<pystorm::bddriver::AMSetAddress>), "Pack AMSetAddress BDWord fields");

    M("pystorm::bddriver").def("GetField", pybind11::overload_cast<uint64_t, pystorm::bddriver::AMReadWrite>(&pystorm::bddriver::GetField<pystorm::bddriver::AMReadWrite>), "Get AMReadWrite BDWord field");

    M("pystorm::bddriver").def("PackWord", (uint64_t (*) (std::vector<std::pair<pystorm::bddriver::AMReadWrite, uint64_t>>))(&pystorm::bddriver::PackWordVectorized<pystorm::bddriver::AMReadWrite>), "Pack AMReadWrite BDWord fields");

    M("pystorm::bddriver").def("GetField", pybind11::overload_cast<uint64_t, pystorm::bddriver::AMIncrement>(&pystorm::bddriver::GetField<pystorm::bddriver::AMIncrement>), "Get AMIncrement BDWord field");

    M("pystorm::bddriver").def("PackWord", (uint64_t (*) (std::vector<std::pair<pystorm::bddriver::AMIncrement, uint64_t>>))(&pystorm::bddriver::PackWordVectorized<pystorm::bddriver::AMIncrement>), "Pack AMIncrement BDWord fields");

    M("pystorm::bddriver").def("GetField", pybind11::overload_cast<uint64_t, pystorm::bddriver::AMEncapsulation>(&pystorm::bddriver::GetField<pystorm::bddriver::AMEncapsulation>), "Get AMEncapsulation BDWord field");

    M("pystorm::bddriver").def("PackWord", (uint64_t (*) (std::vector<std::pair<pystorm::bddriver::AMEncapsulation, uint64_t>>))(&pystorm::bddriver::PackWordVectorized<pystorm::bddriver::AMEncapsulation>), "Pack AMEncapsulation BDWord fields");

    M("pystorm::bddriver").def("GetField", pybind11::overload_cast<uint64_t, pystorm::bddriver::MMEncapsulation>(&pystorm::bddriver::GetField<pystorm::bddriver::MMEncapsulation>), "Get MMEncapsulation BDWord field");

    M("pystorm::bddriver").def("PackWord", (uint64_t (*) (std::vector<std::pair<pystorm::bddriver::MMEncapsulation, uint64_t>>))(&pystorm::bddriver::PackWordVectorized<pystorm::bddriver::MMEncapsulation>), "Pack MMEncapsulation BDWord fields");

    M("pystorm::bddriver").def("GetField", pybind11::overload_cast<uint64_t, pystorm::bddriver::InputTag>(&pystorm::bddriver::GetField<pystorm::bddriver::InputTag>), "Get InputTag BDWord field");

    M("pystorm::bddriver").def("PackWord", (uint64_t (*) (std::vector<std::pair<pystorm::bddriver::InputTag, uint64_t>>))(&pystorm::bddriver::PackWordVectorized<pystorm::bddriver::InputTag>), "Pack InputTag BDWord fields");

    M("pystorm::bddriver").def("GetField", pybind11::overload_cast<uint64_t, pystorm::bddriver::FIFOInputTag>(&pystorm::bddriver::GetField<pystorm::bddriver::FIFOInputTag>), "Get FIFOInputTag BDWord field");

    M("pystorm::bddriver").def("PackWord", (uint64_t (*) (std::vector<std::pair<pystorm::bddriver::FIFOInputTag, uint64_t>>))(&pystorm::bddriver::PackWordVectorized<pystorm::bddriver::FIFOInputTag>), "Pack FIFOInputTag BDWord fields");

    M("pystorm::bddriver").def("GetField", pybind11::overload_cast<uint64_t, pystorm::bddriver::InputSpike>(&pystorm::bddriver::GetField<pystorm::bddriver::InputSpike>), "Get InputSpike BDWord field");

    M("pystorm::bddriver").def("PackWord", (uint64_t (*) (std::vector<std::pair<pystorm::bddriver::InputSpike, uint64_t>>))(&pystorm::bddriver::PackWordVectorized<pystorm::bddriver::InputSpike>), "Pack InputSpike BDWord fields");

    M("pystorm::bddriver").def("GetField", pybind11::overload_cast<uint64_t, pystorm::bddriver::PreFIFOTag>(&pystorm::bddriver::GetField<pystorm::bddriver::PreFIFOTag>), "Get PreFIFOTag BDWord field");

    M("pystorm::bddriver").def("PackWord", (uint64_t (*) (std::vector<std::pair<pystorm::bddriver::PreFIFOTag, uint64_t>>))(&pystorm::bddriver::PackWordVectorized<pystorm::bddriver::PreFIFOTag>), "Pack PreFIFOTag BDWord fields");

    M("pystorm::bddriver").def("GetField", pybind11::overload_cast<uint64_t, pystorm::bddriver::PostFIFOTag>(&pystorm::bddriver::GetField<pystorm::bddriver::PostFIFOTag>), "Get PostFIFOTag BDWord field");

    M("pystorm::bddriver").def("PackWord", (uint64_t (*) (std::vector<std::pair<pystorm::bddriver::PostFIFOTag, uint64_t>>))(&pystorm::bddriver::PackWordVectorized<pystorm::bddriver::PostFIFOTag>), "Pack PostFIFOTag BDWord fields");

    M("pystorm::bddriver").def("GetField", pybind11::overload_cast<uint64_t, pystorm::bddriver::OutputSpike>(&pystorm::bddriver::GetField<pystorm::bddriver::OutputSpike>), "Get OutputSpike BDWord field");

    M("pystorm::bddriver").def("PackWord", (uint64_t (*) (std::vector<std::pair<pystorm::bddriver::OutputSpike, uint64_t>>))(&pystorm::bddriver::PackWordVectorized<pystorm::bddriver::OutputSpike>), "Pack OutputSpike BDWord fields");

    M("pystorm::bddriver").def("GetField", pybind11::overload_cast<uint64_t, pystorm::bddriver::OverflowTag>(&pystorm::bddriver::GetField<pystorm::bddriver::OverflowTag>), "Get OverflowTag BDWord field");

    M("pystorm::bddriver").def("PackWord", (uint64_t (*) (std::vector<std::pair<pystorm::bddriver::OverflowTag, uint64_t>>))(&pystorm::bddriver::PackWordVectorized<pystorm::bddriver::OverflowTag>), "Pack OverflowTag BDWord fields");

    M("pystorm::bddriver").def("GetField", pybind11::overload_cast<uint64_t, pystorm::bddriver::AccOutputTag>(&pystorm::bddriver::GetField<pystorm::bddriver::AccOutputTag>), "Get AccOutputTag BDWord field");

    M("pystorm::bddriver").def("PackWord", (uint64_t (*) (std::vector<std::pair<pystorm::bddriver::AccOutputTag, uint64_t>>))(&pystorm::bddriver::PackWordVectorized<pystorm::bddriver::AccOutputTag>), "Pack AccOutputTag BDWord fields");

    M("pystorm::bddriver").def("GetField", pybind11::overload_cast<uint64_t, pystorm::bddriver::TATOutputTag>(&pystorm::bddriver::GetField<pystorm::bddriver::TATOutputTag>), "Get TATOutputTag BDWord field");

    M("pystorm::bddriver").def("PackWord", (uint64_t (*) (std::vector<std::pair<pystorm::bddriver::TATOutputTag, uint64_t>>))(&pystorm::bddriver::PackWordVectorized<pystorm::bddriver::TATOutputTag>), "Pack TATOutputTag BDWord fields");

    M("pystorm::bddriver").def("GetField", pybind11::overload_cast<uint64_t, pystorm::bddriver::NeuronConfig>(&pystorm::bddriver::GetField<pystorm::bddriver::NeuronConfig>), "Get NeuronConfig BDWord field");

    M("pystorm::bddriver").def("PackWord", (uint64_t (*) (std::vector<std::pair<pystorm::bddriver::NeuronConfig, uint64_t>>))(&pystorm::bddriver::PackWordVectorized<pystorm::bddriver::NeuronConfig>), "Pack NeuronConfig BDWord fields");

    M("pystorm::bddriver").def("GetField", pybind11::overload_cast<uint64_t, pystorm::bddriver::FPGAIO>(&pystorm::bddriver::GetField<pystorm::bddriver::FPGAIO>), "Get FPGAIO BDWord field");

    M("pystorm::bddriver").def("PackWord", (uint64_t (*) (std::vector<std::pair<pystorm::bddriver::FPGAIO, uint64_t>>))(&pystorm::bddriver::PackWordVectorized<pystorm::bddriver::FPGAIO>), "Pack FPGAIO BDWord fields");

    M("pystorm::bddriver").def("GetField", pybind11::overload_cast<uint64_t, pystorm::bddriver::FPGABYTES>(&pystorm::bddriver::GetField<pystorm::bddriver::FPGABYTES>), "Get FPGABYTES BDWord field");

    M("pystorm::bddriver").def("PackWord", (uint64_t (*) (std::vector<std::pair<pystorm::bddriver::FPGABYTES, uint64_t>>))(&pystorm::bddriver::PackWordVectorized<pystorm::bddriver::FPGABYTES>), "Pack FPGABYTES BDWord fields");

    M("pystorm::bddriver").def("GetField", pybind11::overload_cast<uint64_t, pystorm::bddriver::TWOFPGAPAYLOADS>(&pystorm::bddriver::GetField<pystorm::bddriver::TWOFPGAPAYLOADS>), "Get TWOFPGAPAYLOADS BDWord field");

    M("pystorm::bddriver").def("PackWord", (uint64_t (*) (std::vector<std::pair<pystorm::bddriver::TWOFPGAPAYLOADS, uint64_t>>))(&pystorm::bddriver::PackWordVectorized<pystorm::bddriver::TWOFPGAPAYLOADS>), "Pack TWOFPGAPAYLOADS BDWord fields");

    M("pystorm::bddriver").def("GetField", pybind11::overload_cast<uint64_t, pystorm::bddriver::THREEFPGAREGS>(&pystorm::bddriver::GetField<pystorm::bddriver::THREEFPGAREGS>), "Get THREEFPGAREGS BDWord field");

    M("pystorm::bddriver").def("PackWord", (uint64_t (*) (std::vector<std::pair<pystorm::bddriver::THREEFPGAREGS, uint64_t>>))(&pystorm::bddriver::PackWordVectorized<pystorm::bddriver::THREEFPGAREGS>), "Pack THREEFPGAREGS BDWord fields");
}

typedef std::function< pybind11::module & (std::string const &) > ModuleGetter;

void bind_unknown_unknown(std::function< pybind11::module &(std::string const &namespace_) > &M);
void bind_unknown_unknown_1(std::function< pybind11::module &(std::string const &namespace_) > &M);
void bind_unknown_unknown_2(std::function< pybind11::module &(std::string const &namespace_) > &M);
void bind_unknown_unknown_3(std::function< pybind11::module &(std::string const &namespace_) > &M);
void bind_model_BDModelDriver(std::function< pybind11::module &(std::string const &namespace_) > &M);


PYBIND11_PLUGIN(_PyDriver) {
	std::map <std::string, std::shared_ptr<pybind11::module> > modules;
	ModuleGetter M = [&](std::string const &namespace_) -> pybind11::module & {
		auto it = modules.find(namespace_);
		if( it == modules.end() ) throw std::runtime_error("Attempt to access pybind11::module for namespace " + namespace_ + " before it was created!!!");
		return * it->second;
	};

	modules[""] = std::make_shared<pybind11::module>("PyDriver", "BDDriver module");

	std::vector< std::pair<std::string, std::string> > sub_modules {
		{"", "pystorm"},
		{"pystorm", "bddriver"},
		{"pystorm::bddriver", "bdmodel"},
		{"pystorm::bddriver", "bdpars"},
		{"pystorm::bddriver", "driverpars"},
	};
	for(auto &p : sub_modules ) modules[p.first.size() ? p.first+"::"+p.second : p.second] = std::make_shared<pybind11::module>( modules[p.first]->def_submodule(p.second.c_str(), ("Bindings for " + p.first + "::" + p.second + " namespace").c_str() ) );

	pybind11::class_<std::shared_ptr<void>>(M(""), "_encapsulated_data_");

	bind_unknown_unknown(M);
	bind_unknown_unknown_1(M);
	bind_unknown_unknown_2(M);
	bind_unknown_unknown_3(M);
	bind_MemInfo(M);
	bind_model_BDModelDriver(M);
    bind_BDWord(M);

	return modules[""]->ptr();
}
