// File: std/array.cpp
#include <Driver.h>
#include <array>
#include <functional>
#include <initializer_list>
#include <iterator>
#include <map>
#include <memory>
#include <sstream> // __str__
#include <string>
#include <tuple>
#include <utility>
#include <vector>

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <Driver.h>
#include <comm/CommBDModel.h>
#include <common/MutexBuffer.h>

using namespace pystorm;
using namespace bddriver;

typedef pystorm::bddriver::MutexBuffer<unsigned char> MutexBuffer_uchar;

#ifndef BINDER_PYBIND11_TYPE_CASTER
	#define BINDER_PYBIND11_TYPE_CASTER
	PYBIND11_DECLARE_HOLDER_TYPE(T, std::shared_ptr<T>);
	PYBIND11_DECLARE_HOLDER_TYPE(T, T*);
	PYBIND11_MAKE_OPAQUE(std::shared_ptr<void>);
#endif

// File: unknown/unknown.cpp
#include <initializer_list>
#include <iterator>
#include <memory>
#include <sstream> // __str__
#include <utility>
#include <vector>

#include <pybind11/pybind11.h>

#ifndef BINDER_PYBIND11_TYPE_CASTER
	#define BINDER_PYBIND11_TYPE_CASTER
	PYBIND11_DECLARE_HOLDER_TYPE(T, std::shared_ptr<T>);
	PYBIND11_DECLARE_HOLDER_TYPE(T, T*);
	PYBIND11_MAKE_OPAQUE(std::shared_ptr<void>);
#endif

void bind_unknown_unknown(std::function< pybind11::module &(std::string const &namespace_) > &M)
{
	{ // pystorm::bddriver::MutexBuffer file: line:36
		pybind11::class_<pystorm::bddriver::MutexBuffer<unsigned char>, std::shared_ptr<pystorm::bddriver::MutexBuffer<unsigned char>>> cl(M("pystorm::bddriver"), "MutexBuffer_unsigned_char_t", "");
		pybind11::handle cl_type = cl;

		cl.def(pybind11::init<unsigned int>(), pybind11::arg("capacity"));

        /*
		cl.def("Push", [](pystorm::bddriver::MutexBuffer<unsigned char> &o, const class std::vector<unsigned char, class std::allocator<unsigned char> > & a0) -> bool { return o.Push(a0); }, "", pybind11::arg("input"));
		cl.def("Push", (bool (pystorm::bddriver::MutexBuffer<unsigned char>::*)(const class std::vector<unsigned char, class std::allocator<unsigned char> > &, unsigned int)) &pystorm::bddriver::MutexBuffer<unsigned char>::Push, "C++: pystorm::bddriver::MutexBuffer<unsigned char>::Push(const class std::vector<unsigned char, class std::allocator<unsigned char> > &, unsigned int) --> bool", pybind11::arg("input"), pybind11::arg("try_for_us"));
		cl.def("Pop", (unsigned int (pystorm::bddriver::MutexBuffer<unsigned char>::*)(std::vector<unsigned char>*, unsigned int, unsigned int, unsigned int)) &pystorm::bddriver::MutexBuffer<unsigned char>::Pop, "", pybind11::arg("push_to"), pybind11::arg("max_to_pop"), pybind11::arg("try_for_us"), pybind11::arg("multiple"));
		cl.def("Pop", (unsigned int (pystorm::bddriver::MutexBuffer<unsigned char>::*)(unsigned char*, unsigned int, unsigned int, unsigned int)) &pystorm::bddriver::MutexBuffer<unsigned char>::Pop, "", pybind11::arg("copy_to"), pybind11::arg("max_to_pop"), pybind11::arg("try_for_us"), pybind11::arg("multiple"));
		//cl.def("Pop", [](pystorm::bddriver::MutexBuffer<unsigned char> &o, class std::vector<unsigned char, class std::allocator<unsigned char> > * a0, unsigned int  const &a1) -> unsigned int { return o.Pop(a0, a1); }, "", pybind11::arg("push_to"), pybind11::arg("max_to_pop"));
		//cl.def("Pop", [](pystorm::bddriver::MutexBuffer<unsigned char> &o, class std::vector<unsigned char, class std::allocator<unsigned char> > * a0, unsigned int  const &a1, unsigned int  const &a2) -> unsigned int { return o.Pop(a0, a1, a2); }, "", pybind11::arg("push_to"), pybind11::arg("max_to_pop"), pybind11::arg("try_for_us"));
		//cl.def("Pop", (unsigned int (pystorm::bddriver::MutexBuffer<unsigned char>::*)(class std::vector<unsigned char, class std::allocator<unsigned char> > *, unsigned int, unsigned int, unsigned int)) &pystorm::bddriver::MutexBuffer<unsigned char>::Pop, "C++: pystorm::bddriver::MutexBuffer<unsigned char>::Pop(class std::vector<unsigned char, class std::allocator<unsigned char> > *, unsigned int, unsigned int, unsigned int) --> unsigned int", pybind11::arg("push_to"), pybind11::arg("max_to_pop"), pybind11::arg("try_for_us"), pybind11::arg("multiple"));
		cl.def("PopVect", [](pystorm::bddriver::MutexBuffer<unsigned char> &o, unsigned int  const &a0) -> std::vector<unsigned char, class std::allocator<unsigned char> > { return o.PopVect(a0); }, "", pybind11::arg("max_to_pop"));
		cl.def("PopVect", [](pystorm::bddriver::MutexBuffer<unsigned char> &o, unsigned int  const &a0, unsigned int  const &a1) -> std::vector<unsigned char, class std::allocator<unsigned char> > { return o.PopVect(a0, a1); }, "", pybind11::arg("max_to_pop"), pybind11::arg("try_for_us"));
		cl.def("PopVect", (class std::vector<unsigned char, class std::allocator<unsigned char> > (pystorm::bddriver::MutexBuffer<unsigned char>::*)(unsigned int, unsigned int, unsigned int)) &pystorm::bddriver::MutexBuffer<unsigned char>::PopVect, "C++: pystorm::bddriver::MutexBuffer<unsigned char>::PopVect(unsigned int, unsigned int, unsigned int) --> class std::vector<unsigned char, class std::allocator<unsigned char> >", pybind11::arg("max_to_pop"), pybind11::arg("try_for_us"), pybind11::arg("multiple"));
		cl.def("Push", [](pystorm::bddriver::MutexBuffer<unsigned char> &o, const unsigned char * a0, unsigned int  const &a1) -> bool { return o.Push(a0, a1); }, "", pybind11::arg("input"), pybind11::arg("input_len"));
		cl.def("Push", (bool (pystorm::bddriver::MutexBuffer<unsigned char>::*)(const unsigned char *, unsigned int, unsigned int)) &pystorm::bddriver::MutexBuffer<unsigned char>::Push, "C++: pystorm::bddriver::MutexBuffer<unsigned char>::Push(const unsigned char *, unsigned int, unsigned int) --> bool", pybind11::arg("input"), pybind11::arg("input_len"), pybind11::arg("try_for_us"));
		//cl.def("Pop", [](pystorm::bddriver::MutexBuffer<unsigned char> &o, unsigned char * a0, unsigned int  const &a1) -> unsigned int { return o.Pop(a0, a1); }, "", pybind11::arg("copy_to"), pybind11::arg("max_to_pop"));
		//cl.def("Pop", [](pystorm::bddriver::MutexBuffer<unsigned char> &o, unsigned char * a0, unsigned int  const &a1, unsigned int  const &a2) -> unsigned int { return o.Pop(a0, a1, a2); }, "", pybind11::arg("copy_to"), pybind11::arg("max_to_pop"), pybind11::arg("try_for_us"));
		//cl.def("Pop", (unsigned int (pystorm::bddriver::MutexBuffer<unsigned char>::*)(unsigned char *, unsigned int, unsigned int, unsigned int)) &pystorm::bddriver::MutexBuffer<unsigned char>::Pop, "C++: pystorm::bddriver::MutexBuffer<unsigned char>::Pop(unsigned char *, unsigned int, unsigned int, unsigned int) --> unsigned int", pybind11::arg("copy_to"), pybind11::arg("max_to_pop"), pybind11::arg("try_for_us"), pybind11::arg("multiple"));
		cl.def("LockBack", [](pystorm::bddriver::MutexBuffer<unsigned char> &o, unsigned int  const &a0) -> unsigned char * { return o.LockBack(a0); }, "", pybind11::return_value_policy::automatic, pybind11::arg("input_len"));
		cl.def("LockBack", (unsigned char * (pystorm::bddriver::MutexBuffer<unsigned char>::*)(unsigned int, unsigned int)) &pystorm::bddriver::MutexBuffer<unsigned char>::LockBack, "C++: pystorm::bddriver::MutexBuffer<unsigned char>::LockBack(unsigned int, unsigned int) --> unsigned char *", pybind11::return_value_policy::automatic, pybind11::arg("input_len"), pybind11::arg("try_for_us"));
		cl.def("UnlockBack", (void (pystorm::bddriver::MutexBuffer<unsigned char>::*)()) &pystorm::bddriver::MutexBuffer<unsigned char>::UnlockBack, "C++: pystorm::bddriver::MutexBuffer<unsigned char>::UnlockBack() --> void");
		cl.def("LockFront", [](pystorm::bddriver::MutexBuffer<unsigned char> &o, unsigned int  const &a0) -> std::pair<const unsigned char *, unsigned int> { return o.LockFront(a0); }, "", pybind11::arg("max_to_pop"));
		cl.def("LockFront", [](pystorm::bddriver::MutexBuffer<unsigned char> &o, unsigned int  const &a0, unsigned int  const &a1) -> std::pair<const unsigned char *, unsigned int> { return o.LockFront(a0, a1); }, "", pybind11::arg("max_to_pop"), pybind11::arg("try_for_us"));
		cl.def("LockFront", (struct std::pair<const unsigned char *, unsigned int> (pystorm::bddriver::MutexBuffer<unsigned char>::*)(unsigned int, unsigned int, unsigned int)) &pystorm::bddriver::MutexBuffer<unsigned char>::LockFront, "C++: pystorm::bddriver::MutexBuffer<unsigned char>::LockFront(unsigned int, unsigned int, unsigned int) --> struct std::pair<const unsigned char *, unsigned int>", pybind11::arg("max_to_pop"), pybind11::arg("try_for_us"), pybind11::arg("multiple"));
		cl.def("UnlockFront", (void (pystorm::bddriver::MutexBuffer<unsigned char>::*)()) &pystorm::bddriver::MutexBuffer<unsigned char>::UnlockFront, "C++: pystorm::bddriver::MutexBuffer<unsigned char>::UnlockFront() --> void");
        */
	}
}


// File: unknown/unknown_1.cpp
#include <initializer_list>
#include <iterator>
#include <memory>
#include <sstream> // __str__
#include <utility>
#include <vector>

#include <pybind11/pybind11.h>

#ifndef BINDER_PYBIND11_TYPE_CASTER
	#define BINDER_PYBIND11_TYPE_CASTER
	PYBIND11_DECLARE_HOLDER_TYPE(T, std::shared_ptr<T>);
	PYBIND11_DECLARE_HOLDER_TYPE(T, T*);
	PYBIND11_MAKE_OPAQUE(std::shared_ptr<void>);
#endif

// pystorm::bddriver::comm::Comm file: line:65
struct PyCallBack_Comm : public pystorm::bddriver::comm::Comm {
	using pystorm::bddriver::comm::Comm::Comm;

	void StartStreaming() override { 
		pybind11::gil_scoped_acquire gil;
		pybind11::function overload = pybind11::get_overload(static_cast<const pystorm::bddriver::comm::Comm *>(this), "StartStreaming");
		if (overload) {
			auto o = overload.operator()<pybind11::return_value_policy::reference>();
			if (pybind11::detail::cast_is_temporary_value_reference<void>::value) {
				static pybind11::detail::overload_caster_t<void> caster;
				return pybind11::detail::cast_ref<void>(std::move(o), caster);
			}
			else return pybind11::detail::cast_safe<void>(std::move(o));
		}
		pybind11::pybind11_fail("Tried to call pure virtual function \"Comm::StartStreaming\"");
	}
	void StopStreaming() override { 
		pybind11::gil_scoped_acquire gil;
		pybind11::function overload = pybind11::get_overload(static_cast<const pystorm::bddriver::comm::Comm *>(this), "StopStreaming");
		if (overload) {
			auto o = overload.operator()<pybind11::return_value_policy::reference>();
			if (pybind11::detail::cast_is_temporary_value_reference<void>::value) {
				static pybind11::detail::overload_caster_t<void> caster;
				return pybind11::detail::cast_ref<void>(std::move(o), caster);
			}
			else return pybind11::detail::cast_safe<void>(std::move(o));
		}
		pybind11::pybind11_fail("Tried to call pure virtual function \"Comm::StopStreaming\"");
	}
	pystorm::bddriver::comm::CommStreamState GetStreamState() override { 
		pybind11::gil_scoped_acquire gil;
		pybind11::function overload = pybind11::get_overload(static_cast<const pystorm::bddriver::comm::Comm *>(this), "GetStreamState");
		if (overload) {
			auto o = overload.operator()<pybind11::return_value_policy::reference>();
			if (pybind11::detail::cast_is_temporary_value_reference<pystorm::bddriver::comm::CommStreamState>::value) {
				static pybind11::detail::overload_caster_t<pystorm::bddriver::comm::CommStreamState> caster;
				return pybind11::detail::cast_ref<pystorm::bddriver::comm::CommStreamState>(std::move(o), caster);
			}
			else return pybind11::detail::cast_safe<pystorm::bddriver::comm::CommStreamState>(std::move(o));
		}
		pybind11::pybind11_fail("Tried to call pure virtual function \"Comm::GetStreamState\"");
	}
	class pystorm::bddriver::MutexBuffer<unsigned char> * getReadBuffer() override { 
		pybind11::gil_scoped_acquire gil;
		pybind11::function overload = pybind11::get_overload(static_cast<const pystorm::bddriver::comm::Comm *>(this), "getReadBuffer");
		if (overload) {
			auto o = overload.operator()<pybind11::return_value_policy::reference>();
			if (pybind11::detail::cast_is_temporary_value_reference<class pystorm::bddriver::MutexBuffer<unsigned char> *>::value) {
				static pybind11::detail::overload_caster_t<class pystorm::bddriver::MutexBuffer<unsigned char> *> caster;
				return pybind11::detail::cast_ref<class pystorm::bddriver::MutexBuffer<unsigned char> *>(std::move(o), caster);
			}
			else return pybind11::detail::cast_safe<class pystorm::bddriver::MutexBuffer<unsigned char> *>(std::move(o));
		}
		pybind11::pybind11_fail("Tried to call pure virtual function \"Comm::getReadBuffer\"");
	}
	class pystorm::bddriver::MutexBuffer<unsigned char> * getWriteBuffer() override { 
		pybind11::gil_scoped_acquire gil;
		pybind11::function overload = pybind11::get_overload(static_cast<const pystorm::bddriver::comm::Comm *>(this), "getWriteBuffer");
		if (overload) {
			auto o = overload.operator()<pybind11::return_value_policy::reference>();
			if (pybind11::detail::cast_is_temporary_value_reference<class pystorm::bddriver::MutexBuffer<unsigned char> *>::value) {
				static pybind11::detail::overload_caster_t<class pystorm::bddriver::MutexBuffer<unsigned char> *> caster;
				return pybind11::detail::cast_ref<class pystorm::bddriver::MutexBuffer<unsigned char> *>(std::move(o), caster);
			}
			else return pybind11::detail::cast_safe<class pystorm::bddriver::MutexBuffer<unsigned char> *>(std::move(o));
		}
		pybind11::pybind11_fail("Tried to call pure virtual function \"Comm::getWriteBuffer\"");
	}
};

void bind_unknown_unknown_1(std::function< pybind11::module &(std::string const &namespace_) > &M)
{
	// pystorm::bddriver::comm::CommStreamState file: line:18
	pybind11::enum_<pystorm::bddriver::comm::CommStreamState>(M("pystorm::bddriver::comm"), "CommStreamState", "")
		.value("STARTED", pystorm::bddriver::comm::CommStreamState::STARTED)
		.value("STOPPED", pystorm::bddriver::comm::CommStreamState::STOPPED);

;

	{ // pystorm::bddriver::comm::Comm file: line:65
		pybind11::class_<pystorm::bddriver::comm::Comm, std::shared_ptr<pystorm::bddriver::comm::Comm>, PyCallBack_Comm> cl(M("pystorm::bddriver::comm"), "Comm", "auto comm_instance = new CommSoft(input_file, output_file);\n     auto write_buffer  = comm_instance->getWriteBuffer();\n     COMMWordStream wordstream;\n\n         ... Populate the wordstream\n\n     write_buffer->Push(wordstream,write_timeout);\n\n To read data coming from the interface, a user could do the following:\n\n     unsigned int read_timeout = 0;\n     unsigned int max_read_buffer_size = CommSoft::CAPACITY;\n     auto read_buffer = comm_instance->getReadBuffer();\n     auto inputstream = read_buffer(max_read_buffer_size, read_timeout);\n\n         ... Do work with the stream");
		pybind11::handle cl_type = cl;

		cl.def(pybind11::init<>());
		cl.def(pybind11::init<PyCallBack_Comm const &>());
		cl.def("StartStreaming", (void (pystorm::bddriver::comm::Comm::*)()) &pystorm::bddriver::comm::Comm::StartStreaming, "Sets the Comm to a streaming state where words can read and written\n\nC++: pystorm::bddriver::comm::Comm::StartStreaming() --> void");
		cl.def("StopStreaming", (void (pystorm::bddriver::comm::Comm::*)()) &pystorm::bddriver::comm::Comm::StopStreaming, "Sets the Comm to a non-streaming state where words are no longer  read\n or written.\n\nC++: pystorm::bddriver::comm::Comm::StopStreaming() --> void");
		cl.def("GetStreamState", (pystorm::bddriver::comm::CommStreamState (pystorm::bddriver::comm::Comm::*)()) &pystorm::bddriver::comm::Comm::GetStreamState, "Returns the streaming state.\n\nC++: pystorm::bddriver::comm::Comm::GetStreamState() --> pystorm::bddriver::comm::CommStreamState");
		cl.def("getReadBuffer", (class pystorm::bddriver::MutexBuffer<unsigned char> * (pystorm::bddriver::comm::Comm::*)()) &pystorm::bddriver::comm::Comm::getReadBuffer, "Returns the read buffer\n\nC++: pystorm::bddriver::comm::Comm::getReadBuffer() --> class pystorm::bddriver::MutexBuffer<unsigned char> *", pybind11::return_value_policy::automatic);
		cl.def("getWriteBuffer", (class pystorm::bddriver::MutexBuffer<unsigned char> * (pystorm::bddriver::comm::Comm::*)()) &pystorm::bddriver::comm::Comm::getWriteBuffer, "Returns the write buffer\n\nC++: pystorm::bddriver::comm::Comm::getWriteBuffer() --> class pystorm::bddriver::MutexBuffer<unsigned char> *", pybind11::return_value_policy::automatic);
		cl.def("assign", (class pystorm::bddriver::comm::Comm & (pystorm::bddriver::comm::Comm::*)(const class pystorm::bddriver::comm::Comm &)) &pystorm::bddriver::comm::Comm::operator=, "C++: pystorm::bddriver::comm::Comm::operator=(const class pystorm::bddriver::comm::Comm &) --> class pystorm::bddriver::comm::Comm &", pybind11::return_value_policy::automatic, pybind11::arg(""));
	}
}


// File: std/stl_map.cpp
#include <functional>
#include <initializer_list>
#include <iterator>
#include <map>
#include <memory>
#include <sstream> // __str__
#include <typeindex>
#include <typeinfo>
#include <utility>
#include <vector>

#include <pybind11/pybind11.h>

#ifndef BINDER_PYBIND11_TYPE_CASTER
	#define BINDER_PYBIND11_TYPE_CASTER
	PYBIND11_DECLARE_HOLDER_TYPE(T, std::shared_ptr<T>);
	PYBIND11_DECLARE_HOLDER_TYPE(T, T*);
	PYBIND11_MAKE_OPAQUE(std::shared_ptr<void>);
#endif

//void bind_std_stl_map(std::function< pybind11::module &(std::string const &namespace_) > &M)
//{
//	{ // std::map file:bits/stl_map.h line:96
//		pybind11::class_<std::map<pystorm::bddriver::bdpars::ConfigSomaID,std::vector<unsigned int, std::allocator<unsigned int> >,std::less<pystorm::bddriver::bdpars::ConfigSomaID>,std::allocator<std::pair<const pystorm::bddriver::bdpars::ConfigSomaID, std::vector<unsigned int, std::allocator<unsigned int> > > >>, std::shared_ptr<std::map<pystorm::bddriver::bdpars::ConfigSomaID,std::vector<unsigned int, std::allocator<unsigned int> >,std::less<pystorm::bddriver::bdpars::ConfigSomaID>,std::allocator<std::pair<const pystorm::bddriver::bdpars::ConfigSomaID, std::vector<unsigned int, std::allocator<unsigned int> > > >>>> cl(M("std"), "map_pystorm_bddriver_bdpars_ConfigSomaID_std_vector_unsigned_int_std_allocator_unsigned_int_std_less_pystorm_bddriver_bdpars_ConfigSomaID_std_allocator_std_pair_const_pystorm_bddriver_bdpars_ConfigSomaID_std_vector_unsigned_int_std_allocator_unsigned_int_t", "");
//		pybind11::handle cl_type = cl;

//		cl.def(pybind11::init<>());

//		cl.def("__init__", [](std::map<pystorm::bddriver::bdpars::ConfigSomaID,std::vector<unsigned int, std::allocator<unsigned int> >,std::less<pystorm::bddriver::bdpars::ConfigSomaID>,std::allocator<std::pair<const pystorm::bddriver::bdpars::ConfigSomaID, std::vector<unsigned int, std::allocator<unsigned int> > > >> *self_, const struct std::less<pystorm::bddriver::bdpars::ConfigSomaID> & a0) { new (self_) std::map<pystorm::bddriver::bdpars::ConfigSomaID,std::vector<unsigned int, std::allocator<unsigned int> >,std::less<pystorm::bddriver::bdpars::ConfigSomaID>,std::allocator<std::pair<const pystorm::bddriver::bdpars::ConfigSomaID, std::vector<unsigned int, std::allocator<unsigned int> > > >>(a0); }, "doc");
//		cl.def(pybind11::init<const struct std::less<pystorm::bddriver::bdpars::ConfigSomaID> &, const class std::allocator<struct std::pair<const pystorm::bddriver::bdpars::ConfigSomaID, class std::vector<unsigned int, class std::allocator<unsigned int> > > > &>(), pybind11::arg("__comp"), pybind11::arg("__a"));

//		cl.def(pybind11::init<const class std::map<pystorm::bddriver::bdpars::ConfigSomaID, class std::vector<unsigned int, class std::allocator<unsigned int> >, struct std::less<pystorm::bddriver::bdpars::ConfigSomaID>, class std::allocator<struct std::pair<const pystorm::bddriver::bdpars::ConfigSomaID, class std::vector<unsigned int, class std::allocator<unsigned int> > > > > &>(), pybind11::arg("__x"));

//		cl.def("__init__", [](std::map<pystorm::bddriver::bdpars::ConfigSomaID,std::vector<unsigned int, std::allocator<unsigned int> >,std::less<pystorm::bddriver::bdpars::ConfigSomaID>,std::allocator<std::pair<const pystorm::bddriver::bdpars::ConfigSomaID, std::vector<unsigned int, std::allocator<unsigned int> > > >> *self_, class std::initializer_list<struct std::pair<const pystorm::bddriver::bdpars::ConfigSomaID, class std::vector<unsigned int, class std::allocator<unsigned int> > > >  const &a0) { new (self_) std::map<pystorm::bddriver::bdpars::ConfigSomaID,std::vector<unsigned int, std::allocator<unsigned int> >,std::less<pystorm::bddriver::bdpars::ConfigSomaID>,std::allocator<std::pair<const pystorm::bddriver::bdpars::ConfigSomaID, std::vector<unsigned int, std::allocator<unsigned int> > > >>(a0); }, "doc");
//		cl.def("__init__", [](std::map<pystorm::bddriver::bdpars::ConfigSomaID,std::vector<unsigned int, std::allocator<unsigned int> >,std::less<pystorm::bddriver::bdpars::ConfigSomaID>,std::allocator<std::pair<const pystorm::bddriver::bdpars::ConfigSomaID, std::vector<unsigned int, std::allocator<unsigned int> > > >> *self_, class std::initializer_list<struct std::pair<const pystorm::bddriver::bdpars::ConfigSomaID, class std::vector<unsigned int, class std::allocator<unsigned int> > > >  const &a0, const struct std::less<pystorm::bddriver::bdpars::ConfigSomaID> & a1) { new (self_) std::map<pystorm::bddriver::bdpars::ConfigSomaID,std::vector<unsigned int, std::allocator<unsigned int> >,std::less<pystorm::bddriver::bdpars::ConfigSomaID>,std::allocator<std::pair<const pystorm::bddriver::bdpars::ConfigSomaID, std::vector<unsigned int, std::allocator<unsigned int> > > >>(a0, a1); }, "doc");
//		cl.def(pybind11::init<class std::initializer_list<struct std::pair<const pystorm::bddriver::bdpars::ConfigSomaID, class std::vector<unsigned int, class std::allocator<unsigned int> > > >, const struct std::less<pystorm::bddriver::bdpars::ConfigSomaID> &, const class std::allocator<struct std::pair<const pystorm::bddriver::bdpars::ConfigSomaID, class std::vector<unsigned int, class std::allocator<unsigned int> > > > &>(), pybind11::arg("__l"), pybind11::arg("__comp"), pybind11::arg("__a"));

//		cl.def(pybind11::init<const class std::allocator<struct std::pair<const pystorm::bddriver::bdpars::ConfigSomaID, class std::vector<unsigned int, class std::allocator<unsigned int> > > > &>(), pybind11::arg("__a"));

//		cl.def(pybind11::init<const class std::map<pystorm::bddriver::bdpars::ConfigSomaID, class std::vector<unsigned int, class std::allocator<unsigned int> >, struct std::less<pystorm::bddriver::bdpars::ConfigSomaID>, class std::allocator<struct std::pair<const pystorm::bddriver::bdpars::ConfigSomaID, class std::vector<unsigned int, class std::allocator<unsigned int> > > > > &, const class std::allocator<struct std::pair<const pystorm::bddriver::bdpars::ConfigSomaID, class std::vector<unsigned int, class std::allocator<unsigned int> > > > &>(), pybind11::arg("__m"), pybind11::arg("__a"));

//		cl.def(pybind11::init<class std::initializer_list<struct std::pair<const pystorm::bddriver::bdpars::ConfigSomaID, class std::vector<unsigned int, class std::allocator<unsigned int> > > >, const class std::allocator<struct std::pair<const pystorm::bddriver::bdpars::ConfigSomaID, class std::vector<unsigned int, class std::allocator<unsigned int> > > > &>(), pybind11::arg("__l"), pybind11::arg("__a"));

//		cl.def("assign", (class std::map<pystorm::bddriver::bdpars::ConfigSomaID, class std::vector<unsigned int, class std::allocator<unsigned int> >, struct std::less<pystorm::bddriver::bdpars::ConfigSomaID>, class std::allocator<struct std::pair<const pystorm::bddriver::bdpars::ConfigSomaID, class std::vector<unsigned int, class std::allocator<unsigned int> > > > > & (std::map<pystorm::bddriver::bdpars::ConfigSomaID,std::vector<unsigned int, std::allocator<unsigned int> >,std::less<pystorm::bddriver::bdpars::ConfigSomaID>,std::allocator<std::pair<const pystorm::bddriver::bdpars::ConfigSomaID, std::vector<unsigned int, std::allocator<unsigned int> > > >>::*)(const class std::map<pystorm::bddriver::bdpars::ConfigSomaID, class std::vector<unsigned int, class std::allocator<unsigned int> >, struct std::less<pystorm::bddriver::bdpars::ConfigSomaID>, class std::allocator<struct std::pair<const pystorm::bddriver::bdpars::ConfigSomaID, class std::vector<unsigned int, class std::allocator<unsigned int> > > > > &)) &std::map<pystorm::bddriver::bdpars::ConfigSomaID, std::vector<unsigned int, std::allocator<unsigned int> >, std::less<pystorm::bddriver::bdpars::ConfigSomaID>, std::allocator<std::pair<const pystorm::bddriver::bdpars::ConfigSomaID, std::vector<unsigned int, std::allocator<unsigned int> > > > >::operator=, "C++: std::map<pystorm::bddriver::bdpars::ConfigSomaID, std::vector<unsigned int, std::allocator<unsigned int> >, std::less<pystorm::bddriver::bdpars::ConfigSomaID>, std::allocator<std::pair<const pystorm::bddriver::bdpars::ConfigSomaID, std::vector<unsigned int, std::allocator<unsigned int> > > > >::operator=(const class std::map<pystorm::bddriver::bdpars::ConfigSomaID, class std::vector<unsigned int, class std::allocator<unsigned int> >, struct std::less<pystorm::bddriver::bdpars::ConfigSomaID>, class std::allocator<struct std::pair<const pystorm::bddriver::bdpars::ConfigSomaID, class std::vector<unsigned int, class std::allocator<unsigned int> > > > > &) --> class std::map<pystorm::bddriver::bdpars::ConfigSomaID, class std::vector<unsigned int, class std::allocator<unsigned int> >, struct std::less<pystorm::bddriver::bdpars::ConfigSomaID>, class std::allocator<struct std::pair<const pystorm::bddriver::bdpars::ConfigSomaID, class std::vector<unsigned int, class std::allocator<unsigned int> > > > > &", pybind11::return_value_policy::automatic, pybind11::arg("__x"));
//		cl.def("assign", (class std::map<pystorm::bddriver::bdpars::ConfigSomaID, class std::vector<unsigned int, class std::allocator<unsigned int> >, struct std::less<pystorm::bddriver::bdpars::ConfigSomaID>, class std::allocator<struct std::pair<const pystorm::bddriver::bdpars::ConfigSomaID, class std::vector<unsigned int, class std::allocator<unsigned int> > > > > & (std::map<pystorm::bddriver::bdpars::ConfigSomaID,std::vector<unsigned int, std::allocator<unsigned int> >,std::less<pystorm::bddriver::bdpars::ConfigSomaID>,std::allocator<std::pair<const pystorm::bddriver::bdpars::ConfigSomaID, std::vector<unsigned int, std::allocator<unsigned int> > > >>::*)(class std::initializer_list<struct std::pair<const pystorm::bddriver::bdpars::ConfigSomaID, class std::vector<unsigned int, class std::allocator<unsigned int> > > >)) &std::map<pystorm::bddriver::bdpars::ConfigSomaID, std::vector<unsigned int, std::allocator<unsigned int> >, std::less<pystorm::bddriver::bdpars::ConfigSomaID>, std::allocator<std::pair<const pystorm::bddriver::bdpars::ConfigSomaID, std::vector<unsigned int, std::allocator<unsigned int> > > > >::operator=, "C++: std::map<pystorm::bddriver::bdpars::ConfigSomaID, std::vector<unsigned int, std::allocator<unsigned int> >, std::less<pystorm::bddriver::bdpars::ConfigSomaID>, std::allocator<std::pair<const pystorm::bddriver::bdpars::ConfigSomaID, std::vector<unsigned int, std::allocator<unsigned int> > > > >::operator=(class std::initializer_list<struct std::pair<const pystorm::bddriver::bdpars::ConfigSomaID, class std::vector<unsigned int, class std::allocator<unsigned int> > > >) --> class std::map<pystorm::bddriver::bdpars::ConfigSomaID, class std::vector<unsigned int, class std::allocator<unsigned int> >, struct std::less<pystorm::bddriver::bdpars::ConfigSomaID>, class std::allocator<struct std::pair<const pystorm::bddriver::bdpars::ConfigSomaID, class std::vector<unsigned int, class std::allocator<unsigned int> > > > > &", pybind11::return_value_policy::automatic, pybind11::arg("__l"));
//		cl.def("get_allocator", (class std::allocator<struct std::pair<const pystorm::bddriver::bdpars::ConfigSomaID, class std::vector<unsigned int, class std::allocator<unsigned int> > > > (std::map<pystorm::bddriver::bdpars::ConfigSomaID,std::vector<unsigned int, std::allocator<unsigned int> >,std::less<pystorm::bddriver::bdpars::ConfigSomaID>,std::allocator<std::pair<const pystorm::bddriver::bdpars::ConfigSomaID, std::vector<unsigned int, std::allocator<unsigned int> > > >>::*)() const) &std::map<pystorm::bddriver::bdpars::ConfigSomaID, std::vector<unsigned int, std::allocator<unsigned int> >, std::less<pystorm::bddriver::bdpars::ConfigSomaID>, std::allocator<std::pair<const pystorm::bddriver::bdpars::ConfigSomaID, std::vector<unsigned int, std::allocator<unsigned int> > > > >::get_allocator, "C++: std::map<pystorm::bddriver::bdpars::ConfigSomaID, std::vector<unsigned int, std::allocator<unsigned int> >, std::less<pystorm::bddriver::bdpars::ConfigSomaID>, std::allocator<std::pair<const pystorm::bddriver::bdpars::ConfigSomaID, std::vector<unsigned int, std::allocator<unsigned int> > > > >::get_allocator() const --> class std::allocator<struct std::pair<const pystorm::bddriver::bdpars::ConfigSomaID, class std::vector<unsigned int, class std::allocator<unsigned int> > > >");
//		cl.def("begin", (struct std::_Rb_tree_iterator<struct std::pair<const pystorm::bddriver::bdpars::ConfigSomaID, class std::vector<unsigned int, class std::allocator<unsigned int> > > > (std::map<pystorm::bddriver::bdpars::ConfigSomaID,std::vector<unsigned int, std::allocator<unsigned int> >,std::less<pystorm::bddriver::bdpars::ConfigSomaID>,std::allocator<std::pair<const pystorm::bddriver::bdpars::ConfigSomaID, std::vector<unsigned int, std::allocator<unsigned int> > > >>::*)()) &std::map<pystorm::bddriver::bdpars::ConfigSomaID, std::vector<unsigned int, std::allocator<unsigned int> >, std::less<pystorm::bddriver::bdpars::ConfigSomaID>, std::allocator<std::pair<const pystorm::bddriver::bdpars::ConfigSomaID, std::vector<unsigned int, std::allocator<unsigned int> > > > >::begin, "C++: std::map<pystorm::bddriver::bdpars::ConfigSomaID, std::vector<unsigned int, std::allocator<unsigned int> >, std::less<pystorm::bddriver::bdpars::ConfigSomaID>, std::allocator<std::pair<const pystorm::bddriver::bdpars::ConfigSomaID, std::vector<unsigned int, std::allocator<unsigned int> > > > >::begin() --> struct std::_Rb_tree_iterator<struct std::pair<const pystorm::bddriver::bdpars::ConfigSomaID, class std::vector<unsigned int, class std::allocator<unsigned int> > > >");
//		cl.def("begin", (struct std::_Rb_tree_const_iterator<struct std::pair<const pystorm::bddriver::bdpars::ConfigSomaID, class std::vector<unsigned int, class std::allocator<unsigned int> > > > (std::map<pystorm::bddriver::bdpars::ConfigSomaID,std::vector<unsigned int, std::allocator<unsigned int> >,std::less<pystorm::bddriver::bdpars::ConfigSomaID>,std::allocator<std::pair<const pystorm::bddriver::bdpars::ConfigSomaID, std::vector<unsigned int, std::allocator<unsigned int> > > >>::*)() const) &std::map<pystorm::bddriver::bdpars::ConfigSomaID, std::vector<unsigned int, std::allocator<unsigned int> >, std::less<pystorm::bddriver::bdpars::ConfigSomaID>, std::allocator<std::pair<const pystorm::bddriver::bdpars::ConfigSomaID, std::vector<unsigned int, std::allocator<unsigned int> > > > >::begin, "C++: std::map<pystorm::bddriver::bdpars::ConfigSomaID, std::vector<unsigned int, std::allocator<unsigned int> >, std::less<pystorm::bddriver::bdpars::ConfigSomaID>, std::allocator<std::pair<const pystorm::bddriver::bdpars::ConfigSomaID, std::vector<unsigned int, std::allocator<unsigned int> > > > >::begin() const --> struct std::_Rb_tree_const_iterator<struct std::pair<const pystorm::bddriver::bdpars::ConfigSomaID, class std::vector<unsigned int, class std::allocator<unsigned int> > > >");
//		cl.def("end", (struct std::_Rb_tree_iterator<struct std::pair<const pystorm::bddriver::bdpars::ConfigSomaID, class std::vector<unsigned int, class std::allocator<unsigned int> > > > (std::map<pystorm::bddriver::bdpars::ConfigSomaID,std::vector<unsigned int, std::allocator<unsigned int> >,std::less<pystorm::bddriver::bdpars::ConfigSomaID>,std::allocator<std::pair<const pystorm::bddriver::bdpars::ConfigSomaID, std::vector<unsigned int, std::allocator<unsigned int> > > >>::*)()) &std::map<pystorm::bddriver::bdpars::ConfigSomaID, std::vector<unsigned int, std::allocator<unsigned int> >, std::less<pystorm::bddriver::bdpars::ConfigSomaID>, std::allocator<std::pair<const pystorm::bddriver::bdpars::ConfigSomaID, std::vector<unsigned int, std::allocator<unsigned int> > > > >::end, "C++: std::map<pystorm::bddriver::bdpars::ConfigSomaID, std::vector<unsigned int, std::allocator<unsigned int> >, std::less<pystorm::bddriver::bdpars::ConfigSomaID>, std::allocator<std::pair<const pystorm::bddriver::bdpars::ConfigSomaID, std::vector<unsigned int, std::allocator<unsigned int> > > > >::end() --> struct std::_Rb_tree_iterator<struct std::pair<const pystorm::bddriver::bdpars::ConfigSomaID, class std::vector<unsigned int, class std::allocator<unsigned int> > > >");
//		cl.def("end", (struct std::_Rb_tree_const_iterator<struct std::pair<const pystorm::bddriver::bdpars::ConfigSomaID, class std::vector<unsigned int, class std::allocator<unsigned int> > > > (std::map<pystorm::bddriver::bdpars::ConfigSomaID,std::vector<unsigned int, std::allocator<unsigned int> >,std::less<pystorm::bddriver::bdpars::ConfigSomaID>,std::allocator<std::pair<const pystorm::bddriver::bdpars::ConfigSomaID, std::vector<unsigned int, std::allocator<unsigned int> > > >>::*)() const) &std::map<pystorm::bddriver::bdpars::ConfigSomaID, std::vector<unsigned int, std::allocator<unsigned int> >, std::less<pystorm::bddriver::bdpars::ConfigSomaID>, std::allocator<std::pair<const pystorm::bddriver::bdpars::ConfigSomaID, std::vector<unsigned int, std::allocator<unsigned int> > > > >::end, "C++: std::map<pystorm::bddriver::bdpars::ConfigSomaID, std::vector<unsigned int, std::allocator<unsigned int> >, std::less<pystorm::bddriver::bdpars::ConfigSomaID>, std::allocator<std::pair<const pystorm::bddriver::bdpars::ConfigSomaID, std::vector<unsigned int, std::allocator<unsigned int> > > > >::end() const --> struct std::_Rb_tree_const_iterator<struct std::pair<const pystorm::bddriver::bdpars::ConfigSomaID, class std::vector<unsigned int, class std::allocator<unsigned int> > > >");
//		cl.def("cbegin", (struct std::_Rb_tree_const_iterator<struct std::pair<const pystorm::bddriver::bdpars::ConfigSomaID, class std::vector<unsigned int, class std::allocator<unsigned int> > > > (std::map<pystorm::bddriver::bdpars::ConfigSomaID,std::vector<unsigned int, std::allocator<unsigned int> >,std::less<pystorm::bddriver::bdpars::ConfigSomaID>,std::allocator<std::pair<const pystorm::bddriver::bdpars::ConfigSomaID, std::vector<unsigned int, std::allocator<unsigned int> > > >>::*)() const) &std::map<pystorm::bddriver::bdpars::ConfigSomaID, std::vector<unsigned int, std::allocator<unsigned int> >, std::less<pystorm::bddriver::bdpars::ConfigSomaID>, std::allocator<std::pair<const pystorm::bddriver::bdpars::ConfigSomaID, std::vector<unsigned int, std::allocator<unsigned int> > > > >::cbegin, "C++: std::map<pystorm::bddriver::bdpars::ConfigSomaID, std::vector<unsigned int, std::allocator<unsigned int> >, std::less<pystorm::bddriver::bdpars::ConfigSomaID>, std::allocator<std::pair<const pystorm::bddriver::bdpars::ConfigSomaID, std::vector<unsigned int, std::allocator<unsigned int> > > > >::cbegin() const --> struct std::_Rb_tree_const_iterator<struct std::pair<const pystorm::bddriver::bdpars::ConfigSomaID, class std::vector<unsigned int, class std::allocator<unsigned int> > > >");
//		cl.def("cend", (struct std::_Rb_tree_const_iterator<struct std::pair<const pystorm::bddriver::bdpars::ConfigSomaID, class std::vector<unsigned int, class std::allocator<unsigned int> > > > (std::map<pystorm::bddriver::bdpars::ConfigSomaID,std::vector<unsigned int, std::allocator<unsigned int> >,std::less<pystorm::bddriver::bdpars::ConfigSomaID>,std::allocator<std::pair<const pystorm::bddriver::bdpars::ConfigSomaID, std::vector<unsigned int, std::allocator<unsigned int> > > >>::*)() const) &std::map<pystorm::bddriver::bdpars::ConfigSomaID, std::vector<unsigned int, std::allocator<unsigned int> >, std::less<pystorm::bddriver::bdpars::ConfigSomaID>, std::allocator<std::pair<const pystorm::bddriver::bdpars::ConfigSomaID, std::vector<unsigned int, std::allocator<unsigned int> > > > >::cend, "C++: std::map<pystorm::bddriver::bdpars::ConfigSomaID, std::vector<unsigned int, std::allocator<unsigned int> >, std::less<pystorm::bddriver::bdpars::ConfigSomaID>, std::allocator<std::pair<const pystorm::bddriver::bdpars::ConfigSomaID, std::vector<unsigned int, std::allocator<unsigned int> > > > >::cend() const --> struct std::_Rb_tree_const_iterator<struct std::pair<const pystorm::bddriver::bdpars::ConfigSomaID, class std::vector<unsigned int, class std::allocator<unsigned int> > > >");
//		cl.def("empty", (bool (std::map<pystorm::bddriver::bdpars::ConfigSomaID,std::vector<unsigned int, std::allocator<unsigned int> >,std::less<pystorm::bddriver::bdpars::ConfigSomaID>,std::allocator<std::pair<const pystorm::bddriver::bdpars::ConfigSomaID, std::vector<unsigned int, std::allocator<unsigned int> > > >>::*)() const) &std::map<pystorm::bddriver::bdpars::ConfigSomaID, std::vector<unsigned int, std::allocator<unsigned int> >, std::less<pystorm::bddriver::bdpars::ConfigSomaID>, std::allocator<std::pair<const pystorm::bddriver::bdpars::ConfigSomaID, std::vector<unsigned int, std::allocator<unsigned int> > > > >::empty, "C++: std::map<pystorm::bddriver::bdpars::ConfigSomaID, std::vector<unsigned int, std::allocator<unsigned int> >, std::less<pystorm::bddriver::bdpars::ConfigSomaID>, std::allocator<std::pair<const pystorm::bddriver::bdpars::ConfigSomaID, std::vector<unsigned int, std::allocator<unsigned int> > > > >::empty() const --> bool");
//		cl.def("size", (unsigned long (std::map<pystorm::bddriver::bdpars::ConfigSomaID,std::vector<unsigned int, std::allocator<unsigned int> >,std::less<pystorm::bddriver::bdpars::ConfigSomaID>,std::allocator<std::pair<const pystorm::bddriver::bdpars::ConfigSomaID, std::vector<unsigned int, std::allocator<unsigned int> > > >>::*)() const) &std::map<pystorm::bddriver::bdpars::ConfigSomaID, std::vector<unsigned int, std::allocator<unsigned int> >, std::less<pystorm::bddriver::bdpars::ConfigSomaID>, std::allocator<std::pair<const pystorm::bddriver::bdpars::ConfigSomaID, std::vector<unsigned int, std::allocator<unsigned int> > > > >::size, "C++: std::map<pystorm::bddriver::bdpars::ConfigSomaID, std::vector<unsigned int, std::allocator<unsigned int> >, std::less<pystorm::bddriver::bdpars::ConfigSomaID>, std::allocator<std::pair<const pystorm::bddriver::bdpars::ConfigSomaID, std::vector<unsigned int, std::allocator<unsigned int> > > > >::size() const --> unsigned long");
//		cl.def("max_size", (unsigned long (std::map<pystorm::bddriver::bdpars::ConfigSomaID,std::vector<unsigned int, std::allocator<unsigned int> >,std::less<pystorm::bddriver::bdpars::ConfigSomaID>,std::allocator<std::pair<const pystorm::bddriver::bdpars::ConfigSomaID, std::vector<unsigned int, std::allocator<unsigned int> > > >>::*)() const) &std::map<pystorm::bddriver::bdpars::ConfigSomaID, std::vector<unsigned int, std::allocator<unsigned int> >, std::less<pystorm::bddriver::bdpars::ConfigSomaID>, std::allocator<std::pair<const pystorm::bddriver::bdpars::ConfigSomaID, std::vector<unsigned int, std::allocator<unsigned int> > > > >::max_size, "C++: std::map<pystorm::bddriver::bdpars::ConfigSomaID, std::vector<unsigned int, std::allocator<unsigned int> >, std::less<pystorm::bddriver::bdpars::ConfigSomaID>, std::allocator<std::pair<const pystorm::bddriver::bdpars::ConfigSomaID, std::vector<unsigned int, std::allocator<unsigned int> > > > >::max_size() const --> unsigned long");
//		cl.def("__getitem__", (class std::vector<unsigned int, class std::allocator<unsigned int> > & (std::map<pystorm::bddriver::bdpars::ConfigSomaID,std::vector<unsigned int, std::allocator<unsigned int> >,std::less<pystorm::bddriver::bdpars::ConfigSomaID>,std::allocator<std::pair<const pystorm::bddriver::bdpars::ConfigSomaID, std::vector<unsigned int, std::allocator<unsigned int> > > >>::*)(const pystorm::bddriver::bdpars::ConfigSomaID &)) &std::map<pystorm::bddriver::bdpars::ConfigSomaID, std::vector<unsigned int, std::allocator<unsigned int> >, std::less<pystorm::bddriver::bdpars::ConfigSomaID>, std::allocator<std::pair<const pystorm::bddriver::bdpars::ConfigSomaID, std::vector<unsigned int, std::allocator<unsigned int> > > > >::operator[], "C++: std::map<pystorm::bddriver::bdpars::ConfigSomaID, std::vector<unsigned int, std::allocator<unsigned int> >, std::less<pystorm::bddriver::bdpars::ConfigSomaID>, std::allocator<std::pair<const pystorm::bddriver::bdpars::ConfigSomaID, std::vector<unsigned int, std::allocator<unsigned int> > > > >::operator[](const pystorm::bddriver::bdpars::ConfigSomaID &) --> class std::vector<unsigned int, class std::allocator<unsigned int> > &", pybind11::return_value_policy::automatic, pybind11::arg("__k"));
//		cl.def("at", (class std::vector<unsigned int, class std::allocator<unsigned int> > & (std::map<pystorm::bddriver::bdpars::ConfigSomaID,std::vector<unsigned int, std::allocator<unsigned int> >,std::less<pystorm::bddriver::bdpars::ConfigSomaID>,std::allocator<std::pair<const pystorm::bddriver::bdpars::ConfigSomaID, std::vector<unsigned int, std::allocator<unsigned int> > > >>::*)(const pystorm::bddriver::bdpars::ConfigSomaID &)) &std::map<pystorm::bddriver::bdpars::ConfigSomaID, std::vector<unsigned int, std::allocator<unsigned int> >, std::less<pystorm::bddriver::bdpars::ConfigSomaID>, std::allocator<std::pair<const pystorm::bddriver::bdpars::ConfigSomaID, std::vector<unsigned int, std::allocator<unsigned int> > > > >::at, "C++: std::map<pystorm::bddriver::bdpars::ConfigSomaID, std::vector<unsigned int, std::allocator<unsigned int> >, std::less<pystorm::bddriver::bdpars::ConfigSomaID>, std::allocator<std::pair<const pystorm::bddriver::bdpars::ConfigSomaID, std::vector<unsigned int, std::allocator<unsigned int> > > > >::at(const pystorm::bddriver::bdpars::ConfigSomaID &) --> class std::vector<unsigned int, class std::allocator<unsigned int> > &", pybind11::return_value_policy::automatic, pybind11::arg("__k"));
//		cl.def("at", (const class std::vector<unsigned int, class std::allocator<unsigned int> > & (std::map<pystorm::bddriver::bdpars::ConfigSomaID,std::vector<unsigned int, std::allocator<unsigned int> >,std::less<pystorm::bddriver::bdpars::ConfigSomaID>,std::allocator<std::pair<const pystorm::bddriver::bdpars::ConfigSomaID, std::vector<unsigned int, std::allocator<unsigned int> > > >>::*)(const pystorm::bddriver::bdpars::ConfigSomaID &) const) &std::map<pystorm::bddriver::bdpars::ConfigSomaID, std::vector<unsigned int, std::allocator<unsigned int> >, std::less<pystorm::bddriver::bdpars::ConfigSomaID>, std::allocator<std::pair<const pystorm::bddriver::bdpars::ConfigSomaID, std::vector<unsigned int, std::allocator<unsigned int> > > > >::at, "C++: std::map<pystorm::bddriver::bdpars::ConfigSomaID, std::vector<unsigned int, std::allocator<unsigned int> >, std::less<pystorm::bddriver::bdpars::ConfigSomaID>, std::allocator<std::pair<const pystorm::bddriver::bdpars::ConfigSomaID, std::vector<unsigned int, std::allocator<unsigned int> > > > >::at(const pystorm::bddriver::bdpars::ConfigSomaID &) const --> const class std::vector<unsigned int, class std::allocator<unsigned int> > &", pybind11::return_value_policy::automatic, pybind11::arg("__k"));
//		cl.def("insert", (struct std::pair<struct std::_Rb_tree_iterator<struct std::pair<const pystorm::bddriver::bdpars::ConfigSomaID, class std::vector<unsigned int, class std::allocator<unsigned int> > > >, bool> (std::map<pystorm::bddriver::bdpars::ConfigSomaID,std::vector<unsigned int, std::allocator<unsigned int> >,std::less<pystorm::bddriver::bdpars::ConfigSomaID>,std::allocator<std::pair<const pystorm::bddriver::bdpars::ConfigSomaID, std::vector<unsigned int, std::allocator<unsigned int> > > >>::*)(const struct std::pair<const pystorm::bddriver::bdpars::ConfigSomaID, class std::vector<unsigned int, class std::allocator<unsigned int> > > &)) &std::map<pystorm::bddriver::bdpars::ConfigSomaID, std::vector<unsigned int, std::allocator<unsigned int> >, std::less<pystorm::bddriver::bdpars::ConfigSomaID>, std::allocator<std::pair<const pystorm::bddriver::bdpars::ConfigSomaID, std::vector<unsigned int, std::allocator<unsigned int> > > > >::insert, "C++: std::map<pystorm::bddriver::bdpars::ConfigSomaID, std::vector<unsigned int, std::allocator<unsigned int> >, std::less<pystorm::bddriver::bdpars::ConfigSomaID>, std::allocator<std::pair<const pystorm::bddriver::bdpars::ConfigSomaID, std::vector<unsigned int, std::allocator<unsigned int> > > > >::insert(const struct std::pair<const pystorm::bddriver::bdpars::ConfigSomaID, class std::vector<unsigned int, class std::allocator<unsigned int> > > &) --> struct std::pair<struct std::_Rb_tree_iterator<struct std::pair<const pystorm::bddriver::bdpars::ConfigSomaID, class std::vector<unsigned int, class std::allocator<unsigned int> > > >, bool>", pybind11::arg("__x"));
//		cl.def("insert", (void (std::map<pystorm::bddriver::bdpars::ConfigSomaID,std::vector<unsigned int, std::allocator<unsigned int> >,std::less<pystorm::bddriver::bdpars::ConfigSomaID>,std::allocator<std::pair<const pystorm::bddriver::bdpars::ConfigSomaID, std::vector<unsigned int, std::allocator<unsigned int> > > >>::*)(class std::initializer_list<struct std::pair<const pystorm::bddriver::bdpars::ConfigSomaID, class std::vector<unsigned int, class std::allocator<unsigned int> > > >)) &std::map<pystorm::bddriver::bdpars::ConfigSomaID, std::vector<unsigned int, std::allocator<unsigned int> >, std::less<pystorm::bddriver::bdpars::ConfigSomaID>, std::allocator<std::pair<const pystorm::bddriver::bdpars::ConfigSomaID, std::vector<unsigned int, std::allocator<unsigned int> > > > >::insert, "C++: std::map<pystorm::bddriver::bdpars::ConfigSomaID, std::vector<unsigned int, std::allocator<unsigned int> >, std::less<pystorm::bddriver::bdpars::ConfigSomaID>, std::allocator<std::pair<const pystorm::bddriver::bdpars::ConfigSomaID, std::vector<unsigned int, std::allocator<unsigned int> > > > >::insert(class std::initializer_list<struct std::pair<const pystorm::bddriver::bdpars::ConfigSomaID, class std::vector<unsigned int, class std::allocator<unsigned int> > > >) --> void", pybind11::arg("__list"));
//		cl.def("insert", (struct std::_Rb_tree_iterator<struct std::pair<const pystorm::bddriver::bdpars::ConfigSomaID, class std::vector<unsigned int, class std::allocator<unsigned int> > > > (std::map<pystorm::bddriver::bdpars::ConfigSomaID,std::vector<unsigned int, std::allocator<unsigned int> >,std::less<pystorm::bddriver::bdpars::ConfigSomaID>,std::allocator<std::pair<const pystorm::bddriver::bdpars::ConfigSomaID, std::vector<unsigned int, std::allocator<unsigned int> > > >>::*)(struct std::_Rb_tree_const_iterator<struct std::pair<const pystorm::bddriver::bdpars::ConfigSomaID, class std::vector<unsigned int, class std::allocator<unsigned int> > > >, const struct std::pair<const pystorm::bddriver::bdpars::ConfigSomaID, class std::vector<unsigned int, class std::allocator<unsigned int> > > &)) &std::map<pystorm::bddriver::bdpars::ConfigSomaID, std::vector<unsigned int, std::allocator<unsigned int> >, std::less<pystorm::bddriver::bdpars::ConfigSomaID>, std::allocator<std::pair<const pystorm::bddriver::bdpars::ConfigSomaID, std::vector<unsigned int, std::allocator<unsigned int> > > > >::insert, "C++: std::map<pystorm::bddriver::bdpars::ConfigSomaID, std::vector<unsigned int, std::allocator<unsigned int> >, std::less<pystorm::bddriver::bdpars::ConfigSomaID>, std::allocator<std::pair<const pystorm::bddriver::bdpars::ConfigSomaID, std::vector<unsigned int, std::allocator<unsigned int> > > > >::insert(struct std::_Rb_tree_const_iterator<struct std::pair<const pystorm::bddriver::bdpars::ConfigSomaID, class std::vector<unsigned int, class std::allocator<unsigned int> > > >, const struct std::pair<const pystorm::bddriver::bdpars::ConfigSomaID, class std::vector<unsigned int, class std::allocator<unsigned int> > > &) --> struct std::_Rb_tree_iterator<struct std::pair<const pystorm::bddriver::bdpars::ConfigSomaID, class std::vector<unsigned int, class std::allocator<unsigned int> > > >", pybind11::arg("__position"), pybind11::arg("__x"));
//		cl.def("erase", (struct std::_Rb_tree_iterator<struct std::pair<const pystorm::bddriver::bdpars::ConfigSomaID, class std::vector<unsigned int, class std::allocator<unsigned int> > > > (std::map<pystorm::bddriver::bdpars::ConfigSomaID,std::vector<unsigned int, std::allocator<unsigned int> >,std::less<pystorm::bddriver::bdpars::ConfigSomaID>,std::allocator<std::pair<const pystorm::bddriver::bdpars::ConfigSomaID, std::vector<unsigned int, std::allocator<unsigned int> > > >>::*)(struct std::_Rb_tree_const_iterator<struct std::pair<const pystorm::bddriver::bdpars::ConfigSomaID, class std::vector<unsigned int, class std::allocator<unsigned int> > > >)) &std::map<pystorm::bddriver::bdpars::ConfigSomaID, std::vector<unsigned int, std::allocator<unsigned int> >, std::less<pystorm::bddriver::bdpars::ConfigSomaID>, std::allocator<std::pair<const pystorm::bddriver::bdpars::ConfigSomaID, std::vector<unsigned int, std::allocator<unsigned int> > > > >::erase, "C++: std::map<pystorm::bddriver::bdpars::ConfigSomaID, std::vector<unsigned int, std::allocator<unsigned int> >, std::less<pystorm::bddriver::bdpars::ConfigSomaID>, std::allocator<std::pair<const pystorm::bddriver::bdpars::ConfigSomaID, std::vector<unsigned int, std::allocator<unsigned int> > > > >::erase(struct std::_Rb_tree_const_iterator<struct std::pair<const pystorm::bddriver::bdpars::ConfigSomaID, class std::vector<unsigned int, class std::allocator<unsigned int> > > >) --> struct std::_Rb_tree_iterator<struct std::pair<const pystorm::bddriver::bdpars::ConfigSomaID, class std::vector<unsigned int, class std::allocator<unsigned int> > > >", pybind11::arg("__position"));
//		cl.def("erase", (struct std::_Rb_tree_iterator<struct std::pair<const pystorm::bddriver::bdpars::ConfigSomaID, class std::vector<unsigned int, class std::allocator<unsigned int> > > > (std::map<pystorm::bddriver::bdpars::ConfigSomaID,std::vector<unsigned int, std::allocator<unsigned int> >,std::less<pystorm::bddriver::bdpars::ConfigSomaID>,std::allocator<std::pair<const pystorm::bddriver::bdpars::ConfigSomaID, std::vector<unsigned int, std::allocator<unsigned int> > > >>::*)(struct std::_Rb_tree_iterator<struct std::pair<const pystorm::bddriver::bdpars::ConfigSomaID, class std::vector<unsigned int, class std::allocator<unsigned int> > > >)) &std::map<pystorm::bddriver::bdpars::ConfigSomaID, std::vector<unsigned int, std::allocator<unsigned int> >, std::less<pystorm::bddriver::bdpars::ConfigSomaID>, std::allocator<std::pair<const pystorm::bddriver::bdpars::ConfigSomaID, std::vector<unsigned int, std::allocator<unsigned int> > > > >::erase, "C++: std::map<pystorm::bddriver::bdpars::ConfigSomaID, std::vector<unsigned int, std::allocator<unsigned int> >, std::less<pystorm::bddriver::bdpars::ConfigSomaID>, std::allocator<std::pair<const pystorm::bddriver::bdpars::ConfigSomaID, std::vector<unsigned int, std::allocator<unsigned int> > > > >::erase(struct std::_Rb_tree_iterator<struct std::pair<const pystorm::bddriver::bdpars::ConfigSomaID, class std::vector<unsigned int, class std::allocator<unsigned int> > > >) --> struct std::_Rb_tree_iterator<struct std::pair<const pystorm::bddriver::bdpars::ConfigSomaID, class std::vector<unsigned int, class std::allocator<unsigned int> > > >", pybind11::arg("__position"));
//		cl.def("erase", (unsigned long (std::map<pystorm::bddriver::bdpars::ConfigSomaID,std::vector<unsigned int, std::allocator<unsigned int> >,std::less<pystorm::bddriver::bdpars::ConfigSomaID>,std::allocator<std::pair<const pystorm::bddriver::bdpars::ConfigSomaID, std::vector<unsigned int, std::allocator<unsigned int> > > >>::*)(const pystorm::bddriver::bdpars::ConfigSomaID &)) &std::map<pystorm::bddriver::bdpars::ConfigSomaID, std::vector<unsigned int, std::allocator<unsigned int> >, std::less<pystorm::bddriver::bdpars::ConfigSomaID>, std::allocator<std::pair<const pystorm::bddriver::bdpars::ConfigSomaID, std::vector<unsigned int, std::allocator<unsigned int> > > > >::erase, "C++: std::map<pystorm::bddriver::bdpars::ConfigSomaID, std::vector<unsigned int, std::allocator<unsigned int> >, std::less<pystorm::bddriver::bdpars::ConfigSomaID>, std::allocator<std::pair<const pystorm::bddriver::bdpars::ConfigSomaID, std::vector<unsigned int, std::allocator<unsigned int> > > > >::erase(const pystorm::bddriver::bdpars::ConfigSomaID &) --> unsigned long", pybind11::arg("__x"));
//		cl.def("erase", (struct std::_Rb_tree_iterator<struct std::pair<const pystorm::bddriver::bdpars::ConfigSomaID, class std::vector<unsigned int, class std::allocator<unsigned int> > > > (std::map<pystorm::bddriver::bdpars::ConfigSomaID,std::vector<unsigned int, std::allocator<unsigned int> >,std::less<pystorm::bddriver::bdpars::ConfigSomaID>,std::allocator<std::pair<const pystorm::bddriver::bdpars::ConfigSomaID, std::vector<unsigned int, std::allocator<unsigned int> > > >>::*)(struct std::_Rb_tree_const_iterator<struct std::pair<const pystorm::bddriver::bdpars::ConfigSomaID, class std::vector<unsigned int, class std::allocator<unsigned int> > > >, struct std::_Rb_tree_const_iterator<struct std::pair<const pystorm::bddriver::bdpars::ConfigSomaID, class std::vector<unsigned int, class std::allocator<unsigned int> > > >)) &std::map<pystorm::bddriver::bdpars::ConfigSomaID, std::vector<unsigned int, std::allocator<unsigned int> >, std::less<pystorm::bddriver::bdpars::ConfigSomaID>, std::allocator<std::pair<const pystorm::bddriver::bdpars::ConfigSomaID, std::vector<unsigned int, std::allocator<unsigned int> > > > >::erase, "C++: std::map<pystorm::bddriver::bdpars::ConfigSomaID, std::vector<unsigned int, std::allocator<unsigned int> >, std::less<pystorm::bddriver::bdpars::ConfigSomaID>, std::allocator<std::pair<const pystorm::bddriver::bdpars::ConfigSomaID, std::vector<unsigned int, std::allocator<unsigned int> > > > >::erase(struct std::_Rb_tree_const_iterator<struct std::pair<const pystorm::bddriver::bdpars::ConfigSomaID, class std::vector<unsigned int, class std::allocator<unsigned int> > > >, struct std::_Rb_tree_const_iterator<struct std::pair<const pystorm::bddriver::bdpars::ConfigSomaID, class std::vector<unsigned int, class std::allocator<unsigned int> > > >) --> struct std::_Rb_tree_iterator<struct std::pair<const pystorm::bddriver::bdpars::ConfigSomaID, class std::vector<unsigned int, class std::allocator<unsigned int> > > >", pybind11::arg("__first"), pybind11::arg("__last"));
//		cl.def("swap", (void (std::map<pystorm::bddriver::bdpars::ConfigSomaID,std::vector<unsigned int, std::allocator<unsigned int> >,std::less<pystorm::bddriver::bdpars::ConfigSomaID>,std::allocator<std::pair<const pystorm::bddriver::bdpars::ConfigSomaID, std::vector<unsigned int, std::allocator<unsigned int> > > >>::*)(class std::map<pystorm::bddriver::bdpars::ConfigSomaID, class std::vector<unsigned int, class std::allocator<unsigned int> >, struct std::less<pystorm::bddriver::bdpars::ConfigSomaID>, class std::allocator<struct std::pair<const pystorm::bddriver::bdpars::ConfigSomaID, class std::vector<unsigned int, class std::allocator<unsigned int> > > > > &)) &std::map<pystorm::bddriver::bdpars::ConfigSomaID, std::vector<unsigned int, std::allocator<unsigned int> >, std::less<pystorm::bddriver::bdpars::ConfigSomaID>, std::allocator<std::pair<const pystorm::bddriver::bdpars::ConfigSomaID, std::vector<unsigned int, std::allocator<unsigned int> > > > >::swap, "C++: std::map<pystorm::bddriver::bdpars::ConfigSomaID, std::vector<unsigned int, std::allocator<unsigned int> >, std::less<pystorm::bddriver::bdpars::ConfigSomaID>, std::allocator<std::pair<const pystorm::bddriver::bdpars::ConfigSomaID, std::vector<unsigned int, std::allocator<unsigned int> > > > >::swap(class std::map<pystorm::bddriver::bdpars::ConfigSomaID, class std::vector<unsigned int, class std::allocator<unsigned int> >, struct std::less<pystorm::bddriver::bdpars::ConfigSomaID>, class std::allocator<struct std::pair<const pystorm::bddriver::bdpars::ConfigSomaID, class std::vector<unsigned int, class std::allocator<unsigned int> > > > > &) --> void", pybind11::arg("__x"));
//		cl.def("clear", (void (std::map<pystorm::bddriver::bdpars::ConfigSomaID,std::vector<unsigned int, std::allocator<unsigned int> >,std::less<pystorm::bddriver::bdpars::ConfigSomaID>,std::allocator<std::pair<const pystorm::bddriver::bdpars::ConfigSomaID, std::vector<unsigned int, std::allocator<unsigned int> > > >>::*)()) &std::map<pystorm::bddriver::bdpars::ConfigSomaID, std::vector<unsigned int, std::allocator<unsigned int> >, std::less<pystorm::bddriver::bdpars::ConfigSomaID>, std::allocator<std::pair<const pystorm::bddriver::bdpars::ConfigSomaID, std::vector<unsigned int, std::allocator<unsigned int> > > > >::clear, "C++: std::map<pystorm::bddriver::bdpars::ConfigSomaID, std::vector<unsigned int, std::allocator<unsigned int> >, std::less<pystorm::bddriver::bdpars::ConfigSomaID>, std::allocator<std::pair<const pystorm::bddriver::bdpars::ConfigSomaID, std::vector<unsigned int, std::allocator<unsigned int> > > > >::clear() --> void");
//		cl.def("key_comp", (struct std::less<pystorm::bddriver::bdpars::ConfigSomaID> (std::map<pystorm::bddriver::bdpars::ConfigSomaID,std::vector<unsigned int, std::allocator<unsigned int> >,std::less<pystorm::bddriver::bdpars::ConfigSomaID>,std::allocator<std::pair<const pystorm::bddriver::bdpars::ConfigSomaID, std::vector<unsigned int, std::allocator<unsigned int> > > >>::*)() const) &std::map<pystorm::bddriver::bdpars::ConfigSomaID, std::vector<unsigned int, std::allocator<unsigned int> >, std::less<pystorm::bddriver::bdpars::ConfigSomaID>, std::allocator<std::pair<const pystorm::bddriver::bdpars::ConfigSomaID, std::vector<unsigned int, std::allocator<unsigned int> > > > >::key_comp, "C++: std::map<pystorm::bddriver::bdpars::ConfigSomaID, std::vector<unsigned int, std::allocator<unsigned int> >, std::less<pystorm::bddriver::bdpars::ConfigSomaID>, std::allocator<std::pair<const pystorm::bddriver::bdpars::ConfigSomaID, std::vector<unsigned int, std::allocator<unsigned int> > > > >::key_comp() const --> struct std::less<pystorm::bddriver::bdpars::ConfigSomaID>");
//		cl.def("find", (struct std::_Rb_tree_iterator<struct std::pair<const pystorm::bddriver::bdpars::ConfigSomaID, class std::vector<unsigned int, class std::allocator<unsigned int> > > > (std::map<pystorm::bddriver::bdpars::ConfigSomaID,std::vector<unsigned int, std::allocator<unsigned int> >,std::less<pystorm::bddriver::bdpars::ConfigSomaID>,std::allocator<std::pair<const pystorm::bddriver::bdpars::ConfigSomaID, std::vector<unsigned int, std::allocator<unsigned int> > > >>::*)(const pystorm::bddriver::bdpars::ConfigSomaID &)) &std::map<pystorm::bddriver::bdpars::ConfigSomaID, std::vector<unsigned int, std::allocator<unsigned int> >, std::less<pystorm::bddriver::bdpars::ConfigSomaID>, std::allocator<std::pair<const pystorm::bddriver::bdpars::ConfigSomaID, std::vector<unsigned int, std::allocator<unsigned int> > > > >::find, "C++: std::map<pystorm::bddriver::bdpars::ConfigSomaID, std::vector<unsigned int, std::allocator<unsigned int> >, std::less<pystorm::bddriver::bdpars::ConfigSomaID>, std::allocator<std::pair<const pystorm::bddriver::bdpars::ConfigSomaID, std::vector<unsigned int, std::allocator<unsigned int> > > > >::find(const pystorm::bddriver::bdpars::ConfigSomaID &) --> struct std::_Rb_tree_iterator<struct std::pair<const pystorm::bddriver::bdpars::ConfigSomaID, class std::vector<unsigned int, class std::allocator<unsigned int> > > >", pybind11::arg("__x"));
//		cl.def("find", (struct std::_Rb_tree_const_iterator<struct std::pair<const pystorm::bddriver::bdpars::ConfigSomaID, class std::vector<unsigned int, class std::allocator<unsigned int> > > > (std::map<pystorm::bddriver::bdpars::ConfigSomaID,std::vector<unsigned int, std::allocator<unsigned int> >,std::less<pystorm::bddriver::bdpars::ConfigSomaID>,std::allocator<std::pair<const pystorm::bddriver::bdpars::ConfigSomaID, std::vector<unsigned int, std::allocator<unsigned int> > > >>::*)(const pystorm::bddriver::bdpars::ConfigSomaID &) const) &std::map<pystorm::bddriver::bdpars::ConfigSomaID, std::vector<unsigned int, std::allocator<unsigned int> >, std::less<pystorm::bddriver::bdpars::ConfigSomaID>, std::allocator<std::pair<const pystorm::bddriver::bdpars::ConfigSomaID, std::vector<unsigned int, std::allocator<unsigned int> > > > >::find, "C++: std::map<pystorm::bddriver::bdpars::ConfigSomaID, std::vector<unsigned int, std::allocator<unsigned int> >, std::less<pystorm::bddriver::bdpars::ConfigSomaID>, std::allocator<std::pair<const pystorm::bddriver::bdpars::ConfigSomaID, std::vector<unsigned int, std::allocator<unsigned int> > > > >::find(const pystorm::bddriver::bdpars::ConfigSomaID &) const --> struct std::_Rb_tree_const_iterator<struct std::pair<const pystorm::bddriver::bdpars::ConfigSomaID, class std::vector<unsigned int, class std::allocator<unsigned int> > > >", pybind11::arg("__x"));
//		cl.def("count", (unsigned long (std::map<pystorm::bddriver::bdpars::ConfigSomaID,std::vector<unsigned int, std::allocator<unsigned int> >,std::less<pystorm::bddriver::bdpars::ConfigSomaID>,std::allocator<std::pair<const pystorm::bddriver::bdpars::ConfigSomaID, std::vector<unsigned int, std::allocator<unsigned int> > > >>::*)(const pystorm::bddriver::bdpars::ConfigSomaID &) const) &std::map<pystorm::bddriver::bdpars::ConfigSomaID, std::vector<unsigned int, std::allocator<unsigned int> >, std::less<pystorm::bddriver::bdpars::ConfigSomaID>, std::allocator<std::pair<const pystorm::bddriver::bdpars::ConfigSomaID, std::vector<unsigned int, std::allocator<unsigned int> > > > >::count, "C++: std::map<pystorm::bddriver::bdpars::ConfigSomaID, std::vector<unsigned int, std::allocator<unsigned int> >, std::less<pystorm::bddriver::bdpars::ConfigSomaID>, std::allocator<std::pair<const pystorm::bddriver::bdpars::ConfigSomaID, std::vector<unsigned int, std::allocator<unsigned int> > > > >::count(const pystorm::bddriver::bdpars::ConfigSomaID &) const --> unsigned long", pybind11::arg("__x"));
//		cl.def("lower_bound", (struct std::_Rb_tree_iterator<struct std::pair<const pystorm::bddriver::bdpars::ConfigSomaID, class std::vector<unsigned int, class std::allocator<unsigned int> > > > (std::map<pystorm::bddriver::bdpars::ConfigSomaID,std::vector<unsigned int, std::allocator<unsigned int> >,std::less<pystorm::bddriver::bdpars::ConfigSomaID>,std::allocator<std::pair<const pystorm::bddriver::bdpars::ConfigSomaID, std::vector<unsigned int, std::allocator<unsigned int> > > >>::*)(const pystorm::bddriver::bdpars::ConfigSomaID &)) &std::map<pystorm::bddriver::bdpars::ConfigSomaID, std::vector<unsigned int, std::allocator<unsigned int> >, std::less<pystorm::bddriver::bdpars::ConfigSomaID>, std::allocator<std::pair<const pystorm::bddriver::bdpars::ConfigSomaID, std::vector<unsigned int, std::allocator<unsigned int> > > > >::lower_bound, "C++: std::map<pystorm::bddriver::bdpars::ConfigSomaID, std::vector<unsigned int, std::allocator<unsigned int> >, std::less<pystorm::bddriver::bdpars::ConfigSomaID>, std::allocator<std::pair<const pystorm::bddriver::bdpars::ConfigSomaID, std::vector<unsigned int, std::allocator<unsigned int> > > > >::lower_bound(const pystorm::bddriver::bdpars::ConfigSomaID &) --> struct std::_Rb_tree_iterator<struct std::pair<const pystorm::bddriver::bdpars::ConfigSomaID, class std::vector<unsigned int, class std::allocator<unsigned int> > > >", pybind11::arg("__x"));
//		cl.def("lower_bound", (struct std::_Rb_tree_const_iterator<struct std::pair<const pystorm::bddriver::bdpars::ConfigSomaID, class std::vector<unsigned int, class std::allocator<unsigned int> > > > (std::map<pystorm::bddriver::bdpars::ConfigSomaID,std::vector<unsigned int, std::allocator<unsigned int> >,std::less<pystorm::bddriver::bdpars::ConfigSomaID>,std::allocator<std::pair<const pystorm::bddriver::bdpars::ConfigSomaID, std::vector<unsigned int, std::allocator<unsigned int> > > >>::*)(const pystorm::bddriver::bdpars::ConfigSomaID &) const) &std::map<pystorm::bddriver::bdpars::ConfigSomaID, std::vector<unsigned int, std::allocator<unsigned int> >, std::less<pystorm::bddriver::bdpars::ConfigSomaID>, std::allocator<std::pair<const pystorm::bddriver::bdpars::ConfigSomaID, std::vector<unsigned int, std::allocator<unsigned int> > > > >::lower_bound, "C++: std::map<pystorm::bddriver::bdpars::ConfigSomaID, std::vector<unsigned int, std::allocator<unsigned int> >, std::less<pystorm::bddriver::bdpars::ConfigSomaID>, std::allocator<std::pair<const pystorm::bddriver::bdpars::ConfigSomaID, std::vector<unsigned int, std::allocator<unsigned int> > > > >::lower_bound(const pystorm::bddriver::bdpars::ConfigSomaID &) const --> struct std::_Rb_tree_const_iterator<struct std::pair<const pystorm::bddriver::bdpars::ConfigSomaID, class std::vector<unsigned int, class std::allocator<unsigned int> > > >", pybind11::arg("__x"));
//		cl.def("upper_bound", (struct std::_Rb_tree_iterator<struct std::pair<const pystorm::bddriver::bdpars::ConfigSomaID, class std::vector<unsigned int, class std::allocator<unsigned int> > > > (std::map<pystorm::bddriver::bdpars::ConfigSomaID,std::vector<unsigned int, std::allocator<unsigned int> >,std::less<pystorm::bddriver::bdpars::ConfigSomaID>,std::allocator<std::pair<const pystorm::bddriver::bdpars::ConfigSomaID, std::vector<unsigned int, std::allocator<unsigned int> > > >>::*)(const pystorm::bddriver::bdpars::ConfigSomaID &)) &std::map<pystorm::bddriver::bdpars::ConfigSomaID, std::vector<unsigned int, std::allocator<unsigned int> >, std::less<pystorm::bddriver::bdpars::ConfigSomaID>, std::allocator<std::pair<const pystorm::bddriver::bdpars::ConfigSomaID, std::vector<unsigned int, std::allocator<unsigned int> > > > >::upper_bound, "C++: std::map<pystorm::bddriver::bdpars::ConfigSomaID, std::vector<unsigned int, std::allocator<unsigned int> >, std::less<pystorm::bddriver::bdpars::ConfigSomaID>, std::allocator<std::pair<const pystorm::bddriver::bdpars::ConfigSomaID, std::vector<unsigned int, std::allocator<unsigned int> > > > >::upper_bound(const pystorm::bddriver::bdpars::ConfigSomaID &) --> struct std::_Rb_tree_iterator<struct std::pair<const pystorm::bddriver::bdpars::ConfigSomaID, class std::vector<unsigned int, class std::allocator<unsigned int> > > >", pybind11::arg("__x"));
//		cl.def("upper_bound", (struct std::_Rb_tree_const_iterator<struct std::pair<const pystorm::bddriver::bdpars::ConfigSomaID, class std::vector<unsigned int, class std::allocator<unsigned int> > > > (std::map<pystorm::bddriver::bdpars::ConfigSomaID,std::vector<unsigned int, std::allocator<unsigned int> >,std::less<pystorm::bddriver::bdpars::ConfigSomaID>,std::allocator<std::pair<const pystorm::bddriver::bdpars::ConfigSomaID, std::vector<unsigned int, std::allocator<unsigned int> > > >>::*)(const pystorm::bddriver::bdpars::ConfigSomaID &) const) &std::map<pystorm::bddriver::bdpars::ConfigSomaID, std::vector<unsigned int, std::allocator<unsigned int> >, std::less<pystorm::bddriver::bdpars::ConfigSomaID>, std::allocator<std::pair<const pystorm::bddriver::bdpars::ConfigSomaID, std::vector<unsigned int, std::allocator<unsigned int> > > > >::upper_bound, "C++: std::map<pystorm::bddriver::bdpars::ConfigSomaID, std::vector<unsigned int, std::allocator<unsigned int> >, std::less<pystorm::bddriver::bdpars::ConfigSomaID>, std::allocator<std::pair<const pystorm::bddriver::bdpars::ConfigSomaID, std::vector<unsigned int, std::allocator<unsigned int> > > > >::upper_bound(const pystorm::bddriver::bdpars::ConfigSomaID &) const --> struct std::_Rb_tree_const_iterator<struct std::pair<const pystorm::bddriver::bdpars::ConfigSomaID, class std::vector<unsigned int, class std::allocator<unsigned int> > > >", pybind11::arg("__x"));
//		cl.def("equal_range", (struct std::pair<struct std::_Rb_tree_iterator<struct std::pair<const pystorm::bddriver::bdpars::ConfigSomaID, class std::vector<unsigned int, class std::allocator<unsigned int> > > >, struct std::_Rb_tree_iterator<struct std::pair<const pystorm::bddriver::bdpars::ConfigSomaID, class std::vector<unsigned int, class std::allocator<unsigned int> > > > > (std::map<pystorm::bddriver::bdpars::ConfigSomaID,std::vector<unsigned int, std::allocator<unsigned int> >,std::less<pystorm::bddriver::bdpars::ConfigSomaID>,std::allocator<std::pair<const pystorm::bddriver::bdpars::ConfigSomaID, std::vector<unsigned int, std::allocator<unsigned int> > > >>::*)(const pystorm::bddriver::bdpars::ConfigSomaID &)) &std::map<pystorm::bddriver::bdpars::ConfigSomaID, std::vector<unsigned int, std::allocator<unsigned int> >, std::less<pystorm::bddriver::bdpars::ConfigSomaID>, std::allocator<std::pair<const pystorm::bddriver::bdpars::ConfigSomaID, std::vector<unsigned int, std::allocator<unsigned int> > > > >::equal_range, "C++: std::map<pystorm::bddriver::bdpars::ConfigSomaID, std::vector<unsigned int, std::allocator<unsigned int> >, std::less<pystorm::bddriver::bdpars::ConfigSomaID>, std::allocator<std::pair<const pystorm::bddriver::bdpars::ConfigSomaID, std::vector<unsigned int, std::allocator<unsigned int> > > > >::equal_range(const pystorm::bddriver::bdpars::ConfigSomaID &) --> struct std::pair<struct std::_Rb_tree_iterator<struct std::pair<const pystorm::bddriver::bdpars::ConfigSomaID, class std::vector<unsigned int, class std::allocator<unsigned int> > > >, struct std::_Rb_tree_iterator<struct std::pair<const pystorm::bddriver::bdpars::ConfigSomaID, class std::vector<unsigned int, class std::allocator<unsigned int> > > > >", pybind11::arg("__x"));
//		cl.def("equal_range", (struct std::pair<struct std::_Rb_tree_const_iterator<struct std::pair<const pystorm::bddriver::bdpars::ConfigSomaID, class std::vector<unsigned int, class std::allocator<unsigned int> > > >, struct std::_Rb_tree_const_iterator<struct std::pair<const pystorm::bddriver::bdpars::ConfigSomaID, class std::vector<unsigned int, class std::allocator<unsigned int> > > > > (std::map<pystorm::bddriver::bdpars::ConfigSomaID,std::vector<unsigned int, std::allocator<unsigned int> >,std::less<pystorm::bddriver::bdpars::ConfigSomaID>,std::allocator<std::pair<const pystorm::bddriver::bdpars::ConfigSomaID, std::vector<unsigned int, std::allocator<unsigned int> > > >>::*)(const pystorm::bddriver::bdpars::ConfigSomaID &) const) &std::map<pystorm::bddriver::bdpars::ConfigSomaID, std::vector<unsigned int, std::allocator<unsigned int> >, std::less<pystorm::bddriver::bdpars::ConfigSomaID>, std::allocator<std::pair<const pystorm::bddriver::bdpars::ConfigSomaID, std::vector<unsigned int, std::allocator<unsigned int> > > > >::equal_range, "C++: std::map<pystorm::bddriver::bdpars::ConfigSomaID, std::vector<unsigned int, std::allocator<unsigned int> >, std::less<pystorm::bddriver::bdpars::ConfigSomaID>, std::allocator<std::pair<const pystorm::bddriver::bdpars::ConfigSomaID, std::vector<unsigned int, std::allocator<unsigned int> > > > >::equal_range(const pystorm::bddriver::bdpars::ConfigSomaID &) const --> struct std::pair<struct std::_Rb_tree_const_iterator<struct std::pair<const pystorm::bddriver::bdpars::ConfigSomaID, class std::vector<unsigned int, class std::allocator<unsigned int> > > >, struct std::_Rb_tree_const_iterator<struct std::pair<const pystorm::bddriver::bdpars::ConfigSomaID, class std::vector<unsigned int, class std::allocator<unsigned int> > > > >", pybind11::arg("__x"));
//	}
//	{ // std::map file:bits/stl_map.h line:96
//		pybind11::class_<std::map<pystorm::bddriver::bdpars::ConfigSynapseID,std::vector<unsigned int, std::allocator<unsigned int> >,std::less<pystorm::bddriver::bdpars::ConfigSynapseID>,std::allocator<std::pair<const pystorm::bddriver::bdpars::ConfigSynapseID, std::vector<unsigned int, std::allocator<unsigned int> > > >>, std::shared_ptr<std::map<pystorm::bddriver::bdpars::ConfigSynapseID,std::vector<unsigned int, std::allocator<unsigned int> >,std::less<pystorm::bddriver::bdpars::ConfigSynapseID>,std::allocator<std::pair<const pystorm::bddriver::bdpars::ConfigSynapseID, std::vector<unsigned int, std::allocator<unsigned int> > > >>>> cl(M("std"), "map_pystorm_bddriver_bdpars_ConfigSynapseID_std_vector_unsigned_int_std_allocator_unsigned_int_std_less_pystorm_bddriver_bdpars_ConfigSynapseID_std_allocator_std_pair_const_pystorm_bddriver_bdpars_ConfigSynapseID_std_vector_unsigned_int_std_allocator_unsigned_int_t", "");
//		pybind11::handle cl_type = cl;

//		cl.def(pybind11::init<>());

//		cl.def("__init__", [](std::map<pystorm::bddriver::bdpars::ConfigSynapseID,std::vector<unsigned int, std::allocator<unsigned int> >,std::less<pystorm::bddriver::bdpars::ConfigSynapseID>,std::allocator<std::pair<const pystorm::bddriver::bdpars::ConfigSynapseID, std::vector<unsigned int, std::allocator<unsigned int> > > >> *self_, const struct std::less<pystorm::bddriver::bdpars::ConfigSynapseID> & a0) { new (self_) std::map<pystorm::bddriver::bdpars::ConfigSynapseID,std::vector<unsigned int, std::allocator<unsigned int> >,std::less<pystorm::bddriver::bdpars::ConfigSynapseID>,std::allocator<std::pair<const pystorm::bddriver::bdpars::ConfigSynapseID, std::vector<unsigned int, std::allocator<unsigned int> > > >>(a0); }, "doc");
//		cl.def(pybind11::init<const struct std::less<pystorm::bddriver::bdpars::ConfigSynapseID> &, const class std::allocator<struct std::pair<const pystorm::bddriver::bdpars::ConfigSynapseID, class std::vector<unsigned int, class std::allocator<unsigned int> > > > &>(), pybind11::arg("__comp"), pybind11::arg("__a"));

//		cl.def(pybind11::init<const class std::map<pystorm::bddriver::bdpars::ConfigSynapseID, class std::vector<unsigned int, class std::allocator<unsigned int> >, struct std::less<pystorm::bddriver::bdpars::ConfigSynapseID>, class std::allocator<struct std::pair<const pystorm::bddriver::bdpars::ConfigSynapseID, class std::vector<unsigned int, class std::allocator<unsigned int> > > > > &>(), pybind11::arg("__x"));

//		cl.def("__init__", [](std::map<pystorm::bddriver::bdpars::ConfigSynapseID,std::vector<unsigned int, std::allocator<unsigned int> >,std::less<pystorm::bddriver::bdpars::ConfigSynapseID>,std::allocator<std::pair<const pystorm::bddriver::bdpars::ConfigSynapseID, std::vector<unsigned int, std::allocator<unsigned int> > > >> *self_, class std::initializer_list<struct std::pair<const pystorm::bddriver::bdpars::ConfigSynapseID, class std::vector<unsigned int, class std::allocator<unsigned int> > > >  const &a0) { new (self_) std::map<pystorm::bddriver::bdpars::ConfigSynapseID,std::vector<unsigned int, std::allocator<unsigned int> >,std::less<pystorm::bddriver::bdpars::ConfigSynapseID>,std::allocator<std::pair<const pystorm::bddriver::bdpars::ConfigSynapseID, std::vector<unsigned int, std::allocator<unsigned int> > > >>(a0); }, "doc");
//		cl.def("__init__", [](std::map<pystorm::bddriver::bdpars::ConfigSynapseID,std::vector<unsigned int, std::allocator<unsigned int> >,std::less<pystorm::bddriver::bdpars::ConfigSynapseID>,std::allocator<std::pair<const pystorm::bddriver::bdpars::ConfigSynapseID, std::vector<unsigned int, std::allocator<unsigned int> > > >> *self_, class std::initializer_list<struct std::pair<const pystorm::bddriver::bdpars::ConfigSynapseID, class std::vector<unsigned int, class std::allocator<unsigned int> > > >  const &a0, const struct std::less<pystorm::bddriver::bdpars::ConfigSynapseID> & a1) { new (self_) std::map<pystorm::bddriver::bdpars::ConfigSynapseID,std::vector<unsigned int, std::allocator<unsigned int> >,std::less<pystorm::bddriver::bdpars::ConfigSynapseID>,std::allocator<std::pair<const pystorm::bddriver::bdpars::ConfigSynapseID, std::vector<unsigned int, std::allocator<unsigned int> > > >>(a0, a1); }, "doc");
//		cl.def(pybind11::init<class std::initializer_list<struct std::pair<const pystorm::bddriver::bdpars::ConfigSynapseID, class std::vector<unsigned int, class std::allocator<unsigned int> > > >, const struct std::less<pystorm::bddriver::bdpars::ConfigSynapseID> &, const class std::allocator<struct std::pair<const pystorm::bddriver::bdpars::ConfigSynapseID, class std::vector<unsigned int, class std::allocator<unsigned int> > > > &>(), pybind11::arg("__l"), pybind11::arg("__comp"), pybind11::arg("__a"));

//		cl.def(pybind11::init<const class std::allocator<struct std::pair<const pystorm::bddriver::bdpars::ConfigSynapseID, class std::vector<unsigned int, class std::allocator<unsigned int> > > > &>(), pybind11::arg("__a"));

//		cl.def(pybind11::init<const class std::map<pystorm::bddriver::bdpars::ConfigSynapseID, class std::vector<unsigned int, class std::allocator<unsigned int> >, struct std::less<pystorm::bddriver::bdpars::ConfigSynapseID>, class std::allocator<struct std::pair<const pystorm::bddriver::bdpars::ConfigSynapseID, class std::vector<unsigned int, class std::allocator<unsigned int> > > > > &, const class std::allocator<struct std::pair<const pystorm::bddriver::bdpars::ConfigSynapseID, class std::vector<unsigned int, class std::allocator<unsigned int> > > > &>(), pybind11::arg("__m"), pybind11::arg("__a"));

//		cl.def(pybind11::init<class std::initializer_list<struct std::pair<const pystorm::bddriver::bdpars::ConfigSynapseID, class std::vector<unsigned int, class std::allocator<unsigned int> > > >, const class std::allocator<struct std::pair<const pystorm::bddriver::bdpars::ConfigSynapseID, class std::vector<unsigned int, class std::allocator<unsigned int> > > > &>(), pybind11::arg("__l"), pybind11::arg("__a"));

//		cl.def("assign", (class std::map<pystorm::bddriver::bdpars::ConfigSynapseID, class std::vector<unsigned int, class std::allocator<unsigned int> >, struct std::less<pystorm::bddriver::bdpars::ConfigSynapseID>, class std::allocator<struct std::pair<const pystorm::bddriver::bdpars::ConfigSynapseID, class std::vector<unsigned int, class std::allocator<unsigned int> > > > > & (std::map<pystorm::bddriver::bdpars::ConfigSynapseID,std::vector<unsigned int, std::allocator<unsigned int> >,std::less<pystorm::bddriver::bdpars::ConfigSynapseID>,std::allocator<std::pair<const pystorm::bddriver::bdpars::ConfigSynapseID, std::vector<unsigned int, std::allocator<unsigned int> > > >>::*)(const class std::map<pystorm::bddriver::bdpars::ConfigSynapseID, class std::vector<unsigned int, class std::allocator<unsigned int> >, struct std::less<pystorm::bddriver::bdpars::ConfigSynapseID>, class std::allocator<struct std::pair<const pystorm::bddriver::bdpars::ConfigSynapseID, class std::vector<unsigned int, class std::allocator<unsigned int> > > > > &)) &std::map<pystorm::bddriver::bdpars::ConfigSynapseID, std::vector<unsigned int, std::allocator<unsigned int> >, std::less<pystorm::bddriver::bdpars::ConfigSynapseID>, std::allocator<std::pair<const pystorm::bddriver::bdpars::ConfigSynapseID, std::vector<unsigned int, std::allocator<unsigned int> > > > >::operator=, "C++: std::map<pystorm::bddriver::bdpars::ConfigSynapseID, std::vector<unsigned int, std::allocator<unsigned int> >, std::less<pystorm::bddriver::bdpars::ConfigSynapseID>, std::allocator<std::pair<const pystorm::bddriver::bdpars::ConfigSynapseID, std::vector<unsigned int, std::allocator<unsigned int> > > > >::operator=(const class std::map<pystorm::bddriver::bdpars::ConfigSynapseID, class std::vector<unsigned int, class std::allocator<unsigned int> >, struct std::less<pystorm::bddriver::bdpars::ConfigSynapseID>, class std::allocator<struct std::pair<const pystorm::bddriver::bdpars::ConfigSynapseID, class std::vector<unsigned int, class std::allocator<unsigned int> > > > > &) --> class std::map<pystorm::bddriver::bdpars::ConfigSynapseID, class std::vector<unsigned int, class std::allocator<unsigned int> >, struct std::less<pystorm::bddriver::bdpars::ConfigSynapseID>, class std::allocator<struct std::pair<const pystorm::bddriver::bdpars::ConfigSynapseID, class std::vector<unsigned int, class std::allocator<unsigned int> > > > > &", pybind11::return_value_policy::automatic, pybind11::arg("__x"));
//		cl.def("assign", (class std::map<pystorm::bddriver::bdpars::ConfigSynapseID, class std::vector<unsigned int, class std::allocator<unsigned int> >, struct std::less<pystorm::bddriver::bdpars::ConfigSynapseID>, class std::allocator<struct std::pair<const pystorm::bddriver::bdpars::ConfigSynapseID, class std::vector<unsigned int, class std::allocator<unsigned int> > > > > & (std::map<pystorm::bddriver::bdpars::ConfigSynapseID,std::vector<unsigned int, std::allocator<unsigned int> >,std::less<pystorm::bddriver::bdpars::ConfigSynapseID>,std::allocator<std::pair<const pystorm::bddriver::bdpars::ConfigSynapseID, std::vector<unsigned int, std::allocator<unsigned int> > > >>::*)(class std::initializer_list<struct std::pair<const pystorm::bddriver::bdpars::ConfigSynapseID, class std::vector<unsigned int, class std::allocator<unsigned int> > > >)) &std::map<pystorm::bddriver::bdpars::ConfigSynapseID, std::vector<unsigned int, std::allocator<unsigned int> >, std::less<pystorm::bddriver::bdpars::ConfigSynapseID>, std::allocator<std::pair<const pystorm::bddriver::bdpars::ConfigSynapseID, std::vector<unsigned int, std::allocator<unsigned int> > > > >::operator=, "C++: std::map<pystorm::bddriver::bdpars::ConfigSynapseID, std::vector<unsigned int, std::allocator<unsigned int> >, std::less<pystorm::bddriver::bdpars::ConfigSynapseID>, std::allocator<std::pair<const pystorm::bddriver::bdpars::ConfigSynapseID, std::vector<unsigned int, std::allocator<unsigned int> > > > >::operator=(class std::initializer_list<struct std::pair<const pystorm::bddriver::bdpars::ConfigSynapseID, class std::vector<unsigned int, class std::allocator<unsigned int> > > >) --> class std::map<pystorm::bddriver::bdpars::ConfigSynapseID, class std::vector<unsigned int, class std::allocator<unsigned int> >, struct std::less<pystorm::bddriver::bdpars::ConfigSynapseID>, class std::allocator<struct std::pair<const pystorm::bddriver::bdpars::ConfigSynapseID, class std::vector<unsigned int, class std::allocator<unsigned int> > > > > &", pybind11::return_value_policy::automatic, pybind11::arg("__l"));
//		cl.def("get_allocator", (class std::allocator<struct std::pair<const pystorm::bddriver::bdpars::ConfigSynapseID, class std::vector<unsigned int, class std::allocator<unsigned int> > > > (std::map<pystorm::bddriver::bdpars::ConfigSynapseID,std::vector<unsigned int, std::allocator<unsigned int> >,std::less<pystorm::bddriver::bdpars::ConfigSynapseID>,std::allocator<std::pair<const pystorm::bddriver::bdpars::ConfigSynapseID, std::vector<unsigned int, std::allocator<unsigned int> > > >>::*)() const) &std::map<pystorm::bddriver::bdpars::ConfigSynapseID, std::vector<unsigned int, std::allocator<unsigned int> >, std::less<pystorm::bddriver::bdpars::ConfigSynapseID>, std::allocator<std::pair<const pystorm::bddriver::bdpars::ConfigSynapseID, std::vector<unsigned int, std::allocator<unsigned int> > > > >::get_allocator, "C++: std::map<pystorm::bddriver::bdpars::ConfigSynapseID, std::vector<unsigned int, std::allocator<unsigned int> >, std::less<pystorm::bddriver::bdpars::ConfigSynapseID>, std::allocator<std::pair<const pystorm::bddriver::bdpars::ConfigSynapseID, std::vector<unsigned int, std::allocator<unsigned int> > > > >::get_allocator() const --> class std::allocator<struct std::pair<const pystorm::bddriver::bdpars::ConfigSynapseID, class std::vector<unsigned int, class std::allocator<unsigned int> > > >");
//		cl.def("begin", (struct std::_Rb_tree_iterator<struct std::pair<const pystorm::bddriver::bdpars::ConfigSynapseID, class std::vector<unsigned int, class std::allocator<unsigned int> > > > (std::map<pystorm::bddriver::bdpars::ConfigSynapseID,std::vector<unsigned int, std::allocator<unsigned int> >,std::less<pystorm::bddriver::bdpars::ConfigSynapseID>,std::allocator<std::pair<const pystorm::bddriver::bdpars::ConfigSynapseID, std::vector<unsigned int, std::allocator<unsigned int> > > >>::*)()) &std::map<pystorm::bddriver::bdpars::ConfigSynapseID, std::vector<unsigned int, std::allocator<unsigned int> >, std::less<pystorm::bddriver::bdpars::ConfigSynapseID>, std::allocator<std::pair<const pystorm::bddriver::bdpars::ConfigSynapseID, std::vector<unsigned int, std::allocator<unsigned int> > > > >::begin, "C++: std::map<pystorm::bddriver::bdpars::ConfigSynapseID, std::vector<unsigned int, std::allocator<unsigned int> >, std::less<pystorm::bddriver::bdpars::ConfigSynapseID>, std::allocator<std::pair<const pystorm::bddriver::bdpars::ConfigSynapseID, std::vector<unsigned int, std::allocator<unsigned int> > > > >::begin() --> struct std::_Rb_tree_iterator<struct std::pair<const pystorm::bddriver::bdpars::ConfigSynapseID, class std::vector<unsigned int, class std::allocator<unsigned int> > > >");
//		cl.def("begin", (struct std::_Rb_tree_const_iterator<struct std::pair<const pystorm::bddriver::bdpars::ConfigSynapseID, class std::vector<unsigned int, class std::allocator<unsigned int> > > > (std::map<pystorm::bddriver::bdpars::ConfigSynapseID,std::vector<unsigned int, std::allocator<unsigned int> >,std::less<pystorm::bddriver::bdpars::ConfigSynapseID>,std::allocator<std::pair<const pystorm::bddriver::bdpars::ConfigSynapseID, std::vector<unsigned int, std::allocator<unsigned int> > > >>::*)() const) &std::map<pystorm::bddriver::bdpars::ConfigSynapseID, std::vector<unsigned int, std::allocator<unsigned int> >, std::less<pystorm::bddriver::bdpars::ConfigSynapseID>, std::allocator<std::pair<const pystorm::bddriver::bdpars::ConfigSynapseID, std::vector<unsigned int, std::allocator<unsigned int> > > > >::begin, "C++: std::map<pystorm::bddriver::bdpars::ConfigSynapseID, std::vector<unsigned int, std::allocator<unsigned int> >, std::less<pystorm::bddriver::bdpars::ConfigSynapseID>, std::allocator<std::pair<const pystorm::bddriver::bdpars::ConfigSynapseID, std::vector<unsigned int, std::allocator<unsigned int> > > > >::begin() const --> struct std::_Rb_tree_const_iterator<struct std::pair<const pystorm::bddriver::bdpars::ConfigSynapseID, class std::vector<unsigned int, class std::allocator<unsigned int> > > >");
//		cl.def("end", (struct std::_Rb_tree_iterator<struct std::pair<const pystorm::bddriver::bdpars::ConfigSynapseID, class std::vector<unsigned int, class std::allocator<unsigned int> > > > (std::map<pystorm::bddriver::bdpars::ConfigSynapseID,std::vector<unsigned int, std::allocator<unsigned int> >,std::less<pystorm::bddriver::bdpars::ConfigSynapseID>,std::allocator<std::pair<const pystorm::bddriver::bdpars::ConfigSynapseID, std::vector<unsigned int, std::allocator<unsigned int> > > >>::*)()) &std::map<pystorm::bddriver::bdpars::ConfigSynapseID, std::vector<unsigned int, std::allocator<unsigned int> >, std::less<pystorm::bddriver::bdpars::ConfigSynapseID>, std::allocator<std::pair<const pystorm::bddriver::bdpars::ConfigSynapseID, std::vector<unsigned int, std::allocator<unsigned int> > > > >::end, "C++: std::map<pystorm::bddriver::bdpars::ConfigSynapseID, std::vector<unsigned int, std::allocator<unsigned int> >, std::less<pystorm::bddriver::bdpars::ConfigSynapseID>, std::allocator<std::pair<const pystorm::bddriver::bdpars::ConfigSynapseID, std::vector<unsigned int, std::allocator<unsigned int> > > > >::end() --> struct std::_Rb_tree_iterator<struct std::pair<const pystorm::bddriver::bdpars::ConfigSynapseID, class std::vector<unsigned int, class std::allocator<unsigned int> > > >");
//		cl.def("end", (struct std::_Rb_tree_const_iterator<struct std::pair<const pystorm::bddriver::bdpars::ConfigSynapseID, class std::vector<unsigned int, class std::allocator<unsigned int> > > > (std::map<pystorm::bddriver::bdpars::ConfigSynapseID,std::vector<unsigned int, std::allocator<unsigned int> >,std::less<pystorm::bddriver::bdpars::ConfigSynapseID>,std::allocator<std::pair<const pystorm::bddriver::bdpars::ConfigSynapseID, std::vector<unsigned int, std::allocator<unsigned int> > > >>::*)() const) &std::map<pystorm::bddriver::bdpars::ConfigSynapseID, std::vector<unsigned int, std::allocator<unsigned int> >, std::less<pystorm::bddriver::bdpars::ConfigSynapseID>, std::allocator<std::pair<const pystorm::bddriver::bdpars::ConfigSynapseID, std::vector<unsigned int, std::allocator<unsigned int> > > > >::end, "C++: std::map<pystorm::bddriver::bdpars::ConfigSynapseID, std::vector<unsigned int, std::allocator<unsigned int> >, std::less<pystorm::bddriver::bdpars::ConfigSynapseID>, std::allocator<std::pair<const pystorm::bddriver::bdpars::ConfigSynapseID, std::vector<unsigned int, std::allocator<unsigned int> > > > >::end() const --> struct std::_Rb_tree_const_iterator<struct std::pair<const pystorm::bddriver::bdpars::ConfigSynapseID, class std::vector<unsigned int, class std::allocator<unsigned int> > > >");
//		cl.def("cbegin", (struct std::_Rb_tree_const_iterator<struct std::pair<const pystorm::bddriver::bdpars::ConfigSynapseID, class std::vector<unsigned int, class std::allocator<unsigned int> > > > (std::map<pystorm::bddriver::bdpars::ConfigSynapseID,std::vector<unsigned int, std::allocator<unsigned int> >,std::less<pystorm::bddriver::bdpars::ConfigSynapseID>,std::allocator<std::pair<const pystorm::bddriver::bdpars::ConfigSynapseID, std::vector<unsigned int, std::allocator<unsigned int> > > >>::*)() const) &std::map<pystorm::bddriver::bdpars::ConfigSynapseID, std::vector<unsigned int, std::allocator<unsigned int> >, std::less<pystorm::bddriver::bdpars::ConfigSynapseID>, std::allocator<std::pair<const pystorm::bddriver::bdpars::ConfigSynapseID, std::vector<unsigned int, std::allocator<unsigned int> > > > >::cbegin, "C++: std::map<pystorm::bddriver::bdpars::ConfigSynapseID, std::vector<unsigned int, std::allocator<unsigned int> >, std::less<pystorm::bddriver::bdpars::ConfigSynapseID>, std::allocator<std::pair<const pystorm::bddriver::bdpars::ConfigSynapseID, std::vector<unsigned int, std::allocator<unsigned int> > > > >::cbegin() const --> struct std::_Rb_tree_const_iterator<struct std::pair<const pystorm::bddriver::bdpars::ConfigSynapseID, class std::vector<unsigned int, class std::allocator<unsigned int> > > >");
//		cl.def("cend", (struct std::_Rb_tree_const_iterator<struct std::pair<const pystorm::bddriver::bdpars::ConfigSynapseID, class std::vector<unsigned int, class std::allocator<unsigned int> > > > (std::map<pystorm::bddriver::bdpars::ConfigSynapseID,std::vector<unsigned int, std::allocator<unsigned int> >,std::less<pystorm::bddriver::bdpars::ConfigSynapseID>,std::allocator<std::pair<const pystorm::bddriver::bdpars::ConfigSynapseID, std::vector<unsigned int, std::allocator<unsigned int> > > >>::*)() const) &std::map<pystorm::bddriver::bdpars::ConfigSynapseID, std::vector<unsigned int, std::allocator<unsigned int> >, std::less<pystorm::bddriver::bdpars::ConfigSynapseID>, std::allocator<std::pair<const pystorm::bddriver::bdpars::ConfigSynapseID, std::vector<unsigned int, std::allocator<unsigned int> > > > >::cend, "C++: std::map<pystorm::bddriver::bdpars::ConfigSynapseID, std::vector<unsigned int, std::allocator<unsigned int> >, std::less<pystorm::bddriver::bdpars::ConfigSynapseID>, std::allocator<std::pair<const pystorm::bddriver::bdpars::ConfigSynapseID, std::vector<unsigned int, std::allocator<unsigned int> > > > >::cend() const --> struct std::_Rb_tree_const_iterator<struct std::pair<const pystorm::bddriver::bdpars::ConfigSynapseID, class std::vector<unsigned int, class std::allocator<unsigned int> > > >");
//		cl.def("empty", (bool (std::map<pystorm::bddriver::bdpars::ConfigSynapseID,std::vector<unsigned int, std::allocator<unsigned int> >,std::less<pystorm::bddriver::bdpars::ConfigSynapseID>,std::allocator<std::pair<const pystorm::bddriver::bdpars::ConfigSynapseID, std::vector<unsigned int, std::allocator<unsigned int> > > >>::*)() const) &std::map<pystorm::bddriver::bdpars::ConfigSynapseID, std::vector<unsigned int, std::allocator<unsigned int> >, std::less<pystorm::bddriver::bdpars::ConfigSynapseID>, std::allocator<std::pair<const pystorm::bddriver::bdpars::ConfigSynapseID, std::vector<unsigned int, std::allocator<unsigned int> > > > >::empty, "C++: std::map<pystorm::bddriver::bdpars::ConfigSynapseID, std::vector<unsigned int, std::allocator<unsigned int> >, std::less<pystorm::bddriver::bdpars::ConfigSynapseID>, std::allocator<std::pair<const pystorm::bddriver::bdpars::ConfigSynapseID, std::vector<unsigned int, std::allocator<unsigned int> > > > >::empty() const --> bool");
//		cl.def("size", (unsigned long (std::map<pystorm::bddriver::bdpars::ConfigSynapseID,std::vector<unsigned int, std::allocator<unsigned int> >,std::less<pystorm::bddriver::bdpars::ConfigSynapseID>,std::allocator<std::pair<const pystorm::bddriver::bdpars::ConfigSynapseID, std::vector<unsigned int, std::allocator<unsigned int> > > >>::*)() const) &std::map<pystorm::bddriver::bdpars::ConfigSynapseID, std::vector<unsigned int, std::allocator<unsigned int> >, std::less<pystorm::bddriver::bdpars::ConfigSynapseID>, std::allocator<std::pair<const pystorm::bddriver::bdpars::ConfigSynapseID, std::vector<unsigned int, std::allocator<unsigned int> > > > >::size, "C++: std::map<pystorm::bddriver::bdpars::ConfigSynapseID, std::vector<unsigned int, std::allocator<unsigned int> >, std::less<pystorm::bddriver::bdpars::ConfigSynapseID>, std::allocator<std::pair<const pystorm::bddriver::bdpars::ConfigSynapseID, std::vector<unsigned int, std::allocator<unsigned int> > > > >::size() const --> unsigned long");
//		cl.def("max_size", (unsigned long (std::map<pystorm::bddriver::bdpars::ConfigSynapseID,std::vector<unsigned int, std::allocator<unsigned int> >,std::less<pystorm::bddriver::bdpars::ConfigSynapseID>,std::allocator<std::pair<const pystorm::bddriver::bdpars::ConfigSynapseID, std::vector<unsigned int, std::allocator<unsigned int> > > >>::*)() const) &std::map<pystorm::bddriver::bdpars::ConfigSynapseID, std::vector<unsigned int, std::allocator<unsigned int> >, std::less<pystorm::bddriver::bdpars::ConfigSynapseID>, std::allocator<std::pair<const pystorm::bddriver::bdpars::ConfigSynapseID, std::vector<unsigned int, std::allocator<unsigned int> > > > >::max_size, "C++: std::map<pystorm::bddriver::bdpars::ConfigSynapseID, std::vector<unsigned int, std::allocator<unsigned int> >, std::less<pystorm::bddriver::bdpars::ConfigSynapseID>, std::allocator<std::pair<const pystorm::bddriver::bdpars::ConfigSynapseID, std::vector<unsigned int, std::allocator<unsigned int> > > > >::max_size() const --> unsigned long");
//		cl.def("__getitem__", (class std::vector<unsigned int, class std::allocator<unsigned int> > & (std::map<pystorm::bddriver::bdpars::ConfigSynapseID,std::vector<unsigned int, std::allocator<unsigned int> >,std::less<pystorm::bddriver::bdpars::ConfigSynapseID>,std::allocator<std::pair<const pystorm::bddriver::bdpars::ConfigSynapseID, std::vector<unsigned int, std::allocator<unsigned int> > > >>::*)(const pystorm::bddriver::bdpars::ConfigSynapseID &)) &std::map<pystorm::bddriver::bdpars::ConfigSynapseID, std::vector<unsigned int, std::allocator<unsigned int> >, std::less<pystorm::bddriver::bdpars::ConfigSynapseID>, std::allocator<std::pair<const pystorm::bddriver::bdpars::ConfigSynapseID, std::vector<unsigned int, std::allocator<unsigned int> > > > >::operator[], "C++: std::map<pystorm::bddriver::bdpars::ConfigSynapseID, std::vector<unsigned int, std::allocator<unsigned int> >, std::less<pystorm::bddriver::bdpars::ConfigSynapseID>, std::allocator<std::pair<const pystorm::bddriver::bdpars::ConfigSynapseID, std::vector<unsigned int, std::allocator<unsigned int> > > > >::operator[](const pystorm::bddriver::bdpars::ConfigSynapseID &) --> class std::vector<unsigned int, class std::allocator<unsigned int> > &", pybind11::return_value_policy::automatic, pybind11::arg("__k"));
//		cl.def("at", (class std::vector<unsigned int, class std::allocator<unsigned int> > & (std::map<pystorm::bddriver::bdpars::ConfigSynapseID,std::vector<unsigned int, std::allocator<unsigned int> >,std::less<pystorm::bddriver::bdpars::ConfigSynapseID>,std::allocator<std::pair<const pystorm::bddriver::bdpars::ConfigSynapseID, std::vector<unsigned int, std::allocator<unsigned int> > > >>::*)(const pystorm::bddriver::bdpars::ConfigSynapseID &)) &std::map<pystorm::bddriver::bdpars::ConfigSynapseID, std::vector<unsigned int, std::allocator<unsigned int> >, std::less<pystorm::bddriver::bdpars::ConfigSynapseID>, std::allocator<std::pair<const pystorm::bddriver::bdpars::ConfigSynapseID, std::vector<unsigned int, std::allocator<unsigned int> > > > >::at, "C++: std::map<pystorm::bddriver::bdpars::ConfigSynapseID, std::vector<unsigned int, std::allocator<unsigned int> >, std::less<pystorm::bddriver::bdpars::ConfigSynapseID>, std::allocator<std::pair<const pystorm::bddriver::bdpars::ConfigSynapseID, std::vector<unsigned int, std::allocator<unsigned int> > > > >::at(const pystorm::bddriver::bdpars::ConfigSynapseID &) --> class std::vector<unsigned int, class std::allocator<unsigned int> > &", pybind11::return_value_policy::automatic, pybind11::arg("__k"));
//		cl.def("at", (const class std::vector<unsigned int, class std::allocator<unsigned int> > & (std::map<pystorm::bddriver::bdpars::ConfigSynapseID,std::vector<unsigned int, std::allocator<unsigned int> >,std::less<pystorm::bddriver::bdpars::ConfigSynapseID>,std::allocator<std::pair<const pystorm::bddriver::bdpars::ConfigSynapseID, std::vector<unsigned int, std::allocator<unsigned int> > > >>::*)(const pystorm::bddriver::bdpars::ConfigSynapseID &) const) &std::map<pystorm::bddriver::bdpars::ConfigSynapseID, std::vector<unsigned int, std::allocator<unsigned int> >, std::less<pystorm::bddriver::bdpars::ConfigSynapseID>, std::allocator<std::pair<const pystorm::bddriver::bdpars::ConfigSynapseID, std::vector<unsigned int, std::allocator<unsigned int> > > > >::at, "C++: std::map<pystorm::bddriver::bdpars::ConfigSynapseID, std::vector<unsigned int, std::allocator<unsigned int> >, std::less<pystorm::bddriver::bdpars::ConfigSynapseID>, std::allocator<std::pair<const pystorm::bddriver::bdpars::ConfigSynapseID, std::vector<unsigned int, std::allocator<unsigned int> > > > >::at(const pystorm::bddriver::bdpars::ConfigSynapseID &) const --> const class std::vector<unsigned int, class std::allocator<unsigned int> > &", pybind11::return_value_policy::automatic, pybind11::arg("__k"));
//		cl.def("insert", (struct std::pair<struct std::_Rb_tree_iterator<struct std::pair<const pystorm::bddriver::bdpars::ConfigSynapseID, class std::vector<unsigned int, class std::allocator<unsigned int> > > >, bool> (std::map<pystorm::bddriver::bdpars::ConfigSynapseID,std::vector<unsigned int, std::allocator<unsigned int> >,std::less<pystorm::bddriver::bdpars::ConfigSynapseID>,std::allocator<std::pair<const pystorm::bddriver::bdpars::ConfigSynapseID, std::vector<unsigned int, std::allocator<unsigned int> > > >>::*)(const struct std::pair<const pystorm::bddriver::bdpars::ConfigSynapseID, class std::vector<unsigned int, class std::allocator<unsigned int> > > &)) &std::map<pystorm::bddriver::bdpars::ConfigSynapseID, std::vector<unsigned int, std::allocator<unsigned int> >, std::less<pystorm::bddriver::bdpars::ConfigSynapseID>, std::allocator<std::pair<const pystorm::bddriver::bdpars::ConfigSynapseID, std::vector<unsigned int, std::allocator<unsigned int> > > > >::insert, "C++: std::map<pystorm::bddriver::bdpars::ConfigSynapseID, std::vector<unsigned int, std::allocator<unsigned int> >, std::less<pystorm::bddriver::bdpars::ConfigSynapseID>, std::allocator<std::pair<const pystorm::bddriver::bdpars::ConfigSynapseID, std::vector<unsigned int, std::allocator<unsigned int> > > > >::insert(const struct std::pair<const pystorm::bddriver::bdpars::ConfigSynapseID, class std::vector<unsigned int, class std::allocator<unsigned int> > > &) --> struct std::pair<struct std::_Rb_tree_iterator<struct std::pair<const pystorm::bddriver::bdpars::ConfigSynapseID, class std::vector<unsigned int, class std::allocator<unsigned int> > > >, bool>", pybind11::arg("__x"));
//		cl.def("insert", (void (std::map<pystorm::bddriver::bdpars::ConfigSynapseID,std::vector<unsigned int, std::allocator<unsigned int> >,std::less<pystorm::bddriver::bdpars::ConfigSynapseID>,std::allocator<std::pair<const pystorm::bddriver::bdpars::ConfigSynapseID, std::vector<unsigned int, std::allocator<unsigned int> > > >>::*)(class std::initializer_list<struct std::pair<const pystorm::bddriver::bdpars::ConfigSynapseID, class std::vector<unsigned int, class std::allocator<unsigned int> > > >)) &std::map<pystorm::bddriver::bdpars::ConfigSynapseID, std::vector<unsigned int, std::allocator<unsigned int> >, std::less<pystorm::bddriver::bdpars::ConfigSynapseID>, std::allocator<std::pair<const pystorm::bddriver::bdpars::ConfigSynapseID, std::vector<unsigned int, std::allocator<unsigned int> > > > >::insert, "C++: std::map<pystorm::bddriver::bdpars::ConfigSynapseID, std::vector<unsigned int, std::allocator<unsigned int> >, std::less<pystorm::bddriver::bdpars::ConfigSynapseID>, std::allocator<std::pair<const pystorm::bddriver::bdpars::ConfigSynapseID, std::vector<unsigned int, std::allocator<unsigned int> > > > >::insert(class std::initializer_list<struct std::pair<const pystorm::bddriver::bdpars::ConfigSynapseID, class std::vector<unsigned int, class std::allocator<unsigned int> > > >) --> void", pybind11::arg("__list"));
//		cl.def("insert", (struct std::_Rb_tree_iterator<struct std::pair<const pystorm::bddriver::bdpars::ConfigSynapseID, class std::vector<unsigned int, class std::allocator<unsigned int> > > > (std::map<pystorm::bddriver::bdpars::ConfigSynapseID,std::vector<unsigned int, std::allocator<unsigned int> >,std::less<pystorm::bddriver::bdpars::ConfigSynapseID>,std::allocator<std::pair<const pystorm::bddriver::bdpars::ConfigSynapseID, std::vector<unsigned int, std::allocator<unsigned int> > > >>::*)(struct std::_Rb_tree_const_iterator<struct std::pair<const pystorm::bddriver::bdpars::ConfigSynapseID, class std::vector<unsigned int, class std::allocator<unsigned int> > > >, const struct std::pair<const pystorm::bddriver::bdpars::ConfigSynapseID, class std::vector<unsigned int, class std::allocator<unsigned int> > > &)) &std::map<pystorm::bddriver::bdpars::ConfigSynapseID, std::vector<unsigned int, std::allocator<unsigned int> >, std::less<pystorm::bddriver::bdpars::ConfigSynapseID>, std::allocator<std::pair<const pystorm::bddriver::bdpars::ConfigSynapseID, std::vector<unsigned int, std::allocator<unsigned int> > > > >::insert, "C++: std::map<pystorm::bddriver::bdpars::ConfigSynapseID, std::vector<unsigned int, std::allocator<unsigned int> >, std::less<pystorm::bddriver::bdpars::ConfigSynapseID>, std::allocator<std::pair<const pystorm::bddriver::bdpars::ConfigSynapseID, std::vector<unsigned int, std::allocator<unsigned int> > > > >::insert(struct std::_Rb_tree_const_iterator<struct std::pair<const pystorm::bddriver::bdpars::ConfigSynapseID, class std::vector<unsigned int, class std::allocator<unsigned int> > > >, const struct std::pair<const pystorm::bddriver::bdpars::ConfigSynapseID, class std::vector<unsigned int, class std::allocator<unsigned int> > > &) --> struct std::_Rb_tree_iterator<struct std::pair<const pystorm::bddriver::bdpars::ConfigSynapseID, class std::vector<unsigned int, class std::allocator<unsigned int> > > >", pybind11::arg("__position"), pybind11::arg("__x"));
//		cl.def("erase", (struct std::_Rb_tree_iterator<struct std::pair<const pystorm::bddriver::bdpars::ConfigSynapseID, class std::vector<unsigned int, class std::allocator<unsigned int> > > > (std::map<pystorm::bddriver::bdpars::ConfigSynapseID,std::vector<unsigned int, std::allocator<unsigned int> >,std::less<pystorm::bddriver::bdpars::ConfigSynapseID>,std::allocator<std::pair<const pystorm::bddriver::bdpars::ConfigSynapseID, std::vector<unsigned int, std::allocator<unsigned int> > > >>::*)(struct std::_Rb_tree_const_iterator<struct std::pair<const pystorm::bddriver::bdpars::ConfigSynapseID, class std::vector<unsigned int, class std::allocator<unsigned int> > > >)) &std::map<pystorm::bddriver::bdpars::ConfigSynapseID, std::vector<unsigned int, std::allocator<unsigned int> >, std::less<pystorm::bddriver::bdpars::ConfigSynapseID>, std::allocator<std::pair<const pystorm::bddriver::bdpars::ConfigSynapseID, std::vector<unsigned int, std::allocator<unsigned int> > > > >::erase, "C++: std::map<pystorm::bddriver::bdpars::ConfigSynapseID, std::vector<unsigned int, std::allocator<unsigned int> >, std::less<pystorm::bddriver::bdpars::ConfigSynapseID>, std::allocator<std::pair<const pystorm::bddriver::bdpars::ConfigSynapseID, std::vector<unsigned int, std::allocator<unsigned int> > > > >::erase(struct std::_Rb_tree_const_iterator<struct std::pair<const pystorm::bddriver::bdpars::ConfigSynapseID, class std::vector<unsigned int, class std::allocator<unsigned int> > > >) --> struct std::_Rb_tree_iterator<struct std::pair<const pystorm::bddriver::bdpars::ConfigSynapseID, class std::vector<unsigned int, class std::allocator<unsigned int> > > >", pybind11::arg("__position"));
//		cl.def("erase", (struct std::_Rb_tree_iterator<struct std::pair<const pystorm::bddriver::bdpars::ConfigSynapseID, class std::vector<unsigned int, class std::allocator<unsigned int> > > > (std::map<pystorm::bddriver::bdpars::ConfigSynapseID,std::vector<unsigned int, std::allocator<unsigned int> >,std::less<pystorm::bddriver::bdpars::ConfigSynapseID>,std::allocator<std::pair<const pystorm::bddriver::bdpars::ConfigSynapseID, std::vector<unsigned int, std::allocator<unsigned int> > > >>::*)(struct std::_Rb_tree_iterator<struct std::pair<const pystorm::bddriver::bdpars::ConfigSynapseID, class std::vector<unsigned int, class std::allocator<unsigned int> > > >)) &std::map<pystorm::bddriver::bdpars::ConfigSynapseID, std::vector<unsigned int, std::allocator<unsigned int> >, std::less<pystorm::bddriver::bdpars::ConfigSynapseID>, std::allocator<std::pair<const pystorm::bddriver::bdpars::ConfigSynapseID, std::vector<unsigned int, std::allocator<unsigned int> > > > >::erase, "C++: std::map<pystorm::bddriver::bdpars::ConfigSynapseID, std::vector<unsigned int, std::allocator<unsigned int> >, std::less<pystorm::bddriver::bdpars::ConfigSynapseID>, std::allocator<std::pair<const pystorm::bddriver::bdpars::ConfigSynapseID, std::vector<unsigned int, std::allocator<unsigned int> > > > >::erase(struct std::_Rb_tree_iterator<struct std::pair<const pystorm::bddriver::bdpars::ConfigSynapseID, class std::vector<unsigned int, class std::allocator<unsigned int> > > >) --> struct std::_Rb_tree_iterator<struct std::pair<const pystorm::bddriver::bdpars::ConfigSynapseID, class std::vector<unsigned int, class std::allocator<unsigned int> > > >", pybind11::arg("__position"));
//		cl.def("erase", (unsigned long (std::map<pystorm::bddriver::bdpars::ConfigSynapseID,std::vector<unsigned int, std::allocator<unsigned int> >,std::less<pystorm::bddriver::bdpars::ConfigSynapseID>,std::allocator<std::pair<const pystorm::bddriver::bdpars::ConfigSynapseID, std::vector<unsigned int, std::allocator<unsigned int> > > >>::*)(const pystorm::bddriver::bdpars::ConfigSynapseID &)) &std::map<pystorm::bddriver::bdpars::ConfigSynapseID, std::vector<unsigned int, std::allocator<unsigned int> >, std::less<pystorm::bddriver::bdpars::ConfigSynapseID>, std::allocator<std::pair<const pystorm::bddriver::bdpars::ConfigSynapseID, std::vector<unsigned int, std::allocator<unsigned int> > > > >::erase, "C++: std::map<pystorm::bddriver::bdpars::ConfigSynapseID, std::vector<unsigned int, std::allocator<unsigned int> >, std::less<pystorm::bddriver::bdpars::ConfigSynapseID>, std::allocator<std::pair<const pystorm::bddriver::bdpars::ConfigSynapseID, std::vector<unsigned int, std::allocator<unsigned int> > > > >::erase(const pystorm::bddriver::bdpars::ConfigSynapseID &) --> unsigned long", pybind11::arg("__x"));
//		cl.def("erase", (struct std::_Rb_tree_iterator<struct std::pair<const pystorm::bddriver::bdpars::ConfigSynapseID, class std::vector<unsigned int, class std::allocator<unsigned int> > > > (std::map<pystorm::bddriver::bdpars::ConfigSynapseID,std::vector<unsigned int, std::allocator<unsigned int> >,std::less<pystorm::bddriver::bdpars::ConfigSynapseID>,std::allocator<std::pair<const pystorm::bddriver::bdpars::ConfigSynapseID, std::vector<unsigned int, std::allocator<unsigned int> > > >>::*)(struct std::_Rb_tree_const_iterator<struct std::pair<const pystorm::bddriver::bdpars::ConfigSynapseID, class std::vector<unsigned int, class std::allocator<unsigned int> > > >, struct std::_Rb_tree_const_iterator<struct std::pair<const pystorm::bddriver::bdpars::ConfigSynapseID, class std::vector<unsigned int, class std::allocator<unsigned int> > > >)) &std::map<pystorm::bddriver::bdpars::ConfigSynapseID, std::vector<unsigned int, std::allocator<unsigned int> >, std::less<pystorm::bddriver::bdpars::ConfigSynapseID>, std::allocator<std::pair<const pystorm::bddriver::bdpars::ConfigSynapseID, std::vector<unsigned int, std::allocator<unsigned int> > > > >::erase, "C++: std::map<pystorm::bddriver::bdpars::ConfigSynapseID, std::vector<unsigned int, std::allocator<unsigned int> >, std::less<pystorm::bddriver::bdpars::ConfigSynapseID>, std::allocator<std::pair<const pystorm::bddriver::bdpars::ConfigSynapseID, std::vector<unsigned int, std::allocator<unsigned int> > > > >::erase(struct std::_Rb_tree_const_iterator<struct std::pair<const pystorm::bddriver::bdpars::ConfigSynapseID, class std::vector<unsigned int, class std::allocator<unsigned int> > > >, struct std::_Rb_tree_const_iterator<struct std::pair<const pystorm::bddriver::bdpars::ConfigSynapseID, class std::vector<unsigned int, class std::allocator<unsigned int> > > >) --> struct std::_Rb_tree_iterator<struct std::pair<const pystorm::bddriver::bdpars::ConfigSynapseID, class std::vector<unsigned int, class std::allocator<unsigned int> > > >", pybind11::arg("__first"), pybind11::arg("__last"));
//		cl.def("swap", (void (std::map<pystorm::bddriver::bdpars::ConfigSynapseID,std::vector<unsigned int, std::allocator<unsigned int> >,std::less<pystorm::bddriver::bdpars::ConfigSynapseID>,std::allocator<std::pair<const pystorm::bddriver::bdpars::ConfigSynapseID, std::vector<unsigned int, std::allocator<unsigned int> > > >>::*)(class std::map<pystorm::bddriver::bdpars::ConfigSynapseID, class std::vector<unsigned int, class std::allocator<unsigned int> >, struct std::less<pystorm::bddriver::bdpars::ConfigSynapseID>, class std::allocator<struct std::pair<const pystorm::bddriver::bdpars::ConfigSynapseID, class std::vector<unsigned int, class std::allocator<unsigned int> > > > > &)) &std::map<pystorm::bddriver::bdpars::ConfigSynapseID, std::vector<unsigned int, std::allocator<unsigned int> >, std::less<pystorm::bddriver::bdpars::ConfigSynapseID>, std::allocator<std::pair<const pystorm::bddriver::bdpars::ConfigSynapseID, std::vector<unsigned int, std::allocator<unsigned int> > > > >::swap, "C++: std::map<pystorm::bddriver::bdpars::ConfigSynapseID, std::vector<unsigned int, std::allocator<unsigned int> >, std::less<pystorm::bddriver::bdpars::ConfigSynapseID>, std::allocator<std::pair<const pystorm::bddriver::bdpars::ConfigSynapseID, std::vector<unsigned int, std::allocator<unsigned int> > > > >::swap(class std::map<pystorm::bddriver::bdpars::ConfigSynapseID, class std::vector<unsigned int, class std::allocator<unsigned int> >, struct std::less<pystorm::bddriver::bdpars::ConfigSynapseID>, class std::allocator<struct std::pair<const pystorm::bddriver::bdpars::ConfigSynapseID, class std::vector<unsigned int, class std::allocator<unsigned int> > > > > &) --> void", pybind11::arg("__x"));
//		cl.def("clear", (void (std::map<pystorm::bddriver::bdpars::ConfigSynapseID,std::vector<unsigned int, std::allocator<unsigned int> >,std::less<pystorm::bddriver::bdpars::ConfigSynapseID>,std::allocator<std::pair<const pystorm::bddriver::bdpars::ConfigSynapseID, std::vector<unsigned int, std::allocator<unsigned int> > > >>::*)()) &std::map<pystorm::bddriver::bdpars::ConfigSynapseID, std::vector<unsigned int, std::allocator<unsigned int> >, std::less<pystorm::bddriver::bdpars::ConfigSynapseID>, std::allocator<std::pair<const pystorm::bddriver::bdpars::ConfigSynapseID, std::vector<unsigned int, std::allocator<unsigned int> > > > >::clear, "C++: std::map<pystorm::bddriver::bdpars::ConfigSynapseID, std::vector<unsigned int, std::allocator<unsigned int> >, std::less<pystorm::bddriver::bdpars::ConfigSynapseID>, std::allocator<std::pair<const pystorm::bddriver::bdpars::ConfigSynapseID, std::vector<unsigned int, std::allocator<unsigned int> > > > >::clear() --> void");
//		cl.def("key_comp", (struct std::less<pystorm::bddriver::bdpars::ConfigSynapseID> (std::map<pystorm::bddriver::bdpars::ConfigSynapseID,std::vector<unsigned int, std::allocator<unsigned int> >,std::less<pystorm::bddriver::bdpars::ConfigSynapseID>,std::allocator<std::pair<const pystorm::bddriver::bdpars::ConfigSynapseID, std::vector<unsigned int, std::allocator<unsigned int> > > >>::*)() const) &std::map<pystorm::bddriver::bdpars::ConfigSynapseID, std::vector<unsigned int, std::allocator<unsigned int> >, std::less<pystorm::bddriver::bdpars::ConfigSynapseID>, std::allocator<std::pair<const pystorm::bddriver::bdpars::ConfigSynapseID, std::vector<unsigned int, std::allocator<unsigned int> > > > >::key_comp, "C++: std::map<pystorm::bddriver::bdpars::ConfigSynapseID, std::vector<unsigned int, std::allocator<unsigned int> >, std::less<pystorm::bddriver::bdpars::ConfigSynapseID>, std::allocator<std::pair<const pystorm::bddriver::bdpars::ConfigSynapseID, std::vector<unsigned int, std::allocator<unsigned int> > > > >::key_comp() const --> struct std::less<pystorm::bddriver::bdpars::ConfigSynapseID>");
//		cl.def("find", (struct std::_Rb_tree_iterator<struct std::pair<const pystorm::bddriver::bdpars::ConfigSynapseID, class std::vector<unsigned int, class std::allocator<unsigned int> > > > (std::map<pystorm::bddriver::bdpars::ConfigSynapseID,std::vector<unsigned int, std::allocator<unsigned int> >,std::less<pystorm::bddriver::bdpars::ConfigSynapseID>,std::allocator<std::pair<const pystorm::bddriver::bdpars::ConfigSynapseID, std::vector<unsigned int, std::allocator<unsigned int> > > >>::*)(const pystorm::bddriver::bdpars::ConfigSynapseID &)) &std::map<pystorm::bddriver::bdpars::ConfigSynapseID, std::vector<unsigned int, std::allocator<unsigned int> >, std::less<pystorm::bddriver::bdpars::ConfigSynapseID>, std::allocator<std::pair<const pystorm::bddriver::bdpars::ConfigSynapseID, std::vector<unsigned int, std::allocator<unsigned int> > > > >::find, "C++: std::map<pystorm::bddriver::bdpars::ConfigSynapseID, std::vector<unsigned int, std::allocator<unsigned int> >, std::less<pystorm::bddriver::bdpars::ConfigSynapseID>, std::allocator<std::pair<const pystorm::bddriver::bdpars::ConfigSynapseID, std::vector<unsigned int, std::allocator<unsigned int> > > > >::find(const pystorm::bddriver::bdpars::ConfigSynapseID &) --> struct std::_Rb_tree_iterator<struct std::pair<const pystorm::bddriver::bdpars::ConfigSynapseID, class std::vector<unsigned int, class std::allocator<unsigned int> > > >", pybind11::arg("__x"));
//		cl.def("find", (struct std::_Rb_tree_const_iterator<struct std::pair<const pystorm::bddriver::bdpars::ConfigSynapseID, class std::vector<unsigned int, class std::allocator<unsigned int> > > > (std::map<pystorm::bddriver::bdpars::ConfigSynapseID,std::vector<unsigned int, std::allocator<unsigned int> >,std::less<pystorm::bddriver::bdpars::ConfigSynapseID>,std::allocator<std::pair<const pystorm::bddriver::bdpars::ConfigSynapseID, std::vector<unsigned int, std::allocator<unsigned int> > > >>::*)(const pystorm::bddriver::bdpars::ConfigSynapseID &) const) &std::map<pystorm::bddriver::bdpars::ConfigSynapseID, std::vector<unsigned int, std::allocator<unsigned int> >, std::less<pystorm::bddriver::bdpars::ConfigSynapseID>, std::allocator<std::pair<const pystorm::bddriver::bdpars::ConfigSynapseID, std::vector<unsigned int, std::allocator<unsigned int> > > > >::find, "C++: std::map<pystorm::bddriver::bdpars::ConfigSynapseID, std::vector<unsigned int, std::allocator<unsigned int> >, std::less<pystorm::bddriver::bdpars::ConfigSynapseID>, std::allocator<std::pair<const pystorm::bddriver::bdpars::ConfigSynapseID, std::vector<unsigned int, std::allocator<unsigned int> > > > >::find(const pystorm::bddriver::bdpars::ConfigSynapseID &) const --> struct std::_Rb_tree_const_iterator<struct std::pair<const pystorm::bddriver::bdpars::ConfigSynapseID, class std::vector<unsigned int, class std::allocator<unsigned int> > > >", pybind11::arg("__x"));
//		cl.def("count", (unsigned long (std::map<pystorm::bddriver::bdpars::ConfigSynapseID,std::vector<unsigned int, std::allocator<unsigned int> >,std::less<pystorm::bddriver::bdpars::ConfigSynapseID>,std::allocator<std::pair<const pystorm::bddriver::bdpars::ConfigSynapseID, std::vector<unsigned int, std::allocator<unsigned int> > > >>::*)(const pystorm::bddriver::bdpars::ConfigSynapseID &) const) &std::map<pystorm::bddriver::bdpars::ConfigSynapseID, std::vector<unsigned int, std::allocator<unsigned int> >, std::less<pystorm::bddriver::bdpars::ConfigSynapseID>, std::allocator<std::pair<const pystorm::bddriver::bdpars::ConfigSynapseID, std::vector<unsigned int, std::allocator<unsigned int> > > > >::count, "C++: std::map<pystorm::bddriver::bdpars::ConfigSynapseID, std::vector<unsigned int, std::allocator<unsigned int> >, std::less<pystorm::bddriver::bdpars::ConfigSynapseID>, std::allocator<std::pair<const pystorm::bddriver::bdpars::ConfigSynapseID, std::vector<unsigned int, std::allocator<unsigned int> > > > >::count(const pystorm::bddriver::bdpars::ConfigSynapseID &) const --> unsigned long", pybind11::arg("__x"));
//		cl.def("lower_bound", (struct std::_Rb_tree_iterator<struct std::pair<const pystorm::bddriver::bdpars::ConfigSynapseID, class std::vector<unsigned int, class std::allocator<unsigned int> > > > (std::map<pystorm::bddriver::bdpars::ConfigSynapseID,std::vector<unsigned int, std::allocator<unsigned int> >,std::less<pystorm::bddriver::bdpars::ConfigSynapseID>,std::allocator<std::pair<const pystorm::bddriver::bdpars::ConfigSynapseID, std::vector<unsigned int, std::allocator<unsigned int> > > >>::*)(const pystorm::bddriver::bdpars::ConfigSynapseID &)) &std::map<pystorm::bddriver::bdpars::ConfigSynapseID, std::vector<unsigned int, std::allocator<unsigned int> >, std::less<pystorm::bddriver::bdpars::ConfigSynapseID>, std::allocator<std::pair<const pystorm::bddriver::bdpars::ConfigSynapseID, std::vector<unsigned int, std::allocator<unsigned int> > > > >::lower_bound, "C++: std::map<pystorm::bddriver::bdpars::ConfigSynapseID, std::vector<unsigned int, std::allocator<unsigned int> >, std::less<pystorm::bddriver::bdpars::ConfigSynapseID>, std::allocator<std::pair<const pystorm::bddriver::bdpars::ConfigSynapseID, std::vector<unsigned int, std::allocator<unsigned int> > > > >::lower_bound(const pystorm::bddriver::bdpars::ConfigSynapseID &) --> struct std::_Rb_tree_iterator<struct std::pair<const pystorm::bddriver::bdpars::ConfigSynapseID, class std::vector<unsigned int, class std::allocator<unsigned int> > > >", pybind11::arg("__x"));
//		cl.def("lower_bound", (struct std::_Rb_tree_const_iterator<struct std::pair<const pystorm::bddriver::bdpars::ConfigSynapseID, class std::vector<unsigned int, class std::allocator<unsigned int> > > > (std::map<pystorm::bddriver::bdpars::ConfigSynapseID,std::vector<unsigned int, std::allocator<unsigned int> >,std::less<pystorm::bddriver::bdpars::ConfigSynapseID>,std::allocator<std::pair<const pystorm::bddriver::bdpars::ConfigSynapseID, std::vector<unsigned int, std::allocator<unsigned int> > > >>::*)(const pystorm::bddriver::bdpars::ConfigSynapseID &) const) &std::map<pystorm::bddriver::bdpars::ConfigSynapseID, std::vector<unsigned int, std::allocator<unsigned int> >, std::less<pystorm::bddriver::bdpars::ConfigSynapseID>, std::allocator<std::pair<const pystorm::bddriver::bdpars::ConfigSynapseID, std::vector<unsigned int, std::allocator<unsigned int> > > > >::lower_bound, "C++: std::map<pystorm::bddriver::bdpars::ConfigSynapseID, std::vector<unsigned int, std::allocator<unsigned int> >, std::less<pystorm::bddriver::bdpars::ConfigSynapseID>, std::allocator<std::pair<const pystorm::bddriver::bdpars::ConfigSynapseID, std::vector<unsigned int, std::allocator<unsigned int> > > > >::lower_bound(const pystorm::bddriver::bdpars::ConfigSynapseID &) const --> struct std::_Rb_tree_const_iterator<struct std::pair<const pystorm::bddriver::bdpars::ConfigSynapseID, class std::vector<unsigned int, class std::allocator<unsigned int> > > >", pybind11::arg("__x"));
//		cl.def("upper_bound", (struct std::_Rb_tree_iterator<struct std::pair<const pystorm::bddriver::bdpars::ConfigSynapseID, class std::vector<unsigned int, class std::allocator<unsigned int> > > > (std::map<pystorm::bddriver::bdpars::ConfigSynapseID,std::vector<unsigned int, std::allocator<unsigned int> >,std::less<pystorm::bddriver::bdpars::ConfigSynapseID>,std::allocator<std::pair<const pystorm::bddriver::bdpars::ConfigSynapseID, std::vector<unsigned int, std::allocator<unsigned int> > > >>::*)(const pystorm::bddriver::bdpars::ConfigSynapseID &)) &std::map<pystorm::bddriver::bdpars::ConfigSynapseID, std::vector<unsigned int, std::allocator<unsigned int> >, std::less<pystorm::bddriver::bdpars::ConfigSynapseID>, std::allocator<std::pair<const pystorm::bddriver::bdpars::ConfigSynapseID, std::vector<unsigned int, std::allocator<unsigned int> > > > >::upper_bound, "C++: std::map<pystorm::bddriver::bdpars::ConfigSynapseID, std::vector<unsigned int, std::allocator<unsigned int> >, std::less<pystorm::bddriver::bdpars::ConfigSynapseID>, std::allocator<std::pair<const pystorm::bddriver::bdpars::ConfigSynapseID, std::vector<unsigned int, std::allocator<unsigned int> > > > >::upper_bound(const pystorm::bddriver::bdpars::ConfigSynapseID &) --> struct std::_Rb_tree_iterator<struct std::pair<const pystorm::bddriver::bdpars::ConfigSynapseID, class std::vector<unsigned int, class std::allocator<unsigned int> > > >", pybind11::arg("__x"));
//		cl.def("upper_bound", (struct std::_Rb_tree_const_iterator<struct std::pair<const pystorm::bddriver::bdpars::ConfigSynapseID, class std::vector<unsigned int, class std::allocator<unsigned int> > > > (std::map<pystorm::bddriver::bdpars::ConfigSynapseID,std::vector<unsigned int, std::allocator<unsigned int> >,std::less<pystorm::bddriver::bdpars::ConfigSynapseID>,std::allocator<std::pair<const pystorm::bddriver::bdpars::ConfigSynapseID, std::vector<unsigned int, std::allocator<unsigned int> > > >>::*)(const pystorm::bddriver::bdpars::ConfigSynapseID &) const) &std::map<pystorm::bddriver::bdpars::ConfigSynapseID, std::vector<unsigned int, std::allocator<unsigned int> >, std::less<pystorm::bddriver::bdpars::ConfigSynapseID>, std::allocator<std::pair<const pystorm::bddriver::bdpars::ConfigSynapseID, std::vector<unsigned int, std::allocator<unsigned int> > > > >::upper_bound, "C++: std::map<pystorm::bddriver::bdpars::ConfigSynapseID, std::vector<unsigned int, std::allocator<unsigned int> >, std::less<pystorm::bddriver::bdpars::ConfigSynapseID>, std::allocator<std::pair<const pystorm::bddriver::bdpars::ConfigSynapseID, std::vector<unsigned int, std::allocator<unsigned int> > > > >::upper_bound(const pystorm::bddriver::bdpars::ConfigSynapseID &) const --> struct std::_Rb_tree_const_iterator<struct std::pair<const pystorm::bddriver::bdpars::ConfigSynapseID, class std::vector<unsigned int, class std::allocator<unsigned int> > > >", pybind11::arg("__x"));
//		cl.def("equal_range", (struct std::pair<struct std::_Rb_tree_iterator<struct std::pair<const pystorm::bddriver::bdpars::ConfigSynapseID, class std::vector<unsigned int, class std::allocator<unsigned int> > > >, struct std::_Rb_tree_iterator<struct std::pair<const pystorm::bddriver::bdpars::ConfigSynapseID, class std::vector<unsigned int, class std::allocator<unsigned int> > > > > (std::map<pystorm::bddriver::bdpars::ConfigSynapseID,std::vector<unsigned int, std::allocator<unsigned int> >,std::less<pystorm::bddriver::bdpars::ConfigSynapseID>,std::allocator<std::pair<const pystorm::bddriver::bdpars::ConfigSynapseID, std::vector<unsigned int, std::allocator<unsigned int> > > >>::*)(const pystorm::bddriver::bdpars::ConfigSynapseID &)) &std::map<pystorm::bddriver::bdpars::ConfigSynapseID, std::vector<unsigned int, std::allocator<unsigned int> >, std::less<pystorm::bddriver::bdpars::ConfigSynapseID>, std::allocator<std::pair<const pystorm::bddriver::bdpars::ConfigSynapseID, std::vector<unsigned int, std::allocator<unsigned int> > > > >::equal_range, "C++: std::map<pystorm::bddriver::bdpars::ConfigSynapseID, std::vector<unsigned int, std::allocator<unsigned int> >, std::less<pystorm::bddriver::bdpars::ConfigSynapseID>, std::allocator<std::pair<const pystorm::bddriver::bdpars::ConfigSynapseID, std::vector<unsigned int, std::allocator<unsigned int> > > > >::equal_range(const pystorm::bddriver::bdpars::ConfigSynapseID &) --> struct std::pair<struct std::_Rb_tree_iterator<struct std::pair<const pystorm::bddriver::bdpars::ConfigSynapseID, class std::vector<unsigned int, class std::allocator<unsigned int> > > >, struct std::_Rb_tree_iterator<struct std::pair<const pystorm::bddriver::bdpars::ConfigSynapseID, class std::vector<unsigned int, class std::allocator<unsigned int> > > > >", pybind11::arg("__x"));
//		cl.def("equal_range", (struct std::pair<struct std::_Rb_tree_const_iterator<struct std::pair<const pystorm::bddriver::bdpars::ConfigSynapseID, class std::vector<unsigned int, class std::allocator<unsigned int> > > >, struct std::_Rb_tree_const_iterator<struct std::pair<const pystorm::bddriver::bdpars::ConfigSynapseID, class std::vector<unsigned int, class std::allocator<unsigned int> > > > > (std::map<pystorm::bddriver::bdpars::ConfigSynapseID,std::vector<unsigned int, std::allocator<unsigned int> >,std::less<pystorm::bddriver::bdpars::ConfigSynapseID>,std::allocator<std::pair<const pystorm::bddriver::bdpars::ConfigSynapseID, std::vector<unsigned int, std::allocator<unsigned int> > > >>::*)(const pystorm::bddriver::bdpars::ConfigSynapseID &) const) &std::map<pystorm::bddriver::bdpars::ConfigSynapseID, std::vector<unsigned int, std::allocator<unsigned int> >, std::less<pystorm::bddriver::bdpars::ConfigSynapseID>, std::allocator<std::pair<const pystorm::bddriver::bdpars::ConfigSynapseID, std::vector<unsigned int, std::allocator<unsigned int> > > > >::equal_range, "C++: std::map<pystorm::bddriver::bdpars::ConfigSynapseID, std::vector<unsigned int, std::allocator<unsigned int> >, std::less<pystorm::bddriver::bdpars::ConfigSynapseID>, std::allocator<std::pair<const pystorm::bddriver::bdpars::ConfigSynapseID, std::vector<unsigned int, std::allocator<unsigned int> > > > >::equal_range(const pystorm::bddriver::bdpars::ConfigSynapseID &) const --> struct std::pair<struct std::_Rb_tree_const_iterator<struct std::pair<const pystorm::bddriver::bdpars::ConfigSynapseID, class std::vector<unsigned int, class std::allocator<unsigned int> > > >, struct std::_Rb_tree_const_iterator<struct std::pair<const pystorm::bddriver::bdpars::ConfigSynapseID, class std::vector<unsigned int, class std::allocator<unsigned int> > > > >", pybind11::arg("__x"));
//	}
//	{ // std::map file:bits/stl_map.h line:96
//		pybind11::class_<std::map<pystorm::bddriver::bdpars::DiffusorCutLocationId,std::vector<unsigned int, std::allocator<unsigned int> >,std::less<pystorm::bddriver::bdpars::DiffusorCutLocationId>,std::allocator<std::pair<const pystorm::bddriver::bdpars::DiffusorCutLocationId, std::vector<unsigned int, std::allocator<unsigned int> > > >>, std::shared_ptr<std::map<pystorm::bddriver::bdpars::DiffusorCutLocationId,std::vector<unsigned int, std::allocator<unsigned int> >,std::less<pystorm::bddriver::bdpars::DiffusorCutLocationId>,std::allocator<std::pair<const pystorm::bddriver::bdpars::DiffusorCutLocationId, std::vector<unsigned int, std::allocator<unsigned int> > > >>>> cl(M("std"), "map_pystorm_bddriver_bdpars_DiffusorCutLocationId_std_vector_unsigned_int_std_allocator_unsigned_int_std_less_pystorm_bddriver_bdpars_DiffusorCutLocationId_std_allocator_std_pair_const_pystorm_bddriver_bdpars_DiffusorCutLocationId_std_vector_unsigned_int_std_allocator_unsigned_int_t", "");
//		pybind11::handle cl_type = cl;

//		cl.def(pybind11::init<>());

//		cl.def("__init__", [](std::map<pystorm::bddriver::bdpars::DiffusorCutLocationId,std::vector<unsigned int, std::allocator<unsigned int> >,std::less<pystorm::bddriver::bdpars::DiffusorCutLocationId>,std::allocator<std::pair<const pystorm::bddriver::bdpars::DiffusorCutLocationId, std::vector<unsigned int, std::allocator<unsigned int> > > >> *self_, const struct std::less<pystorm::bddriver::bdpars::DiffusorCutLocationId> & a0) { new (self_) std::map<pystorm::bddriver::bdpars::DiffusorCutLocationId,std::vector<unsigned int, std::allocator<unsigned int> >,std::less<pystorm::bddriver::bdpars::DiffusorCutLocationId>,std::allocator<std::pair<const pystorm::bddriver::bdpars::DiffusorCutLocationId, std::vector<unsigned int, std::allocator<unsigned int> > > >>(a0); }, "doc");
//		cl.def(pybind11::init<const struct std::less<pystorm::bddriver::bdpars::DiffusorCutLocationId> &, const class std::allocator<struct std::pair<const pystorm::bddriver::bdpars::DiffusorCutLocationId, class std::vector<unsigned int, class std::allocator<unsigned int> > > > &>(), pybind11::arg("__comp"), pybind11::arg("__a"));

//		cl.def(pybind11::init<const class std::map<pystorm::bddriver::bdpars::DiffusorCutLocationId, class std::vector<unsigned int, class std::allocator<unsigned int> >, struct std::less<pystorm::bddriver::bdpars::DiffusorCutLocationId>, class std::allocator<struct std::pair<const pystorm::bddriver::bdpars::DiffusorCutLocationId, class std::vector<unsigned int, class std::allocator<unsigned int> > > > > &>(), pybind11::arg("__x"));

//		cl.def("__init__", [](std::map<pystorm::bddriver::bdpars::DiffusorCutLocationId,std::vector<unsigned int, std::allocator<unsigned int> >,std::less<pystorm::bddriver::bdpars::DiffusorCutLocationId>,std::allocator<std::pair<const pystorm::bddriver::bdpars::DiffusorCutLocationId, std::vector<unsigned int, std::allocator<unsigned int> > > >> *self_, class std::initializer_list<struct std::pair<const pystorm::bddriver::bdpars::DiffusorCutLocationId, class std::vector<unsigned int, class std::allocator<unsigned int> > > >  const &a0) { new (self_) std::map<pystorm::bddriver::bdpars::DiffusorCutLocationId,std::vector<unsigned int, std::allocator<unsigned int> >,std::less<pystorm::bddriver::bdpars::DiffusorCutLocationId>,std::allocator<std::pair<const pystorm::bddriver::bdpars::DiffusorCutLocationId, std::vector<unsigned int, std::allocator<unsigned int> > > >>(a0); }, "doc");
//		cl.def("__init__", [](std::map<pystorm::bddriver::bdpars::DiffusorCutLocationId,std::vector<unsigned int, std::allocator<unsigned int> >,std::less<pystorm::bddriver::bdpars::DiffusorCutLocationId>,std::allocator<std::pair<const pystorm::bddriver::bdpars::DiffusorCutLocationId, std::vector<unsigned int, std::allocator<unsigned int> > > >> *self_, class std::initializer_list<struct std::pair<const pystorm::bddriver::bdpars::DiffusorCutLocationId, class std::vector<unsigned int, class std::allocator<unsigned int> > > >  const &a0, const struct std::less<pystorm::bddriver::bdpars::DiffusorCutLocationId> & a1) { new (self_) std::map<pystorm::bddriver::bdpars::DiffusorCutLocationId,std::vector<unsigned int, std::allocator<unsigned int> >,std::less<pystorm::bddriver::bdpars::DiffusorCutLocationId>,std::allocator<std::pair<const pystorm::bddriver::bdpars::DiffusorCutLocationId, std::vector<unsigned int, std::allocator<unsigned int> > > >>(a0, a1); }, "doc");
//		cl.def(pybind11::init<class std::initializer_list<struct std::pair<const pystorm::bddriver::bdpars::DiffusorCutLocationId, class std::vector<unsigned int, class std::allocator<unsigned int> > > >, const struct std::less<pystorm::bddriver::bdpars::DiffusorCutLocationId> &, const class std::allocator<struct std::pair<const pystorm::bddriver::bdpars::DiffusorCutLocationId, class std::vector<unsigned int, class std::allocator<unsigned int> > > > &>(), pybind11::arg("__l"), pybind11::arg("__comp"), pybind11::arg("__a"));

//		cl.def(pybind11::init<const class std::allocator<struct std::pair<const pystorm::bddriver::bdpars::DiffusorCutLocationId, class std::vector<unsigned int, class std::allocator<unsigned int> > > > &>(), pybind11::arg("__a"));

//		cl.def(pybind11::init<const class std::map<pystorm::bddriver::bdpars::DiffusorCutLocationId, class std::vector<unsigned int, class std::allocator<unsigned int> >, struct std::less<pystorm::bddriver::bdpars::DiffusorCutLocationId>, class std::allocator<struct std::pair<const pystorm::bddriver::bdpars::DiffusorCutLocationId, class std::vector<unsigned int, class std::allocator<unsigned int> > > > > &, const class std::allocator<struct std::pair<const pystorm::bddriver::bdpars::DiffusorCutLocationId, class std::vector<unsigned int, class std::allocator<unsigned int> > > > &>(), pybind11::arg("__m"), pybind11::arg("__a"));

//		cl.def(pybind11::init<class std::initializer_list<struct std::pair<const pystorm::bddriver::bdpars::DiffusorCutLocationId, class std::vector<unsigned int, class std::allocator<unsigned int> > > >, const class std::allocator<struct std::pair<const pystorm::bddriver::bdpars::DiffusorCutLocationId, class std::vector<unsigned int, class std::allocator<unsigned int> > > > &>(), pybind11::arg("__l"), pybind11::arg("__a"));

//		cl.def("assign", (class std::map<pystorm::bddriver::bdpars::DiffusorCutLocationId, class std::vector<unsigned int, class std::allocator<unsigned int> >, struct std::less<pystorm::bddriver::bdpars::DiffusorCutLocationId>, class std::allocator<struct std::pair<const pystorm::bddriver::bdpars::DiffusorCutLocationId, class std::vector<unsigned int, class std::allocator<unsigned int> > > > > & (std::map<pystorm::bddriver::bdpars::DiffusorCutLocationId,std::vector<unsigned int, std::allocator<unsigned int> >,std::less<pystorm::bddriver::bdpars::DiffusorCutLocationId>,std::allocator<std::pair<const pystorm::bddriver::bdpars::DiffusorCutLocationId, std::vector<unsigned int, std::allocator<unsigned int> > > >>::*)(const class std::map<pystorm::bddriver::bdpars::DiffusorCutLocationId, class std::vector<unsigned int, class std::allocator<unsigned int> >, struct std::less<pystorm::bddriver::bdpars::DiffusorCutLocationId>, class std::allocator<struct std::pair<const pystorm::bddriver::bdpars::DiffusorCutLocationId, class std::vector<unsigned int, class std::allocator<unsigned int> > > > > &)) &std::map<pystorm::bddriver::bdpars::DiffusorCutLocationId, std::vector<unsigned int, std::allocator<unsigned int> >, std::less<pystorm::bddriver::bdpars::DiffusorCutLocationId>, std::allocator<std::pair<const pystorm::bddriver::bdpars::DiffusorCutLocationId, std::vector<unsigned int, std::allocator<unsigned int> > > > >::operator=, "C++: std::map<pystorm::bddriver::bdpars::DiffusorCutLocationId, std::vector<unsigned int, std::allocator<unsigned int> >, std::less<pystorm::bddriver::bdpars::DiffusorCutLocationId>, std::allocator<std::pair<const pystorm::bddriver::bdpars::DiffusorCutLocationId, std::vector<unsigned int, std::allocator<unsigned int> > > > >::operator=(const class std::map<pystorm::bddriver::bdpars::DiffusorCutLocationId, class std::vector<unsigned int, class std::allocator<unsigned int> >, struct std::less<pystorm::bddriver::bdpars::DiffusorCutLocationId>, class std::allocator<struct std::pair<const pystorm::bddriver::bdpars::DiffusorCutLocationId, class std::vector<unsigned int, class std::allocator<unsigned int> > > > > &) --> class std::map<pystorm::bddriver::bdpars::DiffusorCutLocationId, class std::vector<unsigned int, class std::allocator<unsigned int> >, struct std::less<pystorm::bddriver::bdpars::DiffusorCutLocationId>, class std::allocator<struct std::pair<const pystorm::bddriver::bdpars::DiffusorCutLocationId, class std::vector<unsigned int, class std::allocator<unsigned int> > > > > &", pybind11::return_value_policy::automatic, pybind11::arg("__x"));
//		cl.def("assign", (class std::map<pystorm::bddriver::bdpars::DiffusorCutLocationId, class std::vector<unsigned int, class std::allocator<unsigned int> >, struct std::less<pystorm::bddriver::bdpars::DiffusorCutLocationId>, class std::allocator<struct std::pair<const pystorm::bddriver::bdpars::DiffusorCutLocationId, class std::vector<unsigned int, class std::allocator<unsigned int> > > > > & (std::map<pystorm::bddriver::bdpars::DiffusorCutLocationId,std::vector<unsigned int, std::allocator<unsigned int> >,std::less<pystorm::bddriver::bdpars::DiffusorCutLocationId>,std::allocator<std::pair<const pystorm::bddriver::bdpars::DiffusorCutLocationId, std::vector<unsigned int, std::allocator<unsigned int> > > >>::*)(class std::initializer_list<struct std::pair<const pystorm::bddriver::bdpars::DiffusorCutLocationId, class std::vector<unsigned int, class std::allocator<unsigned int> > > >)) &std::map<pystorm::bddriver::bdpars::DiffusorCutLocationId, std::vector<unsigned int, std::allocator<unsigned int> >, std::less<pystorm::bddriver::bdpars::DiffusorCutLocationId>, std::allocator<std::pair<const pystorm::bddriver::bdpars::DiffusorCutLocationId, std::vector<unsigned int, std::allocator<unsigned int> > > > >::operator=, "C++: std::map<pystorm::bddriver::bdpars::DiffusorCutLocationId, std::vector<unsigned int, std::allocator<unsigned int> >, std::less<pystorm::bddriver::bdpars::DiffusorCutLocationId>, std::allocator<std::pair<const pystorm::bddriver::bdpars::DiffusorCutLocationId, std::vector<unsigned int, std::allocator<unsigned int> > > > >::operator=(class std::initializer_list<struct std::pair<const pystorm::bddriver::bdpars::DiffusorCutLocationId, class std::vector<unsigned int, class std::allocator<unsigned int> > > >) --> class std::map<pystorm::bddriver::bdpars::DiffusorCutLocationId, class std::vector<unsigned int, class std::allocator<unsigned int> >, struct std::less<pystorm::bddriver::bdpars::DiffusorCutLocationId>, class std::allocator<struct std::pair<const pystorm::bddriver::bdpars::DiffusorCutLocationId, class std::vector<unsigned int, class std::allocator<unsigned int> > > > > &", pybind11::return_value_policy::automatic, pybind11::arg("__l"));
//		cl.def("get_allocator", (class std::allocator<struct std::pair<const pystorm::bddriver::bdpars::DiffusorCutLocationId, class std::vector<unsigned int, class std::allocator<unsigned int> > > > (std::map<pystorm::bddriver::bdpars::DiffusorCutLocationId,std::vector<unsigned int, std::allocator<unsigned int> >,std::less<pystorm::bddriver::bdpars::DiffusorCutLocationId>,std::allocator<std::pair<const pystorm::bddriver::bdpars::DiffusorCutLocationId, std::vector<unsigned int, std::allocator<unsigned int> > > >>::*)() const) &std::map<pystorm::bddriver::bdpars::DiffusorCutLocationId, std::vector<unsigned int, std::allocator<unsigned int> >, std::less<pystorm::bddriver::bdpars::DiffusorCutLocationId>, std::allocator<std::pair<const pystorm::bddriver::bdpars::DiffusorCutLocationId, std::vector<unsigned int, std::allocator<unsigned int> > > > >::get_allocator, "C++: std::map<pystorm::bddriver::bdpars::DiffusorCutLocationId, std::vector<unsigned int, std::allocator<unsigned int> >, std::less<pystorm::bddriver::bdpars::DiffusorCutLocationId>, std::allocator<std::pair<const pystorm::bddriver::bdpars::DiffusorCutLocationId, std::vector<unsigned int, std::allocator<unsigned int> > > > >::get_allocator() const --> class std::allocator<struct std::pair<const pystorm::bddriver::bdpars::DiffusorCutLocationId, class std::vector<unsigned int, class std::allocator<unsigned int> > > >");
//		cl.def("begin", (struct std::_Rb_tree_iterator<struct std::pair<const pystorm::bddriver::bdpars::DiffusorCutLocationId, class std::vector<unsigned int, class std::allocator<unsigned int> > > > (std::map<pystorm::bddriver::bdpars::DiffusorCutLocationId,std::vector<unsigned int, std::allocator<unsigned int> >,std::less<pystorm::bddriver::bdpars::DiffusorCutLocationId>,std::allocator<std::pair<const pystorm::bddriver::bdpars::DiffusorCutLocationId, std::vector<unsigned int, std::allocator<unsigned int> > > >>::*)()) &std::map<pystorm::bddriver::bdpars::DiffusorCutLocationId, std::vector<unsigned int, std::allocator<unsigned int> >, std::less<pystorm::bddriver::bdpars::DiffusorCutLocationId>, std::allocator<std::pair<const pystorm::bddriver::bdpars::DiffusorCutLocationId, std::vector<unsigned int, std::allocator<unsigned int> > > > >::begin, "C++: std::map<pystorm::bddriver::bdpars::DiffusorCutLocationId, std::vector<unsigned int, std::allocator<unsigned int> >, std::less<pystorm::bddriver::bdpars::DiffusorCutLocationId>, std::allocator<std::pair<const pystorm::bddriver::bdpars::DiffusorCutLocationId, std::vector<unsigned int, std::allocator<unsigned int> > > > >::begin() --> struct std::_Rb_tree_iterator<struct std::pair<const pystorm::bddriver::bdpars::DiffusorCutLocationId, class std::vector<unsigned int, class std::allocator<unsigned int> > > >");
//		cl.def("begin", (struct std::_Rb_tree_const_iterator<struct std::pair<const pystorm::bddriver::bdpars::DiffusorCutLocationId, class std::vector<unsigned int, class std::allocator<unsigned int> > > > (std::map<pystorm::bddriver::bdpars::DiffusorCutLocationId,std::vector<unsigned int, std::allocator<unsigned int> >,std::less<pystorm::bddriver::bdpars::DiffusorCutLocationId>,std::allocator<std::pair<const pystorm::bddriver::bdpars::DiffusorCutLocationId, std::vector<unsigned int, std::allocator<unsigned int> > > >>::*)() const) &std::map<pystorm::bddriver::bdpars::DiffusorCutLocationId, std::vector<unsigned int, std::allocator<unsigned int> >, std::less<pystorm::bddriver::bdpars::DiffusorCutLocationId>, std::allocator<std::pair<const pystorm::bddriver::bdpars::DiffusorCutLocationId, std::vector<unsigned int, std::allocator<unsigned int> > > > >::begin, "C++: std::map<pystorm::bddriver::bdpars::DiffusorCutLocationId, std::vector<unsigned int, std::allocator<unsigned int> >, std::less<pystorm::bddriver::bdpars::DiffusorCutLocationId>, std::allocator<std::pair<const pystorm::bddriver::bdpars::DiffusorCutLocationId, std::vector<unsigned int, std::allocator<unsigned int> > > > >::begin() const --> struct std::_Rb_tree_const_iterator<struct std::pair<const pystorm::bddriver::bdpars::DiffusorCutLocationId, class std::vector<unsigned int, class std::allocator<unsigned int> > > >");
//		cl.def("end", (struct std::_Rb_tree_iterator<struct std::pair<const pystorm::bddriver::bdpars::DiffusorCutLocationId, class std::vector<unsigned int, class std::allocator<unsigned int> > > > (std::map<pystorm::bddriver::bdpars::DiffusorCutLocationId,std::vector<unsigned int, std::allocator<unsigned int> >,std::less<pystorm::bddriver::bdpars::DiffusorCutLocationId>,std::allocator<std::pair<const pystorm::bddriver::bdpars::DiffusorCutLocationId, std::vector<unsigned int, std::allocator<unsigned int> > > >>::*)()) &std::map<pystorm::bddriver::bdpars::DiffusorCutLocationId, std::vector<unsigned int, std::allocator<unsigned int> >, std::less<pystorm::bddriver::bdpars::DiffusorCutLocationId>, std::allocator<std::pair<const pystorm::bddriver::bdpars::DiffusorCutLocationId, std::vector<unsigned int, std::allocator<unsigned int> > > > >::end, "C++: std::map<pystorm::bddriver::bdpars::DiffusorCutLocationId, std::vector<unsigned int, std::allocator<unsigned int> >, std::less<pystorm::bddriver::bdpars::DiffusorCutLocationId>, std::allocator<std::pair<const pystorm::bddriver::bdpars::DiffusorCutLocationId, std::vector<unsigned int, std::allocator<unsigned int> > > > >::end() --> struct std::_Rb_tree_iterator<struct std::pair<const pystorm::bddriver::bdpars::DiffusorCutLocationId, class std::vector<unsigned int, class std::allocator<unsigned int> > > >");
//		cl.def("end", (struct std::_Rb_tree_const_iterator<struct std::pair<const pystorm::bddriver::bdpars::DiffusorCutLocationId, class std::vector<unsigned int, class std::allocator<unsigned int> > > > (std::map<pystorm::bddriver::bdpars::DiffusorCutLocationId,std::vector<unsigned int, std::allocator<unsigned int> >,std::less<pystorm::bddriver::bdpars::DiffusorCutLocationId>,std::allocator<std::pair<const pystorm::bddriver::bdpars::DiffusorCutLocationId, std::vector<unsigned int, std::allocator<unsigned int> > > >>::*)() const) &std::map<pystorm::bddriver::bdpars::DiffusorCutLocationId, std::vector<unsigned int, std::allocator<unsigned int> >, std::less<pystorm::bddriver::bdpars::DiffusorCutLocationId>, std::allocator<std::pair<const pystorm::bddriver::bdpars::DiffusorCutLocationId, std::vector<unsigned int, std::allocator<unsigned int> > > > >::end, "C++: std::map<pystorm::bddriver::bdpars::DiffusorCutLocationId, std::vector<unsigned int, std::allocator<unsigned int> >, std::less<pystorm::bddriver::bdpars::DiffusorCutLocationId>, std::allocator<std::pair<const pystorm::bddriver::bdpars::DiffusorCutLocationId, std::vector<unsigned int, std::allocator<unsigned int> > > > >::end() const --> struct std::_Rb_tree_const_iterator<struct std::pair<const pystorm::bddriver::bdpars::DiffusorCutLocationId, class std::vector<unsigned int, class std::allocator<unsigned int> > > >");
//		cl.def("cbegin", (struct std::_Rb_tree_const_iterator<struct std::pair<const pystorm::bddriver::bdpars::DiffusorCutLocationId, class std::vector<unsigned int, class std::allocator<unsigned int> > > > (std::map<pystorm::bddriver::bdpars::DiffusorCutLocationId,std::vector<unsigned int, std::allocator<unsigned int> >,std::less<pystorm::bddriver::bdpars::DiffusorCutLocationId>,std::allocator<std::pair<const pystorm::bddriver::bdpars::DiffusorCutLocationId, std::vector<unsigned int, std::allocator<unsigned int> > > >>::*)() const) &std::map<pystorm::bddriver::bdpars::DiffusorCutLocationId, std::vector<unsigned int, std::allocator<unsigned int> >, std::less<pystorm::bddriver::bdpars::DiffusorCutLocationId>, std::allocator<std::pair<const pystorm::bddriver::bdpars::DiffusorCutLocationId, std::vector<unsigned int, std::allocator<unsigned int> > > > >::cbegin, "C++: std::map<pystorm::bddriver::bdpars::DiffusorCutLocationId, std::vector<unsigned int, std::allocator<unsigned int> >, std::less<pystorm::bddriver::bdpars::DiffusorCutLocationId>, std::allocator<std::pair<const pystorm::bddriver::bdpars::DiffusorCutLocationId, std::vector<unsigned int, std::allocator<unsigned int> > > > >::cbegin() const --> struct std::_Rb_tree_const_iterator<struct std::pair<const pystorm::bddriver::bdpars::DiffusorCutLocationId, class std::vector<unsigned int, class std::allocator<unsigned int> > > >");
//		cl.def("cend", (struct std::_Rb_tree_const_iterator<struct std::pair<const pystorm::bddriver::bdpars::DiffusorCutLocationId, class std::vector<unsigned int, class std::allocator<unsigned int> > > > (std::map<pystorm::bddriver::bdpars::DiffusorCutLocationId,std::vector<unsigned int, std::allocator<unsigned int> >,std::less<pystorm::bddriver::bdpars::DiffusorCutLocationId>,std::allocator<std::pair<const pystorm::bddriver::bdpars::DiffusorCutLocationId, std::vector<unsigned int, std::allocator<unsigned int> > > >>::*)() const) &std::map<pystorm::bddriver::bdpars::DiffusorCutLocationId, std::vector<unsigned int, std::allocator<unsigned int> >, std::less<pystorm::bddriver::bdpars::DiffusorCutLocationId>, std::allocator<std::pair<const pystorm::bddriver::bdpars::DiffusorCutLocationId, std::vector<unsigned int, std::allocator<unsigned int> > > > >::cend, "C++: std::map<pystorm::bddriver::bdpars::DiffusorCutLocationId, std::vector<unsigned int, std::allocator<unsigned int> >, std::less<pystorm::bddriver::bdpars::DiffusorCutLocationId>, std::allocator<std::pair<const pystorm::bddriver::bdpars::DiffusorCutLocationId, std::vector<unsigned int, std::allocator<unsigned int> > > > >::cend() const --> struct std::_Rb_tree_const_iterator<struct std::pair<const pystorm::bddriver::bdpars::DiffusorCutLocationId, class std::vector<unsigned int, class std::allocator<unsigned int> > > >");
//		cl.def("empty", (bool (std::map<pystorm::bddriver::bdpars::DiffusorCutLocationId,std::vector<unsigned int, std::allocator<unsigned int> >,std::less<pystorm::bddriver::bdpars::DiffusorCutLocationId>,std::allocator<std::pair<const pystorm::bddriver::bdpars::DiffusorCutLocationId, std::vector<unsigned int, std::allocator<unsigned int> > > >>::*)() const) &std::map<pystorm::bddriver::bdpars::DiffusorCutLocationId, std::vector<unsigned int, std::allocator<unsigned int> >, std::less<pystorm::bddriver::bdpars::DiffusorCutLocationId>, std::allocator<std::pair<const pystorm::bddriver::bdpars::DiffusorCutLocationId, std::vector<unsigned int, std::allocator<unsigned int> > > > >::empty, "C++: std::map<pystorm::bddriver::bdpars::DiffusorCutLocationId, std::vector<unsigned int, std::allocator<unsigned int> >, std::less<pystorm::bddriver::bdpars::DiffusorCutLocationId>, std::allocator<std::pair<const pystorm::bddriver::bdpars::DiffusorCutLocationId, std::vector<unsigned int, std::allocator<unsigned int> > > > >::empty() const --> bool");
//		cl.def("size", (unsigned long (std::map<pystorm::bddriver::bdpars::DiffusorCutLocationId,std::vector<unsigned int, std::allocator<unsigned int> >,std::less<pystorm::bddriver::bdpars::DiffusorCutLocationId>,std::allocator<std::pair<const pystorm::bddriver::bdpars::DiffusorCutLocationId, std::vector<unsigned int, std::allocator<unsigned int> > > >>::*)() const) &std::map<pystorm::bddriver::bdpars::DiffusorCutLocationId, std::vector<unsigned int, std::allocator<unsigned int> >, std::less<pystorm::bddriver::bdpars::DiffusorCutLocationId>, std::allocator<std::pair<const pystorm::bddriver::bdpars::DiffusorCutLocationId, std::vector<unsigned int, std::allocator<unsigned int> > > > >::size, "C++: std::map<pystorm::bddriver::bdpars::DiffusorCutLocationId, std::vector<unsigned int, std::allocator<unsigned int> >, std::less<pystorm::bddriver::bdpars::DiffusorCutLocationId>, std::allocator<std::pair<const pystorm::bddriver::bdpars::DiffusorCutLocationId, std::vector<unsigned int, std::allocator<unsigned int> > > > >::size() const --> unsigned long");
//		cl.def("max_size", (unsigned long (std::map<pystorm::bddriver::bdpars::DiffusorCutLocationId,std::vector<unsigned int, std::allocator<unsigned int> >,std::less<pystorm::bddriver::bdpars::DiffusorCutLocationId>,std::allocator<std::pair<const pystorm::bddriver::bdpars::DiffusorCutLocationId, std::vector<unsigned int, std::allocator<unsigned int> > > >>::*)() const) &std::map<pystorm::bddriver::bdpars::DiffusorCutLocationId, std::vector<unsigned int, std::allocator<unsigned int> >, std::less<pystorm::bddriver::bdpars::DiffusorCutLocationId>, std::allocator<std::pair<const pystorm::bddriver::bdpars::DiffusorCutLocationId, std::vector<unsigned int, std::allocator<unsigned int> > > > >::max_size, "C++: std::map<pystorm::bddriver::bdpars::DiffusorCutLocationId, std::vector<unsigned int, std::allocator<unsigned int> >, std::less<pystorm::bddriver::bdpars::DiffusorCutLocationId>, std::allocator<std::pair<const pystorm::bddriver::bdpars::DiffusorCutLocationId, std::vector<unsigned int, std::allocator<unsigned int> > > > >::max_size() const --> unsigned long");
//		cl.def("__getitem__", (class std::vector<unsigned int, class std::allocator<unsigned int> > & (std::map<pystorm::bddriver::bdpars::DiffusorCutLocationId,std::vector<unsigned int, std::allocator<unsigned int> >,std::less<pystorm::bddriver::bdpars::DiffusorCutLocationId>,std::allocator<std::pair<const pystorm::bddriver::bdpars::DiffusorCutLocationId, std::vector<unsigned int, std::allocator<unsigned int> > > >>::*)(const pystorm::bddriver::bdpars::DiffusorCutLocationId &)) &std::map<pystorm::bddriver::bdpars::DiffusorCutLocationId, std::vector<unsigned int, std::allocator<unsigned int> >, std::less<pystorm::bddriver::bdpars::DiffusorCutLocationId>, std::allocator<std::pair<const pystorm::bddriver::bdpars::DiffusorCutLocationId, std::vector<unsigned int, std::allocator<unsigned int> > > > >::operator[], "C++: std::map<pystorm::bddriver::bdpars::DiffusorCutLocationId, std::vector<unsigned int, std::allocator<unsigned int> >, std::less<pystorm::bddriver::bdpars::DiffusorCutLocationId>, std::allocator<std::pair<const pystorm::bddriver::bdpars::DiffusorCutLocationId, std::vector<unsigned int, std::allocator<unsigned int> > > > >::operator[](const pystorm::bddriver::bdpars::DiffusorCutLocationId &) --> class std::vector<unsigned int, class std::allocator<unsigned int> > &", pybind11::return_value_policy::automatic, pybind11::arg("__k"));
//		cl.def("at", (class std::vector<unsigned int, class std::allocator<unsigned int> > & (std::map<pystorm::bddriver::bdpars::DiffusorCutLocationId,std::vector<unsigned int, std::allocator<unsigned int> >,std::less<pystorm::bddriver::bdpars::DiffusorCutLocationId>,std::allocator<std::pair<const pystorm::bddriver::bdpars::DiffusorCutLocationId, std::vector<unsigned int, std::allocator<unsigned int> > > >>::*)(const pystorm::bddriver::bdpars::DiffusorCutLocationId &)) &std::map<pystorm::bddriver::bdpars::DiffusorCutLocationId, std::vector<unsigned int, std::allocator<unsigned int> >, std::less<pystorm::bddriver::bdpars::DiffusorCutLocationId>, std::allocator<std::pair<const pystorm::bddriver::bdpars::DiffusorCutLocationId, std::vector<unsigned int, std::allocator<unsigned int> > > > >::at, "C++: std::map<pystorm::bddriver::bdpars::DiffusorCutLocationId, std::vector<unsigned int, std::allocator<unsigned int> >, std::less<pystorm::bddriver::bdpars::DiffusorCutLocationId>, std::allocator<std::pair<const pystorm::bddriver::bdpars::DiffusorCutLocationId, std::vector<unsigned int, std::allocator<unsigned int> > > > >::at(const pystorm::bddriver::bdpars::DiffusorCutLocationId &) --> class std::vector<unsigned int, class std::allocator<unsigned int> > &", pybind11::return_value_policy::automatic, pybind11::arg("__k"));
//		cl.def("at", (const class std::vector<unsigned int, class std::allocator<unsigned int> > & (std::map<pystorm::bddriver::bdpars::DiffusorCutLocationId,std::vector<unsigned int, std::allocator<unsigned int> >,std::less<pystorm::bddriver::bdpars::DiffusorCutLocationId>,std::allocator<std::pair<const pystorm::bddriver::bdpars::DiffusorCutLocationId, std::vector<unsigned int, std::allocator<unsigned int> > > >>::*)(const pystorm::bddriver::bdpars::DiffusorCutLocationId &) const) &std::map<pystorm::bddriver::bdpars::DiffusorCutLocationId, std::vector<unsigned int, std::allocator<unsigned int> >, std::less<pystorm::bddriver::bdpars::DiffusorCutLocationId>, std::allocator<std::pair<const pystorm::bddriver::bdpars::DiffusorCutLocationId, std::vector<unsigned int, std::allocator<unsigned int> > > > >::at, "C++: std::map<pystorm::bddriver::bdpars::DiffusorCutLocationId, std::vector<unsigned int, std::allocator<unsigned int> >, std::less<pystorm::bddriver::bdpars::DiffusorCutLocationId>, std::allocator<std::pair<const pystorm::bddriver::bdpars::DiffusorCutLocationId, std::vector<unsigned int, std::allocator<unsigned int> > > > >::at(const pystorm::bddriver::bdpars::DiffusorCutLocationId &) const --> const class std::vector<unsigned int, class std::allocator<unsigned int> > &", pybind11::return_value_policy::automatic, pybind11::arg("__k"));
//		cl.def("insert", (struct std::pair<struct std::_Rb_tree_iterator<struct std::pair<const pystorm::bddriver::bdpars::DiffusorCutLocationId, class std::vector<unsigned int, class std::allocator<unsigned int> > > >, bool> (std::map<pystorm::bddriver::bdpars::DiffusorCutLocationId,std::vector<unsigned int, std::allocator<unsigned int> >,std::less<pystorm::bddriver::bdpars::DiffusorCutLocationId>,std::allocator<std::pair<const pystorm::bddriver::bdpars::DiffusorCutLocationId, std::vector<unsigned int, std::allocator<unsigned int> > > >>::*)(const struct std::pair<const pystorm::bddriver::bdpars::DiffusorCutLocationId, class std::vector<unsigned int, class std::allocator<unsigned int> > > &)) &std::map<pystorm::bddriver::bdpars::DiffusorCutLocationId, std::vector<unsigned int, std::allocator<unsigned int> >, std::less<pystorm::bddriver::bdpars::DiffusorCutLocationId>, std::allocator<std::pair<const pystorm::bddriver::bdpars::DiffusorCutLocationId, std::vector<unsigned int, std::allocator<unsigned int> > > > >::insert, "C++: std::map<pystorm::bddriver::bdpars::DiffusorCutLocationId, std::vector<unsigned int, std::allocator<unsigned int> >, std::less<pystorm::bddriver::bdpars::DiffusorCutLocationId>, std::allocator<std::pair<const pystorm::bddriver::bdpars::DiffusorCutLocationId, std::vector<unsigned int, std::allocator<unsigned int> > > > >::insert(const struct std::pair<const pystorm::bddriver::bdpars::DiffusorCutLocationId, class std::vector<unsigned int, class std::allocator<unsigned int> > > &) --> struct std::pair<struct std::_Rb_tree_iterator<struct std::pair<const pystorm::bddriver::bdpars::DiffusorCutLocationId, class std::vector<unsigned int, class std::allocator<unsigned int> > > >, bool>", pybind11::arg("__x"));
//		cl.def("insert", (void (std::map<pystorm::bddriver::bdpars::DiffusorCutLocationId,std::vector<unsigned int, std::allocator<unsigned int> >,std::less<pystorm::bddriver::bdpars::DiffusorCutLocationId>,std::allocator<std::pair<const pystorm::bddriver::bdpars::DiffusorCutLocationId, std::vector<unsigned int, std::allocator<unsigned int> > > >>::*)(class std::initializer_list<struct std::pair<const pystorm::bddriver::bdpars::DiffusorCutLocationId, class std::vector<unsigned int, class std::allocator<unsigned int> > > >)) &std::map<pystorm::bddriver::bdpars::DiffusorCutLocationId, std::vector<unsigned int, std::allocator<unsigned int> >, std::less<pystorm::bddriver::bdpars::DiffusorCutLocationId>, std::allocator<std::pair<const pystorm::bddriver::bdpars::DiffusorCutLocationId, std::vector<unsigned int, std::allocator<unsigned int> > > > >::insert, "C++: std::map<pystorm::bddriver::bdpars::DiffusorCutLocationId, std::vector<unsigned int, std::allocator<unsigned int> >, std::less<pystorm::bddriver::bdpars::DiffusorCutLocationId>, std::allocator<std::pair<const pystorm::bddriver::bdpars::DiffusorCutLocationId, std::vector<unsigned int, std::allocator<unsigned int> > > > >::insert(class std::initializer_list<struct std::pair<const pystorm::bddriver::bdpars::DiffusorCutLocationId, class std::vector<unsigned int, class std::allocator<unsigned int> > > >) --> void", pybind11::arg("__list"));
//		cl.def("insert", (struct std::_Rb_tree_iterator<struct std::pair<const pystorm::bddriver::bdpars::DiffusorCutLocationId, class std::vector<unsigned int, class std::allocator<unsigned int> > > > (std::map<pystorm::bddriver::bdpars::DiffusorCutLocationId,std::vector<unsigned int, std::allocator<unsigned int> >,std::less<pystorm::bddriver::bdpars::DiffusorCutLocationId>,std::allocator<std::pair<const pystorm::bddriver::bdpars::DiffusorCutLocationId, std::vector<unsigned int, std::allocator<unsigned int> > > >>::*)(struct std::_Rb_tree_const_iterator<struct std::pair<const pystorm::bddriver::bdpars::DiffusorCutLocationId, class std::vector<unsigned int, class std::allocator<unsigned int> > > >, const struct std::pair<const pystorm::bddriver::bdpars::DiffusorCutLocationId, class std::vector<unsigned int, class std::allocator<unsigned int> > > &)) &std::map<pystorm::bddriver::bdpars::DiffusorCutLocationId, std::vector<unsigned int, std::allocator<unsigned int> >, std::less<pystorm::bddriver::bdpars::DiffusorCutLocationId>, std::allocator<std::pair<const pystorm::bddriver::bdpars::DiffusorCutLocationId, std::vector<unsigned int, std::allocator<unsigned int> > > > >::insert, "C++: std::map<pystorm::bddriver::bdpars::DiffusorCutLocationId, std::vector<unsigned int, std::allocator<unsigned int> >, std::less<pystorm::bddriver::bdpars::DiffusorCutLocationId>, std::allocator<std::pair<const pystorm::bddriver::bdpars::DiffusorCutLocationId, std::vector<unsigned int, std::allocator<unsigned int> > > > >::insert(struct std::_Rb_tree_const_iterator<struct std::pair<const pystorm::bddriver::bdpars::DiffusorCutLocationId, class std::vector<unsigned int, class std::allocator<unsigned int> > > >, const struct std::pair<const pystorm::bddriver::bdpars::DiffusorCutLocationId, class std::vector<unsigned int, class std::allocator<unsigned int> > > &) --> struct std::_Rb_tree_iterator<struct std::pair<const pystorm::bddriver::bdpars::DiffusorCutLocationId, class std::vector<unsigned int, class std::allocator<unsigned int> > > >", pybind11::arg("__position"), pybind11::arg("__x"));
//		cl.def("erase", (struct std::_Rb_tree_iterator<struct std::pair<const pystorm::bddriver::bdpars::DiffusorCutLocationId, class std::vector<unsigned int, class std::allocator<unsigned int> > > > (std::map<pystorm::bddriver::bdpars::DiffusorCutLocationId,std::vector<unsigned int, std::allocator<unsigned int> >,std::less<pystorm::bddriver::bdpars::DiffusorCutLocationId>,std::allocator<std::pair<const pystorm::bddriver::bdpars::DiffusorCutLocationId, std::vector<unsigned int, std::allocator<unsigned int> > > >>::*)(struct std::_Rb_tree_const_iterator<struct std::pair<const pystorm::bddriver::bdpars::DiffusorCutLocationId, class std::vector<unsigned int, class std::allocator<unsigned int> > > >)) &std::map<pystorm::bddriver::bdpars::DiffusorCutLocationId, std::vector<unsigned int, std::allocator<unsigned int> >, std::less<pystorm::bddriver::bdpars::DiffusorCutLocationId>, std::allocator<std::pair<const pystorm::bddriver::bdpars::DiffusorCutLocationId, std::vector<unsigned int, std::allocator<unsigned int> > > > >::erase, "C++: std::map<pystorm::bddriver::bdpars::DiffusorCutLocationId, std::vector<unsigned int, std::allocator<unsigned int> >, std::less<pystorm::bddriver::bdpars::DiffusorCutLocationId>, std::allocator<std::pair<const pystorm::bddriver::bdpars::DiffusorCutLocationId, std::vector<unsigned int, std::allocator<unsigned int> > > > >::erase(struct std::_Rb_tree_const_iterator<struct std::pair<const pystorm::bddriver::bdpars::DiffusorCutLocationId, class std::vector<unsigned int, class std::allocator<unsigned int> > > >) --> struct std::_Rb_tree_iterator<struct std::pair<const pystorm::bddriver::bdpars::DiffusorCutLocationId, class std::vector<unsigned int, class std::allocator<unsigned int> > > >", pybind11::arg("__position"));
//		cl.def("erase", (struct std::_Rb_tree_iterator<struct std::pair<const pystorm::bddriver::bdpars::DiffusorCutLocationId, class std::vector<unsigned int, class std::allocator<unsigned int> > > > (std::map<pystorm::bddriver::bdpars::DiffusorCutLocationId,std::vector<unsigned int, std::allocator<unsigned int> >,std::less<pystorm::bddriver::bdpars::DiffusorCutLocationId>,std::allocator<std::pair<const pystorm::bddriver::bdpars::DiffusorCutLocationId, std::vector<unsigned int, std::allocator<unsigned int> > > >>::*)(struct std::_Rb_tree_iterator<struct std::pair<const pystorm::bddriver::bdpars::DiffusorCutLocationId, class std::vector<unsigned int, class std::allocator<unsigned int> > > >)) &std::map<pystorm::bddriver::bdpars::DiffusorCutLocationId, std::vector<unsigned int, std::allocator<unsigned int> >, std::less<pystorm::bddriver::bdpars::DiffusorCutLocationId>, std::allocator<std::pair<const pystorm::bddriver::bdpars::DiffusorCutLocationId, std::vector<unsigned int, std::allocator<unsigned int> > > > >::erase, "C++: std::map<pystorm::bddriver::bdpars::DiffusorCutLocationId, std::vector<unsigned int, std::allocator<unsigned int> >, std::less<pystorm::bddriver::bdpars::DiffusorCutLocationId>, std::allocator<std::pair<const pystorm::bddriver::bdpars::DiffusorCutLocationId, std::vector<unsigned int, std::allocator<unsigned int> > > > >::erase(struct std::_Rb_tree_iterator<struct std::pair<const pystorm::bddriver::bdpars::DiffusorCutLocationId, class std::vector<unsigned int, class std::allocator<unsigned int> > > >) --> struct std::_Rb_tree_iterator<struct std::pair<const pystorm::bddriver::bdpars::DiffusorCutLocationId, class std::vector<unsigned int, class std::allocator<unsigned int> > > >", pybind11::arg("__position"));
//		cl.def("erase", (unsigned long (std::map<pystorm::bddriver::bdpars::DiffusorCutLocationId,std::vector<unsigned int, std::allocator<unsigned int> >,std::less<pystorm::bddriver::bdpars::DiffusorCutLocationId>,std::allocator<std::pair<const pystorm::bddriver::bdpars::DiffusorCutLocationId, std::vector<unsigned int, std::allocator<unsigned int> > > >>::*)(const pystorm::bddriver::bdpars::DiffusorCutLocationId &)) &std::map<pystorm::bddriver::bdpars::DiffusorCutLocationId, std::vector<unsigned int, std::allocator<unsigned int> >, std::less<pystorm::bddriver::bdpars::DiffusorCutLocationId>, std::allocator<std::pair<const pystorm::bddriver::bdpars::DiffusorCutLocationId, std::vector<unsigned int, std::allocator<unsigned int> > > > >::erase, "C++: std::map<pystorm::bddriver::bdpars::DiffusorCutLocationId, std::vector<unsigned int, std::allocator<unsigned int> >, std::less<pystorm::bddriver::bdpars::DiffusorCutLocationId>, std::allocator<std::pair<const pystorm::bddriver::bdpars::DiffusorCutLocationId, std::vector<unsigned int, std::allocator<unsigned int> > > > >::erase(const pystorm::bddriver::bdpars::DiffusorCutLocationId &) --> unsigned long", pybind11::arg("__x"));
//		cl.def("erase", (struct std::_Rb_tree_iterator<struct std::pair<const pystorm::bddriver::bdpars::DiffusorCutLocationId, class std::vector<unsigned int, class std::allocator<unsigned int> > > > (std::map<pystorm::bddriver::bdpars::DiffusorCutLocationId,std::vector<unsigned int, std::allocator<unsigned int> >,std::less<pystorm::bddriver::bdpars::DiffusorCutLocationId>,std::allocator<std::pair<const pystorm::bddriver::bdpars::DiffusorCutLocationId, std::vector<unsigned int, std::allocator<unsigned int> > > >>::*)(struct std::_Rb_tree_const_iterator<struct std::pair<const pystorm::bddriver::bdpars::DiffusorCutLocationId, class std::vector<unsigned int, class std::allocator<unsigned int> > > >, struct std::_Rb_tree_const_iterator<struct std::pair<const pystorm::bddriver::bdpars::DiffusorCutLocationId, class std::vector<unsigned int, class std::allocator<unsigned int> > > >)) &std::map<pystorm::bddriver::bdpars::DiffusorCutLocationId, std::vector<unsigned int, std::allocator<unsigned int> >, std::less<pystorm::bddriver::bdpars::DiffusorCutLocationId>, std::allocator<std::pair<const pystorm::bddriver::bdpars::DiffusorCutLocationId, std::vector<unsigned int, std::allocator<unsigned int> > > > >::erase, "C++: std::map<pystorm::bddriver::bdpars::DiffusorCutLocationId, std::vector<unsigned int, std::allocator<unsigned int> >, std::less<pystorm::bddriver::bdpars::DiffusorCutLocationId>, std::allocator<std::pair<const pystorm::bddriver::bdpars::DiffusorCutLocationId, std::vector<unsigned int, std::allocator<unsigned int> > > > >::erase(struct std::_Rb_tree_const_iterator<struct std::pair<const pystorm::bddriver::bdpars::DiffusorCutLocationId, class std::vector<unsigned int, class std::allocator<unsigned int> > > >, struct std::_Rb_tree_const_iterator<struct std::pair<const pystorm::bddriver::bdpars::DiffusorCutLocationId, class std::vector<unsigned int, class std::allocator<unsigned int> > > >) --> struct std::_Rb_tree_iterator<struct std::pair<const pystorm::bddriver::bdpars::DiffusorCutLocationId, class std::vector<unsigned int, class std::allocator<unsigned int> > > >", pybind11::arg("__first"), pybind11::arg("__last"));
//		cl.def("swap", (void (std::map<pystorm::bddriver::bdpars::DiffusorCutLocationId,std::vector<unsigned int, std::allocator<unsigned int> >,std::less<pystorm::bddriver::bdpars::DiffusorCutLocationId>,std::allocator<std::pair<const pystorm::bddriver::bdpars::DiffusorCutLocationId, std::vector<unsigned int, std::allocator<unsigned int> > > >>::*)(class std::map<pystorm::bddriver::bdpars::DiffusorCutLocationId, class std::vector<unsigned int, class std::allocator<unsigned int> >, struct std::less<pystorm::bddriver::bdpars::DiffusorCutLocationId>, class std::allocator<struct std::pair<const pystorm::bddriver::bdpars::DiffusorCutLocationId, class std::vector<unsigned int, class std::allocator<unsigned int> > > > > &)) &std::map<pystorm::bddriver::bdpars::DiffusorCutLocationId, std::vector<unsigned int, std::allocator<unsigned int> >, std::less<pystorm::bddriver::bdpars::DiffusorCutLocationId>, std::allocator<std::pair<const pystorm::bddriver::bdpars::DiffusorCutLocationId, std::vector<unsigned int, std::allocator<unsigned int> > > > >::swap, "C++: std::map<pystorm::bddriver::bdpars::DiffusorCutLocationId, std::vector<unsigned int, std::allocator<unsigned int> >, std::less<pystorm::bddriver::bdpars::DiffusorCutLocationId>, std::allocator<std::pair<const pystorm::bddriver::bdpars::DiffusorCutLocationId, std::vector<unsigned int, std::allocator<unsigned int> > > > >::swap(class std::map<pystorm::bddriver::bdpars::DiffusorCutLocationId, class std::vector<unsigned int, class std::allocator<unsigned int> >, struct std::less<pystorm::bddriver::bdpars::DiffusorCutLocationId>, class std::allocator<struct std::pair<const pystorm::bddriver::bdpars::DiffusorCutLocationId, class std::vector<unsigned int, class std::allocator<unsigned int> > > > > &) --> void", pybind11::arg("__x"));
//		cl.def("clear", (void (std::map<pystorm::bddriver::bdpars::DiffusorCutLocationId,std::vector<unsigned int, std::allocator<unsigned int> >,std::less<pystorm::bddriver::bdpars::DiffusorCutLocationId>,std::allocator<std::pair<const pystorm::bddriver::bdpars::DiffusorCutLocationId, std::vector<unsigned int, std::allocator<unsigned int> > > >>::*)()) &std::map<pystorm::bddriver::bdpars::DiffusorCutLocationId, std::vector<unsigned int, std::allocator<unsigned int> >, std::less<pystorm::bddriver::bdpars::DiffusorCutLocationId>, std::allocator<std::pair<const pystorm::bddriver::bdpars::DiffusorCutLocationId, std::vector<unsigned int, std::allocator<unsigned int> > > > >::clear, "C++: std::map<pystorm::bddriver::bdpars::DiffusorCutLocationId, std::vector<unsigned int, std::allocator<unsigned int> >, std::less<pystorm::bddriver::bdpars::DiffusorCutLocationId>, std::allocator<std::pair<const pystorm::bddriver::bdpars::DiffusorCutLocationId, std::vector<unsigned int, std::allocator<unsigned int> > > > >::clear() --> void");
//		cl.def("key_comp", (struct std::less<pystorm::bddriver::bdpars::DiffusorCutLocationId> (std::map<pystorm::bddriver::bdpars::DiffusorCutLocationId,std::vector<unsigned int, std::allocator<unsigned int> >,std::less<pystorm::bddriver::bdpars::DiffusorCutLocationId>,std::allocator<std::pair<const pystorm::bddriver::bdpars::DiffusorCutLocationId, std::vector<unsigned int, std::allocator<unsigned int> > > >>::*)() const) &std::map<pystorm::bddriver::bdpars::DiffusorCutLocationId, std::vector<unsigned int, std::allocator<unsigned int> >, std::less<pystorm::bddriver::bdpars::DiffusorCutLocationId>, std::allocator<std::pair<const pystorm::bddriver::bdpars::DiffusorCutLocationId, std::vector<unsigned int, std::allocator<unsigned int> > > > >::key_comp, "C++: std::map<pystorm::bddriver::bdpars::DiffusorCutLocationId, std::vector<unsigned int, std::allocator<unsigned int> >, std::less<pystorm::bddriver::bdpars::DiffusorCutLocationId>, std::allocator<std::pair<const pystorm::bddriver::bdpars::DiffusorCutLocationId, std::vector<unsigned int, std::allocator<unsigned int> > > > >::key_comp() const --> struct std::less<pystorm::bddriver::bdpars::DiffusorCutLocationId>");
//		cl.def("find", (struct std::_Rb_tree_iterator<struct std::pair<const pystorm::bddriver::bdpars::DiffusorCutLocationId, class std::vector<unsigned int, class std::allocator<unsigned int> > > > (std::map<pystorm::bddriver::bdpars::DiffusorCutLocationId,std::vector<unsigned int, std::allocator<unsigned int> >,std::less<pystorm::bddriver::bdpars::DiffusorCutLocationId>,std::allocator<std::pair<const pystorm::bddriver::bdpars::DiffusorCutLocationId, std::vector<unsigned int, std::allocator<unsigned int> > > >>::*)(const pystorm::bddriver::bdpars::DiffusorCutLocationId &)) &std::map<pystorm::bddriver::bdpars::DiffusorCutLocationId, std::vector<unsigned int, std::allocator<unsigned int> >, std::less<pystorm::bddriver::bdpars::DiffusorCutLocationId>, std::allocator<std::pair<const pystorm::bddriver::bdpars::DiffusorCutLocationId, std::vector<unsigned int, std::allocator<unsigned int> > > > >::find, "C++: std::map<pystorm::bddriver::bdpars::DiffusorCutLocationId, std::vector<unsigned int, std::allocator<unsigned int> >, std::less<pystorm::bddriver::bdpars::DiffusorCutLocationId>, std::allocator<std::pair<const pystorm::bddriver::bdpars::DiffusorCutLocationId, std::vector<unsigned int, std::allocator<unsigned int> > > > >::find(const pystorm::bddriver::bdpars::DiffusorCutLocationId &) --> struct std::_Rb_tree_iterator<struct std::pair<const pystorm::bddriver::bdpars::DiffusorCutLocationId, class std::vector<unsigned int, class std::allocator<unsigned int> > > >", pybind11::arg("__x"));
//		cl.def("find", (struct std::_Rb_tree_const_iterator<struct std::pair<const pystorm::bddriver::bdpars::DiffusorCutLocationId, class std::vector<unsigned int, class std::allocator<unsigned int> > > > (std::map<pystorm::bddriver::bdpars::DiffusorCutLocationId,std::vector<unsigned int, std::allocator<unsigned int> >,std::less<pystorm::bddriver::bdpars::DiffusorCutLocationId>,std::allocator<std::pair<const pystorm::bddriver::bdpars::DiffusorCutLocationId, std::vector<unsigned int, std::allocator<unsigned int> > > >>::*)(const pystorm::bddriver::bdpars::DiffusorCutLocationId &) const) &std::map<pystorm::bddriver::bdpars::DiffusorCutLocationId, std::vector<unsigned int, std::allocator<unsigned int> >, std::less<pystorm::bddriver::bdpars::DiffusorCutLocationId>, std::allocator<std::pair<const pystorm::bddriver::bdpars::DiffusorCutLocationId, std::vector<unsigned int, std::allocator<unsigned int> > > > >::find, "C++: std::map<pystorm::bddriver::bdpars::DiffusorCutLocationId, std::vector<unsigned int, std::allocator<unsigned int> >, std::less<pystorm::bddriver::bdpars::DiffusorCutLocationId>, std::allocator<std::pair<const pystorm::bddriver::bdpars::DiffusorCutLocationId, std::vector<unsigned int, std::allocator<unsigned int> > > > >::find(const pystorm::bddriver::bdpars::DiffusorCutLocationId &) const --> struct std::_Rb_tree_const_iterator<struct std::pair<const pystorm::bddriver::bdpars::DiffusorCutLocationId, class std::vector<unsigned int, class std::allocator<unsigned int> > > >", pybind11::arg("__x"));
//		cl.def("count", (unsigned long (std::map<pystorm::bddriver::bdpars::DiffusorCutLocationId,std::vector<unsigned int, std::allocator<unsigned int> >,std::less<pystorm::bddriver::bdpars::DiffusorCutLocationId>,std::allocator<std::pair<const pystorm::bddriver::bdpars::DiffusorCutLocationId, std::vector<unsigned int, std::allocator<unsigned int> > > >>::*)(const pystorm::bddriver::bdpars::DiffusorCutLocationId &) const) &std::map<pystorm::bddriver::bdpars::DiffusorCutLocationId, std::vector<unsigned int, std::allocator<unsigned int> >, std::less<pystorm::bddriver::bdpars::DiffusorCutLocationId>, std::allocator<std::pair<const pystorm::bddriver::bdpars::DiffusorCutLocationId, std::vector<unsigned int, std::allocator<unsigned int> > > > >::count, "C++: std::map<pystorm::bddriver::bdpars::DiffusorCutLocationId, std::vector<unsigned int, std::allocator<unsigned int> >, std::less<pystorm::bddriver::bdpars::DiffusorCutLocationId>, std::allocator<std::pair<const pystorm::bddriver::bdpars::DiffusorCutLocationId, std::vector<unsigned int, std::allocator<unsigned int> > > > >::count(const pystorm::bddriver::bdpars::DiffusorCutLocationId &) const --> unsigned long", pybind11::arg("__x"));
//		cl.def("lower_bound", (struct std::_Rb_tree_iterator<struct std::pair<const pystorm::bddriver::bdpars::DiffusorCutLocationId, class std::vector<unsigned int, class std::allocator<unsigned int> > > > (std::map<pystorm::bddriver::bdpars::DiffusorCutLocationId,std::vector<unsigned int, std::allocator<unsigned int> >,std::less<pystorm::bddriver::bdpars::DiffusorCutLocationId>,std::allocator<std::pair<const pystorm::bddriver::bdpars::DiffusorCutLocationId, std::vector<unsigned int, std::allocator<unsigned int> > > >>::*)(const pystorm::bddriver::bdpars::DiffusorCutLocationId &)) &std::map<pystorm::bddriver::bdpars::DiffusorCutLocationId, std::vector<unsigned int, std::allocator<unsigned int> >, std::less<pystorm::bddriver::bdpars::DiffusorCutLocationId>, std::allocator<std::pair<const pystorm::bddriver::bdpars::DiffusorCutLocationId, std::vector<unsigned int, std::allocator<unsigned int> > > > >::lower_bound, "C++: std::map<pystorm::bddriver::bdpars::DiffusorCutLocationId, std::vector<unsigned int, std::allocator<unsigned int> >, std::less<pystorm::bddriver::bdpars::DiffusorCutLocationId>, std::allocator<std::pair<const pystorm::bddriver::bdpars::DiffusorCutLocationId, std::vector<unsigned int, std::allocator<unsigned int> > > > >::lower_bound(const pystorm::bddriver::bdpars::DiffusorCutLocationId &) --> struct std::_Rb_tree_iterator<struct std::pair<const pystorm::bddriver::bdpars::DiffusorCutLocationId, class std::vector<unsigned int, class std::allocator<unsigned int> > > >", pybind11::arg("__x"));
//		cl.def("lower_bound", (struct std::_Rb_tree_const_iterator<struct std::pair<const pystorm::bddriver::bdpars::DiffusorCutLocationId, class std::vector<unsigned int, class std::allocator<unsigned int> > > > (std::map<pystorm::bddriver::bdpars::DiffusorCutLocationId,std::vector<unsigned int, std::allocator<unsigned int> >,std::less<pystorm::bddriver::bdpars::DiffusorCutLocationId>,std::allocator<std::pair<const pystorm::bddriver::bdpars::DiffusorCutLocationId, std::vector<unsigned int, std::allocator<unsigned int> > > >>::*)(const pystorm::bddriver::bdpars::DiffusorCutLocationId &) const) &std::map<pystorm::bddriver::bdpars::DiffusorCutLocationId, std::vector<unsigned int, std::allocator<unsigned int> >, std::less<pystorm::bddriver::bdpars::DiffusorCutLocationId>, std::allocator<std::pair<const pystorm::bddriver::bdpars::DiffusorCutLocationId, std::vector<unsigned int, std::allocator<unsigned int> > > > >::lower_bound, "C++: std::map<pystorm::bddriver::bdpars::DiffusorCutLocationId, std::vector<unsigned int, std::allocator<unsigned int> >, std::less<pystorm::bddriver::bdpars::DiffusorCutLocationId>, std::allocator<std::pair<const pystorm::bddriver::bdpars::DiffusorCutLocationId, std::vector<unsigned int, std::allocator<unsigned int> > > > >::lower_bound(const pystorm::bddriver::bdpars::DiffusorCutLocationId &) const --> struct std::_Rb_tree_const_iterator<struct std::pair<const pystorm::bddriver::bdpars::DiffusorCutLocationId, class std::vector<unsigned int, class std::allocator<unsigned int> > > >", pybind11::arg("__x"));
//		cl.def("upper_bound", (struct std::_Rb_tree_iterator<struct std::pair<const pystorm::bddriver::bdpars::DiffusorCutLocationId, class std::vector<unsigned int, class std::allocator<unsigned int> > > > (std::map<pystorm::bddriver::bdpars::DiffusorCutLocationId,std::vector<unsigned int, std::allocator<unsigned int> >,std::less<pystorm::bddriver::bdpars::DiffusorCutLocationId>,std::allocator<std::pair<const pystorm::bddriver::bdpars::DiffusorCutLocationId, std::vector<unsigned int, std::allocator<unsigned int> > > >>::*)(const pystorm::bddriver::bdpars::DiffusorCutLocationId &)) &std::map<pystorm::bddriver::bdpars::DiffusorCutLocationId, std::vector<unsigned int, std::allocator<unsigned int> >, std::less<pystorm::bddriver::bdpars::DiffusorCutLocationId>, std::allocator<std::pair<const pystorm::bddriver::bdpars::DiffusorCutLocationId, std::vector<unsigned int, std::allocator<unsigned int> > > > >::upper_bound, "C++: std::map<pystorm::bddriver::bdpars::DiffusorCutLocationId, std::vector<unsigned int, std::allocator<unsigned int> >, std::less<pystorm::bddriver::bdpars::DiffusorCutLocationId>, std::allocator<std::pair<const pystorm::bddriver::bdpars::DiffusorCutLocationId, std::vector<unsigned int, std::allocator<unsigned int> > > > >::upper_bound(const pystorm::bddriver::bdpars::DiffusorCutLocationId &) --> struct std::_Rb_tree_iterator<struct std::pair<const pystorm::bddriver::bdpars::DiffusorCutLocationId, class std::vector<unsigned int, class std::allocator<unsigned int> > > >", pybind11::arg("__x"));
//		cl.def("upper_bound", (struct std::_Rb_tree_const_iterator<struct std::pair<const pystorm::bddriver::bdpars::DiffusorCutLocationId, class std::vector<unsigned int, class std::allocator<unsigned int> > > > (std::map<pystorm::bddriver::bdpars::DiffusorCutLocationId,std::vector<unsigned int, std::allocator<unsigned int> >,std::less<pystorm::bddriver::bdpars::DiffusorCutLocationId>,std::allocator<std::pair<const pystorm::bddriver::bdpars::DiffusorCutLocationId, std::vector<unsigned int, std::allocator<unsigned int> > > >>::*)(const pystorm::bddriver::bdpars::DiffusorCutLocationId &) const) &std::map<pystorm::bddriver::bdpars::DiffusorCutLocationId, std::vector<unsigned int, std::allocator<unsigned int> >, std::less<pystorm::bddriver::bdpars::DiffusorCutLocationId>, std::allocator<std::pair<const pystorm::bddriver::bdpars::DiffusorCutLocationId, std::vector<unsigned int, std::allocator<unsigned int> > > > >::upper_bound, "C++: std::map<pystorm::bddriver::bdpars::DiffusorCutLocationId, std::vector<unsigned int, std::allocator<unsigned int> >, std::less<pystorm::bddriver::bdpars::DiffusorCutLocationId>, std::allocator<std::pair<const pystorm::bddriver::bdpars::DiffusorCutLocationId, std::vector<unsigned int, std::allocator<unsigned int> > > > >::upper_bound(const pystorm::bddriver::bdpars::DiffusorCutLocationId &) const --> struct std::_Rb_tree_const_iterator<struct std::pair<const pystorm::bddriver::bdpars::DiffusorCutLocationId, class std::vector<unsigned int, class std::allocator<unsigned int> > > >", pybind11::arg("__x"));
//		cl.def("equal_range", (struct std::pair<struct std::_Rb_tree_iterator<struct std::pair<const pystorm::bddriver::bdpars::DiffusorCutLocationId, class std::vector<unsigned int, class std::allocator<unsigned int> > > >, struct std::_Rb_tree_iterator<struct std::pair<const pystorm::bddriver::bdpars::DiffusorCutLocationId, class std::vector<unsigned int, class std::allocator<unsigned int> > > > > (std::map<pystorm::bddriver::bdpars::DiffusorCutLocationId,std::vector<unsigned int, std::allocator<unsigned int> >,std::less<pystorm::bddriver::bdpars::DiffusorCutLocationId>,std::allocator<std::pair<const pystorm::bddriver::bdpars::DiffusorCutLocationId, std::vector<unsigned int, std::allocator<unsigned int> > > >>::*)(const pystorm::bddriver::bdpars::DiffusorCutLocationId &)) &std::map<pystorm::bddriver::bdpars::DiffusorCutLocationId, std::vector<unsigned int, std::allocator<unsigned int> >, std::less<pystorm::bddriver::bdpars::DiffusorCutLocationId>, std::allocator<std::pair<const pystorm::bddriver::bdpars::DiffusorCutLocationId, std::vector<unsigned int, std::allocator<unsigned int> > > > >::equal_range, "C++: std::map<pystorm::bddriver::bdpars::DiffusorCutLocationId, std::vector<unsigned int, std::allocator<unsigned int> >, std::less<pystorm::bddriver::bdpars::DiffusorCutLocationId>, std::allocator<std::pair<const pystorm::bddriver::bdpars::DiffusorCutLocationId, std::vector<unsigned int, std::allocator<unsigned int> > > > >::equal_range(const pystorm::bddriver::bdpars::DiffusorCutLocationId &) --> struct std::pair<struct std::_Rb_tree_iterator<struct std::pair<const pystorm::bddriver::bdpars::DiffusorCutLocationId, class std::vector<unsigned int, class std::allocator<unsigned int> > > >, struct std::_Rb_tree_iterator<struct std::pair<const pystorm::bddriver::bdpars::DiffusorCutLocationId, class std::vector<unsigned int, class std::allocator<unsigned int> > > > >", pybind11::arg("__x"));
//		cl.def("equal_range", (struct std::pair<struct std::_Rb_tree_const_iterator<struct std::pair<const pystorm::bddriver::bdpars::DiffusorCutLocationId, class std::vector<unsigned int, class std::allocator<unsigned int> > > >, struct std::_Rb_tree_const_iterator<struct std::pair<const pystorm::bddriver::bdpars::DiffusorCutLocationId, class std::vector<unsigned int, class std::allocator<unsigned int> > > > > (std::map<pystorm::bddriver::bdpars::DiffusorCutLocationId,std::vector<unsigned int, std::allocator<unsigned int> >,std::less<pystorm::bddriver::bdpars::DiffusorCutLocationId>,std::allocator<std::pair<const pystorm::bddriver::bdpars::DiffusorCutLocationId, std::vector<unsigned int, std::allocator<unsigned int> > > >>::*)(const pystorm::bddriver::bdpars::DiffusorCutLocationId &) const) &std::map<pystorm::bddriver::bdpars::DiffusorCutLocationId, std::vector<unsigned int, std::allocator<unsigned int> >, std::less<pystorm::bddriver::bdpars::DiffusorCutLocationId>, std::allocator<std::pair<const pystorm::bddriver::bdpars::DiffusorCutLocationId, std::vector<unsigned int, std::allocator<unsigned int> > > > >::equal_range, "C++: std::map<pystorm::bddriver::bdpars::DiffusorCutLocationId, std::vector<unsigned int, std::allocator<unsigned int> >, std::less<pystorm::bddriver::bdpars::DiffusorCutLocationId>, std::allocator<std::pair<const pystorm::bddriver::bdpars::DiffusorCutLocationId, std::vector<unsigned int, std::allocator<unsigned int> > > > >::equal_range(const pystorm::bddriver::bdpars::DiffusorCutLocationId &) const --> struct std::pair<struct std::_Rb_tree_const_iterator<struct std::pair<const pystorm::bddriver::bdpars::DiffusorCutLocationId, class std::vector<unsigned int, class std::allocator<unsigned int> > > >, struct std::_Rb_tree_const_iterator<struct std::pair<const pystorm::bddriver::bdpars::DiffusorCutLocationId, class std::vector<unsigned int, class std::allocator<unsigned int> > > > >", pybind11::arg("__x"));
//	}
//}


// File: unknown/unknown_2.cpp
#include <initializer_list>
#include <iterator>
#include <memory>
#include <sstream> // __str__
#include <vector>

#include <pybind11/pybind11.h>

#ifndef BINDER_PYBIND11_TYPE_CASTER
	#define BINDER_PYBIND11_TYPE_CASTER
	PYBIND11_DECLARE_HOLDER_TYPE(T, std::shared_ptr<T>);
	PYBIND11_DECLARE_HOLDER_TYPE(T, T*);
	PYBIND11_MAKE_OPAQUE(std::shared_ptr<void>);
#endif

void bind_unknown_unknown_2(std::function< pybind11::module &(std::string const &namespace_) > &M)
{
	// pystorm::bddriver::PackV(const class std::vector<unsigned long, class std::allocator<unsigned long> > &, const class std::vector<unsigned int, class std::allocator<unsigned int> > &) file: line:14
	M("pystorm::bddriver").def("PackV", (unsigned long (*)(const class std::vector<unsigned long, class std::allocator<unsigned long> > &, const class std::vector<unsigned int, class std::allocator<unsigned int> > &)) &pystorm::bddriver::PackV<unsigned long,unsigned long>, "C++: pystorm::bddriver::PackV(const class std::vector<unsigned long, class std::allocator<unsigned long> > &, const class std::vector<unsigned int, class std::allocator<unsigned int> > &) --> unsigned long", pybind11::arg("vals"), pybind11::arg("widths"));

	// pystorm::bddriver::PackV(const class std::vector<unsigned int, class std::allocator<unsigned int> > &, const class std::vector<unsigned int, class std::allocator<unsigned int> > &) file: line:14
	M("pystorm::bddriver").def("PackV", (unsigned int (*)(const class std::vector<unsigned int, class std::allocator<unsigned int> > &, const class std::vector<unsigned int, class std::allocator<unsigned int> > &)) &pystorm::bddriver::PackV<unsigned int,unsigned int>, "C++: pystorm::bddriver::PackV(const class std::vector<unsigned int, class std::allocator<unsigned int> > &, const class std::vector<unsigned int, class std::allocator<unsigned int> > &) --> unsigned int", pybind11::arg("vals"), pybind11::arg("widths"));

	// pystorm::bddriver::PackV(const class std::vector<unsigned int, class std::allocator<unsigned int> > &, const class std::vector<unsigned int, class std::allocator<unsigned int> > &) file: line:14
	M("pystorm::bddriver").def("PackV", (unsigned long (*)(const class std::vector<unsigned int, class std::allocator<unsigned int> > &, const class std::vector<unsigned int, class std::allocator<unsigned int> > &)) &pystorm::bddriver::PackV<unsigned int,unsigned long>, "C++: pystorm::bddriver::PackV(const class std::vector<unsigned int, class std::allocator<unsigned int> > &, const class std::vector<unsigned int, class std::allocator<unsigned int> > &) --> unsigned long", pybind11::arg("vals"), pybind11::arg("widths"));

	// pystorm::bddriver::Pack(const unsigned long *, const unsigned int *, unsigned int) file: line:17
	M("pystorm::bddriver").def("Pack", (unsigned long (*)(const unsigned long *, const unsigned int *, unsigned int)) &pystorm::bddriver::Pack<unsigned long,unsigned long>, "C++: pystorm::bddriver::Pack(const unsigned long *, const unsigned int *, unsigned int) --> unsigned long", pybind11::arg("vals"), pybind11::arg("widths"), pybind11::arg("num_fields"));

	// pystorm::bddriver::Pack(const unsigned int *, const unsigned int *, unsigned int) file: line:17
	M("pystorm::bddriver").def("Pack", (unsigned int (*)(const unsigned int *, const unsigned int *, unsigned int)) &pystorm::bddriver::Pack<unsigned int,unsigned int>, "C++: pystorm::bddriver::Pack(const unsigned int *, const unsigned int *, unsigned int) --> unsigned int", pybind11::arg("vals"), pybind11::arg("widths"), pybind11::arg("num_fields"));

	// pystorm::bddriver::Pack(const unsigned int *, const unsigned int *, unsigned int) file: line:17
	M("pystorm::bddriver").def("Pack", (unsigned long (*)(const unsigned int *, const unsigned int *, unsigned int)) &pystorm::bddriver::Pack<unsigned int,unsigned long>, "C++: pystorm::bddriver::Pack(const unsigned int *, const unsigned int *, unsigned int) --> unsigned long", pybind11::arg("vals"), pybind11::arg("widths"), pybind11::arg("num_fields"));

	// pystorm::bddriver::UnpackV(unsigned long, const class std::vector<unsigned int, class std::allocator<unsigned int> > &) file: line:21
	M("pystorm::bddriver").def("UnpackV", (class std::vector<unsigned long, class std::allocator<unsigned long> > (*)(unsigned long, const class std::vector<unsigned int, class std::allocator<unsigned int> > &)) &pystorm::bddriver::UnpackV<unsigned long,unsigned long>, "C++: pystorm::bddriver::UnpackV(unsigned long, const class std::vector<unsigned int, class std::allocator<unsigned int> > &) --> class std::vector<unsigned long, class std::allocator<unsigned long> >", pybind11::arg("val"), pybind11::arg("widths"));

	// pystorm::bddriver::UnpackV(unsigned long, const class std::vector<unsigned int, class std::allocator<unsigned int> > &) file: line:21
	M("pystorm::bddriver").def("UnpackV", (class std::vector<unsigned int, class std::allocator<unsigned int> > (*)(unsigned long, const class std::vector<unsigned int, class std::allocator<unsigned int> > &)) &pystorm::bddriver::UnpackV<unsigned long,unsigned int>, "C++: pystorm::bddriver::UnpackV(unsigned long, const class std::vector<unsigned int, class std::allocator<unsigned int> > &) --> class std::vector<unsigned int, class std::allocator<unsigned int> >", pybind11::arg("val"), pybind11::arg("widths"));

	// pystorm::bddriver::UnpackV(unsigned int, const class std::vector<unsigned int, class std::allocator<unsigned int> > &) file: line:21
	M("pystorm::bddriver").def("UnpackV", (class std::vector<unsigned int, class std::allocator<unsigned int> > (*)(unsigned int, const class std::vector<unsigned int, class std::allocator<unsigned int> > &)) &pystorm::bddriver::UnpackV<unsigned int,unsigned int>, "C++: pystorm::bddriver::UnpackV(unsigned int, const class std::vector<unsigned int, class std::allocator<unsigned int> > &) --> class std::vector<unsigned int, class std::allocator<unsigned int> >", pybind11::arg("val"), pybind11::arg("widths"));

	// pystorm::bddriver::Unpack(unsigned long, const unsigned int *, unsigned long *, unsigned int) file: line:24
	M("pystorm::bddriver").def("Unpack", (void (*)(unsigned long, const unsigned int *, unsigned long *, unsigned int)) &pystorm::bddriver::Unpack<unsigned long,unsigned long>, "C++: pystorm::bddriver::Unpack(unsigned long, const unsigned int *, unsigned long *, unsigned int) --> void", pybind11::arg("val"), pybind11::arg("widths"), pybind11::arg("unpacked_vals"), pybind11::arg("num_fields"));

	// pystorm::bddriver::Unpack(unsigned long, const unsigned int *, unsigned int *, unsigned int) file: line:24
	M("pystorm::bddriver").def("Unpack", (void (*)(unsigned long, const unsigned int *, unsigned int *, unsigned int)) &pystorm::bddriver::Unpack<unsigned long,unsigned int>, "C++: pystorm::bddriver::Unpack(unsigned long, const unsigned int *, unsigned int *, unsigned int) --> void", pybind11::arg("val"), pybind11::arg("widths"), pybind11::arg("unpacked_vals"), pybind11::arg("num_fields"));

	// pystorm::bddriver::Unpack(unsigned int, const unsigned int *, unsigned int *, unsigned int) file: line:24
	M("pystorm::bddriver").def("Unpack", (void (*)(unsigned int, const unsigned int *, unsigned int *, unsigned int)) &pystorm::bddriver::Unpack<unsigned int,unsigned int>, "C++: pystorm::bddriver::Unpack(unsigned int, const unsigned int *, unsigned int *, unsigned int) --> void", pybind11::arg("val"), pybind11::arg("widths"), pybind11::arg("unpacked_vals"), pybind11::arg("num_fields"));

	// pystorm::bddriver::MaxVal(unsigned int) file: line:57
	M("pystorm::bddriver").def("MaxVal", (unsigned long (*)(unsigned int)) &pystorm::bddriver::MaxVal, "C++: pystorm::bddriver::MaxVal(unsigned int) --> unsigned long", pybind11::arg("width"));

	{ // pystorm::bddriver::ToggleWord file: line:276
		pybind11::class_<pystorm::bddriver::ToggleWord, std::shared_ptr<pystorm::bddriver::ToggleWord>> cl(M("pystorm::bddriver"), "ToggleWord", "");
		pybind11::handle cl_type = cl;

		cl.def(pybind11::init<>());
		pybind11::enum_<pystorm::bddriver::ToggleWord::Field>(cl, "Field", "")
			.value("TRAFFIC_ENABLE", pystorm::bddriver::ToggleWord::Field::TRAFFIC_ENABLE)
			.value("DUMP_ENABLE", pystorm::bddriver::ToggleWord::Field::DUMP_ENABLE)
			.export_values();

	}
	{ // pystorm::bddriver::DACWord file: line:279
		pybind11::class_<pystorm::bddriver::DACWord, std::shared_ptr<pystorm::bddriver::DACWord>> cl(M("pystorm::bddriver"), "DACWord", "");
		pybind11::handle cl_type = cl;

		cl.def(pybind11::init<>());
		pybind11::enum_<pystorm::bddriver::DACWord::Field>(cl, "Field", "")
			.value("DAC_TO_ADC_CONN", pystorm::bddriver::DACWord::Field::DAC_TO_ADC_CONN)
			.value("DAC_VALUE", pystorm::bddriver::DACWord::Field::DAC_VALUE)
			.export_values();

	}
	{ // pystorm::bddriver::ADCWord file: line:282
		pybind11::class_<pystorm::bddriver::ADCWord, std::shared_ptr<pystorm::bddriver::ADCWord>> cl(M("pystorm::bddriver"), "ADCWord", "");
		pybind11::handle cl_type = cl;

		cl.def(pybind11::init<>());
		pybind11::enum_<pystorm::bddriver::ADCWord::Field>(cl, "Field", "")
			.value("ADC_SMALL_LARGE_CURRENT_0", pystorm::bddriver::ADCWord::Field::ADC_SMALL_LARGE_CURRENT_0)
			.value("ADC_SMALL_LARGE_CURRENT_1", pystorm::bddriver::ADCWord::Field::ADC_SMALL_LARGE_CURRENT_1)
			.value("ADC_OUTPUT_ENABLE", pystorm::bddriver::ADCWord::Field::ADC_OUTPUT_ENABLE)
			.export_values();

	}
	{ // pystorm::bddriver::DelayWord file: line:285
		pybind11::class_<pystorm::bddriver::DelayWord, std::shared_ptr<pystorm::bddriver::DelayWord>> cl(M("pystorm::bddriver"), "DelayWord", "");
		pybind11::handle cl_type = cl;

		cl.def(pybind11::init<>());
		pybind11::enum_<pystorm::bddriver::DelayWord::Field>(cl, "Field", "")
			.value("READ_DELAY", pystorm::bddriver::DelayWord::Field::READ_DELAY)
			.value("WRITE_DELAY", pystorm::bddriver::DelayWord::Field::WRITE_DELAY)
			.export_values();

	}
	{ // pystorm::bddriver::AMWord file: line:289
		pybind11::class_<pystorm::bddriver::AMWord, std::shared_ptr<pystorm::bddriver::AMWord>> cl(M("pystorm::bddriver"), "AMWord", "");
		pybind11::handle cl_type = cl;

		cl.def(pybind11::init<>());
		pybind11::enum_<pystorm::bddriver::AMWord::Field>(cl, "Field", "")
			.value("ACCUMULATOR_VALUE", pystorm::bddriver::AMWord::Field::ACCUMULATOR_VALUE)
			.value("THRESHOLD", pystorm::bddriver::AMWord::Field::THRESHOLD)
			.value("STOP", pystorm::bddriver::AMWord::Field::STOP)
			.value("NEXT_ADDRESS", pystorm::bddriver::AMWord::Field::NEXT_ADDRESS)
			.export_values();

	}
	{ // pystorm::bddriver::MMWord file: line:292
		pybind11::class_<pystorm::bddriver::MMWord, std::shared_ptr<pystorm::bddriver::MMWord>> cl(M("pystorm::bddriver"), "MMWord", "");
		pybind11::handle cl_type = cl;

		cl.def(pybind11::init<>());
		pybind11::enum_<pystorm::bddriver::MMWord::Field>(cl, "Field", "")
			.value("WEIGHT", pystorm::bddriver::MMWord::Field::WEIGHT)
			.export_values();

	}
	{ // pystorm::bddriver::PATWord file: line:295
		pybind11::class_<pystorm::bddriver::PATWord, std::shared_ptr<pystorm::bddriver::PATWord>> cl(M("pystorm::bddriver"), "PATWord", "");
		pybind11::handle cl_type = cl;

		cl.def(pybind11::init<>());
		pybind11::enum_<pystorm::bddriver::PATWord::Field>(cl, "Field", "")
			.value("AM_ADDRESS", pystorm::bddriver::PATWord::Field::AM_ADDRESS)
			.value("MM_ADDRESS_LO", pystorm::bddriver::PATWord::Field::MM_ADDRESS_LO)
			.value("MM_ADDRESS_HI", pystorm::bddriver::PATWord::Field::MM_ADDRESS_HI)
			.export_values();

	}
	{ // pystorm::bddriver::TATAccWord file: line:298
		pybind11::class_<pystorm::bddriver::TATAccWord, std::shared_ptr<pystorm::bddriver::TATAccWord>> cl(M("pystorm::bddriver"), "TATAccWord", "");
		pybind11::handle cl_type = cl;

		cl.def(pybind11::init<>());
		pybind11::enum_<pystorm::bddriver::TATAccWord::Field>(cl, "Field", "")
			.value("STOP", pystorm::bddriver::TATAccWord::Field::STOP)
			.value("FIXED_0", pystorm::bddriver::TATAccWord::Field::FIXED_0)
			.value("AM_ADDRESS", pystorm::bddriver::TATAccWord::Field::AM_ADDRESS)
			.value("MM_ADDRESS", pystorm::bddriver::TATAccWord::Field::MM_ADDRESS)
			.export_values();

	}
	{ // pystorm::bddriver::TATSpikeWord file: line:304
		pybind11::class_<pystorm::bddriver::TATSpikeWord, std::shared_ptr<pystorm::bddriver::TATSpikeWord>> cl(M("pystorm::bddriver"), "TATSpikeWord", "");
		pybind11::handle cl_type = cl;

		cl.def(pybind11::init<>());
		pybind11::enum_<pystorm::bddriver::TATSpikeWord::Field>(cl, "Field", "")
			.value("STOP", pystorm::bddriver::TATSpikeWord::Field::STOP)
			.value("FIXED_1", pystorm::bddriver::TATSpikeWord::Field::FIXED_1)
			.value("SYNAPSE_ADDRESS_0", pystorm::bddriver::TATSpikeWord::Field::SYNAPSE_ADDRESS_0)
			.value("SYNAPSE_SIGN_0", pystorm::bddriver::TATSpikeWord::Field::SYNAPSE_SIGN_0)
			.value("SYNAPSE_ADDRESS_1", pystorm::bddriver::TATSpikeWord::Field::SYNAPSE_ADDRESS_1)
			.value("SYNAPSE_SIGN_1", pystorm::bddriver::TATSpikeWord::Field::SYNAPSE_SIGN_1)
			.value("UNUSED", pystorm::bddriver::TATSpikeWord::Field::UNUSED)
			.export_values();

	}
	{ // pystorm::bddriver::TATTagWord file: line:313
		pybind11::class_<pystorm::bddriver::TATTagWord, std::shared_ptr<pystorm::bddriver::TATTagWord>> cl(M("pystorm::bddriver"), "TATTagWord", "");
		pybind11::handle cl_type = cl;

		cl.def(pybind11::init<>());
		pybind11::enum_<pystorm::bddriver::TATTagWord::Field>(cl, "Field", "")
			.value("STOP", pystorm::bddriver::TATTagWord::Field::STOP)
			.value("FIXED_2", pystorm::bddriver::TATTagWord::Field::FIXED_2)
			.value("TAG", pystorm::bddriver::TATTagWord::Field::TAG)
			.value("GLOBAL_ROUTE", pystorm::bddriver::TATTagWord::Field::GLOBAL_ROUTE)
			.value("UNUSED", pystorm::bddriver::TATTagWord::Field::UNUSED)
			.export_values();

	}
	{ // pystorm::bddriver::PATWrite file: line:321
		pybind11::class_<pystorm::bddriver::PATWrite, std::shared_ptr<pystorm::bddriver::PATWrite>> cl(M("pystorm::bddriver"), "PATWrite", "");
		pybind11::handle cl_type = cl;

		cl.def(pybind11::init<>());
		pybind11::enum_<pystorm::bddriver::PATWrite::Field>(cl, "Field", "")
			.value("ADDRESS", pystorm::bddriver::PATWrite::Field::ADDRESS)
			.value("FIXED_0", pystorm::bddriver::PATWrite::Field::FIXED_0)
			.value("DATA", pystorm::bddriver::PATWrite::Field::DATA)
			.export_values();

	}
	{ // pystorm::bddriver::PATRead file: line:326
		pybind11::class_<pystorm::bddriver::PATRead, std::shared_ptr<pystorm::bddriver::PATRead>> cl(M("pystorm::bddriver"), "PATRead", "");
		pybind11::handle cl_type = cl;

		cl.def(pybind11::init<>());
		pybind11::enum_<pystorm::bddriver::PATRead::Field>(cl, "Field", "")
			.value("ADDRESS", pystorm::bddriver::PATRead::Field::ADDRESS)
			.value("FIXED_1", pystorm::bddriver::PATRead::Field::FIXED_1)
			.value("UNUSED", pystorm::bddriver::PATRead::Field::UNUSED)
			.export_values();

	}
	{ // pystorm::bddriver::TATSetAddress file: line:331
		pybind11::class_<pystorm::bddriver::TATSetAddress, std::shared_ptr<pystorm::bddriver::TATSetAddress>> cl(M("pystorm::bddriver"), "TATSetAddress", "");
		pybind11::handle cl_type = cl;

		cl.def(pybind11::init<>());
		pybind11::enum_<pystorm::bddriver::TATSetAddress::Field>(cl, "Field", "")
			.value("FIXED_0", pystorm::bddriver::TATSetAddress::Field::FIXED_0)
			.value("ADDRESS", pystorm::bddriver::TATSetAddress::Field::ADDRESS)
			.value("UNUSED", pystorm::bddriver::TATSetAddress::Field::UNUSED)
			.export_values();

	}
	{ // pystorm::bddriver::TATWriteIncrement file: line:336
		pybind11::class_<pystorm::bddriver::TATWriteIncrement, std::shared_ptr<pystorm::bddriver::TATWriteIncrement>> cl(M("pystorm::bddriver"), "TATWriteIncrement", "");
		pybind11::handle cl_type = cl;

		cl.def(pybind11::init<>());
		pybind11::enum_<pystorm::bddriver::TATWriteIncrement::Field>(cl, "Field", "")
			.value("FIXED_1", pystorm::bddriver::TATWriteIncrement::Field::FIXED_1)
			.value("DATA", pystorm::bddriver::TATWriteIncrement::Field::DATA)
			.export_values();

	}
	{ // pystorm::bddriver::TATReadIncrement file: line:340
		pybind11::class_<pystorm::bddriver::TATReadIncrement, std::shared_ptr<pystorm::bddriver::TATReadIncrement>> cl(M("pystorm::bddriver"), "TATReadIncrement", "");
		pybind11::handle cl_type = cl;

		cl.def(pybind11::init<>());
		pybind11::enum_<pystorm::bddriver::TATReadIncrement::Field>(cl, "Field", "")
			.value("FIXED_2", pystorm::bddriver::TATReadIncrement::Field::FIXED_2)
			.value("UNUSED", pystorm::bddriver::TATReadIncrement::Field::UNUSED)
			.export_values();

	}
	{ // pystorm::bddriver::MMSetAddress file: line:344
		pybind11::class_<pystorm::bddriver::MMSetAddress, std::shared_ptr<pystorm::bddriver::MMSetAddress>> cl(M("pystorm::bddriver"), "MMSetAddress", "");
		pybind11::handle cl_type = cl;

		cl.def(pybind11::init<>());
		pybind11::enum_<pystorm::bddriver::MMSetAddress::Field>(cl, "Field", "")
			.value("FIXED_0", pystorm::bddriver::MMSetAddress::Field::FIXED_0)
			.value("ADDRESS", pystorm::bddriver::MMSetAddress::Field::ADDRESS)
			.value("UNUSED", pystorm::bddriver::MMSetAddress::Field::UNUSED)
			.export_values();

	}
	{ // pystorm::bddriver::MMWriteIncrement file: line:349
		pybind11::class_<pystorm::bddriver::MMWriteIncrement, std::shared_ptr<pystorm::bddriver::MMWriteIncrement>> cl(M("pystorm::bddriver"), "MMWriteIncrement", "");
		pybind11::handle cl_type = cl;

		cl.def(pybind11::init<>());
		pybind11::enum_<pystorm::bddriver::MMWriteIncrement::Field>(cl, "Field", "")
			.value("FIXED_1", pystorm::bddriver::MMWriteIncrement::Field::FIXED_1)
			.value("DATA", pystorm::bddriver::MMWriteIncrement::Field::DATA)
			.value("UNUSED", pystorm::bddriver::MMWriteIncrement::Field::UNUSED)
			.export_values();

	}
	{ // pystorm::bddriver::MMReadIncrement file: line:354
		pybind11::class_<pystorm::bddriver::MMReadIncrement, std::shared_ptr<pystorm::bddriver::MMReadIncrement>> cl(M("pystorm::bddriver"), "MMReadIncrement", "");
		pybind11::handle cl_type = cl;

		cl.def(pybind11::init<>());
		pybind11::enum_<pystorm::bddriver::MMReadIncrement::Field>(cl, "Field", "")
			.value("FIXED_2", pystorm::bddriver::MMReadIncrement::Field::FIXED_2)
			.value("UNUSED", pystorm::bddriver::MMReadIncrement::Field::UNUSED)
			.export_values();

	}
	{ // pystorm::bddriver::AMSetAddress file: line:358
		pybind11::class_<pystorm::bddriver::AMSetAddress, std::shared_ptr<pystorm::bddriver::AMSetAddress>> cl(M("pystorm::bddriver"), "AMSetAddress", "");
		pybind11::handle cl_type = cl;

		cl.def(pybind11::init<>());
		pybind11::enum_<pystorm::bddriver::AMSetAddress::Field>(cl, "Field", "")
			.value("FIXED_0", pystorm::bddriver::AMSetAddress::Field::FIXED_0)
			.value("ADDRESS", pystorm::bddriver::AMSetAddress::Field::ADDRESS)
			.value("UNUSED", pystorm::bddriver::AMSetAddress::Field::UNUSED)
			.export_values();

	}
	{ // pystorm::bddriver::AMReadWrite file: line:363
		pybind11::class_<pystorm::bddriver::AMReadWrite, std::shared_ptr<pystorm::bddriver::AMReadWrite>> cl(M("pystorm::bddriver"), "AMReadWrite", "");
		pybind11::handle cl_type = cl;

		cl.def(pybind11::init<>());
		pybind11::enum_<pystorm::bddriver::AMReadWrite::Field>(cl, "Field", "")
			.value("FIXED_1", pystorm::bddriver::AMReadWrite::Field::FIXED_1)
			.value("DATA", pystorm::bddriver::AMReadWrite::Field::DATA)
			.export_values();

	}
	{ // pystorm::bddriver::AMIncrement file: line:367
		pybind11::class_<pystorm::bddriver::AMIncrement, std::shared_ptr<pystorm::bddriver::AMIncrement>> cl(M("pystorm::bddriver"), "AMIncrement", "");
		pybind11::handle cl_type = cl;

		cl.def(pybind11::init<>());
		pybind11::enum_<pystorm::bddriver::AMIncrement::Field>(cl, "Field", "")
			.value("FIXED_2", pystorm::bddriver::AMIncrement::Field::FIXED_2)
			.value("UNUSED", pystorm::bddriver::AMIncrement::Field::UNUSED)
			.export_values();

	}
	{ // pystorm::bddriver::AMEncapsulation file: line:371
		pybind11::class_<pystorm::bddriver::AMEncapsulation, std::shared_ptr<pystorm::bddriver::AMEncapsulation>> cl(M("pystorm::bddriver"), "AMEncapsulation", "");
		pybind11::handle cl_type = cl;

		cl.def(pybind11::init<>());
		pybind11::enum_<pystorm::bddriver::AMEncapsulation::Field>(cl, "Field", "")
			.value("FIXED_0", pystorm::bddriver::AMEncapsulation::Field::FIXED_0)
			.value("PAYLOAD", pystorm::bddriver::AMEncapsulation::Field::PAYLOAD)
			.value("AMMM_STOP", pystorm::bddriver::AMEncapsulation::Field::AMMM_STOP)
			.export_values();

	}
	{ // pystorm::bddriver::MMEncapsulation file: line:376
		pybind11::class_<pystorm::bddriver::MMEncapsulation, std::shared_ptr<pystorm::bddriver::MMEncapsulation>> cl(M("pystorm::bddriver"), "MMEncapsulation", "");
		pybind11::handle cl_type = cl;

		cl.def(pybind11::init<>());
		pybind11::enum_<pystorm::bddriver::MMEncapsulation::Field>(cl, "Field", "")
			.value("FIXED_1", pystorm::bddriver::MMEncapsulation::Field::FIXED_1)
			.value("PAYLOAD", pystorm::bddriver::MMEncapsulation::Field::PAYLOAD)
			.value("AMMM_STOP", pystorm::bddriver::MMEncapsulation::Field::AMMM_STOP)
			.export_values();

	}
	{ // pystorm::bddriver::InputTag file: line:382
		pybind11::class_<pystorm::bddriver::InputTag, std::shared_ptr<pystorm::bddriver::InputTag>> cl(M("pystorm::bddriver"), "InputTag", "");
		pybind11::handle cl_type = cl;

		cl.def(pybind11::init<>());
		pybind11::enum_<pystorm::bddriver::InputTag::Field>(cl, "Field", "")
			.value("COUNT", pystorm::bddriver::InputTag::Field::COUNT)
			.value("TAG", pystorm::bddriver::InputTag::Field::TAG)
			.export_values();

	}
	{ // pystorm::bddriver::FIFOInputTag file: line:385
		pybind11::class_<pystorm::bddriver::FIFOInputTag, std::shared_ptr<pystorm::bddriver::FIFOInputTag>> cl(M("pystorm::bddriver"), "FIFOInputTag", "");
		pybind11::handle cl_type = cl;

		cl.def(pybind11::init<>());
		pybind11::enum_<pystorm::bddriver::FIFOInputTag::Field>(cl, "Field", "")
			.value("TAG", pystorm::bddriver::FIFOInputTag::Field::TAG)
			.export_values();

	}
	{ // pystorm::bddriver::InputSpike file: line:388
		pybind11::class_<pystorm::bddriver::InputSpike, std::shared_ptr<pystorm::bddriver::InputSpike>> cl(M("pystorm::bddriver"), "InputSpike", "");
		pybind11::handle cl_type = cl;

		cl.def(pybind11::init<>());
		pybind11::enum_<pystorm::bddriver::InputSpike::Field>(cl, "Field", "")
			.value("SYNAPSE_SIGN", pystorm::bddriver::InputSpike::Field::SYNAPSE_SIGN)
			.value("SYNAPSE_ADDRESS", pystorm::bddriver::InputSpike::Field::SYNAPSE_ADDRESS)
			.export_values();

	}
	{ // pystorm::bddriver::PreFIFOTag file: line:392
		pybind11::class_<pystorm::bddriver::PreFIFOTag, std::shared_ptr<pystorm::bddriver::PreFIFOTag>> cl(M("pystorm::bddriver"), "PreFIFOTag", "");
		pybind11::handle cl_type = cl;

		cl.def(pybind11::init<>());
		pybind11::enum_<pystorm::bddriver::PreFIFOTag::Field>(cl, "Field", "")
			.value("COUNT", pystorm::bddriver::PreFIFOTag::Field::COUNT)
			.value("TAG", pystorm::bddriver::PreFIFOTag::Field::TAG)
			.export_values();

	}
	{ // pystorm::bddriver::PostFIFOTag file: line:395
		pybind11::class_<pystorm::bddriver::PostFIFOTag, std::shared_ptr<pystorm::bddriver::PostFIFOTag>> cl(M("pystorm::bddriver"), "PostFIFOTag", "");
		pybind11::handle cl_type = cl;

		cl.def(pybind11::init<>());
		pybind11::enum_<pystorm::bddriver::PostFIFOTag::Field>(cl, "Field", "")
			.value("COUNT", pystorm::bddriver::PostFIFOTag::Field::COUNT)
			.value("TAG", pystorm::bddriver::PostFIFOTag::Field::TAG)
			.export_values();

	}
	{ // pystorm::bddriver::OutputSpike file: line:398
		pybind11::class_<pystorm::bddriver::OutputSpike, std::shared_ptr<pystorm::bddriver::OutputSpike>> cl(M("pystorm::bddriver"), "OutputSpike", "");
		pybind11::handle cl_type = cl;

		cl.def(pybind11::init<>());
		pybind11::enum_<pystorm::bddriver::OutputSpike::Field>(cl, "Field", "")
			.value("NEURON_ADDRESS", pystorm::bddriver::OutputSpike::Field::NEURON_ADDRESS)
			.export_values();

	}
	{ // pystorm::bddriver::OverflowTag file: line:401
		pybind11::class_<pystorm::bddriver::OverflowTag, std::shared_ptr<pystorm::bddriver::OverflowTag>> cl(M("pystorm::bddriver"), "OverflowTag", "");
		pybind11::handle cl_type = cl;

		cl.def(pybind11::init<>());
		pybind11::enum_<pystorm::bddriver::OverflowTag::Field>(cl, "Field", "")
			.value("FIXED_1", pystorm::bddriver::OverflowTag::Field::FIXED_1)
			.export_values();

	}
	{ // pystorm::bddriver::AccOutputTag file: line:404
		pybind11::class_<pystorm::bddriver::AccOutputTag, std::shared_ptr<pystorm::bddriver::AccOutputTag>> cl(M("pystorm::bddriver"), "AccOutputTag", "");
		pybind11::handle cl_type = cl;

		cl.def(pybind11::init<>());
		pybind11::enum_<pystorm::bddriver::AccOutputTag::Field>(cl, "Field", "")
			.value("COUNT", pystorm::bddriver::AccOutputTag::Field::COUNT)
			.value("TAG", pystorm::bddriver::AccOutputTag::Field::TAG)
			.value("GLOBAL_ROUTE", pystorm::bddriver::AccOutputTag::Field::GLOBAL_ROUTE)
			.export_values();

	}
	{ // pystorm::bddriver::TATOutputTag file: line:407
		pybind11::class_<pystorm::bddriver::TATOutputTag, std::shared_ptr<pystorm::bddriver::TATOutputTag>> cl(M("pystorm::bddriver"), "TATOutputTag", "");
		pybind11::handle cl_type = cl;

		cl.def(pybind11::init<>());
		pybind11::enum_<pystorm::bddriver::TATOutputTag::Field>(cl, "Field", "")
			.value("COUNT", pystorm::bddriver::TATOutputTag::Field::COUNT)
			.value("TAG", pystorm::bddriver::TATOutputTag::Field::TAG)
			.value("GLOBAL_ROUTE", pystorm::bddriver::TATOutputTag::Field::GLOBAL_ROUTE)
			.export_values();

	}
	{ // pystorm::bddriver::NeuronConfig file: line:411
		pybind11::class_<pystorm::bddriver::NeuronConfig, std::shared_ptr<pystorm::bddriver::NeuronConfig>> cl(M("pystorm::bddriver"), "NeuronConfig", "Receiver packet structure for neuron config memory");
		pybind11::handle cl_type = cl;

		cl.def(pybind11::init<>());
		pybind11::enum_<pystorm::bddriver::NeuronConfig::Field>(cl, "Field", "")
			.value("ROW_HI", pystorm::bddriver::NeuronConfig::Field::ROW_HI)
			.value("COL_HI", pystorm::bddriver::NeuronConfig::Field::COL_HI)
			.value("ROW_LO", pystorm::bddriver::NeuronConfig::Field::ROW_LO)
			.value("COL_LO", pystorm::bddriver::NeuronConfig::Field::COL_LO)
			.value("BIT_VAL", pystorm::bddriver::NeuronConfig::Field::BIT_VAL)
			.value("BIT_SEL", pystorm::bddriver::NeuronConfig::Field::BIT_SEL)
			.value("FIXED_2", pystorm::bddriver::NeuronConfig::Field::FIXED_2)
			.value("TILE_ADDR", pystorm::bddriver::NeuronConfig::Field::TILE_ADDR)
			.export_values();

	}
	{ // pystorm::bddriver::BDWord file: line:424
		pybind11::class_<pystorm::bddriver::BDWord, std::shared_ptr<pystorm::bddriver::BDWord>> cl(M("pystorm::bddriver"), "BDWord", "Basically just a uint64_t with some packing and unpacking methods.\n Being a class doesn't add a lot to this: the member functions could\n just as well act on plain uint64_ts. ");
		pybind11::handle cl_type = cl;

		cl.def(pybind11::init<>());

		cl.def(pybind11::init<unsigned long>(), pybind11::arg("val"));

		cl.def(pybind11::init<const class pystorm::bddriver::BDWord &>(), pybind11::arg(""));

		cl.def("Packed", (unsigned long (pystorm::bddriver::BDWord::*)() const) &pystorm::bddriver::BDWord::Packed<unsigned long>, "C++: pystorm::bddriver::BDWord::Packed() const --> unsigned long");
		cl.def("Packed", (unsigned long (pystorm::bddriver::BDWord::*)() const) &pystorm::bddriver::BDWord::Packed, "Syntactic sugar for Packed(), UINT defaults to uint64_t\n\nC++: pystorm::bddriver::BDWord::Packed() const --> unsigned long");
		cl.def("assign", (class pystorm::bddriver::BDWord & (pystorm::bddriver::BDWord::*)(const class pystorm::bddriver::BDWord &)) &pystorm::bddriver::BDWord::operator=, "C++: pystorm::bddriver::BDWord::operator=(const class pystorm::bddriver::BDWord &) --> class pystorm::bddriver::BDWord &", pybind11::return_value_policy::automatic, pybind11::arg(""));
	}
}


// File: unknown/unknown_3.cpp
#include <array>
#include <sstream> // __str__
#include <utility>

#include <pybind11/pybind11.h>

#ifndef BINDER_PYBIND11_TYPE_CASTER
	#define BINDER_PYBIND11_TYPE_CASTER
	PYBIND11_DECLARE_HOLDER_TYPE(T, std::shared_ptr<T>);
	PYBIND11_DECLARE_HOLDER_TYPE(T, T*);
	PYBIND11_MAKE_OPAQUE(std::shared_ptr<void>);
#endif

void bind_unknown_unknown_3(std::function< pybind11::module &(std::string const &namespace_) > &M)
{
	// pystorm::bddriver::bdpars::ComponentTypeId file: line:522
	pybind11::enum_<pystorm::bddriver::bdpars::ComponentTypeId>(M("pystorm::bddriver::bdpars"), "ComponentTypeId", "Identifier for a broad class of BD components")
		.value("REG", pystorm::bddriver::bdpars::ComponentTypeId::REG)
		.value("MEM", pystorm::bddriver::bdpars::ComponentTypeId::MEM)
		.value("INPUT", pystorm::bddriver::bdpars::ComponentTypeId::INPUT)
		.value("OUTPUT", pystorm::bddriver::bdpars::ComponentTypeId::OUTPUT)
		.value("ComponentTypeIdCount", pystorm::bddriver::bdpars::ComponentTypeId::ComponentTypeIdCount)
		.export_values();

;

	// pystorm::bddriver::bdpars::RegId file: line:532
	pybind11::enum_<pystorm::bddriver::bdpars::RegId>(M("pystorm::bddriver::bdpars"), "RegId", "Identifier for particular BD register")
		.value("TOGGLE_PRE_FIFO", pystorm::bddriver::bdpars::RegId::TOGGLE_PRE_FIFO)
		.value("TOGGLE_POST_FIFO0", pystorm::bddriver::bdpars::RegId::TOGGLE_POST_FIFO0)
		.value("TOGGLE_POST_FIFO1", pystorm::bddriver::bdpars::RegId::TOGGLE_POST_FIFO1)
		.value("NEURON_DUMP_TOGGLE", pystorm::bddriver::bdpars::RegId::NEURON_DUMP_TOGGLE)
		.value("DAC0", pystorm::bddriver::bdpars::RegId::DAC0)
		.value("DAC1", pystorm::bddriver::bdpars::RegId::DAC1)
		.value("DAC2", pystorm::bddriver::bdpars::RegId::DAC2)
		.value("DAC3", pystorm::bddriver::bdpars::RegId::DAC3)
		.value("DAC4", pystorm::bddriver::bdpars::RegId::DAC4)
		.value("DAC5", pystorm::bddriver::bdpars::RegId::DAC5)
		.value("DAC6", pystorm::bddriver::bdpars::RegId::DAC6)
		.value("DAC7", pystorm::bddriver::bdpars::RegId::DAC7)
		.value("DAC8", pystorm::bddriver::bdpars::RegId::DAC8)
		.value("DAC9", pystorm::bddriver::bdpars::RegId::DAC9)
		.value("DAC10", pystorm::bddriver::bdpars::RegId::DAC10)
		.value("DAC11", pystorm::bddriver::bdpars::RegId::DAC11)
		.value("DAC12", pystorm::bddriver::bdpars::RegId::DAC12)
		.value("ADC", pystorm::bddriver::bdpars::RegId::ADC)
		.value("DELAY0", pystorm::bddriver::bdpars::RegId::DELAY0)
		.value("DELAY1", pystorm::bddriver::bdpars::RegId::DELAY1)
		.value("DELAY2", pystorm::bddriver::bdpars::RegId::DELAY2)
		.value("DELAY3", pystorm::bddriver::bdpars::RegId::DELAY3)
		.value("DELAY4", pystorm::bddriver::bdpars::RegId::DELAY4)
		.value("DELAY5", pystorm::bddriver::bdpars::RegId::DELAY5)
		.value("DELAY6", pystorm::bddriver::bdpars::RegId::DELAY6)
		.value("RegIdCount", pystorm::bddriver::bdpars::RegId::RegIdCount)
		.export_values();

;

	// pystorm::bddriver::bdpars::MemId file: line:563
	pybind11::enum_<pystorm::bddriver::bdpars::MemId>(M("pystorm::bddriver::bdpars"), "MemId", "Identifier for particular BD memory")
		.value("AM", pystorm::bddriver::bdpars::MemId::AM)
		.value("MM", pystorm::bddriver::bdpars::MemId::MM)
		.value("TAT0", pystorm::bddriver::bdpars::MemId::TAT0)
		.value("TAT1", pystorm::bddriver::bdpars::MemId::TAT1)
		.value("PAT", pystorm::bddriver::bdpars::MemId::PAT)
		.value("FIFO_DCT", pystorm::bddriver::bdpars::MemId::FIFO_DCT)
		.value("FIFO_PG", pystorm::bddriver::bdpars::MemId::FIFO_PG)
		.value("MemIdCount", pystorm::bddriver::bdpars::MemId::MemIdCount)
		.export_values();

;

	// pystorm::bddriver::bdpars::InputId file: line:576
	pybind11::enum_<pystorm::bddriver::bdpars::InputId>(M("pystorm::bddriver::bdpars"), "InputId", "Identifier for particular BD input stream")
		.value("INPUT_TAGS", pystorm::bddriver::bdpars::InputId::INPUT_TAGS)
		.value("DCT_FIFO_INPUT_TAGS", pystorm::bddriver::bdpars::InputId::DCT_FIFO_INPUT_TAGS)
		.value("HT_FIFO_RESET", pystorm::bddriver::bdpars::InputId::HT_FIFO_RESET)
		.value("TILE_SRAM_INPUTS", pystorm::bddriver::bdpars::InputId::TILE_SRAM_INPUTS)
		.value("INPUT_SPIKES", pystorm::bddriver::bdpars::InputId::INPUT_SPIKES)
		.value("InputIdCount", pystorm::bddriver::bdpars::InputId::InputIdCount)
		.export_values();

;

	// pystorm::bddriver::bdpars::OutputId file: line:587
	pybind11::enum_<pystorm::bddriver::bdpars::OutputId>(M("pystorm::bddriver::bdpars"), "OutputId", "Identifier for particular BD output stream")
		.value("PRE_FIFO_TAGS", pystorm::bddriver::bdpars::OutputId::PRE_FIFO_TAGS)
		.value("POST_FIFO_TAGS0", pystorm::bddriver::bdpars::OutputId::POST_FIFO_TAGS0)
		.value("POST_FIFO_TAGS1", pystorm::bddriver::bdpars::OutputId::POST_FIFO_TAGS1)
		.value("OUTPUT_SPIKES", pystorm::bddriver::bdpars::OutputId::OUTPUT_SPIKES)
		.value("OVERFLOW_TAGS0", pystorm::bddriver::bdpars::OutputId::OVERFLOW_TAGS0)
		.value("OVERFLOW_TAGS1", pystorm::bddriver::bdpars::OutputId::OVERFLOW_TAGS1)
		.value("ACC_OUTPUT_TAGS", pystorm::bddriver::bdpars::OutputId::ACC_OUTPUT_TAGS)
		.value("TAT_OUTPUT_TAGS", pystorm::bddriver::bdpars::OutputId::TAT_OUTPUT_TAGS)
		.value("OutputIdCount", pystorm::bddriver::bdpars::OutputId::OutputIdCount)
		.export_values();

;

	// pystorm::bddriver::bdpars::DACSignalId file: line:602
	pybind11::enum_<pystorm::bddriver::bdpars::DACSignalId>(M("pystorm::bddriver::bdpars"), "DACSignalId", "Identifier for particular DAC signal name")
		.value("DIFF_G", pystorm::bddriver::bdpars::DACSignalId::DIFF_G)
		.value("DIFF_R", pystorm::bddriver::bdpars::DACSignalId::DIFF_R)
		.value("SOMA_OFFSET", pystorm::bddriver::bdpars::DACSignalId::SOMA_OFFSET)
		.value("SYN_LK", pystorm::bddriver::bdpars::DACSignalId::SYN_LK)
		.value("SYN_DC", pystorm::bddriver::bdpars::DACSignalId::SYN_DC)
		.value("SYN_PD", pystorm::bddriver::bdpars::DACSignalId::SYN_PD)
		.value("ADC_BIAS_2", pystorm::bddriver::bdpars::DACSignalId::ADC_BIAS_2)
		.value("ADC_BIAS_1", pystorm::bddriver::bdpars::DACSignalId::ADC_BIAS_1)
		.value("SOMA_REF", pystorm::bddriver::bdpars::DACSignalId::SOMA_REF)
		.value("SOMA_EXC", pystorm::bddriver::bdpars::DACSignalId::SOMA_EXC)
		.value("SOMA_INH", pystorm::bddriver::bdpars::DACSignalId::SOMA_INH)
		.value("SYN_PU", pystorm::bddriver::bdpars::DACSignalId::SYN_PU)
		.value("UNUSED", pystorm::bddriver::bdpars::DACSignalId::UNUSED)
		.value("DACSignalIdCount", pystorm::bddriver::bdpars::DACSignalId::DACSignalIdCount)
		.export_values();

;

	// pystorm::bddriver::bdpars::HornLeafId file: line:621
	pybind11::enum_<pystorm::bddriver::bdpars::HornLeafId>(M("pystorm::bddriver::bdpars"), "HornLeafId", "Identifier for particular BD horn leaf")
		.value("NEURON_INJECT", pystorm::bddriver::bdpars::HornLeafId::NEURON_INJECT)
		.value("RI", pystorm::bddriver::bdpars::HornLeafId::RI)
		.value("PROG_AMMM", pystorm::bddriver::bdpars::HornLeafId::PROG_AMMM)
		.value("PROG_PAT", pystorm::bddriver::bdpars::HornLeafId::PROG_PAT)
		.value("PROG_TAT0", pystorm::bddriver::bdpars::HornLeafId::PROG_TAT0)
		.value("PROG_TAT1", pystorm::bddriver::bdpars::HornLeafId::PROG_TAT1)
		.value("INIT_FIFO_DCT", pystorm::bddriver::bdpars::HornLeafId::INIT_FIFO_DCT)
		.value("INIT_FIFO_HT", pystorm::bddriver::bdpars::HornLeafId::INIT_FIFO_HT)
		.value("TOGGLE_PRE_FIFO_LEAF", pystorm::bddriver::bdpars::HornLeafId::TOGGLE_PRE_FIFO_LEAF)
		.value("TOGGLE_POST_FIFO0_LEAF", pystorm::bddriver::bdpars::HornLeafId::TOGGLE_POST_FIFO0_LEAF)
		.value("TOGGLE_POST_FIFO1_LEAF", pystorm::bddriver::bdpars::HornLeafId::TOGGLE_POST_FIFO1_LEAF)
		.value("NEURON_DUMP_TOGGLE_LEAF", pystorm::bddriver::bdpars::HornLeafId::NEURON_DUMP_TOGGLE_LEAF)
		.value("NEURON_CONFIG", pystorm::bddriver::bdpars::HornLeafId::NEURON_CONFIG)
		.value("DAC0_LEAF", pystorm::bddriver::bdpars::HornLeafId::DAC0_LEAF)
		.value("DAC1_LEAF", pystorm::bddriver::bdpars::HornLeafId::DAC1_LEAF)
		.value("DAC2_LEAF", pystorm::bddriver::bdpars::HornLeafId::DAC2_LEAF)
		.value("DAC3_LEAF", pystorm::bddriver::bdpars::HornLeafId::DAC3_LEAF)
		.value("DAC4_LEAF", pystorm::bddriver::bdpars::HornLeafId::DAC4_LEAF)
		.value("DAC5_LEAF", pystorm::bddriver::bdpars::HornLeafId::DAC5_LEAF)
		.value("DAC6_LEAF", pystorm::bddriver::bdpars::HornLeafId::DAC6_LEAF)
		.value("DAC7_LEAF", pystorm::bddriver::bdpars::HornLeafId::DAC7_LEAF)
		.value("DAC8_LEAF", pystorm::bddriver::bdpars::HornLeafId::DAC8_LEAF)
		.value("DAC9_LEAF", pystorm::bddriver::bdpars::HornLeafId::DAC9_LEAF)
		.value("DAC10_LEAF", pystorm::bddriver::bdpars::HornLeafId::DAC10_LEAF)
		.value("DAC11_LEAF", pystorm::bddriver::bdpars::HornLeafId::DAC11_LEAF)
		.value("DAC12_LEAF", pystorm::bddriver::bdpars::HornLeafId::DAC12_LEAF)
		.value("ADC_LEAF", pystorm::bddriver::bdpars::HornLeafId::ADC_LEAF)
		.value("DELAY0_LEAF", pystorm::bddriver::bdpars::HornLeafId::DELAY0_LEAF)
		.value("DELAY1_LEAF", pystorm::bddriver::bdpars::HornLeafId::DELAY1_LEAF)
		.value("DELAY2_LEAF", pystorm::bddriver::bdpars::HornLeafId::DELAY2_LEAF)
		.value("DELAY3_LEAF", pystorm::bddriver::bdpars::HornLeafId::DELAY3_LEAF)
		.value("DELAY4_LEAF", pystorm::bddriver::bdpars::HornLeafId::DELAY4_LEAF)
		.value("DELAY5_LEAF", pystorm::bddriver::bdpars::HornLeafId::DELAY5_LEAF)
		.value("DELAY6_LEAF", pystorm::bddriver::bdpars::HornLeafId::DELAY6_LEAF)
		.value("HornLeafIdCount", pystorm::bddriver::bdpars::HornLeafId::HornLeafIdCount)
		.export_values();

;

	// pystorm::bddriver::bdpars::FunnelLeafId file: line:661
	pybind11::enum_<pystorm::bddriver::bdpars::FunnelLeafId>(M("pystorm::bddriver::bdpars"), "FunnelLeafId", "Identifier for particular BD funnel leaf")
		.value("RO_ACC", pystorm::bddriver::bdpars::FunnelLeafId::RO_ACC)
		.value("RO_TAT", pystorm::bddriver::bdpars::FunnelLeafId::RO_TAT)
		.value("NRNI", pystorm::bddriver::bdpars::FunnelLeafId::NRNI)
		.value("DUMP_AM", pystorm::bddriver::bdpars::FunnelLeafId::DUMP_AM)
		.value("DUMP_MM", pystorm::bddriver::bdpars::FunnelLeafId::DUMP_MM)
		.value("DUMP_PAT", pystorm::bddriver::bdpars::FunnelLeafId::DUMP_PAT)
		.value("DUMP_TAT0", pystorm::bddriver::bdpars::FunnelLeafId::DUMP_TAT0)
		.value("DUMP_TAT1", pystorm::bddriver::bdpars::FunnelLeafId::DUMP_TAT1)
		.value("DUMP_PRE_FIFO", pystorm::bddriver::bdpars::FunnelLeafId::DUMP_PRE_FIFO)
		.value("DUMP_POST_FIFO0", pystorm::bddriver::bdpars::FunnelLeafId::DUMP_POST_FIFO0)
		.value("DUMP_POST_FIFO1", pystorm::bddriver::bdpars::FunnelLeafId::DUMP_POST_FIFO1)
		.value("OVFLW0", pystorm::bddriver::bdpars::FunnelLeafId::OVFLW0)
		.value("OVFLW1", pystorm::bddriver::bdpars::FunnelLeafId::OVFLW1)
		.value("FunnelLeafIdCount", pystorm::bddriver::bdpars::FunnelLeafId::FunnelLeafIdCount)
		.export_values();

;

	// pystorm::bddriver::bdpars::MiscWidthId file: line:680
	pybind11::enum_<pystorm::bddriver::bdpars::MiscWidthId>(M("pystorm::bddriver::bdpars"), "MiscWidthId", "Identifier for miscellaneous BD hardware width parameters")
		.value("BD_INPUT", pystorm::bddriver::bdpars::MiscWidthId::BD_INPUT)
		.value("BD_OUTPUT", pystorm::bddriver::bdpars::MiscWidthId::BD_OUTPUT)
		.value("MiscWidthIdCount", pystorm::bddriver::bdpars::MiscWidthId::MiscWidthIdCount)
		.export_values();

;

	// pystorm::bddriver::bdpars::ConfigSomaID file: line:688
	pybind11::enum_<pystorm::bddriver::bdpars::ConfigSomaID>(M("pystorm::bddriver::bdpars"), "ConfigSomaID", "Neuron configuration options")
		.value("GAIN_0", pystorm::bddriver::bdpars::ConfigSomaID::GAIN_0)
		.value("GAIN_1", pystorm::bddriver::bdpars::ConfigSomaID::GAIN_1)
		.value("OFFSET_0", pystorm::bddriver::bdpars::ConfigSomaID::OFFSET_0)
		.value("OFFSET_1", pystorm::bddriver::bdpars::ConfigSomaID::OFFSET_1)
		.value("ENABLE", pystorm::bddriver::bdpars::ConfigSomaID::ENABLE)
		.value("SUBTRACT_OFFSET", pystorm::bddriver::bdpars::ConfigSomaID::SUBTRACT_OFFSET);

;

	// pystorm::bddriver::bdpars::ConfigSynapseID file: line:697
	pybind11::enum_<pystorm::bddriver::bdpars::ConfigSynapseID>(M("pystorm::bddriver::bdpars"), "ConfigSynapseID", "")
		.value("SYN_DISABLE", pystorm::bddriver::bdpars::ConfigSynapseID::SYN_DISABLE)
		.value("ADC_DISABLE", pystorm::bddriver::bdpars::ConfigSynapseID::ADC_DISABLE);

;

	// pystorm::bddriver::bdpars::SomaStatusId file: line:702
	pybind11::enum_<pystorm::bddriver::bdpars::SomaStatusId>(M("pystorm::bddriver::bdpars"), "SomaStatusId", "")
		.value("DISABLED", pystorm::bddriver::bdpars::SomaStatusId::DISABLED)
		.value("ENABLED", pystorm::bddriver::bdpars::SomaStatusId::ENABLED);

;

	// pystorm::bddriver::bdpars::SomaGainId file: line:704
	pybind11::enum_<pystorm::bddriver::bdpars::SomaGainId>(M("pystorm::bddriver::bdpars"), "SomaGainId", "")
		.value("ONE_FOURTH", pystorm::bddriver::bdpars::SomaGainId::ONE_FOURTH)
		.value("ONE_THIRD", pystorm::bddriver::bdpars::SomaGainId::ONE_THIRD)
		.value("ONE_HALF", pystorm::bddriver::bdpars::SomaGainId::ONE_HALF)
		.value("ONE", pystorm::bddriver::bdpars::SomaGainId::ONE);

;

	// pystorm::bddriver::bdpars::SomaOffsetSignId file: line:706
	pybind11::enum_<pystorm::bddriver::bdpars::SomaOffsetSignId>(M("pystorm::bddriver::bdpars"), "SomaOffsetSignId", "")
		.value("POSITIVE", pystorm::bddriver::bdpars::SomaOffsetSignId::POSITIVE)
		.value("NEGATIVE", pystorm::bddriver::bdpars::SomaOffsetSignId::NEGATIVE);

;

	// pystorm::bddriver::bdpars::SomaOffsetMultiplierId file: line:708
	pybind11::enum_<pystorm::bddriver::bdpars::SomaOffsetMultiplierId>(M("pystorm::bddriver::bdpars"), "SomaOffsetMultiplierId", "")
		.value("ZERO", pystorm::bddriver::bdpars::SomaOffsetMultiplierId::ZERO)
		.value("ONE", pystorm::bddriver::bdpars::SomaOffsetMultiplierId::ONE)
		.value("TWO", pystorm::bddriver::bdpars::SomaOffsetMultiplierId::TWO)
		.value("THREE", pystorm::bddriver::bdpars::SomaOffsetMultiplierId::THREE);

;

	// pystorm::bddriver::bdpars::SynapseStatusId file: line:710
	pybind11::enum_<pystorm::bddriver::bdpars::SynapseStatusId>(M("pystorm::bddriver::bdpars"), "SynapseStatusId", "")
		.value("ENABLED", pystorm::bddriver::bdpars::SynapseStatusId::ENABLED)
		.value("DISABLED", pystorm::bddriver::bdpars::SynapseStatusId::DISABLED);

;

	// pystorm::bddriver::bdpars::DiffusorCutStatusId file: line:712
	pybind11::enum_<pystorm::bddriver::bdpars::DiffusorCutStatusId>(M("pystorm::bddriver::bdpars"), "DiffusorCutStatusId", "")
		.value("CLOSE", pystorm::bddriver::bdpars::DiffusorCutStatusId::CLOSE)
		.value("OPEN", pystorm::bddriver::bdpars::DiffusorCutStatusId::OPEN);

;

	// pystorm::bddriver::bdpars::DiffusorCutLocationId file: line:714
	pybind11::enum_<pystorm::bddriver::bdpars::DiffusorCutLocationId>(M("pystorm::bddriver::bdpars"), "DiffusorCutLocationId", "")
		.value("NORTH_LEFT", pystorm::bddriver::bdpars::DiffusorCutLocationId::NORTH_LEFT)
		.value("NORTH_RIGHT", pystorm::bddriver::bdpars::DiffusorCutLocationId::NORTH_RIGHT)
		.value("WEST_TOP", pystorm::bddriver::bdpars::DiffusorCutLocationId::WEST_TOP)
		.value("WEST_BOTTOM", pystorm::bddriver::bdpars::DiffusorCutLocationId::WEST_BOTTOM);

;

	{ // pystorm::bddriver::bdpars::LeafInfo file: line:719
		pybind11::class_<pystorm::bddriver::bdpars::LeafInfo, std::shared_ptr<pystorm::bddriver::bdpars::LeafInfo>> cl(M("pystorm::bddriver::bdpars"), "LeafInfo", "");
		pybind11::handle cl_type = cl;

		cl.def(pybind11::init<>());
		cl.def_readwrite("component_type", &pystorm::bddriver::bdpars::LeafInfo::component_type);
		cl.def_readwrite("component", &pystorm::bddriver::bdpars::LeafInfo::component);
		cl.def_readwrite("route_val", &pystorm::bddriver::bdpars::LeafInfo::route_val);
		cl.def_readwrite("route_len", &pystorm::bddriver::bdpars::LeafInfo::route_len);
		cl.def_readwrite("data_width", &pystorm::bddriver::bdpars::LeafInfo::data_width);
		cl.def_readwrite("serialization", &pystorm::bddriver::bdpars::LeafInfo::serialization);
		cl.def_readwrite("chunk_width", &pystorm::bddriver::bdpars::LeafInfo::chunk_width);
		cl.def_readwrite("description", &pystorm::bddriver::bdpars::LeafInfo::description);
	}
	{ // pystorm::bddriver::bdpars::MemInfo file: line:731
		pybind11::class_<pystorm::bddriver::bdpars::MemInfo, std::shared_ptr<pystorm::bddriver::bdpars::MemInfo>> cl(M("pystorm::bddriver::bdpars"), "MemInfo", "");
		pybind11::handle cl_type = cl;

		cl.def(pybind11::init<>());
		cl.def_readwrite("size", &pystorm::bddriver::bdpars::MemInfo::size);
		cl.def_readwrite("prog_leaf", &pystorm::bddriver::bdpars::MemInfo::prog_leaf);
		cl.def_readwrite("dump_leaf", &pystorm::bddriver::bdpars::MemInfo::dump_leaf);
		cl.def_readwrite("delay_reg", &pystorm::bddriver::bdpars::MemInfo::delay_reg);
	}
	{ // pystorm::bddriver::bdpars::RegInfo file: line:738
		pybind11::class_<pystorm::bddriver::bdpars::RegInfo, std::shared_ptr<pystorm::bddriver::bdpars::RegInfo>> cl(M("pystorm::bddriver::bdpars"), "RegInfo", "");
		pybind11::handle cl_type = cl;

		cl.def(pybind11::init<>());
		cl.def_readwrite("leaf", &pystorm::bddriver::bdpars::RegInfo::leaf);
	}
	{ // pystorm::bddriver::bdpars::InputInfo file: line:742
		pybind11::class_<pystorm::bddriver::bdpars::InputInfo, std::shared_ptr<pystorm::bddriver::bdpars::InputInfo>> cl(M("pystorm::bddriver::bdpars"), "InputInfo", "");
		pybind11::handle cl_type = cl;

		cl.def(pybind11::init<>());
		cl.def_readwrite("leaf", &pystorm::bddriver::bdpars::InputInfo::leaf);
	}
	{ // pystorm::bddriver::bdpars::OutputInfo file: line:746
		pybind11::class_<pystorm::bddriver::bdpars::OutputInfo, std::shared_ptr<pystorm::bddriver::bdpars::OutputInfo>> cl(M("pystorm::bddriver::bdpars"), "OutputInfo", "");
		pybind11::handle cl_type = cl;

		cl.def(pybind11::init<>());
		cl.def_readwrite("leaf", &pystorm::bddriver::bdpars::OutputInfo::leaf);
	}
	{ // pystorm::bddriver::bdpars::BDPars file: line:757
		pybind11::class_<pystorm::bddriver::bdpars::BDPars, std::shared_ptr<pystorm::bddriver::bdpars::BDPars>> cl(M("pystorm::bddriver::bdpars"), "BDPars", "BDPars holds all the nitty-gritty information about the BD hardware's parameters.\n\n BDPars contains several array data members containing structs, keyed by enums.\n The enums refer to particular hardware elements or concepts, such as the name of a memory,\n register, or a particular type of programming word.\n The structs contain relevant information about this hardware element or concept,\n such as the size of the memory, or the WordStructure that describes the programming word type.");
		pybind11::handle cl_type = cl;

		cl.def(pybind11::init<>());

		cl.def("HornRoute", (struct std::pair<unsigned int, unsigned int> (pystorm::bddriver::bdpars::BDPars::*)(pystorm::bddriver::bdpars::HornLeafId) const) &pystorm::bddriver::bdpars::BDPars::HornRoute, "Get the route to a given horn leaf\n\nC++: pystorm::bddriver::bdpars::BDPars::HornRoute(pystorm::bddriver::bdpars::HornLeafId) const --> struct std::pair<unsigned int, unsigned int>", pybind11::arg("leaf"));
		cl.def("HornRoute", (struct std::pair<unsigned int, unsigned int> (pystorm::bddriver::bdpars::BDPars::*)(unsigned int) const) &pystorm::bddriver::bdpars::BDPars::HornRoute, "Get the route to a given horn leaf\n\nC++: pystorm::bddriver::bdpars::BDPars::HornRoute(unsigned int) const --> struct std::pair<unsigned int, unsigned int>", pybind11::arg("leaf"));
		cl.def("FunnelRoute", (struct std::pair<unsigned int, unsigned int> (pystorm::bddriver::bdpars::BDPars::*)(pystorm::bddriver::bdpars::FunnelLeafId) const) &pystorm::bddriver::bdpars::BDPars::FunnelRoute, "Get the route to a given funnel leaf\n\nC++: pystorm::bddriver::bdpars::BDPars::FunnelRoute(pystorm::bddriver::bdpars::FunnelLeafId) const --> struct std::pair<unsigned int, unsigned int>", pybind11::arg("leaf"));
		cl.def("FunnelRoute", (struct std::pair<unsigned int, unsigned int> (pystorm::bddriver::bdpars::BDPars::*)(unsigned int) const) &pystorm::bddriver::bdpars::BDPars::FunnelRoute, "Get the route to a given funnel leaf\n\nC++: pystorm::bddriver::bdpars::BDPars::FunnelRoute(unsigned int) const --> struct std::pair<unsigned int, unsigned int>", pybind11::arg("leaf"));
		cl.def("HornIdx", (unsigned int (pystorm::bddriver::bdpars::BDPars::*)(pystorm::bddriver::bdpars::HornLeafId) const) &pystorm::bddriver::bdpars::BDPars::HornIdx, "Horn leaf ids may be used to index tables, this performs that mapping\n\nC++: pystorm::bddriver::bdpars::BDPars::HornIdx(pystorm::bddriver::bdpars::HornLeafId) const --> unsigned int", pybind11::arg("leaf"));
		cl.def("FunnelIdx", (unsigned int (pystorm::bddriver::bdpars::BDPars::*)(pystorm::bddriver::bdpars::FunnelLeafId) const) &pystorm::bddriver::bdpars::BDPars::FunnelIdx, "Funnel leaf ids may be used to index tables, this performs that mapping\n\nC++: pystorm::bddriver::bdpars::BDPars::FunnelIdx(pystorm::bddriver::bdpars::FunnelLeafId) const --> unsigned int", pybind11::arg("leaf"));
		cl.def("HornRoutes", (const struct std::array<struct std::pair<unsigned int, unsigned int>, 34> * (pystorm::bddriver::bdpars::BDPars::*)() const) &pystorm::bddriver::bdpars::BDPars::HornRoutes, "return reference to Horn routing table\n\nC++: pystorm::bddriver::bdpars::BDPars::HornRoutes() const --> const struct std::array<struct std::pair<unsigned int, unsigned int>, 34> *", pybind11::return_value_policy::automatic);
		cl.def("FunnelRoutes", (const struct std::array<struct std::pair<unsigned int, unsigned int>, 13> * (pystorm::bddriver::bdpars::BDPars::*)() const) &pystorm::bddriver::bdpars::BDPars::FunnelRoutes, "return reference to Funnel routing table\n\nC++: pystorm::bddriver::bdpars::BDPars::FunnelRoutes() const --> const struct std::array<struct std::pair<unsigned int, unsigned int>, 13> *", pybind11::return_value_policy::automatic);
		cl.def("Serialization", (unsigned int (pystorm::bddriver::bdpars::BDPars::*)(pystorm::bddriver::bdpars::HornLeafId) const) &pystorm::bddriver::bdpars::BDPars::Serialization, "Get serialization for a given horn leaf.\n This many messages are concatenated at the horn leaf before being sent on.\n\nC++: pystorm::bddriver::bdpars::BDPars::Serialization(pystorm::bddriver::bdpars::HornLeafId) const --> unsigned int", pybind11::arg("leaf"));
		cl.def("Serialization", (unsigned int (pystorm::bddriver::bdpars::BDPars::*)(pystorm::bddriver::bdpars::FunnelLeafId) const) &pystorm::bddriver::bdpars::BDPars::Serialization, "Get serialization for a given funnel leaf\n The driver should concatenate this many messages from this leaf before interpreting it.\n\nC++: pystorm::bddriver::bdpars::BDPars::Serialization(pystorm::bddriver::bdpars::FunnelLeafId) const --> unsigned int", pybind11::arg("leaf"));
		cl.def("HornLeafIdFor", (pystorm::bddriver::bdpars::HornLeafId (pystorm::bddriver::bdpars::BDPars::*)(pystorm::bddriver::bdpars::MemId) const) &pystorm::bddriver::bdpars::BDPars::HornLeafIdFor, "Map from a memory to it's programming horn leaf\n\nC++: pystorm::bddriver::bdpars::BDPars::HornLeafIdFor(pystorm::bddriver::bdpars::MemId) const --> pystorm::bddriver::bdpars::HornLeafId", pybind11::arg("object"));
		cl.def("HornLeafIdFor", (pystorm::bddriver::bdpars::HornLeafId (pystorm::bddriver::bdpars::BDPars::*)(pystorm::bddriver::bdpars::RegId) const) &pystorm::bddriver::bdpars::BDPars::HornLeafIdFor, "Map from a register to it's horn leaf\n\nC++: pystorm::bddriver::bdpars::BDPars::HornLeafIdFor(pystorm::bddriver::bdpars::RegId) const --> pystorm::bddriver::bdpars::HornLeafId", pybind11::arg("object"));
		cl.def("HornLeafIdFor", (pystorm::bddriver::bdpars::HornLeafId (pystorm::bddriver::bdpars::BDPars::*)(pystorm::bddriver::bdpars::InputId) const) &pystorm::bddriver::bdpars::BDPars::HornLeafIdFor, "Map from an input to it's horn leaf\n\nC++: pystorm::bddriver::bdpars::BDPars::HornLeafIdFor(pystorm::bddriver::bdpars::InputId) const --> pystorm::bddriver::bdpars::HornLeafId", pybind11::arg("object"));
		cl.def("FunnelLeafIdFor", (pystorm::bddriver::bdpars::FunnelLeafId (pystorm::bddriver::bdpars::BDPars::*)(pystorm::bddriver::bdpars::MemId) const) &pystorm::bddriver::bdpars::BDPars::FunnelLeafIdFor, "Map from a memory to it's dump funnel leaf\n\nC++: pystorm::bddriver::bdpars::BDPars::FunnelLeafIdFor(pystorm::bddriver::bdpars::MemId) const --> pystorm::bddriver::bdpars::FunnelLeafId", pybind11::arg("object"));
		cl.def("FunnelLeafIdFor", (pystorm::bddriver::bdpars::FunnelLeafId (pystorm::bddriver::bdpars::BDPars::*)(pystorm::bddriver::bdpars::OutputId) const) &pystorm::bddriver::bdpars::BDPars::FunnelLeafIdFor, "Map from an output to it's funnel leaf\n\nC++: pystorm::bddriver::bdpars::BDPars::FunnelLeafIdFor(pystorm::bddriver::bdpars::OutputId) const --> pystorm::bddriver::bdpars::FunnelLeafId", pybind11::arg("object"));
		cl.def("DelayRegForMem", (pystorm::bddriver::bdpars::RegId (pystorm::bddriver::bdpars::BDPars::*)(pystorm::bddriver::bdpars::MemId) const) &pystorm::bddriver::bdpars::BDPars::DelayRegForMem, "C++: pystorm::bddriver::bdpars::BDPars::DelayRegForMem(pystorm::bddriver::bdpars::MemId) const --> pystorm::bddriver::bdpars::RegId", pybind11::arg("object"));
		cl.def("ComponentTypeIdFor", (pystorm::bddriver::bdpars::ComponentTypeId (pystorm::bddriver::bdpars::BDPars::*)(pystorm::bddriver::bdpars::HornLeafId) const) &pystorm::bddriver::bdpars::BDPars::ComponentTypeIdFor, "going from a FH leaf to the value of the associate with the type of the component it services\n\nC++: pystorm::bddriver::bdpars::BDPars::ComponentTypeIdFor(pystorm::bddriver::bdpars::HornLeafId) const --> pystorm::bddriver::bdpars::ComponentTypeId", pybind11::arg("leaf"));
		cl.def("ComponentTypeIdFor", (pystorm::bddriver::bdpars::ComponentTypeId (pystorm::bddriver::bdpars::BDPars::*)(pystorm::bddriver::bdpars::FunnelLeafId) const) &pystorm::bddriver::bdpars::BDPars::ComponentTypeIdFor, "going from a FH leaf to the value of the associate with the type of the component it services\n\nC++: pystorm::bddriver::bdpars::BDPars::ComponentTypeIdFor(pystorm::bddriver::bdpars::FunnelLeafId) const --> pystorm::bddriver::bdpars::ComponentTypeId", pybind11::arg("leaf"));
		cl.def("ComponentIdxFor", (unsigned int (pystorm::bddriver::bdpars::BDPars::*)(pystorm::bddriver::bdpars::HornLeafId) const) &pystorm::bddriver::bdpars::BDPars::ComponentIdxFor, "going from a FH leaf to the value of the associate with the component it services\n\nC++: pystorm::bddriver::bdpars::BDPars::ComponentIdxFor(pystorm::bddriver::bdpars::HornLeafId) const --> unsigned int", pybind11::arg("leaf"));
		cl.def("ComponentIdxFor", (unsigned int (pystorm::bddriver::bdpars::BDPars::*)(pystorm::bddriver::bdpars::FunnelLeafId) const) &pystorm::bddriver::bdpars::BDPars::ComponentIdxFor, "going from a FH leaf to the value of the associate with the component it services\n\nC++: pystorm::bddriver::bdpars::BDPars::ComponentIdxFor(pystorm::bddriver::bdpars::FunnelLeafId) const --> unsigned int", pybind11::arg("leaf"));
		cl.def("Width", (unsigned int (pystorm::bddriver::bdpars::BDPars::*)(pystorm::bddriver::bdpars::HornLeafId) const) &pystorm::bddriver::bdpars::BDPars::Width, "Get data width (after deserialization) at horn leaf\n\nC++: pystorm::bddriver::bdpars::BDPars::Width(pystorm::bddriver::bdpars::HornLeafId) const --> unsigned int", pybind11::arg("object"));
		cl.def("Width", (unsigned int (pystorm::bddriver::bdpars::BDPars::*)(pystorm::bddriver::bdpars::FunnelLeafId) const) &pystorm::bddriver::bdpars::BDPars::Width, "Get data width (after deserialization) at funnel leaf\n\nC++: pystorm::bddriver::bdpars::BDPars::Width(pystorm::bddriver::bdpars::FunnelLeafId) const --> unsigned int", pybind11::arg("object"));
		cl.def("Width", (unsigned int (pystorm::bddriver::bdpars::BDPars::*)(pystorm::bddriver::bdpars::MiscWidthId) const) &pystorm::bddriver::bdpars::BDPars::Width, "Get data width of some misc hardware thing\n\nC++: pystorm::bddriver::bdpars::BDPars::Width(pystorm::bddriver::bdpars::MiscWidthId) const --> unsigned int", pybind11::arg("object"));
		cl.def("Size", (unsigned int (pystorm::bddriver::bdpars::BDPars::*)(const pystorm::bddriver::bdpars::MemId) const) &pystorm::bddriver::bdpars::BDPars::Size, "return the capacity of a memory\n\nC++: pystorm::bddriver::bdpars::BDPars::Size(const pystorm::bddriver::bdpars::MemId) const --> unsigned int", pybind11::arg("object"));
		cl.def("DACSignalIdToDACRegisterId", (pystorm::bddriver::bdpars::RegId (pystorm::bddriver::bdpars::BDPars::*)(pystorm::bddriver::bdpars::DACSignalId) const) &pystorm::bddriver::bdpars::BDPars::DACSignalIdToDACRegisterId, "Map from a DAC signal name to it's register id\n\nC++: pystorm::bddriver::bdpars::BDPars::DACSignalIdToDACRegisterId(pystorm::bddriver::bdpars::DACSignalId) const --> pystorm::bddriver::bdpars::RegId", pybind11::arg("id"));
		cl.def("NumReg", (unsigned int (pystorm::bddriver::bdpars::BDPars::*)() const) &pystorm::bddriver::bdpars::BDPars::NumReg, "Get the total number of registers\n\nC++: pystorm::bddriver::bdpars::BDPars::NumReg() const --> unsigned int");
		cl.def("NumCores", (unsigned int (pystorm::bddriver::bdpars::BDPars::*)() const) &pystorm::bddriver::bdpars::BDPars::NumCores, "Get the total number of cores\n\nC++: pystorm::bddriver::bdpars::BDPars::NumCores() const --> unsigned int");
	}
}


// File: unknown/unknown_4.cpp
#include <initializer_list>
#include <iterator>
#include <memory>
#include <sstream> // __str__
#include <string>

#include <pybind11/pybind11.h>

#ifndef BINDER_PYBIND11_TYPE_CASTER
	#define BINDER_PYBIND11_TYPE_CASTER
	PYBIND11_DECLARE_HOLDER_TYPE(T, std::shared_ptr<T>);
	PYBIND11_DECLARE_HOLDER_TYPE(T, T*);
	PYBIND11_MAKE_OPAQUE(std::shared_ptr<void>);
#endif

void bind_unknown_unknown_4(std::function< pybind11::module &(std::string const &namespace_) > &M)
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

	// pystorm::bddriver::driverpars::CommType file: line:43
	pybind11::enum_<pystorm::bddriver::driverpars::CommType>(M("pystorm::bddriver::driverpars"), "CommType", "")
		.value("SOFT", pystorm::bddriver::driverpars::CommType::SOFT)
		.value("BDMODEL", pystorm::bddriver::driverpars::CommType::BDMODEL)
		.value("LIBUSB", pystorm::bddriver::driverpars::CommType::LIBUSB)
		.value("LastCommType", pystorm::bddriver::driverpars::CommType::LastCommType)
		.export_values();

;

	{ // pystorm::bddriver::driverpars::DriverPars file: line:52
		pybind11::class_<pystorm::bddriver::driverpars::DriverPars, std::shared_ptr<pystorm::bddriver::driverpars::DriverPars>> cl(M("pystorm::bddriver::driverpars"), "DriverPars", "Stores parameters that modify driver object parameters/functions");
		pybind11::handle cl_type = cl;

		cl.def(pybind11::init<>());

		cl.def("Get", (unsigned int (pystorm::bddriver::driverpars::DriverPars::*)(pystorm::bddriver::driverpars::DriverParId) const) &pystorm::bddriver::driverpars::DriverPars::Get, "C++: pystorm::bddriver::driverpars::DriverPars::Get(pystorm::bddriver::driverpars::DriverParId) const --> unsigned int", pybind11::arg("par_id"));
		cl.def("Get", (const std::string * (pystorm::bddriver::driverpars::DriverPars::*)(pystorm::bddriver::driverpars::DriverStringParId) const) &pystorm::bddriver::driverpars::DriverPars::Get, "C++: pystorm::bddriver::driverpars::DriverPars::Get(pystorm::bddriver::driverpars::DriverStringParId) const --> const std::string *", pybind11::return_value_policy::automatic, pybind11::arg("par_id"));
	}
}


// File: unknown/unknown_5.cpp
#include <Driver.h>
#include <array>
#include <functional>
#include <initializer_list>
#include <iterator>
#include <map>
#include <memory>
#include <sstream> // __str__
#include <string>
#include <tuple>
#include <utility>
#include <vector>

#include <pybind11/pybind11.h>

#ifndef BINDER_PYBIND11_TYPE_CASTER
	#define BINDER_PYBIND11_TYPE_CASTER
	PYBIND11_DECLARE_HOLDER_TYPE(T, std::shared_ptr<T>);
	PYBIND11_DECLARE_HOLDER_TYPE(T, T*);
	PYBIND11_MAKE_OPAQUE(std::shared_ptr<void>);
#endif

// pystorm::bddriver::Xcoder file: line:17
struct PyCallBack_Xcoder_unsigned_char_pystorm_bddriver_DecOutput_t : public pystorm::bddriver::Xcoder<unsigned char,pystorm::bddriver::DecOutput> {
	using pystorm::bddriver::Xcoder<unsigned char,pystorm::bddriver::DecOutput>::Xcoder;

	void RunOnce() override { 
		pybind11::gil_scoped_acquire gil;
		pybind11::function overload = pybind11::get_overload(static_cast<const pystorm::bddriver::Xcoder<unsigned char,pystorm::bddriver::DecOutput> *>(this), "RunOnce");
		if (overload) {
			auto o = overload.operator()<pybind11::return_value_policy::reference>();
			if (pybind11::detail::cast_is_temporary_value_reference<void>::value) {
				static pybind11::detail::overload_caster_t<void> caster;
				return pybind11::detail::cast_ref<void>(std::move(o), caster);
			}
			else return pybind11::detail::cast_safe<void>(std::move(o));
		}
		pybind11::pybind11_fail("Tried to call pure virtual function \"Xcoder::RunOnce\"");
	}
};

// pystorm::bddriver::Xcoder file: line:17
struct PyCallBack_Xcoder_pystorm_bddriver_EncInput_unsigned_char_t : public pystorm::bddriver::Xcoder<pystorm::bddriver::EncInput,unsigned char> {
	using pystorm::bddriver::Xcoder<pystorm::bddriver::EncInput,unsigned char>::Xcoder;

	void RunOnce() override { 
		pybind11::gil_scoped_acquire gil;
		pybind11::function overload = pybind11::get_overload(static_cast<const pystorm::bddriver::Xcoder<pystorm::bddriver::EncInput,unsigned char> *>(this), "RunOnce");
		if (overload) {
			auto o = overload.operator()<pybind11::return_value_policy::reference>();
			if (pybind11::detail::cast_is_temporary_value_reference<void>::value) {
				static pybind11::detail::overload_caster_t<void> caster;
				return pybind11::detail::cast_ref<void>(std::move(o), caster);
			}
			else return pybind11::detail::cast_safe<void>(std::move(o));
		}
		pybind11::pybind11_fail("Tried to call pure virtual function \"Xcoder::RunOnce\"");
	}
};

// pystorm::bddriver::Decoder file: line:18
struct PyCallBack_Decoder : public pystorm::bddriver::Decoder {
	using pystorm::bddriver::Decoder::Decoder;

	void RunOnce() override { 
		pybind11::gil_scoped_acquire gil;
		pybind11::function overload = pybind11::get_overload(static_cast<const pystorm::bddriver::Decoder *>(this), "RunOnce");
		if (overload) {
			auto o = overload.operator()<pybind11::return_value_policy::reference>();
			if (pybind11::detail::cast_is_temporary_value_reference<void>::value) {
				static pybind11::detail::overload_caster_t<void> caster;
				return pybind11::detail::cast_ref<void>(std::move(o), caster);
			}
			else return pybind11::detail::cast_safe<void>(std::move(o));
		}
		pybind11::pybind11_fail("Tried to call pure virtual function \"Xcoder::RunOnce\"");
	}
};

// pystorm::bddriver::Encoder file: line:17
struct PyCallBack_Encoder : public pystorm::bddriver::Encoder {
	using pystorm::bddriver::Encoder::Encoder;

	void RunOnce() override { 
		pybind11::gil_scoped_acquire gil;
		pybind11::function overload = pybind11::get_overload(static_cast<const pystorm::bddriver::Encoder *>(this), "RunOnce");
		if (overload) {
			auto o = overload.operator()<pybind11::return_value_policy::reference>();
			if (pybind11::detail::cast_is_temporary_value_reference<void>::value) {
				static pybind11::detail::overload_caster_t<void> caster;
				return pybind11::detail::cast_ref<void>(std::move(o), caster);
			}
			else return pybind11::detail::cast_safe<void>(std::move(o));
		}
		pybind11::pybind11_fail("Tried to call pure virtual function \"Xcoder::RunOnce\"");
	}
};

void bind_unknown_unknown_5(std::function< pybind11::module &(std::string const &namespace_) > &M)
{
	{ // pystorm::bddriver::DecOutput file: line:16
		pybind11::class_<pystorm::bddriver::DecOutput, std::shared_ptr<pystorm::bddriver::DecOutput>> cl(M("pystorm::bddriver"), "DecOutput", "/////////////////////////////////////");
		pybind11::handle cl_type = cl;

		cl.def(pybind11::init<>());
		cl.def_readwrite("payload", &pystorm::bddriver::DecOutput::payload);
		cl.def_readwrite("core_id", &pystorm::bddriver::DecOutput::core_id);
		cl.def_readwrite("time_epoch", &pystorm::bddriver::DecOutput::time_epoch);
	}
	{ // pystorm::bddriver::EncInput file: line:24
		pybind11::class_<pystorm::bddriver::EncInput, std::shared_ptr<pystorm::bddriver::EncInput>> cl(M("pystorm::bddriver"), "EncInput", "");
		pybind11::handle cl_type = cl;

		cl.def(pybind11::init<>());

		cl.def(pybind11::init<const struct pystorm::bddriver::EncInput &>(), pybind11::arg(""));

		cl.def_readwrite("core_id", &pystorm::bddriver::EncInput::core_id);
		cl.def_readwrite("leaf_id", &pystorm::bddriver::EncInput::leaf_id);
		cl.def_readwrite("payload", &pystorm::bddriver::EncInput::payload);
	}
	{ // pystorm::bddriver::BDState file: line:23
		pybind11::class_<pystorm::bddriver::BDState, std::shared_ptr<pystorm::bddriver::BDState>> cl(M("pystorm::bddriver"), "BDState", "Keeps track of currently set register values, toggle states, memory values, etc.\n\n Also encodes timing assumptions: e.g. as soon as the traffic toggles are turned\n off in software, it is not necessarily safe to start programming memories:\n there is some amount of time that we must wait for the traffic to drain completely.\n the length of this delay is kept in DriverPars, and is used by BDState to implement\n an interface that the driver can use to block until it is safe.");
		pybind11::handle cl_type = cl;

		cl.def(pybind11::init<const class pystorm::bddriver::bdpars::BDPars *, const class pystorm::bddriver::driverpars::DriverPars *>(), pybind11::arg("bd_pars"), pybind11::arg("driver_pars"));

		cl.def(pybind11::init<const class pystorm::bddriver::BDState &>(), pybind11::arg(""));

		cl.def("SetMem", (void (pystorm::bddriver::BDState::*)(pystorm::bddriver::bdpars::MemId, unsigned int, const class std::vector<class pystorm::bddriver::BDWord, class std::allocator<class pystorm::bddriver::BDWord> > &)) &pystorm::bddriver::BDState::SetMem, "C++: pystorm::bddriver::BDState::SetMem(pystorm::bddriver::bdpars::MemId, unsigned int, const class std::vector<class pystorm::bddriver::BDWord, class std::allocator<class pystorm::bddriver::BDWord> > &) --> void", pybind11::arg("mem_id"), pybind11::arg("start_addr"), pybind11::arg("data"));
		cl.def("GetMem", (const class std::vector<class pystorm::bddriver::BDWord, class std::allocator<class pystorm::bddriver::BDWord> > * (pystorm::bddriver::BDState::*)(pystorm::bddriver::bdpars::MemId) const) &pystorm::bddriver::BDState::GetMem, "C++: pystorm::bddriver::BDState::GetMem(pystorm::bddriver::bdpars::MemId) const --> const class std::vector<class pystorm::bddriver::BDWord, class std::allocator<class pystorm::bddriver::BDWord> > *", pybind11::return_value_policy::automatic, pybind11::arg("mem_id"));
		cl.def("SetReg", (void (pystorm::bddriver::BDState::*)(pystorm::bddriver::bdpars::RegId, class pystorm::bddriver::BDWord)) &pystorm::bddriver::BDState::SetReg, "C++: pystorm::bddriver::BDState::SetReg(pystorm::bddriver::bdpars::RegId, class pystorm::bddriver::BDWord) --> void", pybind11::arg("reg_id"), pybind11::arg("data"));
		cl.def("GetReg", (const struct std::pair<const class pystorm::bddriver::BDWord *, bool> (pystorm::bddriver::BDState::*)(pystorm::bddriver::bdpars::RegId) const) &pystorm::bddriver::BDState::GetReg, "C++: pystorm::bddriver::BDState::GetReg(pystorm::bddriver::bdpars::RegId) const --> const struct std::pair<const class pystorm::bddriver::BDWord *, bool>", pybind11::arg("reg_id"));
		cl.def("SetNeuronConfigMem", (void (pystorm::bddriver::BDState::*)(unsigned int, unsigned int, unsigned int, pystorm::bddriver::bdpars::ConfigSomaID, unsigned int)) &pystorm::bddriver::BDState::SetNeuronConfigMem, "C++: pystorm::bddriver::BDState::SetNeuronConfigMem(unsigned int, unsigned int, unsigned int, pystorm::bddriver::bdpars::ConfigSomaID, unsigned int) --> void", pybind11::arg("core_id"), pybind11::arg("tile_id"), pybind11::arg("elem_id"), pybind11::arg("config_type"), pybind11::arg("config_value"));
		cl.def("SetNeuronConfigMem", (void (pystorm::bddriver::BDState::*)(unsigned int, unsigned int, unsigned int, pystorm::bddriver::bdpars::ConfigSynapseID, unsigned int)) &pystorm::bddriver::BDState::SetNeuronConfigMem, "C++: pystorm::bddriver::BDState::SetNeuronConfigMem(unsigned int, unsigned int, unsigned int, pystorm::bddriver::bdpars::ConfigSynapseID, unsigned int) --> void", pybind11::arg("core_id"), pybind11::arg("tile_id"), pybind11::arg("elem_id"), pybind11::arg("config_type"), pybind11::arg("config_value"));
		cl.def("SetNeuronConfigMem", (void (pystorm::bddriver::BDState::*)(unsigned int, unsigned int, unsigned int, pystorm::bddriver::bdpars::DiffusorCutLocationId, unsigned int)) &pystorm::bddriver::BDState::SetNeuronConfigMem, "C++: pystorm::bddriver::BDState::SetNeuronConfigMem(unsigned int, unsigned int, unsigned int, pystorm::bddriver::bdpars::DiffusorCutLocationId, unsigned int) --> void", pybind11::arg("core_id"), pybind11::arg("tile_id"), pybind11::arg("elem_id"), pybind11::arg("config_type"), pybind11::arg("config_value"));
		cl.def("SetToggle", (void (pystorm::bddriver::BDState::*)(pystorm::bddriver::bdpars::RegId, bool, bool)) &pystorm::bddriver::BDState::SetToggle, "C++: pystorm::bddriver::BDState::SetToggle(pystorm::bddriver::bdpars::RegId, bool, bool) --> void", pybind11::arg("reg_id"), pybind11::arg("traffic_en"), pybind11::arg("dump_en"));
		cl.def("GetToggle", (class std::tuple<bool, bool, bool> (pystorm::bddriver::BDState::*)(pystorm::bddriver::bdpars::RegId) const) &pystorm::bddriver::BDState::GetToggle, "C++: pystorm::bddriver::BDState::GetToggle(pystorm::bddriver::bdpars::RegId) const --> class std::tuple<bool, bool, bool>", pybind11::arg("reg_id"));
		cl.def("AreTrafficRegsOff", (bool (pystorm::bddriver::BDState::*)() const) &pystorm::bddriver::BDState::AreTrafficRegsOff, "C++: pystorm::bddriver::BDState::AreTrafficRegsOff() const --> bool");
		cl.def("IsTrafficOff", (bool (pystorm::bddriver::BDState::*)() const) &pystorm::bddriver::BDState::IsTrafficOff, "is traffic_en == false for all traffic_regs_\n\nC++: pystorm::bddriver::BDState::IsTrafficOff() const --> bool");
		cl.def("WaitForTrafficOff", (void (pystorm::bddriver::BDState::*)() const) &pystorm::bddriver::BDState::WaitForTrafficOff, "has AreTrafficRegsOff been true for traffic_drain_us\n\nC++: pystorm::bddriver::BDState::WaitForTrafficOff() const --> void");
	}
	{ // pystorm::bddriver::Xcoder file: line:17
		pybind11::class_<pystorm::bddriver::Xcoder<unsigned char,pystorm::bddriver::DecOutput>, std::shared_ptr<pystorm::bddriver::Xcoder<unsigned char,pystorm::bddriver::DecOutput>>, PyCallBack_Xcoder_unsigned_char_pystorm_bddriver_DecOutput_t> cl(M("pystorm::bddriver"), "Xcoder_unsigned_char_pystorm_bddriver_DecOutput_t", "");
		pybind11::handle cl_type = cl;

		cl.def("Start", (void (pystorm::bddriver::Xcoder<unsigned char,pystorm::bddriver::DecOutput>::*)()) &pystorm::bddriver::Xcoder<unsigned char, pystorm::bddriver::DecOutput>::Start, "C++: pystorm::bddriver::Xcoder<unsigned char, pystorm::bddriver::DecOutput>::Start() --> void");
		cl.def("Stop", (void (pystorm::bddriver::Xcoder<unsigned char,pystorm::bddriver::DecOutput>::*)()) &pystorm::bddriver::Xcoder<unsigned char, pystorm::bddriver::DecOutput>::Stop, "C++: pystorm::bddriver::Xcoder<unsigned char, pystorm::bddriver::DecOutput>::Stop() --> void");
	}
	{ // pystorm::bddriver::Xcoder file: line:17
		pybind11::class_<pystorm::bddriver::Xcoder<pystorm::bddriver::EncInput,unsigned char>, std::shared_ptr<pystorm::bddriver::Xcoder<pystorm::bddriver::EncInput,unsigned char>>, PyCallBack_Xcoder_pystorm_bddriver_EncInput_unsigned_char_t> cl(M("pystorm::bddriver"), "Xcoder_pystorm_bddriver_EncInput_unsigned_char_t", "");
		pybind11::handle cl_type = cl;

		cl.def("Start", (void (pystorm::bddriver::Xcoder<pystorm::bddriver::EncInput,unsigned char>::*)()) &pystorm::bddriver::Xcoder<pystorm::bddriver::EncInput, unsigned char>::Start, "C++: pystorm::bddriver::Xcoder<pystorm::bddriver::EncInput, unsigned char>::Start() --> void");
		cl.def("Stop", (void (pystorm::bddriver::Xcoder<pystorm::bddriver::EncInput,unsigned char>::*)()) &pystorm::bddriver::Xcoder<pystorm::bddriver::EncInput, unsigned char>::Stop, "C++: pystorm::bddriver::Xcoder<pystorm::bddriver::EncInput, unsigned char>::Stop() --> void");
	}
	{ // pystorm::bddriver::Decoder file: line:18
		pybind11::class_<pystorm::bddriver::Decoder, std::shared_ptr<pystorm::bddriver::Decoder>, PyCallBack_Decoder, pystorm::bddriver::Xcoder<unsigned char,pystorm::bddriver::DecOutput>> cl(M("pystorm::bddriver"), "Decoder", "");
		pybind11::handle cl_type = cl;

	}
	{ // pystorm::bddriver::Encoder file: line:17
		pybind11::class_<pystorm::bddriver::Encoder, std::shared_ptr<pystorm::bddriver::Encoder>, PyCallBack_Encoder, pystorm::bddriver::Xcoder<pystorm::bddriver::EncInput,unsigned char>> cl(M("pystorm::bddriver"), "Encoder", "");
		pybind11::handle cl_type = cl;

	}
	{ // pystorm::bddriver::Driver file:Driver.h line:99
		pybind11::class_<pystorm::bddriver::Driver, std::shared_ptr<pystorm::bddriver::Driver>> cl(M("pystorm::bddriver"), "Driver", "Driver provides low-level, but not dead-stupid, control over the BD hardware.\n Driver tries to provide a complete but not needlessly tedious interface to BD.\n It also tries to prevent the user to do anything that would crash the chip.\n\n Driver looks like this:\n\n                              (user/HAL)\n\n  ---[fns]--[fns]--[fns]----------------------[fns]-----------------------[fns]----  API\n       |      |      |            |             A                           A\n       V      V      V            |             |                           |\n  [private fns, e.g. PackWords]   |        [XXXX private fns, e.g. UnpackWords XXXX]\n          |        |              |             A                           A\n          V        V           [BDState]        |                           |\n   [MutexBuffer:enc_buf_in_]      |      [M.B.:dec_buf_out_[0]]    [M.B.:dec_buf_out_[0]] ...\n              |                   |                   A                   A\n              |                   |                   |                   |\n   ----------------------------[BDPars]------------------------------------------- funnel/horn payloads,\n              |                   |                   |                   |           organized by leaf\n              V                   |                   |                   |\n      [Encoder:encoder_]          |        [XXXXXXXXXXXX Decoder:decoder_ XXXXXXXXXX]\n              |                   |                        A\n              V                   |                        |\n   [MutexBuffer:enc_buf_out_]     |           [MutexBuffer:dec_buf_in_]\n              |                   |                      A\n              |                   |                      |\n  --------------------------------------------------------------------------------- raw data\n              |                                          |\n              V                                          |\n         [XXXXXXXXXXXXXXXXXXXX Comm:comm_ XXXXXXXXXXXXXXXXXXXX]\n                               |      A\n                               V      |\n  --------------------------------------------------------------------------------- USB\n\n                              (Braindrop)\n\n At the heart of driver are a few primary components:\n\n - Encoder\n     Inputs: raw payloads (already serialized, if necessary) and BD horn ids to send them to\n     Outputs: inputs suitable to send to BD, packed into char stream\n     Spawns its own thread.\n\n - Decoder\n     Inputs: char stream of outputs from BD\n     Outputs: one stream per horn leaf of raw payloads from that leaf\n     Spawns its own thread.\n\n - Comm\n     Communicates with BD using libUSB, taking inputs from/giving outputs to\n     the Encoder/Decoder. Spawns its own thread.\n\n - MutexBuffers\n     Provide thread-safe communication and buffering for the inputs and outputs of Encoder\n     and decoder. Note that there are many decoder output buffers, one per funnel leaf.\n\n - BDPars\n     Holds all the nitty-gritty hardware information. The rest of the driver doesn't know\n     anything about word field orders or sizes, for example.\n\n - BDState\n     Software model of the hardware state. Keep track of all the memory words that have\n     been programmed, registers that have been set, etc.\n     Also keeps track of timing assumptions, e.g. whether the traffic has drained after\n     turning off all of the toggles that stop it.");
		pybind11::handle cl_type = cl;

		cl.def(pybind11::init<>());

		cl.def(pybind11::init<const class pystorm::bddriver::Driver &>(), pybind11::arg(""));

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
		cl.def("SetConfigMemory", (void (pystorm::bddriver::Driver::*)(unsigned int, unsigned int, class std::map<pystorm::bddriver::bdpars::ConfigSomaID, class std::vector<unsigned int, class std::allocator<unsigned int> >, struct std::less<pystorm::bddriver::bdpars::ConfigSomaID>, class std::allocator<struct std::pair<const pystorm::bddriver::bdpars::ConfigSomaID, class std::vector<unsigned int, class std::allocator<unsigned int> > > > >, pystorm::bddriver::bdpars::ConfigSomaID, unsigned int)) &pystorm::bddriver::Driver::SetConfigMemory<pystorm::bddriver::bdpars::ConfigSomaID>, "C++: pystorm::bddriver::Driver::SetConfigMemory(unsigned int, unsigned int, class std::map<pystorm::bddriver::bdpars::ConfigSomaID, class std::vector<unsigned int, class std::allocator<unsigned int> >, struct std::less<pystorm::bddriver::bdpars::ConfigSomaID>, class std::allocator<struct std::pair<const pystorm::bddriver::bdpars::ConfigSomaID, class std::vector<unsigned int, class std::allocator<unsigned int> > > > >, pystorm::bddriver::bdpars::ConfigSomaID, unsigned int) --> void", pybind11::arg("core_id"), pybind11::arg("elem_id"), pybind11::arg("config_map"), pybind11::arg("config_type"), pybind11::arg("config_value"));
		cl.def("SetConfigMemory", (void (pystorm::bddriver::Driver::*)(unsigned int, unsigned int, class std::map<pystorm::bddriver::bdpars::ConfigSynapseID, class std::vector<unsigned int, class std::allocator<unsigned int> >, struct std::less<pystorm::bddriver::bdpars::ConfigSynapseID>, class std::allocator<struct std::pair<const pystorm::bddriver::bdpars::ConfigSynapseID, class std::vector<unsigned int, class std::allocator<unsigned int> > > > >, pystorm::bddriver::bdpars::ConfigSynapseID, unsigned int)) &pystorm::bddriver::Driver::SetConfigMemory<pystorm::bddriver::bdpars::ConfigSynapseID>, "C++: pystorm::bddriver::Driver::SetConfigMemory(unsigned int, unsigned int, class std::map<pystorm::bddriver::bdpars::ConfigSynapseID, class std::vector<unsigned int, class std::allocator<unsigned int> >, struct std::less<pystorm::bddriver::bdpars::ConfigSynapseID>, class std::allocator<struct std::pair<const pystorm::bddriver::bdpars::ConfigSynapseID, class std::vector<unsigned int, class std::allocator<unsigned int> > > > >, pystorm::bddriver::bdpars::ConfigSynapseID, unsigned int) --> void", pybind11::arg("core_id"), pybind11::arg("elem_id"), pybind11::arg("config_map"), pybind11::arg("config_type"), pybind11::arg("config_value"));
		cl.def("SetConfigMemory", (void (pystorm::bddriver::Driver::*)(unsigned int, unsigned int, class std::map<pystorm::bddriver::bdpars::DiffusorCutLocationId, class std::vector<unsigned int, class std::allocator<unsigned int> >, struct std::less<pystorm::bddriver::bdpars::DiffusorCutLocationId>, class std::allocator<struct std::pair<const pystorm::bddriver::bdpars::DiffusorCutLocationId, class std::vector<unsigned int, class std::allocator<unsigned int> > > > >, pystorm::bddriver::bdpars::DiffusorCutLocationId, unsigned int)) &pystorm::bddriver::Driver::SetConfigMemory<pystorm::bddriver::bdpars::DiffusorCutLocationId>, "C++: pystorm::bddriver::Driver::SetConfigMemory(unsigned int, unsigned int, class std::map<pystorm::bddriver::bdpars::DiffusorCutLocationId, class std::vector<unsigned int, class std::allocator<unsigned int> >, struct std::less<pystorm::bddriver::bdpars::DiffusorCutLocationId>, class std::allocator<struct std::pair<const pystorm::bddriver::bdpars::DiffusorCutLocationId, class std::vector<unsigned int, class std::allocator<unsigned int> > > > >, pystorm::bddriver::bdpars::DiffusorCutLocationId, unsigned int) --> void", pybind11::arg("core_id"), pybind11::arg("elem_id"), pybind11::arg("config_map"), pybind11::arg("config_type"), pybind11::arg("config_value"));
		cl.def("GetBDPars", (const class pystorm::bddriver::bdpars::BDPars * (pystorm::bddriver::Driver::*)()) &pystorm::bddriver::Driver::GetBDPars, "C++: pystorm::bddriver::Driver::GetBDPars() --> const class pystorm::bddriver::bdpars::BDPars *", pybind11::return_value_policy::automatic);
		cl.def("GetDriverPars", (const class pystorm::bddriver::driverpars::DriverPars * (pystorm::bddriver::Driver::*)()) &pystorm::bddriver::Driver::GetDriverPars, "C++: pystorm::bddriver::Driver::GetDriverPars() --> const class pystorm::bddriver::driverpars::DriverPars *", pybind11::return_value_policy::automatic);
		cl.def("GetState", (const class pystorm::bddriver::BDState * (pystorm::bddriver::Driver::*)(unsigned int)) &pystorm::bddriver::Driver::GetState, "C++: pystorm::bddriver::Driver::GetState(unsigned int) --> const class pystorm::bddriver::BDState *", pybind11::return_value_policy::automatic, pybind11::arg("core_id"));
		cl.def("testcall", (void (pystorm::bddriver::Driver::*)(const std::string &)) &pystorm::bddriver::Driver::testcall, "C++: pystorm::bddriver::Driver::testcall(const class std::__cxx11::basic_string<char> &) --> void", pybind11::arg("msg"));
		cl.def("Start", (void (pystorm::bddriver::Driver::*)()) &pystorm::bddriver::Driver::Start, "starts child workers, e.g. encoder and decoder\n\nC++: pystorm::bddriver::Driver::Start() --> void");
		cl.def("Stop", (void (pystorm::bddriver::Driver::*)()) &pystorm::bddriver::Driver::Stop, "stops the child workers\n\nC++: pystorm::bddriver::Driver::Stop() --> void");
		cl.def("InitBD", (void (pystorm::bddriver::Driver::*)()) &pystorm::bddriver::Driver::InitBD, "initializes hardware state\n\nC++: pystorm::bddriver::Driver::InitBD() --> void");
		cl.def("InitFIFO", (void (pystorm::bddriver::Driver::*)(unsigned int)) &pystorm::bddriver::Driver::InitFIFO, "C++: pystorm::bddriver::Driver::InitFIFO(unsigned int) --> void", pybind11::arg("core_id"));
		cl.def("SetTagTrafficState", (void (pystorm::bddriver::Driver::*)(unsigned int, bool)) &pystorm::bddriver::Driver::SetTagTrafficState, "Control tag traffic\n\nC++: pystorm::bddriver::Driver::SetTagTrafficState(unsigned int, bool) --> void", pybind11::arg("core_id"), pybind11::arg("en"));
		cl.def("SetSpikeTrafficState", (void (pystorm::bddriver::Driver::*)(unsigned int, bool)) &pystorm::bddriver::Driver::SetSpikeTrafficState, "Control spike traffic from neuron array to datapath\n\nC++: pystorm::bddriver::Driver::SetSpikeTrafficState(unsigned int, bool) --> void", pybind11::arg("core_id"), pybind11::arg("en"));
		cl.def("SetSpikeDumpState", (void (pystorm::bddriver::Driver::*)(unsigned int, bool)) &pystorm::bddriver::Driver::SetSpikeDumpState, "Control spike traffic from neuron array to driver\n\nC++: pystorm::bddriver::Driver::SetSpikeDumpState(unsigned int, bool) --> void", pybind11::arg("core_id"), pybind11::arg("en"));
		cl.def("SetDACValue", (void (pystorm::bddriver::Driver::*)(unsigned int, pystorm::bddriver::bdpars::DACSignalId, unsigned int)) &pystorm::bddriver::Driver::SetDACValue, "Program DAC value\n\nC++: pystorm::bddriver::Driver::SetDACValue(unsigned int, pystorm::bddriver::bdpars::DACSignalId, unsigned int) --> void", pybind11::arg("core_id"), pybind11::arg("signal_id"), pybind11::arg("value"));
		cl.def("SetDACtoADCConnectionState", (void (pystorm::bddriver::Driver::*)(unsigned int, pystorm::bddriver::bdpars::DACSignalId, bool)) &pystorm::bddriver::Driver::SetDACtoADCConnectionState, "Make DAC-to-ADC connection for calibration for a particular DAC\n\nC++: pystorm::bddriver::Driver::SetDACtoADCConnectionState(unsigned int, pystorm::bddriver::bdpars::DACSignalId, bool) --> void", pybind11::arg("core_id"), pybind11::arg("dac_signal_id"), pybind11::arg("en"));
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
		cl.def("SetMem", (void (pystorm::bddriver::Driver::*)(unsigned int, pystorm::bddriver::bdpars::MemId, const class std::vector<class pystorm::bddriver::BDWord, class std::allocator<class pystorm::bddriver::BDWord> > &, unsigned int)) &pystorm::bddriver::Driver::SetMem, "Program a memory.\n BDWords must be constructed as the correct word type for the mem_id\n\nC++: pystorm::bddriver::Driver::SetMem(unsigned int, pystorm::bddriver::bdpars::MemId, const class std::vector<class pystorm::bddriver::BDWord, class std::allocator<class pystorm::bddriver::BDWord> > &, unsigned int) --> void", pybind11::arg("core_id"), pybind11::arg("mem_id"), pybind11::arg("data"), pybind11::arg("start_addr"));
		cl.def("DumpMem", (class std::vector<class pystorm::bddriver::BDWord, class std::allocator<class pystorm::bddriver::BDWord> > (pystorm::bddriver::Driver::*)(unsigned int, pystorm::bddriver::bdpars::MemId)) &pystorm::bddriver::Driver::DumpMem, "Dump the contents of one of the memories.\n BDWords must subsequently be unpacked as the correct word type for the mem_id\n\nC++: pystorm::bddriver::Driver::DumpMem(unsigned int, pystorm::bddriver::bdpars::MemId) --> class std::vector<class pystorm::bddriver::BDWord, class std::allocator<class pystorm::bddriver::BDWord> >", pybind11::arg("core_id"), pybind11::arg("mem_id"));
		cl.def("SetPreFIFODumpState", (void (pystorm::bddriver::Driver::*)(unsigned int, bool)) &pystorm::bddriver::Driver::SetPreFIFODumpState, "Dump copy of traffic pre-FIFO\n\nC++: pystorm::bddriver::Driver::SetPreFIFODumpState(unsigned int, bool) --> void", pybind11::arg("core_id"), pybind11::arg("dump_en"));
		cl.def("SetPostFIFODumpState", (void (pystorm::bddriver::Driver::*)(unsigned int, bool)) &pystorm::bddriver::Driver::SetPostFIFODumpState, "Dump copy of traffic post-FIFO, tag msbs = 0\n\nC++: pystorm::bddriver::Driver::SetPostFIFODumpState(unsigned int, bool) --> void", pybind11::arg("core_id"), pybind11::arg("dump_en"));
		cl.def("GetPreFIFODump", (class std::vector<class pystorm::bddriver::BDWord, class std::allocator<class pystorm::bddriver::BDWord> > (pystorm::bddriver::Driver::*)(unsigned int, unsigned int)) &pystorm::bddriver::Driver::GetPreFIFODump, "Get pre-FIFO tags recorded during dump\n\nC++: pystorm::bddriver::Driver::GetPreFIFODump(unsigned int, unsigned int) --> class std::vector<class pystorm::bddriver::BDWord, class std::allocator<class pystorm::bddriver::BDWord> >", pybind11::arg("core_id"), pybind11::arg("n_tags"));
		cl.def("GetPostFIFODump", (struct std::pair<class std::vector<class pystorm::bddriver::BDWord, class std::allocator<class pystorm::bddriver::BDWord> >, class std::vector<class pystorm::bddriver::BDWord, class std::allocator<class pystorm::bddriver::BDWord> > > (pystorm::bddriver::Driver::*)(unsigned int, unsigned int, unsigned int)) &pystorm::bddriver::Driver::GetPostFIFODump, "Get post-FIFO tags recorded during dump\n\nC++: pystorm::bddriver::Driver::GetPostFIFODump(unsigned int, unsigned int, unsigned int) --> struct std::pair<class std::vector<class pystorm::bddriver::BDWord, class std::allocator<class pystorm::bddriver::BDWord> >, class std::vector<class pystorm::bddriver::BDWord, class std::allocator<class pystorm::bddriver::BDWord> > >", pybind11::arg("core_id"), pybind11::arg("n_tags0"), pybind11::arg("n_tags1"));
		cl.def("GetFIFOOverflowCounts", (struct std::pair<unsigned int, unsigned int> (pystorm::bddriver::Driver::*)(unsigned int)) &pystorm::bddriver::Driver::GetFIFOOverflowCounts, "Get warning count\n\nC++: pystorm::bddriver::Driver::GetFIFOOverflowCounts(unsigned int) --> struct std::pair<unsigned int, unsigned int>", pybind11::arg("core_id"));
		cl.def("SendSpikes", (void (pystorm::bddriver::Driver::*)(const class std::vector<unsigned int, class std::allocator<unsigned int> > &, const class std::vector<class pystorm::bddriver::BDWord, class std::allocator<class pystorm::bddriver::BDWord> > &, const class std::vector<unsigned int, class std::allocator<unsigned int> >)) &pystorm::bddriver::Driver::SendSpikes, "Send a stream of spikes to neurons\n\nC++: pystorm::bddriver::Driver::SendSpikes(const class std::vector<unsigned int, class std::allocator<unsigned int> > &, const class std::vector<class pystorm::bddriver::BDWord, class std::allocator<class pystorm::bddriver::BDWord> > &, const class std::vector<unsigned int, class std::allocator<unsigned int> >) --> void", pybind11::arg("core_ids"), pybind11::arg("spikes"), pybind11::arg("times"));
		cl.def("RecvSpikes", (class std::tuple<class std::vector<unsigned int, class std::allocator<unsigned int> >, class std::vector<class pystorm::bddriver::BDWord, class std::allocator<class pystorm::bddriver::BDWord> >, class std::vector<unsigned int, class std::allocator<unsigned int> > > (pystorm::bddriver::Driver::*)(unsigned int)) &pystorm::bddriver::Driver::RecvSpikes, "Receive a stream of spikes\n\nC++: pystorm::bddriver::Driver::RecvSpikes(unsigned int) --> class std::tuple<class std::vector<unsigned int, class std::allocator<unsigned int> >, class std::vector<class pystorm::bddriver::BDWord, class std::allocator<class pystorm::bddriver::BDWord> >, class std::vector<unsigned int, class std::allocator<unsigned int> > >", pybind11::arg("max_to_recv"));
		cl.def("SendTags", (void (pystorm::bddriver::Driver::*)(const class std::vector<unsigned int, class std::allocator<unsigned int> > &, const class std::vector<class pystorm::bddriver::BDWord, class std::allocator<class pystorm::bddriver::BDWord> > &, const class std::vector<unsigned int, class std::allocator<unsigned int> >)) &pystorm::bddriver::Driver::SendTags, "Send a stream of tags\n\nC++: pystorm::bddriver::Driver::SendTags(const class std::vector<unsigned int, class std::allocator<unsigned int> > &, const class std::vector<class pystorm::bddriver::BDWord, class std::allocator<class pystorm::bddriver::BDWord> > &, const class std::vector<unsigned int, class std::allocator<unsigned int> >) --> void", pybind11::arg("core_ids"), pybind11::arg("tags"), pybind11::arg("times"));
		cl.def("RecvTags", (class std::tuple<class std::vector<unsigned int, class std::allocator<unsigned int> >, class std::vector<class pystorm::bddriver::BDWord, class std::allocator<class pystorm::bddriver::BDWord> >, class std::vector<unsigned int, class std::allocator<unsigned int> > > (pystorm::bddriver::Driver::*)(unsigned int)) &pystorm::bddriver::Driver::RecvTags, "Receive a stream of tags\n receive from both tag output leaves, the Acc and TAT\n Use TATOutputTags to unpack\n\nC++: pystorm::bddriver::Driver::RecvTags(unsigned int) --> class std::tuple<class std::vector<unsigned int, class std::allocator<unsigned int> >, class std::vector<class pystorm::bddriver::BDWord, class std::allocator<class pystorm::bddriver::BDWord> >, class std::vector<unsigned int, class std::allocator<unsigned int> > >", pybind11::arg("max_to_recv"));
		cl.def("GetRegState", (const struct std::pair<const class pystorm::bddriver::BDWord *, bool> (pystorm::bddriver::Driver::*)(unsigned int, pystorm::bddriver::bdpars::RegId) const) &pystorm::bddriver::Driver::GetRegState, "Get register contents by name.\n\nC++: pystorm::bddriver::Driver::GetRegState(unsigned int, pystorm::bddriver::bdpars::RegId) const --> const struct std::pair<const class pystorm::bddriver::BDWord *, bool>", pybind11::arg("core_id"), pybind11::arg("reg_id"));
		cl.def("GetMemState", (const class std::vector<class pystorm::bddriver::BDWord, class std::allocator<class pystorm::bddriver::BDWord> > * (pystorm::bddriver::Driver::*)(pystorm::bddriver::bdpars::MemId, unsigned int) const) &pystorm::bddriver::Driver::GetMemState, "Get software state of memory contents: this DOES NOT dump the memory.\n\nC++: pystorm::bddriver::Driver::GetMemState(pystorm::bddriver::bdpars::MemId, unsigned int) const --> const class std::vector<class pystorm::bddriver::BDWord, class std::allocator<class pystorm::bddriver::BDWord> > *", pybind11::return_value_policy::automatic, pybind11::arg("mem_id"), pybind11::arg("core_id"));
	}
}


// File: unknown/unknown_6.cpp
#include <array>
#include <initializer_list>
#include <iterator>
#include <memory>
#include <sstream> // __str__
#include <string>
#include <tuple>
#include <utility>
#include <vector>

#include <pybind11/pybind11.h>

#ifndef BINDER_PYBIND11_TYPE_CASTER
	#define BINDER_PYBIND11_TYPE_CASTER
	PYBIND11_DECLARE_HOLDER_TYPE(T, std::shared_ptr<T>);
	PYBIND11_DECLARE_HOLDER_TYPE(T, T*);
	PYBIND11_MAKE_OPAQUE(std::shared_ptr<void>);
#endif

void bind_unknown_unknown_6(std::function< pybind11::module &(std::string const &namespace_) > &M)
{
	// pystorm::bddriver::bdmodel::FPGAInput(class std::vector<unsigned char, class std::allocator<unsigned char> >, const class pystorm::bddriver::bdpars::BDPars *) file: line:20
	M("pystorm::bddriver::bdmodel").def("FPGAInput", (class std::vector<unsigned int, class std::allocator<unsigned int> > (*)(class std::vector<unsigned char, class std::allocator<unsigned char> >, const class pystorm::bddriver::bdpars::BDPars *)) &pystorm::bddriver::bdmodel::FPGAInput, "Packs byte stream, will do other stuff at some point\n\nC++: pystorm::bddriver::bdmodel::FPGAInput(class std::vector<unsigned char, class std::allocator<unsigned char> >, const class pystorm::bddriver::bdpars::BDPars *) --> class std::vector<unsigned int, class std::allocator<unsigned int> >", pybind11::arg("inputs"), pybind11::arg("pars"));

	// pystorm::bddriver::bdmodel::Horn(const class std::vector<unsigned int, class std::allocator<unsigned int> > &, const class pystorm::bddriver::bdpars::BDPars *) file: line:23
	M("pystorm::bddriver::bdmodel").def("Horn", (struct std::array<class std::vector<unsigned int, class std::allocator<unsigned int> >, 34> (*)(const class std::vector<unsigned int, class std::allocator<unsigned int> > &, const class pystorm::bddriver::bdpars::BDPars *)) &pystorm::bddriver::bdmodel::Horn, "Does horn operation\n\nC++: pystorm::bddriver::bdmodel::Horn(const class std::vector<unsigned int, class std::allocator<unsigned int> > &, const class pystorm::bddriver::bdpars::BDPars *) --> struct std::array<class std::vector<unsigned int, class std::allocator<unsigned int> >, 34>", pybind11::arg("inputs"), pybind11::arg("pars"));

	// pystorm::bddriver::bdmodel::DeserializeHorn(const class std::vector<unsigned int, class std::allocator<unsigned int> > &, pystorm::bddriver::bdpars::HornLeafId, const class pystorm::bddriver::bdpars::BDPars *) file: line:27
	M("pystorm::bddriver::bdmodel").def("DeserializeHorn", (struct std::pair<class std::vector<unsigned long, class std::allocator<unsigned long> >, class std::vector<unsigned int, class std::allocator<unsigned int> > > (*)(const class std::vector<unsigned int, class std::allocator<unsigned int> > &, pystorm::bddriver::bdpars::HornLeafId, const class pystorm::bddriver::bdpars::BDPars *)) &pystorm::bddriver::bdmodel::DeserializeHorn, "Does deserialization\n\nC++: pystorm::bddriver::bdmodel::DeserializeHorn(const class std::vector<unsigned int, class std::allocator<unsigned int> > &, pystorm::bddriver::bdpars::HornLeafId, const class pystorm::bddriver::bdpars::BDPars *) --> struct std::pair<class std::vector<unsigned long, class std::allocator<unsigned long> >, class std::vector<unsigned int, class std::allocator<unsigned int> > >", pybind11::arg("inputs"), pybind11::arg("leaf_id"), pybind11::arg("bd_pars"));

	// pystorm::bddriver::bdmodel::SerializeFunnel(const class std::vector<unsigned long, class std::allocator<unsigned long> > &, pystorm::bddriver::bdpars::FunnelLeafId, const class pystorm::bddriver::bdpars::BDPars *) file: line:38
	M("pystorm::bddriver::bdmodel").def("SerializeFunnel", (struct std::pair<class std::vector<unsigned long, class std::allocator<unsigned long> >, unsigned int> (*)(const class std::vector<unsigned long, class std::allocator<unsigned long> > &, pystorm::bddriver::bdpars::FunnelLeafId, const class pystorm::bddriver::bdpars::BDPars *)) &pystorm::bddriver::bdmodel::SerializeFunnel, "Does serialization, returns pairs of {serialized words chunks, word chunk widths}\n\nC++: pystorm::bddriver::bdmodel::SerializeFunnel(const class std::vector<unsigned long, class std::allocator<unsigned long> > &, pystorm::bddriver::bdpars::FunnelLeafId, const class pystorm::bddriver::bdpars::BDPars *) --> struct std::pair<class std::vector<unsigned long, class std::allocator<unsigned long> >, unsigned int>", pybind11::arg("inputs"), pybind11::arg("leaf_id"), pybind11::arg("bd_pars"));

	// pystorm::bddriver::bdmodel::FPGAOutput(class std::vector<unsigned long, class std::allocator<unsigned long> >, const class pystorm::bddriver::bdpars::BDPars *) file: line:49
	M("pystorm::bddriver::bdmodel").def("FPGAOutput", (class std::vector<unsigned char, class std::allocator<unsigned char> > (*)(class std::vector<unsigned long, class std::allocator<unsigned long> >, const class pystorm::bddriver::bdpars::BDPars *)) &pystorm::bddriver::bdmodel::FPGAOutput, "Unpacks byte stream, will do other stuff eventually\n\nC++: pystorm::bddriver::bdmodel::FPGAOutput(class std::vector<unsigned long, class std::allocator<unsigned long> >, const class pystorm::bddriver::bdpars::BDPars *) --> class std::vector<unsigned char, class std::allocator<unsigned char> >", pybind11::arg("inputs"), pybind11::arg("pars"));

	{ // pystorm::bddriver::bdmodel::BDModel file: line:22
		pybind11::class_<pystorm::bddriver::bdmodel::BDModel, std::shared_ptr<pystorm::bddriver::bdmodel::BDModel>> cl(M("pystorm::bddriver::bdmodel"), "BDModel", "BDModel pretends to be the BD hardware.\n Public ifc is threadsafe.");
		pybind11::handle cl_type = cl;

		cl.def(pybind11::init<const class pystorm::bddriver::bdpars::BDPars *, const class pystorm::bddriver::driverpars::DriverPars *>(), pybind11::arg("bd_pars"), pybind11::arg("driver_pars"));

		cl.def("ParseInput", (void (pystorm::bddriver::bdmodel::BDModel::*)(const class std::vector<unsigned char, class std::allocator<unsigned char> > &)) &pystorm::bddriver::bdmodel::BDModel::ParseInput, "parse input stream to update internal BDState object and other state\n\nC++: pystorm::bddriver::bdmodel::BDModel::ParseInput(const class std::vector<unsigned char, class std::allocator<unsigned char> > &) --> void", pybind11::arg("input_stream"));
		cl.def("GenerateOutputs", (class std::vector<unsigned char, class std::allocator<unsigned char> > (pystorm::bddriver::bdmodel::BDModel::*)()) &pystorm::bddriver::bdmodel::BDModel::GenerateOutputs, "given internal state, generate requested output stream\n\nC++: pystorm::bddriver::bdmodel::BDModel::GenerateOutputs() --> class std::vector<unsigned char, class std::allocator<unsigned char> >");
		cl.def("PushOutput", (void (pystorm::bddriver::bdmodel::BDModel::*)(pystorm::bddriver::bdpars::OutputId, const class std::vector<class pystorm::bddriver::BDWord, class std::allocator<class pystorm::bddriver::BDWord> > &)) &pystorm::bddriver::bdmodel::BDModel::PushOutput, "C++: pystorm::bddriver::bdmodel::BDModel::PushOutput(pystorm::bddriver::bdpars::OutputId, const class std::vector<class pystorm::bddriver::BDWord, class std::allocator<class pystorm::bddriver::BDWord> > &) --> void", pybind11::arg("output_id"), pybind11::arg("to_append"));
		cl.def("LockState", (const class pystorm::bddriver::BDState * (pystorm::bddriver::bdmodel::BDModel::*)()) &pystorm::bddriver::bdmodel::BDModel::LockState, "lock the model, get a const ptr to the state, examine it as you like...\n\nC++: pystorm::bddriver::bdmodel::BDModel::LockState() --> const class pystorm::bddriver::BDState *", pybind11::return_value_policy::automatic);
		cl.def("UnlockState", (void (pystorm::bddriver::bdmodel::BDModel::*)()) &pystorm::bddriver::bdmodel::BDModel::UnlockState, "then unlock the model when you're done\n\nC++: pystorm::bddriver::bdmodel::BDModel::UnlockState() --> void");
		cl.def("PopSpikes", (class std::vector<class pystorm::bddriver::BDWord, class std::allocator<class pystorm::bddriver::BDWord> > (pystorm::bddriver::bdmodel::BDModel::*)()) &pystorm::bddriver::bdmodel::BDModel::PopSpikes, "C++: pystorm::bddriver::bdmodel::BDModel::PopSpikes() --> class std::vector<class pystorm::bddriver::BDWord, class std::allocator<class pystorm::bddriver::BDWord> >");
		cl.def("PopTags", (class std::vector<class pystorm::bddriver::BDWord, class std::allocator<class pystorm::bddriver::BDWord> > (pystorm::bddriver::bdmodel::BDModel::*)()) &pystorm::bddriver::bdmodel::BDModel::PopTags, "C++: pystorm::bddriver::bdmodel::BDModel::PopTags() --> class std::vector<class pystorm::bddriver::BDWord, class std::allocator<class pystorm::bddriver::BDWord> >");
	}
}


// File: unknown/unknown_7.cpp
#include <initializer_list>
#include <iterator>
#include <memory>
#include <sstream> // __str__
#include <string>
#include <utility>
#include <vector>

#include <pybind11/pybind11.h>

#ifndef BINDER_PYBIND11_TYPE_CASTER
	#define BINDER_PYBIND11_TYPE_CASTER
	PYBIND11_DECLARE_HOLDER_TYPE(T, std::shared_ptr<T>);
	PYBIND11_DECLARE_HOLDER_TYPE(T, T*);
	PYBIND11_MAKE_OPAQUE(std::shared_ptr<void>);
#endif

// pystorm::bddriver::comm::CommBDModel file: line:20
struct PyCallBack_CommBDModel : public pystorm::bddriver::comm::CommBDModel {
	using pystorm::bddriver::comm::CommBDModel::CommBDModel;

	void StartStreaming() override { 
		pybind11::gil_scoped_acquire gil;
		pybind11::function overload = pybind11::get_overload(static_cast<const pystorm::bddriver::comm::CommBDModel *>(this), "StartStreaming");
		if (overload) {
			auto o = overload.operator()<pybind11::return_value_policy::reference>();
			if (pybind11::detail::cast_is_temporary_value_reference<void>::value) {
				static pybind11::detail::overload_caster_t<void> caster;
				return pybind11::detail::cast_ref<void>(std::move(o), caster);
			}
			else return pybind11::detail::cast_safe<void>(std::move(o));
		}
		return CommBDModel::StartStreaming();
	}
	void StopStreaming() override { 
		pybind11::gil_scoped_acquire gil;
		pybind11::function overload = pybind11::get_overload(static_cast<const pystorm::bddriver::comm::CommBDModel *>(this), "StopStreaming");
		if (overload) {
			auto o = overload.operator()<pybind11::return_value_policy::reference>();
			if (pybind11::detail::cast_is_temporary_value_reference<void>::value) {
				static pybind11::detail::overload_caster_t<void> caster;
				return pybind11::detail::cast_ref<void>(std::move(o), caster);
			}
			else return pybind11::detail::cast_safe<void>(std::move(o));
		}
		return CommBDModel::StopStreaming();
	}
	pystorm::bddriver::comm::CommStreamState GetStreamState() override { 
		pybind11::gil_scoped_acquire gil;
		pybind11::function overload = pybind11::get_overload(static_cast<const pystorm::bddriver::comm::CommBDModel *>(this), "GetStreamState");
		if (overload) {
			auto o = overload.operator()<pybind11::return_value_policy::reference>();
			if (pybind11::detail::cast_is_temporary_value_reference<pystorm::bddriver::comm::CommStreamState>::value) {
				static pybind11::detail::overload_caster_t<pystorm::bddriver::comm::CommStreamState> caster;
				return pybind11::detail::cast_ref<pystorm::bddriver::comm::CommStreamState>(std::move(o), caster);
			}
			else return pybind11::detail::cast_safe<pystorm::bddriver::comm::CommStreamState>(std::move(o));
		}
		return CommBDModel::GetStreamState();
	}
	class pystorm::bddriver::MutexBuffer<unsigned char> * getReadBuffer() override { 
		pybind11::gil_scoped_acquire gil;
		pybind11::function overload = pybind11::get_overload(static_cast<const pystorm::bddriver::comm::CommBDModel *>(this), "getReadBuffer");
		if (overload) {
			auto o = overload.operator()<pybind11::return_value_policy::reference>();
			if (pybind11::detail::cast_is_temporary_value_reference<class pystorm::bddriver::MutexBuffer<unsigned char> *>::value) {
				static pybind11::detail::overload_caster_t<class pystorm::bddriver::MutexBuffer<unsigned char> *> caster;
				return pybind11::detail::cast_ref<class pystorm::bddriver::MutexBuffer<unsigned char> *>(std::move(o), caster);
			}
			else return pybind11::detail::cast_safe<class pystorm::bddriver::MutexBuffer<unsigned char> *>(std::move(o));
		}
		return CommBDModel::getReadBuffer();
	}
	class pystorm::bddriver::MutexBuffer<unsigned char> * getWriteBuffer() override { 
		pybind11::gil_scoped_acquire gil;
		pybind11::function overload = pybind11::get_overload(static_cast<const pystorm::bddriver::comm::CommBDModel *>(this), "getWriteBuffer");
		if (overload) {
			auto o = overload.operator()<pybind11::return_value_policy::reference>();
			if (pybind11::detail::cast_is_temporary_value_reference<class pystorm::bddriver::MutexBuffer<unsigned char> *>::value) {
				static pybind11::detail::overload_caster_t<class pystorm::bddriver::MutexBuffer<unsigned char> *> caster;
				return pybind11::detail::cast_ref<class pystorm::bddriver::MutexBuffer<unsigned char> *>(std::move(o), caster);
			}
			else return pybind11::detail::cast_safe<class pystorm::bddriver::MutexBuffer<unsigned char> *>(std::move(o));
		}
		return CommBDModel::getWriteBuffer();
	}
};

void bind_unknown_unknown_7(std::function< pybind11::module &(std::string const &namespace_) > &M)
{
	{ // pystorm::bddriver::comm::CommBDModel file: line:20
		pybind11::class_<pystorm::bddriver::comm::CommBDModel, std::shared_ptr<pystorm::bddriver::comm::CommBDModel>, PyCallBack_CommBDModel, pystorm::bddriver::comm::Comm> cl(M("pystorm::bddriver::comm"), "CommBDModel", "CommBDModel is a Comm object which uses a BDModel to parse and generate\n traffic streams as if they were from the BD hardware.\n Takes BDModel ptr as an argument, user controls BDModel directly to \n create upstream traffic.");
		pybind11::handle cl_type = cl;

		cl.def(pybind11::init<class pystorm::bddriver::bdmodel::BDModel *, const class pystorm::bddriver::driverpars::DriverPars *, class pystorm::bddriver::MutexBuffer<unsigned char> *, class pystorm::bddriver::MutexBuffer<unsigned char> *>(), pybind11::arg("model"), pybind11::arg("driver_pars"), pybind11::arg("read_buffer"), pybind11::arg("write_buffer"));

		cl.def("StartStreaming", (void (pystorm::bddriver::comm::CommBDModel::*)()) &pystorm::bddriver::comm::CommBDModel::StartStreaming, "C++: pystorm::bddriver::comm::CommBDModel::StartStreaming() --> void");
		cl.def("StopStreaming", (void (pystorm::bddriver::comm::CommBDModel::*)()) &pystorm::bddriver::comm::CommBDModel::StopStreaming, "C++: pystorm::bddriver::comm::CommBDModel::StopStreaming() --> void");
		cl.def("GetStreamState", (pystorm::bddriver::comm::CommStreamState (pystorm::bddriver::comm::CommBDModel::*)()) &pystorm::bddriver::comm::CommBDModel::GetStreamState, "C++: pystorm::bddriver::comm::CommBDModel::GetStreamState() --> pystorm::bddriver::comm::CommStreamState");
		cl.def("getReadBuffer", (class pystorm::bddriver::MutexBuffer<unsigned char> * (pystorm::bddriver::comm::CommBDModel::*)()) &pystorm::bddriver::comm::CommBDModel::getReadBuffer, "C++: pystorm::bddriver::comm::CommBDModel::getReadBuffer() --> class pystorm::bddriver::MutexBuffer<unsigned char> *", pybind11::return_value_policy::automatic);
		cl.def("getWriteBuffer", (class pystorm::bddriver::MutexBuffer<unsigned char> * (pystorm::bddriver::comm::CommBDModel::*)()) &pystorm::bddriver::comm::CommBDModel::getWriteBuffer, "C++: pystorm::bddriver::comm::CommBDModel::getWriteBuffer() --> class pystorm::bddriver::MutexBuffer<unsigned char> *", pybind11::return_value_policy::automatic);
	}
}


// File: model/BDModelDriver.cpp
#include <iterator>
#include <memory>
#include <model/BDModelDriver.h>
#include <sstream> // __str__
#include <vector>

#include <pybind11/pybind11.h>

#ifndef BINDER_PYBIND11_TYPE_CASTER
	#define BINDER_PYBIND11_TYPE_CASTER
	PYBIND11_DECLARE_HOLDER_TYPE(T, std::shared_ptr<T>);
	PYBIND11_DECLARE_HOLDER_TYPE(T, T*);
	PYBIND11_MAKE_OPAQUE(std::shared_ptr<void>);
#endif

void bind_model_BDModelDriver(std::function< pybind11::module &(std::string const &namespace_) > &M)
{
	{ // pystorm::bddriver::BDModelDriver file:model/BDModelDriver.h line:15
		pybind11::class_<pystorm::bddriver::BDModelDriver, std::shared_ptr<pystorm::bddriver::BDModelDriver>, pystorm::bddriver::Driver> cl(M("pystorm::bddriver"), "BDModelDriver", "Specialization of Driver that uses BDModelComm.\n I could have made Driver depend on BDModel, and have an optional constructor\n argument. I opted to subclass instead to contain the dependency to this \n particular (testing-only) use case.");
		pybind11::handle cl_type = cl;

		cl.def(pybind11::init<>());

		cl.def(pybind11::init<const class pystorm::bddriver::BDModelDriver &>(), pybind11::arg(""));

		cl.def("GetBDModel", (class pystorm::bddriver::bdmodel::BDModel * (pystorm::bddriver::BDModelDriver::*)()) &pystorm::bddriver::BDModelDriver::GetBDModel, "C++: pystorm::bddriver::BDModelDriver::GetBDModel() --> class pystorm::bddriver::bdmodel::BDModel *", pybind11::return_value_policy::automatic);
	}
}


#include <map>
#include <memory>
#include <stdexcept>
#include <functional>

#include <pybind11/pybind11.h>

typedef std::function< pybind11::module & (std::string const &) > ModuleGetter;

void bind_std_array(std::function< pybind11::module &(std::string const &namespace_) > &M);
void bind_std_stl_vector(std::function< pybind11::module &(std::string const &namespace_) > &M);
void bind_unknown_unknown(std::function< pybind11::module &(std::string const &namespace_) > &M);
void bind_unknown_unknown_1(std::function< pybind11::module &(std::string const &namespace_) > &M);
void bind_std_stl_map(std::function< pybind11::module &(std::string const &namespace_) > &M);
void bind_unknown_unknown_2(std::function< pybind11::module &(std::string const &namespace_) > &M);
void bind_unknown_unknown_3(std::function< pybind11::module &(std::string const &namespace_) > &M);
void bind_unknown_unknown_4(std::function< pybind11::module &(std::string const &namespace_) > &M);
void bind_unknown_unknown_5(std::function< pybind11::module &(std::string const &namespace_) > &M);
void bind_unknown_unknown_6(std::function< pybind11::module &(std::string const &namespace_) > &M);
void bind_unknown_unknown_7(std::function< pybind11::module &(std::string const &namespace_) > &M);
void bind_model_BDModelDriver(std::function< pybind11::module &(std::string const &namespace_) > &M);


PYBIND11_PLUGIN(PyDriver) {
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
		{"pystorm::bddriver", "comm"},
		{"pystorm::bddriver", "driverpars"},
		{"", "std"},
	};
	for(auto &p : sub_modules ) modules[p.first.size() ? p.first+"::"+p.second : p.second] = std::make_shared<pybind11::module>( modules[p.first]->def_submodule(p.second.c_str(), ("Bindings for " + p.first + "::" + p.second + " namespace").c_str() ) );

	//pybind11::class_<std::shared_ptr<void>>(M(""), "_encapsulated_data_");

	//bind_std_array(M);
	//bind_std_stl_vector(M);
	bind_unknown_unknown(M);
	bind_unknown_unknown_1(M);
	//bind_std_stl_map(M);
	bind_unknown_unknown_2(M);
	bind_unknown_unknown_3(M);
	bind_unknown_unknown_4(M);
	bind_unknown_unknown_5(M);
	bind_unknown_unknown_6(M);
	bind_unknown_unknown_7(M);
	bind_model_BDModelDriver(M);

	return modules[""]->ptr();
}

// Source list file: /mnt/f/Studio/pystorm/src/bindings/python/3.5/BDDriver.sources
// BDDriver.cpp
// std/array.cpp
// std/stl_vector.cpp
// unknown/unknown.cpp
// unknown/unknown_1.cpp
// std/stl_map.cpp
// unknown/unknown_2.cpp
// unknown/unknown_3.cpp
// unknown/unknown_4.cpp
// unknown/unknown_5.cpp
// unknown/unknown_6.cpp
// unknown/unknown_7.cpp
// model/BDModelDriver.cpp

// Modules list file: /mnt/f/Studio/pystorm/src/bindings/python/3.5/BDDriver.modules
// pystorm pystorm.bddriver pystorm.bddriver.bdmodel pystorm.bddriver.bdpars pystorm.bddriver.comm pystorm.bddriver.driverpars std 
