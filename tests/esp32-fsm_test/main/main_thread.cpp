/*
 * ESP32 device test.
 * 
 * @author Michel Megens
 * @email  dev@bietje.net
 */

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <esp_heap_caps.h>
#include <lwiot.h>

#include <lwiot/system.h>
#include <lwiot/function.h>

#include <lwiot/stl/bind.h>
#include <lwiot/util/application.h>
#include <lwiot/kernel/fsm.h>
#include <lwiot/kernel/asyncfsm.h>

static uint8_t expected[] = {
		2,3,3,4,1,2,3,3,4,5
};

static uint8_t actual[] = {
		0,0,0,0,0,0,0,0,0
};

struct test {
	bool tst2(int x)
	{
		print_dbg("Test 2 called!\n");
		return x == 5;
	}

	void tst3(int x)
	{
		print_dbg("Test 3 called!\n");
	}
};

typedef enum {
	EVENT_1 = 1,
	EVENT_2,
	EVENT_3,
	EVENT_4
} FSM_EVENT_TYPE;

using FsmType = lwiot::FSM;
using HandlerType = FsmType::HandlerType;
using FsmStateType = FsmType::StateType;

class MySignal : public FsmType::SignalType {
public:
	MySignal() : FsmType::SignalType(), done(false), y(21), value(9)
	{ }

	MySignal(int x, int y) : FsmType::SignalType(), done(false), y(y), value(x)
	{ }

	void log()
	{
		print_dbg("Signal value: [%i:%i]\n", y, value);
	}

	MySignal(MySignal&& other) noexcept : done(other.done), y(other.y), value(other.value)
	{
		other.y = other.value = 0;
	}

	MySignal& operator=(MySignal&& rhs) noexcept
	{
		this->y = rhs.y;
		this->value = rhs.value;

		rhs.y = rhs.value = 0;
		return *this;
	}

	bool done;

private:
	int y;
	int value;
};

/* Async definitions */

using AsyncFsmType = lwiot::AsyncFsm;
using AsyncHandlerType = AsyncFsmType::HandlerType;
using AsyncFsmStateType = AsyncFsmType::StateType;

class MyAsyncSignal : public AsyncFsmType::SignalType {
public:
	MyAsyncSignal() : AsyncFsmType::SignalType(), done(false), y(21), value(9)
	{ }

	MyAsyncSignal(int x, int y) : AsyncFsmType::SignalType(), done(false), y(y), value(x)
	{ }

	void log()
	{
		print_dbg("Signal value: [%i:%i]\n", value, y);
	}

	MyAsyncSignal(MyAsyncSignal&& other) noexcept : done(other.done), y(other.y), value(other.value)
	{
		other.y = other.value = 0;
	}

	MyAsyncSignal& operator=(MyAsyncSignal&& rhs) noexcept
	{
		this->y = rhs.y;
		this->value = rhs.value;

		rhs.y = rhs.value = 0;
		return *this;
	}

	bool done;

private:
	int y;
	int value;
};


class EspApplication : public lwiot::Functor {
private:
public:
	explicit EspApplication()
	{
	}

	virtual ~EspApplication()
	{
	}

	void test_fsm()
	{
		FsmType fsm;
		FsmStateType state1;
		FsmStateType state2;
		FsmStateType state3;
		FsmStateType state4;
		FsmStateType state5;
		FsmStateType super;
		int s1_executed = 0;
		bool s4_executed = false;
		int idx = 0;

		using namespace lwiot;
		print_dbg("FSM size: %u\n", sizeof(FsmType));
		print_dbg("State size: %u\n", sizeof(FsmStateType));

		fsm.addAlphabetSymbol(EVENT_1);
		fsm.addAlphabetSymbol(EVENT_2);
		fsm.addAlphabetSymbol(EVENT_3);
		fsm.addAlphabetSymbol(EVENT_4);

		state5.setAction([&](const FsmType::SignalPointer& ptr) {
			auto signal = SignalAs<MySignal>(ptr);

			print_dbg("State 5: ");
			signal->log();
			actual[idx++] = 5;

			return true;
		});

		state1.setAction([&](const stl::SharedPointer<FsmType::SignalType>& signal) {
			auto value = SignalAs<MySignal>(signal);

			assert(value);
			assert(s1_executed <= 2);
			s1_executed = true;

			MySignal s(1, 3);
			s1_executed++;

			print_dbg("State 1: ");
			fsm.transition(1, stl::move(s));
			value->log();
			actual[idx] = 1;
			idx += 1;

			return true;
		});

		state2.setAction([&](const stl::SharedPointer<FsmType::SignalType>& signal) {
			auto value = SignalAs<MySignal>(signal);
			assert(value);
			MySignal s(12, 31);

			fsm.raise(1, stl::move(s));

			print_dbg("State 2: ");
			value->log();
			actual[idx++] = 2;

			return true;
		});

		state3.setAction([&](const stl::SharedPointer<FsmType::SignalType>& signal) {
			auto value = SignalAs<MySignal>(signal);
			assert(value);
			MySignal s(1, 3);

			s.done = true;

			if(value->done)
				fsm.transition(3, stl::move(s));
			else
				fsm.transition(2, stl::move(s));

			print_dbg("State 3: ");
			value->log();
			actual[idx++] = 3;

			return true;
		});

		state4.setAction([&](const stl::SharedPointer<FsmType::SignalType>& signal) {
			auto value = SignalAs<MySignal>(signal);
			auto rv = !s4_executed;

			print_dbg("State 4: ");
			value->log();

			s4_executed = true;

			actual[idx++] = 4;
			return rv;
		});

		state1.addTransition(EVENT_1, state2.id(), [](const stl::SharedPointer<FsmType::SignalType>& signal) { return true; });
		state1.addTransition(EVENT_2, state1.id());
		state1.addTransition(EVENT_3, state1.id());
		state1.addTransition(EVENT_4, state1.id());

		state2.addTransition(EVENT_1, state3.id(), [](const stl::SharedPointer<FsmType::SignalType>& signal) { return true; });
		state2.addTransition(EVENT_2, state1.id());
		state2.addTransition(EVENT_3, state1.id());
		state2.addTransition(EVENT_4, state1.id());

		state3.addTransition(EVENT_2, state3.id(), [](const stl::SharedPointer<FsmType::SignalType>& signal) { return true; });
		state3.addTransition(EVENT_3, state4.id(), [](const stl::SharedPointer<FsmType::SignalType>& signal) { return true; });

		super.addTransition(EVENT_1, state1.id());
		super.addTransition(EVENT_4, state1.id());

		state4.setParent(super);
		state3.setParent(super);

		state4.addTransition(EVENT_2, state4.id());
		state4.addTransition(EVENT_3, state1.id(), [](const stl::SharedPointer<FsmType::SignalType>& signal) { return true; });

		state5.addTransition(EVENT_4, state5.id());
		state5.addTransition(EVENT_3, state5.id());
		state5.addTransition(EVENT_2, state5.id());
		state5.addTransition(EVENT_1, state5.id());

		auto s_super = fsm.addState(super);
		auto s1 = fsm.addState(state1);
		auto s2 = fsm.addState(state2);
		auto s3 = fsm.addState(state3);
		auto s4 = fsm.addState(state4);
		auto s5 = fsm.addState(state5);

		fsm.addStopState(s4.first);
		fsm.setStartState(s1.first);
		fsm.setErrorState(s5.first);

		assert(s1.second);
		assert(s2.second);
		assert(s3.second);
		assert(s4.second);
		assert(s5.second);
		assert(s_super.second);

		assert(fsm.valid());
		assert(fsm.deterministic());

		fsm.start();

		MySignal signal1;
		MySignal signal2;

		fsm.raise(EVENT_1, stl::move(signal1));
		auto now = lwiot_tick_ms();
		auto done = false;

		fsm.block(false);

#define RAISE_TMO 100
#define DONE_TMO  1000

		for(;;) {
			if(lwiot_tick_ms() > now+RAISE_TMO && !done) {
				fsm.raise(EVENT_4, stl::move(signal2));
				done = true;
			}

			if(lwiot_tick_ms() > now+DONE_TMO && done)
				break;

			fsm.run();
		}

		fsm.stop();

		assert(s1_executed <= 2);

		for(auto i = 0UL; i < sizeof(actual); i++) {
			assert(actual[i] == expected[i]);
		}
	}

	void test_async_fsm()
	{
		auto freesize_1 = heap_caps_get_free_size(MALLOC_CAP_8BIT | MALLOC_CAP_DEFAULT) / 1024;

		AsyncFsmType fsm;
		AsyncFsmStateType state1;
		AsyncFsmStateType state2;
		AsyncFsmStateType state3;
		AsyncFsmStateType state4;
		AsyncFsmStateType state5;
		AsyncFsmStateType super;
		int s1_executed = 0;
		bool s4_executed = false;
		int idx = 0;

		using namespace lwiot;
		print_dbg("FSM size: %u\n", sizeof(AsyncFsmType));
		print_dbg("State size: %u\n", sizeof(AsyncFsmStateType));

		fsm.addAlphabetSymbol(EVENT_1);
		fsm.addAlphabetSymbol(EVENT_2);
		fsm.addAlphabetSymbol(EVENT_3);
		fsm.addAlphabetSymbol(EVENT_4);

		state5.setAction([&](const AsyncFsm::SignalPointer& ptr) {
			auto signal = SignalAs<MySignal>(ptr);

			print_dbg("State 5: ");
			signal->log();
			actual[idx++] = 5;

			return true;
		});

		state1.setAction([&](const stl::SharedPointer<FsmType::SignalType>& signal) {
			auto value = SignalAs<MySignal>(signal);

			assert(value);
			assert(s1_executed <= 2);
			s1_executed = true;

			MySignal s(1, 2);
			s1_executed++;

			print_dbg("State 1: ");
			fsm.transition(1, stl::move(s));
			value->log();
			actual[idx] = 1;
			idx += 1;

			return true;
		});

		state2.setAction([&](const stl::SharedPointer<FsmType::SignalType>& signal) {
			auto value = SignalAs<MySignal>(signal);
			assert(value);
			MySignal s(2, 3);

			fsm.transition(1, stl::move(s));

			print_dbg("State 2: ");
			value->log();
			actual[idx] = 2;
			idx++;

			return true;
		});

		state3.setAction([&](const stl::SharedPointer<FsmType::SignalType>& signal) {
			auto value = SignalAs<MySignal>(signal);
			assert(value);


			if(value->done) {
				MySignal s(3, 4);
				s.done = true;

				fsm.transition(3, stl::move(s));
			} else {
				MySignal s(3, 3);
				s.done = true;

				fsm.transition(2, stl::move(s));
			}

			print_dbg("State 3: ");
			value->log();
			actual[idx++] = 3;

			return true;
		});

		state4.setAction([&](const stl::SharedPointer<FsmType::SignalType>& signal) {
			auto value = SignalAs<MySignal>(signal);
			auto rv = !s4_executed;

			print_dbg("State 4: ");
			value->log();

			s4_executed = true;

			actual[idx++] = 4;
			return rv;
		});

		state5.addTransition(EVENT_4, state5.id());
		state5.addTransition(EVENT_3, state5.id());
		state5.addTransition(EVENT_2, state5.id());
		state5.addTransition(EVENT_1, state5.id());

		super.addTransition(EVENT_4, state1.id());
		super.addTransition(EVENT_1, state1.id());

		state1.addTransition(EVENT_1, state2.id(), [](const stl::SharedPointer<FsmType::SignalType>& signal) { return true; });
		state1.addTransition(EVENT_2, state1.id());
		state1.addTransition(EVENT_3, state1.id());
		state1.addTransition(EVENT_4, state1.id());

		state2.addTransition(EVENT_1, state3.id(), [](const stl::SharedPointer<FsmType::SignalType>& signal) {
			return true;
		});
		state2.addTransition(EVENT_2, state1.id());
		state2.addTransition(EVENT_3, state1.id());
		state2.addTransition(EVENT_4, state1.id());

		state3.addTransition(EVENT_2, state3.id(), [](const stl::SharedPointer<FsmType::SignalType>& signal) {
			return true;
		});

		state3.addTransition(EVENT_3, state4.id(), [](const stl::SharedPointer<FsmType::SignalType>& signal) {
			return true;
		});

		state4.setParent(super);
		state3.setParent(super);

		state4.addTransition(EVENT_2, state4.id());
		state4.addTransition(EVENT_3, state1.id(), [](const stl::SharedPointer<FsmType::SignalType>& signal) {
			return true;
		});

		auto s_super = fsm.addState(super);
		auto s1 = fsm.addState(state1);
		auto s2 = fsm.addState(state2);
		auto s3 = fsm.addState(state3);
		auto s4 = fsm.addState(state4);
		auto s5 = fsm.addState(state5);

		fsm.addStopState(s4.first);
		fsm.setStartState(s1.first);
		fsm.setErrorState(s5.first);

		assert(s1.second);
		assert(s2.second);
		assert(s3.second);
		assert(s4.second);
		assert(s5.second);
		assert(s_super.second);

		assert(fsm.valid());
		assert(fsm.deterministic());

		fsm.start();

		MySignal signal1(1,1);
		MySignal signal2(16, 17);

		fsm.raise(EVENT_1, stl::move(signal1));
		lwiot::System::delay(1000);
		auto freesize_2 = heap_caps_get_free_size(MALLOC_CAP_8BIT | MALLOC_CAP_DEFAULT) / 1024;
		fsm.raise(EVENT_4, stl::move(signal2));
		lwiot::System::delay(1000);
		fsm.stop();

		assert(s1_executed <= 2);

		for(auto i = 0UL; i < sizeof(actual); i++) {
			assert(actual[i] == expected[i]);
		}

		auto freesize_3 = heap_caps_get_free_size(MALLOC_CAP_8BIT | MALLOC_CAP_DEFAULT) / 1024;
		print_dbg("Memory 1: %u\n", freesize_1);
		print_dbg("Memory 2: %u\n", freesize_2);
		print_dbg("Memory 3: %u\n", freesize_3);

		print_dbg("Used memory: %u\n", freesize_1 - freesize_2);
	}

	void bind_test()
	{
		test t;
		auto f1 = lwiot::stl::bind(&test::tst2, t, lwiot::stl::placeholders::_1);
		auto f2 = lwiot::stl::bind(&test::tst3, t, lwiot::stl::placeholders::_1);
		lwiot::Function<bool(int)> g1;
		lwiot::Function<void(int)> g2 = f2;
		int x = 5;
		int y = 2;

		f1(5);
		f2(y);

		g1 = f1;

		g1(x);
		g2(2);
	}

protected:
	void run() override
	{
		this->bind_test();

		print_dbg("Testing async FSM:\n");
		memset(&actual[0], 0, sizeof(actual));
		this->test_async_fsm();

		print_dbg("\n");
		print_dbg("\n");
		print_dbg("\n");

		print_dbg("Testing synchronous FSM:\n");
		memset(&actual[0], 0, sizeof(actual));
		this->test_fsm();

		print_dbg("Done testing...");

		while(true) {
			lwiot::System::delay(1000);
		}
	}
};

extern "C" void main_start(void)
{
	auto runner = new EspApplication();
	lwiot::Application app(*runner);

	app.start();
}

