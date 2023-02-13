// SPDX-License-Identifier: BSD-2-Clause
// Copyright CM4all GmbH
// author: Max Kellermann <mk@cm4all.com>

#include "co/MultiAwaitable.hxx"

#include <gtest/gtest.h>

class PauseTask {
	std::coroutine_handle<> continuation;

	bool resumed = false;

	struct Awaitable final {
		PauseTask &task;

		Awaitable(PauseTask &_task) noexcept:task(_task) {}

		~Awaitable() noexcept {
			task.continuation = {};
		}

		Awaitable(const Awaitable &) = delete;
		Awaitable &operator=(const Awaitable &) = delete;

		bool await_ready() const noexcept {
			return task.resumed;
		}

		std::coroutine_handle<> await_suspend(std::coroutine_handle<> _continuation) noexcept {
			task.continuation = _continuation;
			return std::noop_coroutine();
		}

		void await_resume() noexcept {
		}
	};

public:
	Awaitable operator co_await() noexcept {
		return {*this};
	}

	bool IsAwaited() const noexcept {
		return (bool)continuation;
	}

	void Resume() noexcept {
		assert(!resumed);

		resumed = true;

		if (continuation)
			continuation.resume();
	}
};

static Co::Task<void>
NoTask()
{
	co_return;
}

static Co::Task<void>
MyTask(auto &&task)
{
	co_await task;
}

template<typename T>
static Co::EagerTask<void>
Waiter(T &&task, bool &complete_r)
{
	assert(!complete_r);
	co_await task;

	assert(!complete_r);
	complete_r = true;
}

TEST(MultiAwaitable, CompleteEarly)
{
	Co::MultiAwaitable m{NoTask()};

	std::array complete{false, false, false};
	std::array waiters{
		Waiter(m, complete[0]),
		Waiter(m, complete[1]),
	};

	ASSERT_TRUE(complete[0]);
	ASSERT_TRUE(complete[1]);
}

TEST(MultiAwaitable, CompleteLate)
{
	PauseTask pause;
	Co::MultiAwaitable m{MyTask(pause)};

	ASSERT_TRUE(pause.IsAwaited());

	std::array complete{false, false, false};
	std::array waiters{
		Waiter(m, complete[0]),
		Waiter(m, complete[1]),
		Waiter(m, complete[2]),
	};

	ASSERT_FALSE(complete[0]);
	ASSERT_FALSE(complete[1]);
	ASSERT_FALSE(complete[2]);

	ASSERT_TRUE(pause.IsAwaited());
	pause.Resume();

	ASSERT_TRUE(complete[0]);
	ASSERT_TRUE(complete[1]);
	ASSERT_TRUE(complete[2]);
}

TEST(MultiAwaitable, CancelOne)
{
	PauseTask pause;
	Co::MultiAwaitable m{MyTask(pause)};

	ASSERT_TRUE(pause.IsAwaited());

	std::array complete{false, false, false};

	auto w0 = Waiter(m, complete[0]);
	std::optional w1 = Waiter(m, complete[1]);
	auto w2 = Waiter(m, complete[2]);

	ASSERT_FALSE(complete[0]);
	ASSERT_FALSE(complete[1]);
	ASSERT_FALSE(complete[2]);

	w1.reset();

	ASSERT_TRUE(pause.IsAwaited());
	pause.Resume();

	ASSERT_TRUE(complete[0]);
	ASSERT_FALSE(complete[1]);
	ASSERT_TRUE(complete[2]);
}

TEST(MultiAwaitable, CancelAll)
{
	PauseTask pause;
	Co::MultiAwaitable m{MyTask(pause)};

	ASSERT_TRUE(pause.IsAwaited());

	std::array complete{false, false, false};

	std::optional w0 = Waiter(m, complete[0]);
	std::optional w1 = Waiter(m, complete[1]);
	std::optional w2 = Waiter(m, complete[2]);

	ASSERT_FALSE(complete[0]);
	ASSERT_FALSE(complete[1]);
	ASSERT_FALSE(complete[2]);

	ASSERT_TRUE(pause.IsAwaited());
	w0.reset();
	ASSERT_TRUE(pause.IsAwaited());
	w1.reset();
	ASSERT_TRUE(pause.IsAwaited());
	w2.reset();
	ASSERT_FALSE(pause.IsAwaited());

	ASSERT_FALSE(complete[0]);
	ASSERT_FALSE(complete[1]);
	ASSERT_FALSE(complete[2]);
}
