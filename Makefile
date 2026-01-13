CC      := clang
CFLAGS  := -Wall -Wextra -I. -I./common -g -fno-omit-frame-pointer
LDFLAGS := -lpthread

SRC     := bounded_queue.c
TEST    := tests/test_stress.c

# ---------- Default build ----------

all: test_stress

test_stress: $(SRC) $(TEST)
	$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS)

# ---------- Sanitizer builds ----------

test_stress_asan: CFLAGS += -fsanitize=address,undefined
test_stress_asan: $(SRC) $(TEST)
	$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS)

test_stress_tsan: CFLAGS += -fsanitize=thread
test_stress_tsan: $(SRC) $(TEST)
	$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS)

asan: test_stress_asan
tsan: test_stress_tsan

# ---------- Housekeeping ----------

clean:
	rm -f test_stress test_stress_asan test_stress_tsan

